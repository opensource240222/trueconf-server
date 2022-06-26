#pragma once

#include "net/UDPRouter.h"
#include "net/Error.h"
#include "net/Lib.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/debuglog/VS_Debug.h"

#include "std-generic/compat/memory.h"
#include <algorithm>

#define DEBUG_CURRENT_MODULE VS_DM_OTHER
#define NET_VERBOSE_LOGS 0

namespace net { namespace ur {

inline bool CheckIfPacketTruncated(boost::system::error_code& ec, size_t& bytes_transferred, size_t buffer_size)
{
#if defined(_WIN32)
	if (ec == boost::system::error_code(ERROR_MORE_DATA, boost::system::system_category()))
	{
		ec = net::errc::packet_truncated;
		return true;
	}
#elif defined(__linux__)
	if (bytes_transferred > buffer_size)
	{
		bytes_transferred = buffer_size;
		ec = net::errc::packet_truncated;
		return true;
	}
#endif
	return false;
}

template <class Protocol>
std::mutex Router<Protocol>::s_mutex;

template <class Protocol>
vs::map<typename Router<Protocol>::endpoint_type, std::weak_ptr<Router<Protocol>>> Router<Protocol>::s_routers;

template <class Protocol>
RouterPtr<Protocol> Router<Protocol>::Get(const endpoint_type& local_ep)
{
	RouterPtr<Protocol> router;

	std::lock_guard<decltype(s_mutex)> lock(s_mutex);
	auto it = s_routers.find(local_ep);
	if (it != s_routers.end())
	{
		router = it->second.lock();
		if (!router)
			s_routers.erase(it); // Remove stale record for a closed Router
		else if (router->DontCloseWhenUnused())
			return router;
	}

	router = nullptr;
	return router;
}

template <class Protocol>
RouterPtr<Protocol> Router<Protocol>::Get(const endpoint_type& local_ep, boost::asio::io_service& ios, boost::system::error_code& ec)
{
	RouterPtr<Protocol> router;

	std::lock_guard<decltype(s_mutex)> lock(s_mutex);
	auto it = s_routers.find(local_ep);
	if (it != s_routers.end())
	{
		router = it->second.lock();
		if (!router)
			s_routers.erase(it); // Remove stale record for a closed Router
		else if (router->DontCloseWhenUnused())
		{
			ec = boost::asio::error::already_open;
			return router;
		}
	}

	router.reset(new Router(ios, local_ep, ec));
	if (!ec)
	{
		router->StartRead();
		s_routers.emplace(local_ep, router);
	}
	else
		router = nullptr;
	return router;
}

template <class Protocol>
Router<Protocol>::Router(boost::asio::io_service& ios, const endpoint_type& local_ep, boost::system::error_code& ec)
	: m_socket(ios)
	, m_local_ep(local_ep)
	, m_max_packet_size(0x10000)
	, m_accept_queue_size(100)
	, m_close_when_unused(false)
	, m_receive_buffer_size(0)
{
	VS_SCOPE_EXIT {
		if (ec)
			m_socket.close();
	};

	m_socket.open(m_local_ep.protocol(), ec);
	if (ec)
	{
		dstream3 << "UDPRouter(" << LocalEndpoint() << "): open failed: " << ec.message();
		return;
	}
	m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
	if (ec)
	{
		dstream3 << "UDPRouter(" << LocalEndpoint() << "): set SO_REUSEADDR failed: " << ec.message();
		return;
	}
	m_socket.bind(m_local_ep, ec);
	if (ec)
	{
		dstream3 << "UDPRouter(" << LocalEndpoint() << "): bind failed: " << ec.message();
		return;
	}
#if NET_VERBOSE_LOGS
	dstream4 << "UDPRouter(" << LocalEndpoint() << "): new";
#endif
}

template <class Protocol>
Router<Protocol>::~Router()
{
	assert(m_connections.empty());
	assert(m_accept_queue.empty());
	assert(!m_accept_op);

	m_socket.close();
}

template <class Protocol>
void Router<Protocol>::CloseWhenUnused()
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	m_close_when_unused = true;
	CloseIfUnused();
}


template <class Protocol>
Connection<Protocol> Router<Protocol>::Connect(const endpoint_type& destination, boost::system::error_code& ec)
{
	Connection<Protocol> connection;

	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	if (m_connections.count(destination) > 0)
	{
		ec = boost::asio::error::already_open;
		return connection;
	}

	connection = Connection<Protocol>(this->shared_from_this(), destination, ec);
	if (ec)
	{
		connection = {};
		return connection;
	}

	m_connections.emplace(destination, connection.m_impl.get());
	return connection;
}

template <class Protocol>
Connection<Protocol> Router<Protocol>::Connect(boost::asio::io_service& ios, net::port local_port,
	const endpoint_type& destination, boost::system::error_code& ec)
{
	Connection<Protocol> connection = Connection<Protocol>(ios, local_port, destination, ec);
	if (ec)
		return {};
	return connection;
}

template <class Protocol>
Connection<Protocol> Router<Protocol>::TryAccept()
{
	Connection<Protocol> connection;

	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	if (m_accept_queue.empty())
		return connection;

	connection = std::move(m_accept_queue.back());
	m_accept_queue.pop_back();
	return connection;
}

template <class Protocol>
void Router<Protocol>::RegisterConnection(const endpoint_type& destination,
	typename Connection<Protocol>::Impl* impl, boost::system::error_code& ec)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	if (!m_connections.emplace(destination, impl).second)
		ec = boost::asio::error::already_open;
}

template <class Protocol>
void Router<Protocol>::UnregisterConnection(const endpoint_type& remote_ep)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	m_connections.erase(remote_ep);
	CloseIfUnused();
}

template <class Protocol>
bool Router<Protocol>::DontCloseWhenUnused()
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	if (!m_close_when_unused)
		return true;

	if (m_socket.is_open())
	{
		m_close_when_unused = false;
		return true;
	}

	return false;
}

template <class Protocol>
void Router<Protocol>::CloseIfUnused()
{
	// Check if only unaccepted connections are remaining.
	if (m_close_when_unused && m_connections.size() == m_accept_queue.size())
	{
#if NET_VERBOSE_LOGS
		dstream4 << "UDPRouter(" << LocalEndpoint() << "): close";
#endif
		m_socket.close();
		m_connections.clear();
		for (const auto& x : m_accept_queue)
			x.m_impl->Close(false); // No need to unregister each individual connection.
		m_accept_queue.clear();
		while (m_accept_op)
		{
			auto op = std::move(m_accept_op);
			m_accept_op = std::move(op->next);
			Complete(std::move(op), get_io_service(), boost::asio::error::operation_aborted);
		}
	}
}

template <class Protocol>
void Router<Protocol>::StartRead()
{
	if (!m_socket.is_open())
		return;

	const auto max_packet_size = GetMaxPacketSize();
	if (m_receive_buffer_size < max_packet_size || max_packet_size < m_receive_buffer_size / 2)
	{
		m_receive_buffer_size = max_packet_size;
		m_receive_buffer = vs::make_unique_default_init<char[]>(m_receive_buffer_size);
	}

	const auto buffer = boost::asio::buffer(m_receive_buffer.get(), m_receive_buffer_size);
	typename socket_type::message_flags flags = 0;
#if defined(__linux__)
	flags |= MSG_TRUNC;
#endif
	m_socket.async_receive_from(buffer, m_receive_ep, flags, [this, self = this->shared_from_this()](boost::system::error_code ec, std::size_t bytes_transferred) {
		if (ec == boost::asio::error::operation_aborted)
			return;

#if NET_VERBOSE_LOGS
		if (ec)
			dstream4 << "UDPRouter(" << LocalEndpoint() << "): read error: " << ec.message();
#endif

		if (net::IsRecoverableUDPReadError(ec))
		{
			StartRead();
			return;
		}

		const bool truncated = CheckIfPacketTruncated(ec, bytes_transferred, m_receive_buffer_size);

		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		if (ec && !truncated)
		{
			for (auto& kv : m_connections)
				kv.second->ReadResult(ec);
			while (m_accept_op)
			{
				auto op = std::move(m_accept_op);
				m_accept_op = std::move(op->next);
				Complete(std::move(op), get_io_service(), ec);
			}
		}
		else
		{
			auto it = m_connections.find(m_receive_ep);
			if (it != m_connections.end())
			{
				it->second->ReadResult(ec, m_receive_buffer.get(), bytes_transferred);
			}
			else if (m_accept_queue.size() < m_accept_queue_size.load(std::memory_order_relaxed))
			{
				boost::system::error_code conn_ec;
				Connection<Protocol> connection(this->shared_from_this(), m_receive_ep, conn_ec);
				if (!conn_ec)
				{
					m_connections.emplace(m_receive_ep, connection.m_impl.get());
					connection.m_impl->ReadResult(ec, m_receive_buffer.get(), bytes_transferred);
					if (m_accept_op)
					{
						auto op = std::move(m_accept_op);
						m_accept_op = std::move(op->next);
						Complete(std::move(op), get_io_service(), std::move(connection));
					}
					else
						m_accept_queue.push_back(std::move(connection));
				}
				else
				{
					if (m_accept_op)
					{
						auto op = std::move(m_accept_op);
						m_accept_op = std::move(op->next);
						Complete(std::move(op), get_io_service(), conn_ec);
					}
				}
			}
			else
				dstream3 << "UDPRouter(" << LocalEndpoint() << "): accept queue overflow, connection from " << m_receive_ep << " dropped";
		}

		StartRead();
	});
}

template <class Protocol>
Connection<Protocol>::Impl::Impl(RouterPtr<Protocol> router, const endpoint_type& remote_ep, boost::system::error_code& ec)
	: m_router(std::move(router))
	, m_socket(m_router->get_io_service())
	, m_remote_ep(remote_ep)
	, m_read_queue_size(100)
	, m_read_in_progress(false)
	, m_receive_buffer_size(0)
{
	VS_SCOPE_EXIT {
		if (ec)
		{
			m_socket.close();
			m_router = nullptr;
		}
	};

	m_socket.open(m_router->LocalEndpoint().protocol(), ec);
	if (ec)
	{
		dstream3 << "UDPConnection(" << m_router->LocalEndpoint() << "<->" << m_remote_ep << "): open failed: " << ec.message();
		return;
	}
	m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
	if (ec)
	{
		dstream3 << "UDPConnection(" << m_router->LocalEndpoint() << "<->" << m_remote_ep << "): set SO_REUSEADDR failed: " << ec.message();
		return;
	}
	m_socket.bind(m_router->LocalEndpoint(), ec);
	if (ec)
	{
		dstream3 << "UDPConnection(" << m_router->LocalEndpoint() << "<->" << m_remote_ep << "): bind failed: " << ec.message();
		return;
	}
	m_socket.connect(m_remote_ep, ec);
	if (ec)
	{
		dstream3 << "UDPConnection(" << m_router->LocalEndpoint() << "<->" << m_remote_ep << "): connect failed: " << ec.message();
		return;
	}
#if NET_VERBOSE_LOGS
	dstream4 << "UDPConnection(" << m_router->LocalEndpoint() << "<->" << m_remote_ep << "): new";
#endif
}

template <class Protocol>
Connection<Protocol>::Impl::Impl(boost::asio::io_service& ios, net::port local_port,
	const endpoint_type& remote_ep, boost::system::error_code& ec)
	: m_socket(ios)
	, m_remote_ep(remote_ep)
	, m_read_queue_size(100)
	, m_read_in_progress(false)
	, m_receive_buffer_size(0)
{
	VS_SCOPE_EXIT {
		if (ec)
		{
			m_socket.close();
			m_router = nullptr;
		}
	};

	m_socket.open(remote_ep.protocol(), ec);
	if (ec)
	{
		dstream3 << "UDPConnection(" << "->" << m_remote_ep << "): open failed: " << ec.message();
		return;
	}
	m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
	if (ec)
	{
		dstream3 << "UDPConnection(" << "->" << m_remote_ep << "): set SO_REUSEADDR failed: " << ec.message();
		return;
	}
	m_socket.bind(boost::asio::ip::udp::endpoint(remote_ep.protocol(), local_port), ec);
	if (ec)
	{
		dstream3 << "UDPConnection(" << "->" << m_remote_ep << "): bind failed: " << ec.message();
		return;
	}
	m_socket.connect(m_remote_ep, ec);
	if (ec)
	{
		dstream3 << "UDPConnection(" << "->" << m_remote_ep << "): connect failed: " << ec.message();
		return;
	}
	auto local_endpoint = m_socket.local_endpoint(ec);
	if (ec)
	{
		dstream3 << "UDPConnection(" << "->" << m_remote_ep << "): get local endpoint failed: " << ec.message();
		return;
	}
	m_router = Router<Protocol>::Get(local_endpoint, ios, ec);
	if (ec == boost::asio::error::already_open)
		ec = {};
	if (ec)
	{
		dstream3 << "UDPConnection(" << local_endpoint <<  "<->" << m_remote_ep
			<< "): Router::Get() failed: " << ec.message();
		return;
	}
	m_router->RegisterConnection(remote_ep, this, ec);

#if NET_VERBOSE_LOGS
	dstream4 << "UDPConnection(" << local_endpoint << "<->" << m_remote_ep << "): new";
#endif
}

template <class Protocol>
Connection<Protocol>::Impl::~Impl()
{
	assert(!m_socket.is_open());
	assert(!m_router);
}

template <class Protocol>
void Connection<Protocol>::Impl::Close(bool unregister)
{
	if (!m_socket.is_open())
	{
		assert(!m_router);
		return;
	}

	assert(m_router);
#if NET_VERBOSE_LOGS
	dstream4 << "UDPConnection(" << m_router->LocalEndpoint() << "<->" << m_remote_ep << "): close";
#endif
	if (unregister)
		m_router->UnregisterConnection(m_remote_ep);

	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	ReadResultInternal(boost::asio::error::operation_aborted);
	m_socket.close();
	m_router = nullptr;
}

template <class Protocol>
void Connection<Protocol>::Impl::ReadResult(const boost::system::error_code& ec)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	ReadResultInternal(ec);
}

template <class Protocol>
void Connection<Protocol>::Impl::ReadResultInternal(const boost::system::error_code& ec)
{
	while (m_read_op)
	{
		auto op = std::move(m_read_op);
		m_read_op = std::move(op->next);
		Complete(std::move(op), m_socket.get_io_service(), ec);
	}
}

template <class Protocol>
void Connection<Protocol>::Impl::ReadResult(const boost::system::error_code& ec, const void* data, size_t size)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	ReadResultInternal(ec, data, size);
}

template <class Protocol>
void Connection<Protocol>::Impl::ReadResultInternal(const boost::system::error_code& ec, const void* data, size_t size)
{
	if (m_read_op)
	{
		auto op = std::move(m_read_op);
		m_read_op = std::move(op->next);
		Complete(std::move(op), m_socket.get_io_service(), ec, data, size);
	}
	else if (m_read_queue.Size() < m_read_queue_size.load(std::memory_order_relaxed))
		m_read_queue.PushBack(data, size);
	else
		dstream3 << "UDPConnection(" << m_router->LocalEndpoint() << "<->" << m_remote_ep << "): read queue overflow, packet dropped (sz=" << size << ")";
}

template <class Protocol>
void Connection<Protocol>::Impl::StartRead()
{
	const auto max_packet_size = m_router->GetMaxPacketSize();
	if (m_receive_buffer_size < max_packet_size || max_packet_size < m_receive_buffer_size / 2)
	{
		m_receive_buffer_size = max_packet_size;
		m_receive_buffer = vs::make_unique_default_init<char[]>(m_receive_buffer_size);
	}

	const auto buffer = boost::asio::buffer(m_receive_buffer.get(), m_receive_buffer_size);
	typename socket_type::message_flags flags = 0;
#if defined(__linux__)
	flags |= MSG_TRUNC;
#endif
	m_socket.async_receive(buffer, flags, [this, self = boost::intrusive_ptr<Impl>(this)](boost::system::error_code ec, std::size_t bytes_transferred) {
		if (ec == boost::asio::error::operation_aborted)
			return;

#if NET_VERBOSE_LOGS
		if (ec)
			dstream4 << "UDPConnection(" << m_router->LocalEndpoint() << "<->" << m_remote_ep << "): read error: " << ec.message();
#endif

		if (net::IsRecoverableUDPReadError(ec))
		{
			StartRead();
			return;
		}

		const bool truncated = CheckIfPacketTruncated(ec, bytes_transferred, m_receive_buffer_size);

		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		assert(m_read_in_progress);
		if (ec && !truncated)
			ReadResultInternal(ec);
		else
			ReadResultInternal(ec, m_receive_buffer.get(), bytes_transferred);

		if (m_read_op)
			StartRead();
		else
			m_read_in_progress = false;
	});
}

}}

#undef DEBUG_CURRENT_MODULE
#undef NET_VERBOSE_LOGS
