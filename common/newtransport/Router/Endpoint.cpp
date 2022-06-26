#include "Endpoint.h"
#include "MessageFactory.h"
#include "Router.h"
#include "SecureProvider.h"
#include "../../transport/Message.h"
#include "../../net/Connect.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "../../std/debuglog/VS_Debug.h"
#include "std-generic/cpplib/ignore.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

#define TRANSPORT_VERBOSE_LOGS 0

namespace transport {

static bool IsTCPKeepAliveAllowed()
{
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	uint32_t value = 1;
	key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "TCP KeepAlive");
	return 0 != value;
}

struct Endpoint::LogPrefix
{
	explicit LogPrefix(const Endpoint* obj_) : obj(obj_) {}
	const Endpoint* obj;

	template <class CharT, class Traits>
	friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, LogPrefix x)
	{
		s << "transport::Endpoint(" << x.GetLogID() << "): ";
		return s;
	}
private:
	string_view GetLogID() const { return obj->LogID(); }
};

Endpoint::Endpoint(boost::asio::io_service& ios, std::shared_ptr<Router> router, string_view endpoint_id)
	: EndpointBase(std::move(router), endpoint_id)
	, base_t(ios)
	, m_timer(ios)
	, m_version(0)
	, m_secure_handshake_version(0)
	, m_max_conn_silence_ms(8000)
	, m_fatal_silence_coef(3)
	, m_tcp_keep_alive_support(false)
	, m_last_connect_time()
	, m_last_disconnect_time()
{
#if TRANSPORT_VERBOSE_LOGS
	dstream4 << LogPrefix(this) << "created";
#endif
}

Endpoint::~Endpoint()
{
#if TRANSPORT_VERBOSE_LOGS
	dstream4 << LogPrefix(this) << "destroyed";
#endif
}

void Endpoint::SetConnection(socket_type&& socket, unsigned version, unsigned secure_handshake_version, uint8_t hops, bool tcp_keep_alive_support)
{
	m_version = version;
	m_secure_handshake_version = secure_handshake_version;
	m_hops = hops;
	m_tcp_keep_alive_support = tcp_keep_alive_support && IsTCPKeepAliveAllowed();

	m_last_connect_time = std::chrono::steady_clock::now();
	socket.set_option(boost::asio::socket_base::keep_alive(m_tcp_keep_alive_support));
	auto r = m_router.lock();
	if (IsSecure())
	{
		m_sec_storage = SecureProvider::Create();
		m_sec = m_sec_storage.get();
		if (r)
			m_sec->StartSecureHandshake(m_secure_handshake_version, handshake_type_Server, r->SrvPrivateKey(), r->SrvCertChain());
	}
	HandshakeResult result = HandshakeResult::ok;
	if ((version & ~c_ssl_support_mask) < c_version_min)
		result = HandshakeResult::antique_you;
	else if ((version & ~c_ssl_support_mask) > c_version)
		result = HandshakeResult::antique_me;
	if (r)
		Accept(std::move(socket), CreateHandshakeReply(r->EndpointName(), m_id, result, m_max_conn_silence_ms, m_fatal_silence_coef, 1, IsSecure(), m_tcp_keep_alive_support).get());
	Periodic();
}

void Endpoint::StartConnection()
{
	m_version = c_version | c_ssl_support_mask;
	{
		uint32_t pkey_sz = 0;
		auto r = m_router.lock();
		if (r)
		{
			auto&& pkey = r->SrvPrivateKey();
			pkey.GetPrivateKey(store_PEM_BUF, 0, &pkey_sz);
		}
		m_secure_handshake_version = (pkey_sz)? 2: 1;
	}
	m_hops = 1;
	Authorize(m_id); // the connections to servers are always the authorised ones.
	m_tcp_keep_alive_support = IsTCPKeepAliveAllowed();

	net::Connect<boost::asio::ip::tcp>(m_strand, m_id, [this, self = shared_from_this()](const boost::system::error_code& ec, boost::asio::ip::tcp::socket&& socket) {
		if (ec)
		{
			dstream4 << LogPrefix(this) << "connect failed: " << ec.message();
			return;
		}

		// todo(kt): after connect and two handshakes, need to do for server (hops=1):
		// 1) endpoint->Authorize();
		// 2) service_kv.second->OnEndpointConnect
		// 3) service_kv.second->OnEndpointIP

		m_last_connect_time = std::chrono::steady_clock::now();
		auto r = m_router.lock();
		if (r)
			Start(std::move(socket), CreateHandshake(r->EndpointName(), m_id, m_hops, IsSecure(), m_tcp_keep_alive_support).get());
	});
}

void Endpoint::Close()
{
	m_timer.cancel();
	base_t::Close();
}

void Endpoint::Shutdown()
{
	m_timer.cancel();
	base_t::Shutdown();
}

void Endpoint::ProcessMessage(const Message& message)
{
	m_recv_stats.Update(message.Size());
	auto message_type = message.AddString_sv();
	if (message_type.size() != 1)
		return;

	switch (message_type[0])
	{
	case c_ping_opcode:
#if TRANSPORT_VERBOSE_LOGS
		dstream4 << LogPrefix(this) << "ping received";
#endif
		break;
	case c_connect_opcode:
#if TRANSPORT_VERBOSE_LOGS
		dstream4 << LogPrefix(this) << "connect(?) received";
#endif
		break;
	case c_disconnect_opcode:
		dstream4 << LogPrefix(this) << "disconnect by remote request";
		Shutdown();
		break;
	default:
		dstream4 << LogPrefix(this) << "message of unknown type (" << static_cast<unsigned>(message_type[0]) << ')';
	}
}

void Endpoint::SendToPeer(const Message& message)
{
	m_last_send_time = std::chrono::steady_clock::now();
	m_send_stats.Update(message.Size());
	Send(message);
}

void Endpoint::SendPing()
{
	Send(CreatePing(m_id, m_hops > 0, IsAuthorized()));
}

void Endpoint::SendDisconnect()
{
	auto r = m_router.lock();
	if (r)
		Send(CreateDisconnect(r->EndpointName(), m_id, m_hops > 0, IsAuthorized()));
}

std::string Endpoint::GetRemoteIp()
{
	boost::system::error_code ec;
	const auto ep = m_socket.remote_endpoint(ec);
	if (ec)
		return {};
	return ep.address().to_string(vs::ignore<boost::system::error_code>());
}

void Endpoint::FillMonitorStruct(Monitor::TmReply::Endpoint &endpoint)
{
	if (m_hops)
	{
		endpoint.ep_type = 2;
	}
	else if (m_authorized)
	{
		endpoint.ep_type = 1;
	}
	else
	{
		endpoint.ep_type = 0;
	}
	endpoint.id = m_id;
	endpoint.username = m_authorized ? m_user_id : m_id;
	endpoint.protocol = "TCP";
	endpoint.last_connect_date_time = m_last_connect_time;
	endpoint.last_disconnect_date_time = m_last_disconnect_time;
	endpoint.remote_host = m_socket.remote_endpoint(vs::ignore<boost::system::error_code>()).address().to_string();
	endpoint.remote_port = m_socket.remote_endpoint(vs::ignore<boost::system::error_code>()).port();
	endpoint.local_host = m_socket.local_endpoint(vs::ignore<boost::system::error_code>()).address().to_string();
	endpoint.local_port = m_socket.local_endpoint(vs::ignore<boost::system::error_code>()).port();
	endpoint.send_stats = m_send_stats;
	endpoint.recv_stats = m_recv_stats;
}

bool Endpoint::IsSecure() const
{
	return (m_version & c_ssl_support_mask) != 0;
}

bool Endpoint::IsDisconnected() const
{
	return m_last_disconnect_time!=decltype(m_last_disconnect_time)()	// is ever disconnected
		&& m_last_connect_time < m_last_disconnect_time
		&& std::chrono::steady_clock::now() - m_last_disconnect_time > std::chrono::seconds(30);
}

void Endpoint::Periodic()
{
	m_timer.expires_from_now(std::chrono::seconds(1));
	if (IsDisconnected())
		return;
	m_timer.async_wait(m_strand.wrap([this, self = shared_from_this()](const boost::system::error_code& ec) {
#if TRANSPORT_VERBOSE_LOGS
				dstream4 << LogPrefix(this) << "timer event";
#endif
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (!m_tcp_keep_alive_support)
		{
			const auto now = std::chrono::steady_clock::now();
			const auto ping_interval = std::chrono::milliseconds(m_max_conn_silence_ms);
			const auto close_threshold = ping_interval * m_fatal_silence_coef;
			if (now - m_last_receive_time > close_threshold)
			{
				dstream1 << LogPrefix(this) << "closing connection: no receive for " << std::chrono::duration_cast<std::chrono::milliseconds>(close_threshold).count() << "ms";
				Close();
			}
			if (now - m_last_send_time > ping_interval)
				SendPing();
		}
		Periodic();
	}));
}

void Endpoint::OnHandshakeReply(HandshakeResult result, uint16_t max_conn_silence_ms, uint8_t fatal_silence_coef, uint8_t hops, const char* /*server_id*/, const char* /*client_id*/, bool tcp_keep_alive_support)
{
	if (result != HandshakeResult::ok)
	{
		dstream1 << LogPrefix(this) << "closing connection: unsuccessful handshake: result=" << static_cast<unsigned>(result);
		Close();
		return;
	}

	m_max_conn_silence_ms = max_conn_silence_ms;
	m_fatal_silence_coef = fatal_silence_coef;
	m_hops = hops;
	m_tcp_keep_alive_support = tcp_keep_alive_support && IsTCPKeepAliveAllowed();
	m_socket.set_option(boost::asio::socket_base::keep_alive(m_tcp_keep_alive_support));

	if (IsSecure())
	{
		m_sec_storage = SecureProvider::Create();
		m_sec = m_sec_storage.get();
		auto r = m_router.lock();
		if (r)
			m_sec->StartSecureHandshake(m_secure_handshake_version, handshake_type_Client, r->SrvPrivateKey(), r->SrvCertChain());
	}
	Periodic();
}

void Endpoint::OnMessage(Message&& message)
{
	if (!message.IsValid())
		return;

	m_recv_stats.Update(message.Size());
	PreprocessMessage(message);
	auto r = m_router.lock();
	if (r)
		r->ProcessMessage(std::move(message));
}

size_t Endpoint::OnReceive(const void* data, size_t size)
{
	m_last_receive_time = std::chrono::steady_clock::now();
	const auto consumed = base_t::OnReceive(data, size);
	auto r = m_router.lock();
	if (consumed && r)
		r->NotifyRead(consumed);
	return consumed;
}

void Endpoint::OnSend(size_t bytes_transferred)
{
	auto r = m_router.lock();
	if (r)
		r->NotifyWrite(bytes_transferred);
}

bool Endpoint::OnError(const boost::system::error_code& ec)
{
	dstream1 << LogPrefix(this) << "error: " << ec.message();
	return false;
}

void Endpoint::OnClose()
{
	m_last_disconnect_time = std::chrono::steady_clock::now();
	base_t::OnClose();

	// TODO: This has to be moved somewhere else when support for reconnects will be added.
	auto r = m_router.lock();
	if (r)
		r->RemoveEndpoint(std::static_pointer_cast<Endpoint>(shared_from_this()));
}

string_view Endpoint::LogID() const
{
	return m_id;
}

}
