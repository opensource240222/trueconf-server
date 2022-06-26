#pragma once

#include "BufferedConnection.h"
#include "std-generic/cpplib/deleters.h"
#include "../std/cpplib/iostream_utils.h"
#include "../std/debuglog/VS_Debug.h"

#include "std-generic/compat/memory.h"
#include <cassert>
#include <cstring>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

#define NET_VERBOSE_LOGS 0

namespace net {

template <class Socket>
struct BufferedConnection<Socket>::LogPrefix
{
	explicit LogPrefix(const BufferedConnection* obj_) : obj(obj_) {}
	const BufferedConnection* obj;

	template <class CharT, class Traits>
	friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, LogPrefix x)
	{
		s << "net::BufferedConnection(";
		const auto id = x.GetLogID();
		if (id.empty())
			s << pointer_value(x.obj);
		else
			s << id;
		s << "): ";
		return s;
	}
private:
	string_view GetLogID() const { return obj->LogID(); }
};

template <class Socket>
BufferedConnection<Socket>::BufferedConnection(boost::asio::io_service& ios, size_t write_buffer_size, size_t read_buffer_size)
	: m_strand(ios)
	, m_socket(ios)
	, m_state(BufferedConnectionState::empty)
	, m_max_write_data_size(write_buffer_size)
	, m_write_data_size(0)
	, m_read_buffer(vs::make_unique_default_init<unsigned char[]>(read_buffer_size))
	, m_read_buffer_size(read_buffer_size)
	, m_read_data_size(0)
	, m_write_in_progress(false)
	, m_read_in_progress(false)
{
}

template <class Socket>
BufferedConnection<Socket>::~BufferedConnection()
{
	assert(m_state == BufferedConnectionState::empty);
#if NET_VERBOSE_LOGS
	if (m_write_data_size > 0)
		dstream4 << LogPrefix(this) << m_write_data_size << " bytes were not sent";
	if (m_read_data_size > 0)
		dstream4 << LogPrefix(this) << m_read_data_size << " bytes were not processed";
#endif
}

template <class Socket>
void BufferedConnection<Socket>::SetSocket(Socket&& socket, const net::QoSFlowSharedPtr &flow, acs::Handler::stream_buffer &&init_read_buff, bool clear_read_buffer, bool clear_write_buffer)
{
	assert(socket.is_open());

	m_strand.dispatch([this, self = this->shared_from_this(),
						p_socket = std::make_shared<Socket>(std::move(socket)),
						p_read_buff = std::make_shared<acs::Handler::stream_buffer>(std::move(init_read_buff)),
						clear_read_buffer, clear_write_buffer, flow]() {
		if (m_state.load(std::memory_order_relaxed) != BufferedConnectionState::empty)
		{
			boost::system::error_code ec = boost::asio::error::already_open;
			while (ec)
			{
				if (!OnError(ec))
					return;
				if (m_flow)
				{
					m_flow->RemoveSocket(m_socket.native_handle());
				}
				m_socket.close(ec);
			}
		}
		assert(!m_socket.is_open());

		m_state.store(BufferedConnectionState::active, std::memory_order_relaxed);
		m_socket = std::move(*p_socket);
		m_flow = nullptr;
		if (flow && flow->AddSocket(m_socket.native_handle()))
		{
			m_flow = flow;
		}
		if (clear_read_buffer)
			m_read_data_size = 0;
		if (clear_write_buffer)
			m_write_data_size = 0;

		if (p_read_buff && !p_read_buff->empty()) {
			assert(m_read_buffer_size >= p_read_buff->size());
			m_read_data_size = p_read_buff->size();
			memcpy(m_read_buffer.get(), p_read_buff->data(), p_read_buff->size());
			ProcessReadBuffer();
		}

		// There are may be unfinished read/write operations from previous socket.
		if (!m_write_in_progress)
			StartWrite();
		if (!m_read_in_progress)
			StartRead();
	});
}

template <class Socket>
void BufferedConnection<Socket>::Close()
{
	m_strand.dispatch([this, self = this->shared_from_this()]() {
		const auto state = m_state.load(std::memory_order_relaxed);
		if (state == BufferedConnectionState::empty || state == BufferedConnectionState::close)
			return; // Already stopped or stopping, nothing to do.

		CloseInternal();
	});
}

template <class Socket>
void BufferedConnection<Socket>::Shutdown()
{
	m_strand.dispatch([this, self = this->shared_from_this()]() {
		const auto state = m_state.load(std::memory_order_relaxed);
		if (
			state == BufferedConnectionState::empty ||
			state == BufferedConnectionState::shutdown ||
			state == BufferedConnectionState::close)
			return; // Already stopped or stopping, nothing to do.

		if (m_write_data_size > 0)
		{
			assert(m_write_in_progress);
			// Outstanding write operation will complete close.
			m_state.store(BufferedConnectionState::shutdown, std::memory_order_relaxed);
		}
		else
		{
			assert(!m_write_in_progress);
			// All writes are finished, we can (and have to) close now.
			CloseInternal();
		}
	});
}

template <class Socket>
void BufferedConnection<Socket>::CloseInternal()
{
	assert(m_strand.running_in_this_thread());
	assert(m_state.load(std::memory_order_relaxed) != BufferedConnectionState::empty);

	boost::system::error_code ec;
	if (m_flow)
	{
		m_flow->RemoveSocket(m_socket.native_handle());
		m_flow = nullptr;
	}
	m_socket.close(ec);
	if (ec)
		dstream4 << LogPrefix(this) << "close failed: " << ec.message();
	m_state.store(BufferedConnectionState::close, std::memory_order_relaxed);
	CompleteClose();
}

template <class Socket>
void BufferedConnection<Socket>::CompleteClose()
{
	assert(m_strand.running_in_this_thread());
	assert(m_state.load(std::memory_order_relaxed) == BufferedConnectionState::close);

#if NET_VERBOSE_LOGS
	dstream4 << LogPrefix(this) << "complete close: write " << (m_write_in_progress ? "not finished" : "finished") << ", read " << (m_read_in_progress ? "not finished" : "finished");
#endif
	if (m_write_in_progress || m_read_in_progress)
		return;

	m_state.store(BufferedConnectionState::empty, std::memory_order_relaxed);
	OnClose();
}

template <class Socket>
void BufferedConnection<Socket>::Send(vs::SharedBuffer&& buffer)
{
	if (buffer.empty())
		return;
	if (m_strand.running_in_this_thread())
		SendInternal(std::move(buffer));
	else
		m_strand.post([this, self = this->shared_from_this(), buffer = std::move(buffer)]() mutable
			{
				SendInternal(std::move(buffer));
			}
		);
}

template <class Socket>
void BufferedConnection<Socket>::Send(const void* data, size_t size)
{
	vs::SharedBuffer buffer(size);
	memcpy(buffer.data(), data, size);
	Send(std::move(buffer));
}

template <class Socket>
void BufferedConnection<Socket>::SendInternal(vs::SharedBuffer&& buffer)
{
	assert(m_strand.running_in_this_thread());

	if (m_write_data_size + buffer.size() > m_max_write_data_size)
	{
		if (!OnError(boost::asio::error::no_buffer_space))
		{
			// Close if OnError didn't do that already.
			if (m_state.load(std::memory_order_relaxed) != BufferedConnectionState::empty)
				CloseInternal();
		}
		return;
	}
	m_write_data_size += buffer.size();
	m_write_buffers.emplace(std::move(buffer));
	if (!m_write_in_progress && m_state.load(std::memory_order_relaxed) == BufferedConnectionState::active)
		StartWrite();
}

template <class Socket>
void BufferedConnection<Socket>::StartWrite()
{
	assert(m_strand.running_in_this_thread());
	assert(!m_write_in_progress);
	assert(m_socket.is_open());
	assert(m_write_data_size <= m_max_write_data_size);

	if (m_write_data_size == 0)
		return;

#if NET_VERBOSE_LOGS
	dstream4 << LogPrefix(this) << "write start: " << m_write_data_size << " bytes";
#endif
	m_write_in_progress = true;
	auto buffer = m_write_buffers.front();
	m_socket.async_send(boost::asio::buffer(buffer.template data<const void>(), buffer.size()), m_strand.wrap([this, self = this->shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred) {
		m_write_in_progress = false;

		if (ec)
		{
			assert(bytes_transferred == 0);
			if (ec != boost::asio::error::operation_aborted && !OnError(ec))
			{
				dstream4 << LogPrefix(this) << "close after write error: " << ec.message();
				// Close if OnError didn't do that already.
				if (m_state.load(std::memory_order_relaxed) != BufferedConnectionState::empty)
					CloseInternal();
				return;
			}
		}
		else
		{
			auto& queueFront = m_write_buffers.front();
			assert(bytes_transferred <= queueFront.size());
#if NET_VERBOSE_LOGS
			dstream4 << LogPrefix(this) << "write end: " << bytes_transferred << " bytes";
#endif
			if (bytes_transferred < queueFront.size())
				queueFront.shrink(bytes_transferred, queueFront.size() - bytes_transferred);
			else
				m_write_buffers.pop();
			m_write_data_size -= bytes_transferred;
			OnSend(bytes_transferred);
		}

		switch (m_state.load(std::memory_order_relaxed))
		{
			case BufferedConnectionState::active:
				StartWrite();
				break;
			case BufferedConnectionState::shutdown:
				if (m_write_data_size == 0)
					CloseInternal();
				else
					StartWrite();
				break;
			case BufferedConnectionState::close:
				CompleteClose();
				break;
			case BufferedConnectionState::empty: // We can be in this state if OnSend handler called Close()
				break;
		}
	}));
}

template <class Socket>
void BufferedConnection<Socket>::StartRead()
{
	assert(m_strand.running_in_this_thread());
	assert(!m_read_in_progress);
	assert(m_socket.is_open());
	assert(m_read_data_size <= m_read_buffer_size);

	if (m_read_data_size == m_read_buffer_size)
		return;

	const auto buffer = boost::asio::buffer(m_read_buffer.get() + m_read_data_size, m_read_buffer_size - m_read_data_size);
#if NET_VERBOSE_LOGS
	dstream4 << LogPrefix(this) << "read start: " << boost::asio::buffer_size(buffer) << " bytes";
#endif
	m_read_in_progress = true;
	m_socket.async_receive(buffer, m_strand.wrap([this, self = this->shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred) {
		m_read_in_progress = false;

		if (ec)
		{
			assert(bytes_transferred == 0);
			if (ec != boost::asio::error::operation_aborted && !OnError(ec))
			{
				dstream4 << LogPrefix(this) << "close after read error: " << ec.message();
				// Close if OnError didn't do that already.
				if (m_state.load(std::memory_order_relaxed) != BufferedConnectionState::empty)
					CloseInternal();
				return;
			}
		}
		else
		{
			assert(bytes_transferred <= m_read_buffer_size - m_read_data_size);
#if NET_VERBOSE_LOGS
			dstream4 << LogPrefix(this) << "read end: " << bytes_transferred << " bytes";
#endif
			m_read_data_size += bytes_transferred;
			if (m_state.load(std::memory_order_relaxed) == BufferedConnectionState::active)
				ProcessReadBuffer();
		}

		switch (m_state.load(std::memory_order_relaxed))
		{
			case BufferedConnectionState::active:
				StartRead();
				break;
			case BufferedConnectionState::shutdown:
				break; // Nothing to do, write handler will complete close.
			case BufferedConnectionState::close:
				CompleteClose();
				break;
			case BufferedConnectionState::empty: // We can be in this state if OnReceive handler called Close()
				break;
		}
	}));
}

template <class Socket>
void BufferedConnection<Socket>::ProcessReadBuffer()
{
	assert(m_strand.running_in_this_thread());

	size_t data_size = m_read_data_size;
	auto data = m_read_buffer.get();
	while (data_size > 0)
	{
#if 0
		// This disabled block simulates partial reads from the network.
		size_t consumed = 0;
		for (size_t part_size = 1; part_size <= data_size; ++part_size)
			if (0 != (consumed = OnReceive(data, part_size)))
				break;
#else
		const auto consumed = OnReceive(data, data_size);
#endif

#if NET_VERBOSE_LOGS
			dstream4 << LogPrefix(this) << "handler consumed " << consumed << " bytes";
#endif
		if (consumed == 0)
			break;
		if (consumed < data_size)
		{
			data_size -= consumed;
			data += consumed;
		}
		else
			data_size = 0;
	}
	if (data_size > 0)
	{
		assert(data <= m_read_buffer.get() + m_read_data_size);
		std::memmove(m_read_buffer.get(), data, data_size);
	}
	m_read_data_size = data_size;
}

template <class Socket>
size_t BufferedConnection<Socket>::OnReceive(const void* /*data*/, size_t size)
{
	return size;
}

template <class Socket>
void BufferedConnection<Socket>::OnSend(size_t /*bytes_transferred*/)
{
}

template <class Socket>
bool BufferedConnection<Socket>::OnError(const boost::system::error_code& /*ec*/)
{
	return false;
}

template <class Socket>
void BufferedConnection<Socket>::OnClose()
{
}

template <class Socket>
string_view BufferedConnection<Socket>::LogID() const
{
	return {};
}

}

#undef DEBUG_CURRENT_MODULE
