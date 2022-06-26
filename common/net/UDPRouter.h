#pragma once

#include "net/Port.h"
#include "net/UDPRouter/AcceptOp.h"
#include "net/UDPRouter/fwd.h"
#include "net/UDPRouter/ReadOp.h"
#include "std-generic/compat/map.h"
#include "std-generic/cpplib/move_handler.h"
#include "std/cpplib/BlockQueue.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/intrusive_ptr.hpp>

#include <atomic>
#include <cassert>
#include <mutex>
#include <vector>

namespace net {

using UDPRouter = ur::Router<boost::asio::ip::udp>;
using UDPRouterPtr = ur::RouterPtr<boost::asio::ip::udp>;
using UDPConnection = ur::Connection<boost::asio::ip::udp>;

namespace ur {

template <class Protocol>
class Connection
{
	using socket_type = typename Protocol::socket;

public:
	using endpoint_type = typename Protocol::endpoint;
	using native_handle_type = typename socket_type::native_handle_type;

	// Connection is a move-only class.
	Connection() = default;
	Connection(const Connection&) = delete;
	Connection(Connection&& x) noexcept
		: m_impl(std::move(x.m_impl))
	{
		x.m_impl = nullptr;
	}
	Connection& operator=(const Connection&) = delete;
	Connection& operator=(Connection&& x) noexcept
	{
		if (this == &x)
			return *this;
		if (m_impl)
			m_impl->Close();
		m_impl = std::move(x.m_impl);
		return *this;
	}
	// Closes connection.
	// Handlers for all unfinished read and write operations will be called with ec == boost::asio::error::operation_aborted.
	// All read packets which were not yet received by user code (with AsyncReceive) are discarded.
	~Connection()
	{
		if (m_impl)
			m_impl->Close();
	}

	Router<Protocol>& GetRouter() const
	{
		return *m_impl->m_router;
	}

	// Allows to set (and get) maximum number of packets that will we waiting in a queue for read operation.
	void SetReadQueueSize(unsigned packets)
	{
		m_impl->m_read_queue_size.store(packets, std::memory_order_relaxed);
	}
	unsigned GetReadQueueSize() const
	{
		return m_impl->m_read_queue_size.load(std::memory_order_relaxed);
	}

	// Functions below mimic asio::basic_stream_socket API

	boost::asio::io_service& get_io_service()
	{
		return m_impl->m_socket.get_io_service();
	}

	// Returns true if connection is open.
	// Default-constructed and moved-from instances are considered closed.
	// If connection is closed no functions other than is_open() shall be called.
	bool is_open() const
	{
		return m_impl != nullptr;
	}

	native_handle_type native_handle()
	{
		return m_impl->m_socket.native_handle();
	}

	const endpoint_type& local_endpoint() const
	{
		return m_impl->m_router->LocalEndpoint();
	}
	const endpoint_type& local_endpoint(boost::system::error_code& ec) const
	{
		ec = {};
		return local_endpoint();
	}

	const endpoint_type& remote_endpoint() const
	{
		return m_impl->m_remote_ep;
	}
	const endpoint_type& remote_endpoint(boost::system::error_code& ec) const
	{
		ec = {};
		return remote_endpoint();
	}

	// Starts asynchronous read operation, see documentation for boost::asio::ip::udp::socket::async_receive for details.
	// Only the first 'm_max_packet_size' bytes of the incoming packet are stored in 'buffers', if packet was truncated is such way then 'ec' will be set to net::errc::packet_truncated.
	// Multiple read operations may be started in a row, but order in which they complete in indeterminate.
	template <class MutableBufferSequence, class ReadHandler>
	void async_receive(const MutableBufferSequence& buffers, ReadHandler&& handler)
	{
		m_impl->AsyncReceive(buffers, std::forward<ReadHandler>(handler));
	}
	template <class MutableBufferSequence, class ReadHandler>
	void async_receive(const MutableBufferSequence& buffers, boost::asio::socket_base::message_flags /*flags*/, ReadHandler&& handler)
	{
		m_impl->AsyncReceive(buffers, std::forward<ReadHandler>(handler));
	}

	// Starts asynchronous write operation, see documentation for boost::asio::ip::udp::socket::async_send for details.
	// Multiple write operations may be started in a row, but order in which they complete in indeterminate.
	template <class ConstBufferSequence, class WriteHandler>
	void async_send(const ConstBufferSequence& buffers, WriteHandler&& handler)
	{
		m_impl->AsyncSend(buffers, 0, std::forward<WriteHandler>(handler));
	}
	template <class ConstBufferSequence, class WriteHandler>
	void async_send(const ConstBufferSequence& buffers, boost::asio::socket_base::message_flags flags, WriteHandler&& handler)
	{
		m_impl->AsyncSend(buffers, flags, std::forward<WriteHandler>(handler));
	}

private:
	friend class Router<Protocol>;
	// These functions are called by Router
	Connection(RouterPtr<Protocol> router, const endpoint_type& remote_ep, boost::system::error_code& ec)
		: m_impl(new Impl(std::move(router), remote_ep, ec))
	{
	}
	Connection(boost::asio::io_service& ios, net::port local_port,
		const endpoint_type& remote_ep, boost::system::error_code& ec)
		: m_impl(new Impl(ios, local_port, remote_ep, ec))
	{
	}

private:
	class Impl
	{
		friend Connection;
	public:
		Impl(RouterPtr<Protocol> router, const endpoint_type& remote_ep, boost::system::error_code& ec);
		Impl(boost::asio::io_service& ios, net::port local_port,
			const endpoint_type& remote_ep, boost::system::error_code& ec);
		~Impl();

		void Close(bool unregister = true);

		template <class MutableBufferSequence, class ReadHandler>
		void AsyncReceive(const MutableBufferSequence& buffers, ReadHandler&& handler);

		template <class ConstBufferSequence, class WriteHandler>
		void AsyncSend(const ConstBufferSequence& buffers, boost::asio::socket_base::message_flags flags, WriteHandler&& handler);

		// These functions are called by Router
		void ReadResult(const boost::system::error_code& ec);
		void ReadResult(const boost::system::error_code& ec, const void* data, size_t size);

	private:
		void StartRead();
		void ReadResultInternal(const boost::system::error_code& ec);
		void ReadResultInternal(const boost::system::error_code& ec, const void* data, size_t size);

		friend void intrusive_ptr_add_ref(Impl* p)
		{
			p->m_ref_count.fetch_add(1, std::memory_order_relaxed);
		}

		friend void intrusive_ptr_release(Impl* p)
		{
			if (p->m_ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
				delete p;
		}

	private:
		std::atomic<unsigned> m_ref_count {0};
		RouterPtr<Protocol> m_router; // Connection owns its Router
		socket_type m_socket;
		const endpoint_type m_remote_ep;

		std::mutex m_mutex;
		ReadOpPtr m_read_op;
		vs::BlockQueue m_read_queue;
		std::atomic<unsigned> m_read_queue_size;
		bool m_read_in_progress;

		// Read loop data
		std::unique_ptr<char[]> m_receive_buffer;
		size_t m_receive_buffer_size;
	};
	boost::intrusive_ptr<Impl> m_impl;
};

template <class Protocol>
class Router : public std::enable_shared_from_this<Router<Protocol>>
{
	using socket_type = typename Protocol::socket;

public:
	using endpoint_type = typename Protocol::endpoint;

	// Returns pointer to existing router bound to 'local_ep'.
	// If such router doesn't exist return nullptr.
	static RouterPtr<Protocol> Get(const endpoint_type& local_ep);

	// Creates new router, binding it to 'local_ep', and returns a pointer to it.
	// If such router already exists, sets 'ec' to boost::asio::error::already_open and returns a pointer to existing router.
	// If an error occurs during creation it is returned via 'ec', function return nullptr in this case.
	static RouterPtr<Protocol> Get(const endpoint_type& local_ep, boost::asio::io_service& ios, boost::system::error_code& ec);

	~Router();

	boost::asio::io_service& get_io_service()
	{
		return m_socket.get_io_service();
	}

	const endpoint_type& LocalEndpoint() const
	{
		return m_local_ep;
	}

	// Allows to set (and get) maximum packet size expected to be received, all packets larger than 'size' will be truncated by OS.
	void SetMaxPacketSize(size_t size)
	{
		m_max_packet_size.store(size, std::memory_order_relaxed);
	}
	size_t GetMaxPacketSize() const
	{
		return m_max_packet_size.load(std::memory_order_relaxed);
	}

	// Allows to set (and get) maximum number of connections that will we waiting in a queue for accept operation.
	void SetAcceptQueueSize(unsigned connections)
	{
		m_accept_queue_size.store(connections, std::memory_order_relaxed);
	}
	unsigned GetAcceptQueueSize() const
	{
		return m_accept_queue_size.load(std::memory_order_relaxed);
	}

	// Instructs router to close and unregister itself when it becomes unused: when there are no connections managed by user code that use this router.
	// When router closes:
	//   * Handlers for all unfinished accept operations will be called with ec == boost::asio::error::operation_aborted.
	//   * All connections which were not yet accepted by user code (with AsyncAccept) will be closed.
	void CloseWhenUnused();

	// Creates a connection to 'destination'.
	// If a connection to 'destination' already exists, sets 'ec' to boost::asio::error::already_open and returns nullptr.
	// If an error occurs during creation it is returned via 'ec', function return nullptr in this case.
	// Note: Nothing is actually gets sent to 'destination' at that point, this is why this function is not asynchronous.
	Connection<Protocol> Connect(const endpoint_type& destination, boost::system::error_code& ec);

	static Connection<Protocol> Connect(boost::asio::io_service& ios, net::port local_port, const endpoint_type& destination,
		boost::system::error_code& ec);

	// Starts asynchronous accept operation.
	// 'handler' is expected to have this signature: void (const boost::system::error_code& ec, Connection<Protocol> connection)
	// If operation completes successfully 'connection' is non-null.
	// if operation completes with an error that error is passed via 'ec'
	// Multiple accept operations may be started in a row, but order in which they complete in indeterminate.
	template <class AcceptHandler>
	void AsyncAccept(AcceptHandler&& handler);

	// If there is an unaccepted connection, immediately accepts it a pointer to it, otherwise returns nullptr.
	Connection<Protocol> TryAccept();

private:
	friend class Connection<Protocol>::Impl;
	// These functions are called by Connection::Impl
	void RegisterConnection(const endpoint_type& destination,
		typename Connection<Protocol>::Impl* impl, boost::system::error_code& ec);
	void UnregisterConnection(const endpoint_type& remote_ep);

private:
	Router(boost::asio::io_service& ios, const endpoint_type& local_ep, boost::system::error_code& ec);
	bool DontCloseWhenUnused();
	void CloseIfUnused();
	void StartRead();

private:
	socket_type m_socket;
	const endpoint_type m_local_ep;

	std::atomic<size_t> m_max_packet_size;
	std::atomic<unsigned> m_accept_queue_size;

	std::mutex m_mutex;
	vs::map<endpoint_type, typename Connection<Protocol>::Impl*> m_connections; // Router doesn't own connections accepted/connected by user code to avoid circular ownership.
	std::vector<Connection<Protocol>> m_accept_queue; // Router owns connections not yet accepted by user code.
	AcceptOpPtr<Protocol> m_accept_op;
	bool m_close_when_unused;

	// Read loop data
	endpoint_type m_receive_ep;
	std::unique_ptr<char[]> m_receive_buffer;
	size_t m_receive_buffer_size;

	static std::mutex s_mutex;
	static vs::map<endpoint_type, std::weak_ptr<Router>> s_routers;
};

extern template class Router<boost::asio::ip::udp>;
extern template class Connection<boost::asio::ip::udp>;

template <class Protocol>
template <class AcceptHandler>
void Router<Protocol>::AsyncAccept(AcceptHandler&& handler)
{
	std::unique_lock<decltype(m_mutex)> lock(m_mutex);
	assert(!(m_accept_op && !m_accept_queue.empty())); // There shouldn't be any unfinished accept operations when accept queue isn't empty.
	if (!m_accept_queue.empty())
	{
		auto connection = std::move(m_accept_queue.back());
		m_accept_queue.pop_back();

		lock.unlock();
		get_io_service().post(vs::move_handler([handler = std::forward<AcceptHandler>(handler), connection = std::move(connection)]() mutable {
			handler(boost::system::error_code(), std::move(connection));
		}));
	}
	else
	{
		auto next_accept = std::move(m_accept_op);
		m_accept_op = AcceptOp<Protocol>::Make(std::forward<AcceptHandler>(handler));
		m_accept_op->next = std::move(next_accept);
	}
}

template <class Protocol>
template <class MutableBufferSequence, class ReadHandler>
void Connection<Protocol>::Impl::AsyncReceive(const MutableBufferSequence& buffers, ReadHandler&& handler)
{
	if (!m_socket.is_open())
	{
		m_socket.get_io_service().post([handler = std::forward<ReadHandler>(handler)]() mutable {
			handler(boost::asio::error::not_connected, 0);
		});
		return;
	}

	std::unique_lock<decltype(m_mutex)> lock(m_mutex);
	assert(!(m_read_op && !m_read_queue.Empty())); // There shouldn't be any unfinished read operations when read queue isn't empty.
	if (!m_read_queue.Empty())
	{
		const auto bytes_transferred = boost::asio::buffer_copy(buffers,
			boost::asio::buffer(m_read_queue.Front().Data(), m_read_queue.Front().Size())
		);
		m_read_queue.PopFront();

		lock.unlock();
		m_socket.get_io_service().post([handler = std::forward<ReadHandler>(handler), bytes_transferred]() mutable {
			handler(boost::system::error_code(), bytes_transferred);
		});
	}
	else
	{
		auto next_read = std::move(m_read_op);
		m_read_op = ReadOp::Make(buffers, handler);
		m_read_op->next = std::move(next_read);
		if (!m_read_in_progress)
		{
			m_read_in_progress = true;

			lock.unlock();
			StartRead();
		}
	}
}

template <class Protocol>
template <class ConstBufferSequence, class WriteHandler>
void Connection<Protocol>::Impl::AsyncSend(const ConstBufferSequence& buffers, boost::asio::socket_base::message_flags flags, WriteHandler&& handler)
{
	m_socket.async_send(buffers, flags, std::forward<WriteHandler>(handler));
}

}
}
