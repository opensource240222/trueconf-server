#include "VS_SignalChannel.h"
#include "net/Lib.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/event.h"
#include "std/cpplib/MakeShared.h"
#include "std/debuglog/VS_Debug.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>

#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/ignore.h"

#include <atomic>
#include <cassert>
#include <queue>
#include "net/Logger/LoggerInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

#define GW_VERBOSE_LOGS 0

class VS_SignalChannelImpl
	: public VS_SignalChannel
	, public vs::enable_shared_from_this<VS_SignalChannelImpl>
{
	static const size_t c_read_size = 16 * 1024;

	enum class State
	{
		Initial,
		TCP_Listening,
		TCP_Connecting,
		TCP_Accepted,
		TCP_Connected,
		UDP_Listening,
		UDP_Connected,
	};

public:
	~VS_SignalChannelImpl();

	bool Open(unsigned long flags, const net::address& bind_addr, net::port bind_port, const net::address& connect_addr, net::port connect_port, const net::QoSFlowSharedPtr& flow) override;
	bool Open(const net::QoSFlowSharedPtr &flow = nullptr) override;
	void Close(bool wait_for_send) override;
	void Send(vs::SharedBuffer&& buffer) override;
	net::address LocalAddress() const override;
	net::port LocalPort() const override;
	net::address RemoteAddress() const override;
	net::port RemotePort() const override;

protected:
	explicit VS_SignalChannelImpl(boost::asio::io_service& ios, const std::shared_ptr<net::LoggerInterface> &logger);

private:
	// Should be called while on strand
	bool POpen();
	void PClose();
	void CloseSockets();
	void ReadTCP();
	void ReadUDP_First();
	void ReadUDP();
	void WriteTCP();
	void WriteUDP();

private:
	std::atomic<State> m_state;
	mutable boost::asio::io_service::strand m_strand; // mutable is required to use post/dispatch in const functions
	unsigned long m_flags;
	net::address m_bind_addr;
	net::address m_connect_addr;
	net::port m_bind_port;
	net::port m_connect_port;

	boost::asio::ip::tcp::acceptor m_tcp_acceptor;
	boost::asio::ip::tcp::socket m_tcp;
	boost::asio::ip::udp::socket m_udp;
	boost::asio::ip::udp::endpoint m_udp_remote_ep; // remote endpoint from which we "accepted" connection
	bool m_close_after_send;
	bool m_read_in_progress;
	bool m_write_in_progress;
	unsigned m_socket_id; // used to identify operations executed on a previous socket
	std::unique_ptr<unsigned char[]> m_read_buffer;
	std::queue<vs::SharedBuffer> m_out_queue;

	net::QoSFlowSharedPtr m_flow;
	std::shared_ptr<net::LoggerInterface> m_logger;
	net::LoggerInterface::ConnectionInfo m_channel_log_info; // For logging purpose
};

std::function<std::shared_ptr<VS_SignalChannel> (boost::asio::io_service& ios)> VS_SignalChannel::s_factory;

std::shared_ptr<VS_SignalChannel> VS_SignalChannel::Create(boost::asio::io_service& ios, const std::shared_ptr<net::LoggerInterface> &logger)
{
	if (s_factory)
		return s_factory(ios);
	return vs::MakeShared<VS_SignalChannelImpl>(ios, logger);
}

VS_SignalChannelImpl::VS_SignalChannelImpl(boost::asio::io_service& ios, const std::shared_ptr<net::LoggerInterface> &logger)
	: m_state(State::Initial)
	, m_strand(ios)
	, m_flags(0)
	, m_bind_port(0)
	, m_connect_port(0)
	, m_tcp_acceptor(ios)
	, m_tcp(ios)
	, m_udp(ios)
	, m_close_after_send(false)
	, m_read_in_progress(false)
	, m_write_in_progress(false)
	, m_socket_id(1)
	, m_read_buffer(vs::make_unique_default_init<unsigned char[]>(c_read_size))
	, m_logger(logger)
{
}

VS_SignalChannelImpl::~VS_SignalChannelImpl()
{
	assert(m_state == State::Initial);
	assert(!m_tcp_acceptor.is_open());
	assert(!m_tcp.is_open());
	assert(!m_udp.is_open());
	dstream4 << "VS_SignalChannel(" << this << "): destroyed\n";
}

bool VS_SignalChannelImpl::Open(unsigned long flags, const net::address& bind_addr, net::port bind_port, const net::address& connect_addr, net::port connect_port, const net::QoSFlowSharedPtr& flow)
{
	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };

		bool reopen = true;
		switch (m_state)
		{
		case State::TCP_Listening:
			if (m_flags & flags & LISTEN_TCP // listening via TCP is allowed in both old and new flags
				&& m_bind_port == bind_port
				&& (bind_addr.is_unspecified() || m_bind_addr == bind_addr))
			{
				reopen = false;
			}
			if (flags & CONNECT_TCP && !connect_addr.is_unspecified() && connect_port != 0)
			{
				reopen = true;
			}
			break;
		case State::TCP_Accepted:
		{
			const auto local_ep = m_tcp.local_endpoint(vs::ignore<boost::system::error_code>()); // TODO: We probably can use m_bind_{addr,port} instead. Port chould be the same, address may change from unspecified to address of some interface, but that is ok because we won't compare two unspecified addresses.
			if (m_flags & flags & LISTEN_TCP // listening via TCP is allowed in both old and new flags
				&& local_ep.port() == bind_port
				&& (bind_addr.is_unspecified() || local_ep.address() == bind_addr))
			{
				reopen = false;
			}
		}
			break;
		case State::TCP_Connecting:
		case State::TCP_Connected:
		{
			const auto local_ep = m_tcp.local_endpoint(vs::ignore<boost::system::error_code>());
			if (m_flags & flags & CONNECT_TCP // connecting via TCP is allowed in both old and new flags
				&& m_connect_addr == connect_addr
				&& m_connect_port == connect_port
				&& (bind_addr.is_unspecified()
					? (bind_port == 0 || local_ep.port() == bind_port)
					: local_ep.address() == bind_addr))
			{
				reopen = false;
			}
		}
			break;
		case State::UDP_Listening:
			if (m_flags & flags & LISTEN_UDP // listening via UDP is allowed in both old and new flags
				&& m_bind_port == bind_port
				&& (bind_addr.is_unspecified() || m_bind_addr == bind_addr))
			{
				reopen = false;
			}
			if (flags & CONNECT_UDP && !connect_addr.is_unspecified() && connect_port != 0)
			{
				reopen = true;
			}
			break;
		case State::UDP_Connected:
		{
			const auto local_ep = m_udp.local_endpoint(vs::ignore<boost::system::error_code>());
			if (m_flags & flags & CONNECT_UDP // connecting via UDP is allowed in both old and new flags
				&& m_connect_addr == connect_addr
				&& m_connect_port == connect_port
				&& (bind_addr.is_unspecified()
					? (bind_port == 0 || local_ep.port() == bind_port)
					: local_ep.address() == bind_addr))
			{
				reopen = false;
			}
		}
			break;
		case State::Initial:
			break;
		}

		m_flags = flags;
		m_bind_addr = bind_addr;
		m_bind_port = bind_port;
		m_connect_addr = connect_addr;
		m_connect_port = connect_port;
		if (!reopen)
		{
			result = true;
			return;
		}
		m_flow = flow;
		result = POpen();
	});
	ready.wait();
	return result;
}

bool VS_SignalChannelImpl::Open(const net::QoSFlowSharedPtr &flow)
{
	if (m_state != State::Initial)
		return true;

	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };

		if (m_state != State::Initial)
		{
			result = true;
			return;
		}

		if (flow)
			m_flow = flow;
		result = POpen();
	});
	ready.wait();
	return result;
}

bool VS_SignalChannelImpl::POpen()
{
	assert(m_strand.running_in_this_thread());

	CloseSockets();
	m_state = State::Initial;

	//m_thread->RegisterTimeout(shared_from_this());
	boost::system::error_code ec;
	// Helper function to report network API errors
	auto is_ok = [&](const char* operation) {
		if (!ec)
			return true;

		dstream2 << "VS_SignalChannel(" << this << "): " << operation << " failed: " << ec.message();
		CloseSockets();
		return false;
	};

	if (m_flags & CONNECT_TCP && !m_connect_addr.is_unspecified() && m_connect_port != 0)
	{
		m_tcp.open(m_connect_addr.is_v4() ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(), ec);
		if (!is_ok("TCP open"))
			return false;
		if (m_bind_port != 0)
		{
			m_tcp.bind(boost::asio::ip::tcp::endpoint(m_bind_addr, m_bind_port), ec);
			if (!is_ok("TCP bind"))
				return false;
		}

		m_state = State::TCP_Connecting;
		const boost::asio::ip::tcp::endpoint remote_ep(m_connect_addr, m_connect_port);
		dstream3 << "VS_SignalChannel(" << this << "): started TCP connect to " << remote_ep;

		m_tcp.async_connect(remote_ep, m_strand.wrap([this, socket_id = m_socket_id, remote_ep, self = shared_from_this()](const boost::system::error_code& ec)
		{
			if (ec == boost::asio::error::operation_aborted)
				return;
			if (socket_id != m_socket_id)
				return; // This was an operation on a closed socket

			assert(m_state == State::TCP_Connecting);
			if (ec)
			{
				dstream2 << "VS_SignalChannel(" << this << "): TCP connect failed: " << ec.message();
				PClose();
				return;
			}

			if (m_flow)
			{
				if (!m_flow->AddSocket(m_tcp.native_handle(), remote_ep.data()))
					dstream2 << "VS_SignalChannel(" << this << "): failed to apply QoS";
			}

			m_state = State::TCP_Connected;
			dstream3 << "VS_SignalChannel(" << this << "): connected by TCP to " << remote_ep;
			m_signal_ChannelOpened();
			if (m_state != State::TCP_Connected)
				return;

			ReadTCP();
			if (!m_out_queue.empty())
				WriteTCP();
		}));
		return true;
	}

	if (m_flags & LISTEN_TCP && m_bind_port != 0)
	{
		m_tcp_acceptor.open(m_bind_addr.is_v4() ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(), ec);
		if (!is_ok("TCP open"))
			return false;
		const boost::asio::ip::tcp::endpoint bind_ep(m_bind_addr, m_bind_port);
		m_tcp_acceptor.bind(bind_ep, ec);
		if (ec == boost::asio::error::address_in_use)
			return false; // Expected error, no need to log it
		if (!is_ok("TCP bind"))
			return false;
		m_tcp_acceptor.listen(boost::asio::ip::tcp::socket::max_connections, ec);
		if (!is_ok("TCP listen"))
			return false;

		m_state = State::TCP_Listening;
		dstream3 << "VS_SignalChannel(" << this << "): started TCP listen on " << bind_ep;

		m_tcp_acceptor.async_accept(m_tcp, m_strand.wrap([this, socket_id = m_socket_id, self = shared_from_this()](const boost::system::error_code& ec)
		{
			if (ec == boost::asio::error::operation_aborted)
				return;
			if (socket_id != m_socket_id)
				return; // This was an operation on a closed socket

			assert(m_state == State::TCP_Listening);
			if (ec)
			{
				dstream2 << "VS_SignalChannel(" << this << "): TCP accept failed: " << ec.message();
				PClose();
				return;
			}

			const auto remote_ep = m_tcp.remote_endpoint(vs::ignore<boost::system::error_code>());
			if (m_flow)
			{
				if (!m_flow->AddSocket(m_tcp.native_handle(), remote_ep.data()))
					dstream2 << "VS_SignalChannel(" << this << "): failed to apply QoS";
			}

			m_state = State::TCP_Accepted;
			dstream3 << "VS_SignalChannel(" << this << "): accepted by TCP from " << remote_ep;
			m_signal_ChannelOpened();
			if (m_state != State::TCP_Accepted)
				return;

			ReadTCP();
			if (!m_out_queue.empty())
				WriteTCP();
		}));
		return true;
	}

	if (m_flags & CONNECT_UDP && !m_connect_addr.is_unspecified() && m_connect_port != 0)
	{
		m_udp.open(m_connect_addr.is_v4() ? boost::asio::ip::udp::v4() : boost::asio::ip::udp::v6(), ec);
		if (!is_ok("UDP open"))
			return false;
		m_udp.bind(boost::asio::ip::udp::endpoint(m_bind_addr, m_bind_port), ec);
		if (!is_ok("UDP bind"))
			return false;
		m_udp_remote_ep = boost::asio::ip::udp::endpoint(m_connect_addr, m_connect_port);
		m_udp.connect(m_udp_remote_ep, ec);
		if (!is_ok("UDP connect"))
			return false;

		if (m_flow)
		{
			if (!m_flow->AddSocket(m_udp.native_handle(), m_udp_remote_ep.data()))
				dstream2 << "VS_SignalChannel(" << this << "): failed to apply QoS";
		}

		m_state = State::UDP_Connected;
		dstream3 << "VS_SignalChannel(" << this << "): connected by UDP to " << m_udp_remote_ep;
		m_signal_ChannelOpened();
		if (m_state != State::UDP_Connected)
			return false;

		ReadUDP();
		if (!m_out_queue.empty())
			WriteUDP();

		return true;
	}

	if (m_flags & LISTEN_UDP && m_bind_port != 0)
	{
		m_udp.open(m_bind_addr.is_v4() ? boost::asio::ip::udp::v4() : boost::asio::ip::udp::v6(), ec);
		if (!is_ok("UDP open"))
			return false;
		const boost::asio::ip::udp::endpoint bind_ep(m_bind_addr, m_bind_port);
		m_udp.bind(bind_ep, ec);
		if (ec == boost::asio::error::address_in_use)
			return false; // Expected error, no need to log it
		if (!is_ok("UDP bind"))
			return false;

		m_state = State::UDP_Listening;
		dstream3 << "VS_SignalChannel(" << this << "): started UDP listen on " << bind_ep;

		m_signal_ChannelOpened();
		if (m_state != State::UDP_Listening)
			return false;

		ReadUDP_First();

		return true;
	}

	return false;
}

void VS_SignalChannelImpl::Close(bool wait_for_send)
{
	m_strand.dispatch([this, wait_for_send, self = shared_from_this()]() {
		if (wait_for_send && !m_out_queue.empty())
			m_close_after_send = true;
		else
		{
			PClose();
			// Don't allow reopen because we were explicitly asked to close the channel.
			m_flags &= ~(LISTEN_TCP | LISTEN_UDP | CONNECT_TCP | CONNECT_UDP);
		}
	});
}

void VS_SignalChannelImpl::PClose()
{
	assert(m_strand.running_in_this_thread());

	switch (m_state)
	{
	case State::TCP_Accepted:
		dstream3 << "VS_SignalChannel(" << this << "): closing incoming TCP connection from " << m_tcp.remote_endpoint(vs::ignore<boost::system::error_code>());
		break;
	case State::TCP_Connected:
		dstream3 << "VS_SignalChannel(" << this << "): closing outgoing TCP connection to " << m_tcp.remote_endpoint(vs::ignore<boost::system::error_code>());
		break;
	case State::UDP_Connected:
		dstream3 << "VS_SignalChannel(" << this << "): closing UDP connection to " << m_udp_remote_ep;
		break;
	case State::Initial:
	case State::TCP_Listening:
	case State::TCP_Connecting:
	case State::UDP_Listening:
		break;
	}

	if (m_flow) {
		m_flow->RemoveSocket(m_tcp.native_handle());
		m_flow->RemoveSocket(m_udp.native_handle());
	}
	CloseSockets();
	m_state = State::Initial;
	m_signal_ChannelClosed();
}

void VS_SignalChannelImpl::Send(vs::SharedBuffer&& buffer)
{
	m_strand.dispatch([this, buffer = std::move(buffer), self = shared_from_this()]() mutable {
		m_out_queue.push(std::move(buffer));
		if (m_write_in_progress)
			return;
		switch (m_state)
		{
		case State::TCP_Accepted:
		case State::TCP_Connected:
			WriteTCP();
			break;
		case State::UDP_Connected:
			WriteUDP();
			break;
		case State::Initial:
		case State::TCP_Listening:
		case State::TCP_Connecting:
		case State::UDP_Listening:
			break;
		}
	});
}

net::address VS_SignalChannelImpl::LocalAddress() const
{
	net::address result;
	if (m_state == State::Initial)
		return result;

	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		switch (m_state)
		{
		case State::TCP_Listening:
			if (m_tcp_acceptor.is_open())
				result = m_tcp_acceptor.local_endpoint(vs::ignore<boost::system::error_code>()).address();
			break;
		case State::TCP_Connecting:
		case State::TCP_Accepted:
		case State::TCP_Connected:
			if (m_tcp.is_open())
				result = m_tcp.local_endpoint(vs::ignore<boost::system::error_code>()).address();
			break;
		case State::UDP_Listening:
		case State::UDP_Connected:
			if (m_udp.is_open())
				result = m_udp.local_endpoint(vs::ignore<boost::system::error_code>()).address();
			break;
		case State::Initial:
			break;
		}
	});
	ready.wait();
	return result;
}

net::port VS_SignalChannelImpl::LocalPort() const
{
	net::port result = 0;
	if (m_state == State::Initial)
		return result;

	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		switch (m_state)
		{
		case State::TCP_Listening:
			if (m_tcp_acceptor.is_open())
				result = m_tcp_acceptor.local_endpoint(vs::ignore<boost::system::error_code>()).port();
			break;
		case State::TCP_Connecting:
		case State::TCP_Accepted:
		case State::TCP_Connected:
			if (m_tcp.is_open())
				result = m_tcp.local_endpoint(vs::ignore<boost::system::error_code>()).port();
			break;
		case State::UDP_Listening:
		case State::UDP_Connected:
			if (m_udp.is_open())
				result = m_udp.local_endpoint(vs::ignore<boost::system::error_code>()).port();
			break;
		case State::Initial:
			break;
		}
	});
	ready.wait();
	return result;
}

net::address VS_SignalChannelImpl::RemoteAddress() const
{
	net::address result;
	State state = m_state;
	if (state != State::TCP_Accepted
	 && state != State::TCP_Connected
	 && state != State::UDP_Connected)
		return result;

	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		switch (m_state)
		{
		case State::TCP_Accepted:
		case State::TCP_Connected:
			if (m_tcp.is_open())
				result = m_tcp.remote_endpoint(vs::ignore<boost::system::error_code>()).address();
			break;
		case State::UDP_Connected:
			result = m_udp_remote_ep.address();
			break;
		case State::Initial:
		case State::TCP_Listening:
		case State::TCP_Connecting:
		case State::UDP_Listening:
			break;
		}
	});
	ready.wait();
	return result;
}

net::port VS_SignalChannelImpl::RemotePort() const
{
	net::port result = 0;
	State state = m_state;
	if (state != State::TCP_Accepted
	 && state != State::TCP_Connected
	 && state != State::UDP_Connected)
		return result;

	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		switch (m_state)
		{
		case State::TCP_Accepted:
		case State::TCP_Connected:
			if (m_tcp.is_open())
				result = m_tcp.remote_endpoint(vs::ignore<boost::system::error_code>()).port();
			break;
		case State::UDP_Connected:
			result = m_udp_remote_ep.port();
			break;
		case State::Initial:
		case State::TCP_Listening:
		case State::TCP_Connecting:
		case State::UDP_Listening:
			break;
		}
	});
	ready.wait();
	return result;
}

void VS_SignalChannelImpl::CloseSockets()
{
	assert(m_strand.running_in_this_thread());

	++m_socket_id; // This will tell pending io handlers that sockets were closed
	m_tcp_acceptor.close();
	m_tcp.close();
	m_udp.close();
	m_close_after_send = false;
	m_read_in_progress = false;
	m_write_in_progress = false;
}

void VS_SignalChannelImpl::ReadTCP()
{
	assert(m_strand.running_in_this_thread());
	assert(m_state == State::TCP_Accepted || m_state == State::TCP_Connected);
	assert(m_tcp.is_open());
	assert(!m_read_in_progress);

	m_read_in_progress = true;
	const auto buffer = boost::asio::buffer(m_read_buffer.get(), c_read_size);
#if GW_VERBOSE_LOGS
	dstream4 << "VS_SignalChannel(" << this << "): TCP read start: size=" << boost::asio::buffer_size(buffer);
#endif
	m_tcp.async_receive(buffer, m_strand.wrap([this, socket_id = m_socket_id, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;
		if (socket_id != m_socket_id)
			return; // This was an operation on a closed socket

		assert(m_state == State::TCP_Accepted || m_state == State::TCP_Connected);
		m_read_in_progress = false;
		if (ec)
		{
			dstream2 << "VS_SignalChannel(" << this << "): TCP read failed: " << ec.message();
			PClose();
			return;
		}

#if GW_VERBOSE_LOGS
		dstream4 << "VS_SignalChannel(" << this << "): TCP read ok: size=" << bytes_transferred;
#endif
		if (m_logger)
			m_logger->Log(m_read_buffer.get(), bytes_transferred,m_tcp.remote_endpoint(vs::ignore<boost::system::error_code>()),
				m_tcp.local_endpoint(vs::ignore<boost::system::error_code>()), m_channel_log_info, net::protocol::TCP, false);
		m_signal_DataReceived(m_read_buffer.get(), bytes_transferred);
		if (!(m_state == State::TCP_Accepted || m_state == State::TCP_Connected))
			return;
		ReadTCP();
	}));
}

void VS_SignalChannelImpl::ReadUDP_First()
{
	assert(m_strand.running_in_this_thread());
	assert(m_state == State::UDP_Listening);
	assert(m_udp.is_open());
	assert(!m_read_in_progress);

	m_read_in_progress = true;
	const auto buffer = boost::asio::buffer(m_read_buffer.get(), c_read_size);
#if GW_VERBOSE_LOGS
	dstream4 << "VS_SignalChannel(" << this << "): UDP read start: size=" << boost::asio::buffer_size(buffer);
#endif
	m_udp.async_receive_from(buffer, m_udp_remote_ep, m_strand.wrap([this, socket_id = m_socket_id, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;
		if (socket_id != m_socket_id)
			return; // This was an operation on a closed socket

		assert(m_state == State::UDP_Listening);
		m_read_in_progress = false;
		if (ec)
		{
			dstream2 << "VS_SignalChannel(" << this << "): UDP read failed: " << ec.message();
			if (net::IsRecoverableUDPReadError(ec))
			{
				ReadUDP_First();
				return;
			}
			PClose();
			return;
		}

		boost::system::error_code ec_connect;
		m_udp.connect(m_udp_remote_ep, ec_connect);
		if (ec_connect)
		{
			dstream2 << "VS_SignalChannel(" << this << "): UDP connect failed: " << ec_connect.message();
			PClose();
			return;
		}

		if (m_flow)
		{
			if (!m_flow->AddSocket(m_udp.native_handle(), m_udp_remote_ep.data()))
				dstream2 << "VS_SignalChannel(" << this << "): failed to apply QoS";
		}

		m_state = State::UDP_Connected;
		dstream3 << "VS_SignalChannel(" << this << "): accepted by UDP from " << m_udp_remote_ep;
		if (m_logger)
			m_logger->Log(m_read_buffer.get(), bytes_transferred, m_udp.remote_endpoint(vs::ignore<boost::system::error_code>()),
				m_udp.local_endpoint(vs::ignore<boost::system::error_code>()), m_channel_log_info, net::protocol::UDP);
		m_signal_ChannelOpened();
		if (m_state != State::UDP_Connected)
			return;

#if GW_VERBOSE_LOGS
		dstream4 << "VS_SignalChannel(" << this << "): UDP read ok: size=" << bytes_transferred;
#endif
		m_signal_DataReceived(m_read_buffer.get(), bytes_transferred);
		if (m_state != State::UDP_Connected)
			return;

		ReadUDP();
		if (!m_out_queue.empty())
			WriteUDP();
	}));
}

void VS_SignalChannelImpl::ReadUDP()
{
	assert(m_strand.running_in_this_thread());
	assert(m_state == State::UDP_Connected);
	assert(m_udp.is_open());
	assert(!m_read_in_progress);

	m_read_in_progress = true;
	const auto buffer = boost::asio::buffer(m_read_buffer.get(), c_read_size);
#if GW_VERBOSE_LOGS
	dstream4 << "VS_SignalChannel(" << this << "): UDP read start: size=" << boost::asio::buffer_size(buffer);
#endif
	m_udp.async_receive(buffer, m_strand.wrap([this, socket_id = m_socket_id, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;
		if (socket_id != m_socket_id)
			return; // This was an operation on a closed socket

		assert(m_state == State::UDP_Connected);
		m_read_in_progress = false;
		if (ec)
		{
			dstream2 << "VS_SignalChannel(" << this << "): UDP read failed: " << ec.message();
			if (net::IsRecoverableUDPReadError(ec))
			{
				ReadUDP();
				return;
			}
			PClose();
			return;
		}

#if GW_VERBOSE_LOGS
		dstream4 << "VS_SignalChannel(" << this << "): UDP read ok: size=" << bytes_transferred;
#endif
		if (m_logger)
			m_logger->Log(m_read_buffer.get(), bytes_transferred, m_udp.remote_endpoint(vs::ignore<boost::system::error_code>()),
				m_udp.local_endpoint(vs::ignore<boost::system::error_code>()), m_channel_log_info, net::protocol::UDP);
		m_signal_DataReceived(m_read_buffer.get(), bytes_transferred);
		if (m_state != State::UDP_Connected)
			return;

		ReadUDP();
	}));
}

void VS_SignalChannelImpl::WriteTCP()
{
	assert(m_strand.running_in_this_thread());
	assert(m_state == State::TCP_Accepted || m_state == State::TCP_Connected);
	assert(m_tcp.is_open());
	assert(!m_write_in_progress);

	m_write_in_progress = true;
	const auto buffer = boost::asio::buffer(m_out_queue.front().data<const void>(), m_out_queue.front().size());
#if GW_VERBOSE_LOGS
	dstream4 << "VS_SignalChannel(" << this << "): TCP write start: size=" << boost::asio::buffer_size(buffer);
#endif
	boost::asio::async_write(m_tcp, buffer, m_strand.wrap([this, socket_id = m_socket_id, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;
		if (socket_id != m_socket_id)
			return; // This was an operation on a closed socket

		assert(m_state == State::TCP_Accepted || m_state == State::TCP_Connected);
		m_write_in_progress = false;
		if (ec)
		{
			dstream2 << "VS_SignalChannel(" << this << "): TCP write failed: " << ec.message();
			PClose();
			return;
		}

		assert(!m_out_queue.empty());
		assert(bytes_transferred == m_out_queue.front().size()); // boost::asio::async_write guarantees to either write everything or return an error
#if GW_VERBOSE_LOGS
		dstream4 << "VS_SignalChannel(" << this << "): TCP write ok: size=" << bytes_transferred;
#endif
		if (m_logger)
			m_logger->Log(m_out_queue.front().data<const void>(), m_out_queue.front().size(), m_tcp.local_endpoint(vs::ignore<boost::system::error_code>()),
				m_tcp.remote_endpoint(vs::ignore<boost::system::error_code>()), m_channel_log_info, net::protocol::TCP, true);
		m_out_queue.pop();
		if (m_out_queue.empty())
		{
			if (m_close_after_send)
			{
				PClose();
				// Don't allow reopen because we were explicitly asked to close the channel.
				m_flags &= ~(LISTEN_TCP | LISTEN_UDP | CONNECT_TCP | CONNECT_UDP);
			}
			return;
		}

		WriteTCP();
	}));
}

void VS_SignalChannelImpl::WriteUDP()
{
	assert(m_strand.running_in_this_thread());
	assert(m_state == State::UDP_Connected);
	assert(m_udp.is_open());
	assert(!m_write_in_progress);

	m_write_in_progress = true;
	const auto buffer = boost::asio::buffer(m_out_queue.front().data<const void>(), m_out_queue.front().size());
#if GW_VERBOSE_LOGS
	dstream4 << "VS_SignalChannel(" << this << "): UDP write start: size=" << boost::asio::buffer_size(buffer);
#endif
	m_udp.async_send(buffer, m_strand.wrap([this, socket_id = m_socket_id, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;
		if (socket_id != m_socket_id)
			return; // This was an operation on a closed socket

		assert(m_state == State::UDP_Connected);
		m_write_in_progress = false;
		if (ec)
		{
			dstream2 << "VS_SignalChannel(" << this << "): UDP write failed: " << ec.message();
			PClose();
			return;
		}

		assert(!m_out_queue.empty());
#if GW_VERBOSE_LOGS
		if (bytes_transferred != m_out_queue.front().size())
			dstream4 << "VS_SignalChannel(" << this << "): partial UDP write: size=" << bytes_transferred << ", expected=" << m_out_queue.front().size();
		else
			dstream4 << "VS_SignalChannel(" << this << "): UDP write ok: size=" << bytes_transferred;
#endif
		if (m_logger)
			m_logger->Log(m_out_queue.front().data<const void>(), bytes_transferred, m_udp.local_endpoint(vs::ignore<boost::system::error_code>()),
				m_udp.remote_endpoint(vs::ignore<boost::system::error_code>()), m_channel_log_info, net::protocol::UDP);
		m_out_queue.pop();
		if (m_out_queue.empty())
		{
			if (m_close_after_send)
				PClose();
			return;
		}

		WriteUDP();
	}));
}
