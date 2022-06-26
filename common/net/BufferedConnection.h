#pragma once

#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/SharedBuffer.h"
#include "acs_v2/Handler.h"
#include "QoS.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <atomic>
#include <queue>

#include "std/cpplib/enable_shared_from_this_virtual_std.h"

#include "net/BufferedConnectionState.h"

namespace net {

template <class Socket = boost::asio::ip::tcp::socket>
class BufferedConnection : public enable_shared_from_this_virtual<BufferedConnection<Socket>>
{
protected:
	using socket_type = Socket;

	BufferedConnection(boost::asio::io_service& ios, size_t max_write_data_size = 1024 * 1024,
		size_t read_buffer_size = 16*1024);
	virtual ~BufferedConnection();

	BufferedConnectionState GetState() const
	{
		return m_state.load(std::memory_order_relaxed);
	}

	// Wrap a socket, start sending and receiving data.
	// If connection was already open then
	// OnError(boost::asio::error::already_open) is called.
	void SetSocket(socket_type&& socket, const net::QoSFlowSharedPtr &flow = nullptr, acs::Handler::stream_buffer &&init_read_buff = {}, bool clear_read_buffer = true, bool clear_write_buffer = true);

	// Immediately close connection, unfinished read/write operations will be
	// canceled, data in the send queue will not be sent.
	void Close();

	// Stop handling incoming data and close connection after data in the send
	// queue will be sent.
	void Shutdown();

	// Adds buffer to the send buffer queue.
	// If send buffer queue is too big already then
	// OnIOError(boost::asio::error::no_buffer_space)) is called.
	void Send(vs::SharedBuffer&& buffer);
	void Send(const void* data, size_t size);

	// All event handlers are called on the strand.

	// Called multiple times after read operation is completed successfully.
	// Should return number of bytes consumed (handled) from receive buffer.
	// Default implementation returns 'size' (discards all data).
	virtual size_t OnReceive(const void* data, size_t size);

	// Called after write operation is completed successfully.
	// Default implementation does nothing.
	virtual void OnSend(size_t bytes_transferred);

	// Called if asynchronous operation completes with an error.
	// Return value indicates if connection should be closed (false) or error
	// should be ignored (true).
	// Default implementation returns false;
	virtual bool OnError(const boost::system::error_code& ec);

	// Called after connection is completely closed.
	virtual void OnClose();

	// Can be used to provide custom ID to use in logs (object address is used by default).
	virtual string_view LogID() const;

private:
	void CloseInternal();
	void CompleteClose();
	void SendInternal(vs::SharedBuffer&& buffer);
	void StartWrite();
	void StartRead();
	void ProcessReadBuffer();

protected:
	boost::asio::io_service::strand m_strand;
	socket_type m_socket;
private:
	std::atomic<BufferedConnectionState> m_state;
	std::queue<vs::SharedBuffer> m_write_buffers;
	size_t m_max_write_data_size;
	size_t m_write_data_size;
	std::unique_ptr<unsigned char[]> m_read_buffer;
	const size_t m_read_buffer_size;
	size_t m_read_data_size;
	bool m_write_in_progress;
	bool m_read_in_progress;
	net::QoSFlowSharedPtr m_flow;

	struct LogPrefix;
};
extern template class BufferedConnection<>;

}
