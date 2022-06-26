#include "TrueGateway/clientcontrols/VS_ClientControlAllocatorInterface.h"
#include "TrueGateway/clientcontrols/VS_ClientControlInterface.h"
#include "TrueGateway/interfaces/VS_ParserInterface.h"
#include "TrueGateway/TransportTools.h"
#include "TrueGateway/VS_TransportConnection.h"
#include "TrueGateway/VS_GatewayParticipantInfo.h"
#include "FakeClient/VS_ConferenceInfo.h"
#include "net/Lib.h"
#include "net/QoSSettings.h"
#include "net/VS_MediaAddress.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/ignore.h"
#include "std/cpplib/event.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_UserData.h"
#include "std/VS_TransceiverInfo.h"
#include "tools/Server/VS_MediaChannelInfo.h"
#include "std/debuglog/VS_Debug.h"

#include <boost/algorithm/string/predicate.hpp>

#include <algorithm>
#include <atomic>
#include <thread>
#include "net/Logger/PcapLogger.h"

#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

#define GW_VERBOSE_LOGS 0

extern std::string g_tr_endpoint_name;

extern VS_ConferenceProtocolInterface::ConferenceStatus to_conference_status(VS_CallInviteStatus loginStatus);

static std::atomic<unsigned> s_concurrent_async_invite_threads { 0 };

struct VS_TransportConnection::LogPrefix
{
	explicit LogPrefix(const VS_TransportConnection* obj_) : obj(obj_) {}
	const VS_TransportConnection* obj;

	template <class CharT, class Traits>
	friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, LogPrefix x)
	{
		s << "TransportConnection(" << x.obj << "): ";
		return s;
	}
};

class VS_TransportConnection::Channel : public std::enable_shared_from_this<Channel>
{
public:
	explicit Channel(std::shared_ptr<VS_TransportConnection> parent);
	~Channel();

	void Accept(boost::asio::ip::tcp::socket&& socket, const void* data, size_t size);
	void Accept(net::UDPConnection&& connection, const void* data, size_t size);
	void Connect(const net::address& address, net::port port, net::protocol protocols, VS_ChannelID channel_id);
	void Close();

	void Timeout();
	void ResetConnectionTimeout();
	void TryStartWrite();

	bool IsConnected() const { return m_connected; }
	const net::address& LocalAddress()  const { return m_local_address; }
	const net::address& RemoteAddress() const { return m_remote_address; }
	net::port LocalPort()  const { return m_local_port; }
	net::port RemotePort() const { return m_remote_port; }
	net::protocol ConnectionProtocol() const;

private:
	void TryConnectTCP();
	bool TryConnectUDP();
	void OnOpen(const void* data, size_t size);
	void StartRead();

	const std::shared_ptr<VS_TransportConnection> m_parent;

	boost::asio::ip::tcp::socket m_tcp;
	net::UDPConnection m_udp;
	VS_ChannelID m_channel_id;

	net::address m_local_address;
	net::address m_remote_address;
	net::port m_local_port;
	net::port m_remote_port;

	static const size_t c_read_buffer_size = 16*1024;
	size_t m_write_buffer_size;
	std::unique_ptr<unsigned char[]> m_read_buffer;
	std::unique_ptr<unsigned char[]> m_write_buffer;

	net::protocol m_connect_protocols; // Should be net::protocol::none for accepted connections
	std::chrono::steady_clock::time_point m_connect_timeout;

	bool m_connected;
	bool m_write_in_progress;

	static const std::chrono::seconds c_connect_timeout;

	struct LogPrefix;
	net::LoggerInterface::ConnectionInfo m_channel_log_info;
};

const std::chrono::seconds VS_TransportConnection::Channel::c_connect_timeout = std::chrono::seconds(7);

struct VS_TransportConnection::Channel::LogPrefix
{
	explicit LogPrefix(const VS_TransportConnection::Channel* obj_) : obj(obj_) {}
	const VS_TransportConnection::Channel* obj;

	template <class CharT, class Traits>
	void Write(std::basic_ostream<CharT, Traits>& s)
	{
		s << "TransportConnection(" << obj->m_parent.get() << "): ";
		s << "Channel(" << obj << "): ";
		if (obj->m_tcp.is_open() || obj->m_udp.is_open())
		{
			if (obj->m_tcp.is_open())
				s << "TCP[";
			else if (obj->m_udp.is_open())
				s << "UDP[";
			s << obj->m_local_address << ':' << obj->m_local_port << " <-> " << obj->m_remote_address << ':' << obj->m_remote_port << "]: ";
		}
	}

	template <class CharT, class Traits>
	friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, LogPrefix x)
	{
		x.Write(s);
		return s;
	}
};

VS_TransportConnection::Channel::Channel(std::shared_ptr<VS_TransportConnection> parent)
	: m_parent(std::move(parent))
	, m_tcp(m_parent->m_strand.get_io_service())
	, m_channel_id(e_noChannelID)
	, m_local_port(0)
	, m_remote_port(0)
	, m_write_buffer_size(512)
	, m_read_buffer(vs::make_unique_default_init<unsigned char[]>(c_read_buffer_size))
	, m_write_buffer(vs::make_unique_default_init<unsigned char[]>(m_write_buffer_size))
	, m_connect_protocols(net::protocol::none)
	, m_connected(false)
	, m_write_in_progress(false)
{
}

VS_TransportConnection::Channel::~Channel()
{
#if GW_VERBOSE_LOGS
	dstream4 << LogPrefix(this) << "deleted";
#endif
	assert(!m_connected);
	assert(!m_tcp.is_open());
	assert(!m_udp.is_open());
}

void VS_TransportConnection::Channel::Close()
{
	assert(m_parent->m_strand.running_in_this_thread());

	m_tcp.close();
	m_udp = {};
	m_write_in_progress = false;

	if (m_connected)
	{
		dstream3 << LogPrefix(this) << "closed";
		m_connected = false;
	}
	m_parent->OnChannelClose(this);
}

void VS_TransportConnection::Channel::Accept(boost::asio::ip::tcp::socket&& socket, const void* data, size_t size)
{
	assert(m_parent->m_strand.running_in_this_thread());
	assert(socket.is_open());

	m_channel_id = m_parent->m_parser->GetChannelID(data, size, vs::ignore<bool>());
	m_tcp = std::move(socket);

	boost::system::error_code ec;
	const auto local_ep = m_tcp.local_endpoint(ec);
	if (ec)
	{
		dstream2 << LogPrefix(this) << "local_endpoint() falied: " << ec.message();
	}
	m_local_address = local_ep.address();
	m_local_port = local_ep.port();

	const auto remote_ep = m_tcp.remote_endpoint(ec);
	if (ec)
	{
		dstream2 << LogPrefix(this) << "remote_endpoint() falied: " << ec.message();
	}
	m_remote_address = remote_ep.address();
	m_remote_port = remote_ep.port();

	if (m_parent->m_logger)
		m_parent->m_logger->Log(data, size, remote_ep, local_ep, m_channel_log_info, net::protocol::TCP, false);

	OnOpen(data, size);
}

void VS_TransportConnection::Channel::Accept(net::UDPConnection&& connection, const void* data, size_t size)
{
	assert(m_parent->m_strand.running_in_this_thread());
	assert(connection.is_open());

	m_channel_id = m_parent->m_parser->GetChannelID(data, size, vs::ignore<bool>());
	m_udp = std::move(connection);

	const auto local_ep = m_udp.local_endpoint();
	m_local_address = local_ep.address();
	m_local_port = local_ep.port();

	const auto remote_ep = m_udp.remote_endpoint();
	m_remote_address = remote_ep.address();
	m_remote_port = remote_ep.port();

	if (m_parent->m_logger)
		m_parent->m_logger->Log(data, size, remote_ep, local_ep, m_channel_log_info, net::protocol::UDP);

	OnOpen(data, size);
}

void VS_TransportConnection::Channel::Connect(const net::address& address, net::port port, net::protocol protocols, VS_ChannelID channel_id)
{
	assert(m_parent->m_strand.running_in_this_thread());

	m_channel_id = channel_id;
	m_remote_address = address;
	m_remote_port = port;
	m_connect_protocols = protocols;

	if (static_cast<bool>(m_connect_protocols & net::protocol::TCP))
		TryConnectTCP();
	else if (static_cast<bool>(m_connect_protocols & net::protocol::UDP))
	{
		if (!TryConnectUDP())
			Close();
	}
	else
	{
		assert(false);
		Close();
	}
}

void VS_TransportConnection::Channel::TryConnectTCP()
{
	assert(m_parent->m_strand.running_in_this_thread());
	assert(static_cast<bool>(m_connect_protocols & net::protocol::TCP));

	m_connect_timeout = std::chrono::steady_clock::now() + c_connect_timeout;

#if GW_VERBOSE_LOGS
	dstream4 << LogPrefix(this) << "starting TCP connect";
#endif
	m_tcp.async_connect(boost::asio::ip::tcp::endpoint(m_remote_address, m_remote_port), m_parent->m_strand.wrap([this, self = shared_from_this()](const boost::system::error_code& ec) {
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (ec)
		{
			dstream2 << LogPrefix(this) << "TCP connect failed: " << ec.message();
			if (static_cast<bool>(m_connect_protocols & net::protocol::UDP))
			{
				m_tcp.close();
				if (!TryConnectUDP())
					Close();
			}
			else
				Close();
			return;
		}

		boost::system::error_code ec2;
		const auto local_ep = m_tcp.local_endpoint(ec2);
		if (ec2)
		{
			dstream2 << LogPrefix(this) << "local_endpoint() falied: " << ec.message();
		}
		m_local_address = local_ep.address();
		m_local_port = local_ep.port();

		m_connect_protocols = net::protocol::none;
		OnOpen(nullptr, 0);
	}));
}

bool VS_TransportConnection::Channel::TryConnectUDP()
{
	assert(m_parent->m_strand.running_in_this_thread());
	assert(static_cast<bool>(m_connect_protocols & net::protocol::UDP));

	m_local_port = [&]() {
		switch (m_channel_id)
		{
		case e_RAS:
			return 1719;
		case e_RAS_DISCOVERY:
			return 1718;
		case e_H225:
			return 1720;
		case e_SIP_CS:
		case e_SIP_Register:
			return 5060;
		}
		return 1721;
	}();
	boost::system::error_code ec;

	m_udp = net::UDPRouter::Connect(m_parent->m_strand.get_io_service(), m_local_port, boost::asio::ip::udp::endpoint(m_remote_address, m_remote_port), ec);
	if (ec)
		return false;

	auto local_endpoint = m_udp.local_endpoint();
	m_local_address = local_endpoint.address();
	m_local_port = local_endpoint.port();
#if GW_VERBOSE_LOGS
	dstream4 << LogPrefix(this) << "assuming UDP connect";
#endif

	m_connect_protocols = net::protocol::none;
	OnOpen(nullptr, 0);
	return true;
}

void VS_TransportConnection::Channel::OnOpen(const void* data, size_t size)
{
	assert(m_parent->m_strand.running_in_this_thread());
	assert((m_tcp.is_open() != m_udp.is_open()) && "TCP or UDP socket should be open, but not both");

	const bool is_udp = m_udp.is_open();

	if (!is_udp && IsTCPKeepAliveAllowed())
		net::EnableTCPKeepAlive(m_tcp.native_handle(), 20, 2);

	net::QoSFlowSharedPtr flow;
	if      (m_channel_id == e_H225)
		flow = net::QoSSettings::GetInstance().GetH323QoSFlow(is_udp, m_remote_address.is_v6());
	else if (m_channel_id == e_RTSP)
		flow = net::QoSSettings::GetInstance().GetRTSPQoSFlow(m_remote_address.is_v6());
	if (flow)
	{
		if (!flow->AddSocket(
				is_udp ? m_udp.native_handle() : m_tcp.native_handle(),
				is_udp
					? m_udp.remote_endpoint(vs::ignore<boost::system::error_code>()).data()
					: m_tcp.remote_endpoint(vs::ignore<boost::system::error_code>()).data()
			))
		{
			dstream2 << LogPrefix(this) << "failed to apply QoS";
		}
	}

	dstream3 << LogPrefix(this) << "new";
	m_connected = !is_udp || size > 0; // We treat UDP connections as established only if we received something.
	StartRead();
	TryStartWrite();

	m_parent->m_parser->SetMyCsAddress({m_local_address, m_local_port, is_udp ? net::protocol::UDP : net::protocol::TCP});
	m_parent->m_parser->SetPeerCSAddress("", {m_remote_address, m_remote_port, is_udp ? net::protocol::UDP : net::protocol::TCP});

	if (ts::UseLocalTransceiver())
		m_parent->m_parser->SetMyMediaAddress(m_local_address);
	else
		m_parent->m_parser->SetMyMediaAddress(net::GetRTPAddress(m_parent->m_strand.get_io_service()));

	if (data && size > 0)
		m_parent->m_parser->SetRecvBuf(data, size, m_channel_id, m_remote_address, m_remote_port, m_local_address, m_local_port);

	m_parent->OnChannelOpen(this);
}

net::protocol VS_TransportConnection::Channel::ConnectionProtocol() const
{
	if (m_tcp.is_open())
		return net::protocol::TCP;
	else if (m_udp.is_open())
		return net::protocol::UDP;
	else
		return net::protocol::none;
}

void VS_TransportConnection::Channel::Timeout()
{
	assert(m_parent->m_strand.running_in_this_thread());

	if (m_connect_protocols != net::protocol::none && std::chrono::steady_clock::now() > m_connect_timeout)
	{
		if (m_connect_protocols == (net::protocol::TCP | net::protocol::UDP))
		{
			dstream2 << LogPrefix(this) << "connect timeout, retrying with UDP";
			m_connect_protocols = net::protocol::UDP;
			m_tcp.close();
			if (!TryConnectUDP())
				Close();
		}
		else
		{
			dstream2 << LogPrefix(this) << "connect timeout, closing connection";
			Close();
		}
	}

	if (m_tcp.is_open() || m_udp.is_open())
		TryStartWrite();
}

void VS_TransportConnection::Channel::ResetConnectionTimeout()
{
	assert(m_parent->m_strand.running_in_this_thread());
	m_connect_timeout = std::chrono::steady_clock::now() + c_connect_timeout;
}

void VS_TransportConnection::Channel::StartRead()
{
	assert(m_parent->m_strand.running_in_this_thread());

	if (!(m_tcp.is_open() || m_udp.is_open()))
		return;

	const auto buffer = boost::asio::buffer(m_read_buffer.get(), c_read_buffer_size);
#if GW_VERBOSE_LOGS
	dstream4 << LogPrefix(this) << "read start: size=" << boost::asio::buffer_size(buffer);
#endif
	auto handler = [this, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (ec)
		{
			dstream2 << LogPrefix(this) << "read failed: " << ec.message();
			Close();
			return;
		}

#if GW_VERBOSE_LOGS
		dstream4 << LogPrefix(this) << "read ok: size=" << bytes_transferred;
#endif
		if (m_tcp.is_open() && m_parent->m_logger)
			m_parent->m_logger->Log(m_read_buffer.get(), bytes_transferred, m_remote_address, m_remote_port, m_local_address, m_local_port, m_channel_log_info, net::protocol::TCP, false);
		else if (m_udp.is_open() && m_parent->m_logger)
			m_parent->m_logger->Log(m_read_buffer.get(), bytes_transferred, m_remote_address, m_remote_port, m_local_address, m_local_port, m_channel_log_info, net::protocol::UDP);
		m_connected = true; // This is for UDP, we treat UDP connections as established only if we received something.
		if (m_parent->m_parser->SetRecvBuf(m_read_buffer.get(), bytes_transferred, m_channel_id, m_remote_address, m_remote_port, m_local_address, m_local_port) <= 0)
		{
			dstream2 << LogPrefix(this) << "parser rejected the data";
			Close();
			return;
		}
		TryStartWrite();

		StartRead();
	};
	if (m_tcp.is_open())
		m_tcp.async_receive(buffer, m_parent->m_strand.wrap(std::move(handler)));
	else if (m_udp.is_open())
		m_udp.async_receive(buffer, m_parent->m_strand.wrap(std::move(handler)));
	else
		assert(false);
}

void VS_TransportConnection::Channel::TryStartWrite()
{
	assert(m_parent->m_strand.running_in_this_thread());

	if (!(m_tcp.is_open() || m_udp.is_open()))
		return;
	if (m_write_in_progress)
		return;

	size_t size = m_write_buffer_size;
	if (!m_parent->m_parser->GetBufForSend(m_write_buffer.get(), size, m_channel_id, m_remote_address, m_remote_port, m_local_address, m_local_port))
	{
		if (size > m_write_buffer_size)
		{
			constexpr size_t max_buffer_size = 0x10000;
			m_write_buffer_size = std::min(max_buffer_size, std::max(size, m_write_buffer_size / 2 * 3));
			m_write_buffer = vs::make_unique_default_init<unsigned char[]>(m_write_buffer_size);
			size = m_write_buffer_size;
			if (!m_parent->m_parser->GetBufForSend(m_write_buffer.get(), size, m_channel_id, m_remote_address, m_remote_port, m_local_address, m_local_port))
				return;
		}
		else
			return;
	}

	m_write_in_progress = true;
	const auto buffer = boost::asio::buffer(m_write_buffer.get(), size);
#if GW_VERBOSE_LOGS
	dstream4 << LogPrefix(this) << "write start: size=" << boost::asio::buffer_size(buffer);
#endif
	auto handler = [this, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (ec)
		{
			dstream2 << LogPrefix(this) << "write failed: " << ec.message();
			Close();
			return;
		}

#if GW_VERBOSE_LOGS
		dstream4 << LogPrefix(this) << "write ok: size=" << bytes_transferred;
#endif
		if (m_tcp.is_open() && m_parent->m_logger)
			m_parent->m_logger->Log(m_write_buffer.get(), bytes_transferred, m_local_address, m_local_port, m_remote_address, m_remote_port, m_channel_log_info, net::protocol::TCP, true);
		else if (m_udp.is_open() && m_parent->m_logger)
			m_parent->m_logger->Log(m_write_buffer.get(), bytes_transferred, m_local_address, m_local_port, m_remote_address, m_remote_port, m_channel_log_info, net::protocol::UDP);
		m_write_in_progress = false;
		TryStartWrite();
	};
	if (m_tcp.is_open())
		m_tcp.async_send(buffer, m_parent->m_strand.wrap(std::move(handler)));
	else if (m_udp.is_open())
		m_udp.async_send(buffer, m_parent->m_strand.wrap(std::move(handler)));	
	else
		assert(false);
}

struct VS_TransportConnection::TranscoderInfo
{
	boost::shared_ptr<VS_ClientControlInterface> transcoder;

	boost::signals2::scoped_connection onZombieConn;
	boost::signals2::scoped_connection HangupFromVisiConn;
	boost::signals2::scoped_connection FastUpdatePicConn;
	boost::signals2::scoped_connection LoggedOutAsUserConn;
	boost::signals2::scoped_connection SetMediaChannelsConn;
	boost::signals2::scoped_connection InviteReplayConn;
	boost::signals2::scoped_connection InviteConn;
	boost::signals2::scoped_connection ChatConn;
	boost::signals2::scoped_connection CommandConn;

	// Signalize, that this is shared transcoder. That means, it's real owner is
	// one of registration connection, so this connection should not logout transcoder.
	// Used for H225 RAS.
	bool is_my_transcoder = true;

	~TranscoderInfo()
	{
		if (transcoder)
		{
			transcoder->ReleaseCallbacks();
			if (auto disp = VS_ClientControlAllocatorInterface::Instance())
				disp->ReleaseTranscoder(transcoder);
		}
	}
};

VS_TransportConnection::VS_TransportConnection(boost::asio::io_service::strand& strand, std::shared_ptr<VS_ParserInterface> parser,
	const UserStatusFunction& get_status, const std::weak_ptr<ts::IPool>& transc_pool, const std::shared_ptr<net::LoggerInterface> &logger)
	: m_strand(strand)
	, m_timer(m_strand.get_io_service())
	, m_parser(std::move(parser))
	, m_get_user_status(get_status)
	, m_transc_pool(transc_pool)
	, m_is_closed(false)
	, m_n_requested_connections(0)
	, m_connection_protocol(net::protocol::none)
	, m_is_ipv4(true)
	, m_for_registred_user(false)
	, m_logger(logger)
{
	assert(m_parser);
}

void VS_TransportConnection::Init()
{
	ScheduleTimer();
	m_DialogFinishedConn = m_parser->Connect_DialogFinished([this, self_weak = weak_from_this()](string_view dialog_id)
	{
		auto self = self_weak.lock();
		if (!self)
			return;

		m_strand.dispatch([this, self_weak, dialog_id = std::string(dialog_id)]()
		{
			auto self = self_weak.lock();
			if (!self)
				return;

			dstream3 << LogPrefix(this) << "OnDialogFinished: dialog_id=" << dialog_id;

			std::shared_ptr<TranscoderInfo> info;
			bool transcoders_empty;
			m_transcoders.withLock([&](transcoders_map_t& transcoders) {
				auto it = transcoders.find(dialog_id);
				if (it == transcoders.end())
					return;
				info = std::move(it->second);
				transcoders.erase(it);
				transcoders_empty = transcoders.empty();
			});

			if (!info)
				return;
			info->transcoder->ClearDialogId(dialog_id);
			if (transcoders_empty && !m_parser->NeedPermanentConnection())
				m_inactivity_start_time = std::chrono::steady_clock::now();

			if (info.use_count() > 1)
			{
				auto ds = dstream1;
				ds << LogPrefix(this) << "Transcoder remains alive"
					<< ": use_count=" << info.use_count()
					<< ", dialog_id=" << dialog_id
					<< ", transcoder_id=" << info->transcoder->GetTranscoderID()
					<< ", transcoder_dialog_id=" << info->transcoder->GetDialogID()
					<< ", used by: ";
				m_transcoders.withLock([&](transcoders_map_t& transcoders) {
					bool first = true;
					for (const auto& x : transcoders)
						if (x.second == info)
						{
							if (!first)
							{
								first = false;
								ds << ", ";
							}
							ds << '"' << x.first << '"';
						}
				});
			}
		});
	});

	m_parser->SetConfCallBack(shared_from_this());
	m_parser->SetUserToDialogIdCallback([this, w_this = weak_from_this()](string_view login, string_view dialog_id) {
		if(auto self = w_this.lock())
			self->SetUserDialogID(login, dialog_id);
	});
}

VS_TransportConnection::~VS_TransportConnection()
{
	assert(std::all_of(m_channels.begin(), m_channels.end(), [](Channel* x) { return x == nullptr; }));
}

void VS_TransportConnection::Close()
{
	m_strand.dispatch([this, self_weak = weak_from_this()]() {
		auto self = self_weak.lock();
		if (!self)
			return;

		m_parser->Shutdown();

		for (const auto& channel : m_channels)
			if (channel)
				channel->Close();
		assert(std::all_of(m_channels.begin(), m_channels.end(), [](Channel* x) { return x == nullptr; }));
	});
}

void VS_TransportConnection::Accept(boost::asio::ip::tcp::socket&& socket, const void* data, size_t size)
{
	assert(m_strand.running_in_this_thread());
	assert(m_channels.empty());

	auto channel = std::make_shared<Channel>(shared_from_this());
	m_channels.emplace_back(channel.get());
	channel->Accept(std::move(socket), data, size);
}

void VS_TransportConnection::Accept(net::UDPConnection&& connection, const void* data, size_t size)
{
	assert(m_strand.running_in_this_thread());
	assert(m_channels.empty());

	auto channel = std::make_shared<Channel>(shared_from_this());
	m_channels.emplace_back(channel.get());
	channel->Accept(std::move(connection), data, size);
}

bool VS_TransportConnection::IsClosed() const
{
	return m_is_closed.load();
}

bool VS_TransportConnection::IsTrunkFull() const
{
	bool result;
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { done.set(); };
		result = m_parser->IsTrunkFull();
	});
	done.wait();
	return result;
}


void VS_TransportConnection::ResetInactivity()
{
	assert(m_strand.running_in_this_thread());
	m_inactivity_start_time = decltype(m_inactivity_start_time)();
}

void VS_TransportConnection::ResetConnectionTimeout()
{
	assert(m_strand.running_in_this_thread());
	if (auto cs_channel = GetCSChannel())
		cs_channel->ResetConnectionTimeout();
}

void VS_TransportConnection::OnChannelOpen(Channel* channel)
{
	assert(m_strand.running_in_this_thread());

	if (!m_channels.empty() && m_channels[0] == channel)
	{
		m_connection_protocol = channel->ConnectionProtocol();
		m_is_ipv4 = channel->LocalAddress().is_v4();
	}

	if (!m_DelayedInvite.to.empty())
	{
		InviteFromVisi(std::move(m_DelayedInvite.dialog_id), std::move(m_DelayedInvite.from), std::move(m_DelayedInvite.to), m_DelayedInvite.is_group_conf, m_DelayedInvite.use_new_dialog_id, std::move(m_DelayedInvite.dn_from_utf8));
		m_DelayedInvite.to.clear();
	}
}

void VS_TransportConnection::OnChannelClose(Channel* channel)
{
	assert(m_strand.running_in_this_thread());

	auto ch_it = std::find(m_channels.begin(), m_channels.end(), channel);
	if (ch_it == m_channels.end())
		return; // Channel has already closed before.

	// By default, we logout and free all transcoders after any connection crash.
	bool need_to_free_transcoders = true;
	bool need_to_hangup = true;

	// Keep transcoders if there are any new connections.
	if (m_n_requested_connections.load() > 0)
	{
		need_to_free_transcoders = false;
		need_to_hangup = false;
	}

	// Keep transcoders this is registration connection and we should delete another connection, so m_channels[0] will exists.
	if (m_for_registred_user && ch_it != m_channels.begin())
	{
		need_to_free_transcoders = false;
	}

	std::vector<boost::shared_ptr<VS_ClientControlInterface>> to_hangup;
	std::vector<boost::shared_ptr<VS_ClientControlInterface>> to_logout;
	m_transcoders.withLock([&](transcoders_map_t& transcoders) {
		to_hangup.reserve(transcoders.size());
		to_logout.reserve(transcoders.size());
		if (need_to_hangup)
		{
			for (const auto& kv : transcoders)
			{
				to_hangup.emplace_back(kv.second->transcoder);
				// We should logout only own transcoders, if this is shared transcoder,
				// then it should be logouted by another transport connection.
				if (need_to_free_transcoders && kv.second->is_my_transcoder)
					to_logout.emplace_back(kv.second->transcoder);
			}
		}
		if (need_to_free_transcoders)
		{
			transcoders.clear();
			m_inactivity_start_time = std::chrono::steady_clock::now();
		}
	});
	for (const auto& x : to_hangup)
		x->Hangup();
	for (const auto& x : to_logout)
		x->LogoutUser({});

	if (ch_it == m_channels.begin())
		m_is_closed.store(true);

	*ch_it = nullptr;
}

VS_TransportConnection::Channel* VS_TransportConnection::GetCSChannel() const
{
	assert(m_strand.running_in_this_thread());
	return !m_channels.empty() ? m_channels[0] : nullptr;
}

bool VS_TransportConnection::InviteMethod(string_view dialog_id, string_view from_id, string_view to_id, const VS_ConferenceInfo& info, string_view dn_from_utf8, bool create_session, bool force_create)
{
	// NOTE: Not using strand because class members are not accessed.

	dstream3 << LogPrefix(this) << "InviteMethod"
		": dialog_id=" << dialog_id <<
		", from_id=" << from_id <<
		", to_id=" << to_id <<
		", isGroupConf=" << std::boolalpha << info.is_group_conf <<
		", dn_from_utf8=" << dn_from_utf8;

	if (!to_id.empty())
	{
		auto status = m_get_user_status(to_id, true, VS_IsRTPCallID(to_id));

		auto user_p = boost::get<UserStatusInfo::User>(&status.info);
		if (user_p)
		{
			if (VS_IsRTPCallID(status.real_id))
				user_p->status = USER_AVAIL;
			if (user_p->status == USER_STATUS_UNDEF || user_p->status == USER_INVALID || user_p->status == USER_LOGOFF || user_p->status == USER_BUSY)
				return false;
		}
		else
		{
			auto conf_p = boost::get<UserStatusInfo::Conf>(&status.info);
			if (!conf_p || !conf_p->conf_exist)
				return false;
		}
	}

	auto transcoder = NewTranscoder(dialog_id);
	if (!transcoder)
	{
		dstream3 << LogPrefix(this) << "InviteMethod: NewTranscoder(" << dialog_id << ") failed";
		return false;
	}

	if (!dn_from_utf8.empty())
		transcoder->UpdateDisplayName(dn_from_utf8, false);

	if (transcoder->InviteMethod(from_id, to_id, info, m_is_ipv4) != VS_CallInviteStatus::SUCCESS)
	{
		dstream3 << LogPrefix(this) << "InviteMethod: transcoder->InviteMethod() failed";
		return false;
	}
	return true;
}

bool VS_TransportConnection::InviteReplay(string_view dialog_id, VS_CallConfirmCode confirm_code, bool /*isGroupConf*/, string_view /*conf_name*/, string_view /*to_display_name*/)
{
	auto transcoder = GetTranscoder(dialog_id);
	return transcoder ? transcoder->InviteReply(confirm_code) : false;
}

void VS_TransportConnection::Hangup(string_view dialog_id)
{
	auto transcoder = GetTranscoder(dialog_id);
	if (transcoder)
		transcoder->Hangup();

	m_strand.dispatch([self_weak = weak_from_this()]() {
		if (auto self = self_weak.lock())
			if (auto channel = self->GetCSChannel())
				channel->TryStartWrite();
	});
}

void VS_TransportConnection::Logout(string_view dialog_id)
{
	auto transcoder = GetTranscoder(dialog_id);
	if (transcoder)
		transcoder->LogoutUser({});
}

void VS_TransportConnection::UpdateDisplayName(string_view dialog_id, string_view display_name, bool update_immediately)
{
	auto transcoder = GetTranscoder(dialog_id);
	if (transcoder)
		transcoder->UpdateDisplayName(display_name, update_immediately);
}

void VS_TransportConnection::FastUpdatePicture(string_view dialog_id)
{
	auto transcoder = GetTranscoder(dialog_id);
	if (transcoder)
		transcoder->FastUpdatePictureFromSIP();
}

void VS_TransportConnection::BitrateRestriction(string_view dialog_id, int type, int value, int scope)
{
	auto transcoder = GetTranscoder(dialog_id);
	if (transcoder)
		transcoder->BitrateRestrictionFromH323(type, value, scope);
}

bool VS_TransportConnection::SetMediaChannels(string_view dialog_id, const std::vector<VS_MediaChannelInfo>& channels, const std::string& /*existingConfID*/, int32_t /*bandw_rcv*/)
{
	auto transcoder = GetTranscoder(dialog_id);
	if (!transcoder)
		return false;

	std::string proxyReservationToken;
	if (!transcoder->RtpControlIsReady())
	{
		if (!gw::InitWithRtpControl(m_transc_pool, transcoder, transcoder->GetStreamConfID(), proxyReservationToken))
			return false;
		if (!proxyReservationToken.empty())
			transcoder->SetProxyReservation(proxyReservationToken);
	}

	return transcoder->SetMediaChannels(channels);
}

void VS_TransportConnection::LoggedOutAsUser(string_view dialog_id)
{
	m_strand.dispatch([this, self_weak = weak_from_this(), dialog_id = std::string(dialog_id)]()
	{
		auto self = self_weak.lock();
		if (!self)
			return;

		m_parser->LoggedOutAsUser(dialog_id);
		if (auto channel = GetCSChannel())
			channel->TryStartWrite();
	});
}

void VS_TransportConnection::InviteFromVisi(std::string&& dialog_id, std::string&& from, std::string&& to, bool is_group_conf, bool use_new_dialog_id, std::string&& dn_from_utf8)
{
	assert(m_strand.running_in_this_thread());

	auto cs_channel = GetCSChannel();
	if (!cs_channel || !cs_channel->IsConnected())
	{
		m_DelayedInvite.dialog_id = std::move(dialog_id);
		m_DelayedInvite.from = std::move(from);
		m_DelayedInvite.to = std::move(to);
		m_DelayedInvite.is_group_conf = is_group_conf;
		m_DelayedInvite.use_new_dialog_id = use_new_dialog_id;
		m_DelayedInvite.dn_from_utf8 = std::move(dn_from_utf8);
		return;
	}

	std::string new_dialog_id;
	if (use_new_dialog_id)
	{
		new_dialog_id = m_parser->NewDialogID(to, {}, {}, {});
		SetUserDialogID(dialog_id, new_dialog_id);
		dialog_id = std::move(new_dialog_id);
	}

	/**
		TODO: for determinate external or internal ip (for choosing rule for filling sdp when we are working through NAT).
		It will be more properly use special protocols, but it is very fast fix.
	*/
	m_parser->SetPeerCSAddress(dialog_id, {cs_channel->RemoteAddress(), cs_channel->RemotePort(), cs_channel->ConnectionProtocol()});
	if (!m_parser->InviteMethod(dialog_id, from, to, VS_ConferenceInfo(is_group_conf, false), dn_from_utf8))
		InviteReplay(dialog_id, e_call_none, false, {}, {});

	cs_channel->TryStartWrite();
}

boost::shared_ptr<VS_ClientControlInterface> VS_TransportConnection::GetTranscoder(string_view dialog_id)
{
	return m_transcoders.withLock([&](transcoders_map_t& transcoders) {
		auto it = transcoders.find(dialog_id);
		return it != transcoders.end() ? it->second->transcoder : nullptr;
	});
}

boost::shared_ptr<VS_ClientControlInterface> VS_TransportConnection::FindLoggedinTranscoder(string_view to_call_id)
{
	return m_transcoders.withLock([&](transcoders_map_t& transcoders) {
		auto it = std::find_if(transcoders.begin(), transcoders.end(), [&](const transcoders_map_t::value_type& x) {
			return x.second->transcoder && x.second->transcoder->IsLoggedIn(to_call_id);
		});
		return it != transcoders.end() ? it->second->transcoder : nullptr;
	});
}

boost::shared_ptr<VS_ClientControlInterface> VS_TransportConnection::NewTranscoder(string_view dialog_id)
{
	boost::shared_ptr<VS_ClientControlInterface> result;
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { done.set(); };

		result = GetTranscoder(dialog_id);
		if (result)
			return;

		auto disp = VS_ClientControlAllocatorInterface::Instance();
		if (!disp)
			return;
		do
		{
			result = disp->GetTranscoder();
			if (!result)
				return;
			if (!result->IsReady())
			{
				disp->ReleaseTranscoder(result);
				result = nullptr;
			}
		} while (!result); /// попытаться получить транскодер несколько раз
		PutTranscoder(dialog_id, result, false);
	});
	done.wait();
	return result;
}

void VS_TransportConnection::PutTranscoder(string_view dialog_id, boost::shared_ptr<VS_ClientControlInterface> transcoder, bool shared)
{
	auto info = std::make_shared<TranscoderInfo>();

	info->onZombieConn = transcoder->ConnectOnZombie([this, self_weak = weak_from_this()](string_view name)
	{
		if (self_weak.expired())
			return;
		m_strand.dispatch([this, self_weak, name = std::string(name)]()
		{
			auto self = self_weak.lock();
			if (!self)
				return;

			dstream3 << LogPrefix(this) << "OnZombieTranscoder: transcoder=" << name;

			std::vector<std::string> to_hangup;
			m_transcoders.withLock([&](transcoders_map_t& transcoders) {
				to_hangup.reserve(transcoders.size());
				for (const auto& kv : transcoders)
					if (boost::iequals(kv.second->transcoder->GetTranscoderID(), name))
						to_hangup.emplace_back(kv.first);
			});
			for (const auto& x : to_hangup)
				m_parser->Hangup(x);
			if (auto channel = GetCSChannel())
				channel->TryStartWrite();
		});
	});

	info->HangupFromVisiConn = transcoder->ConnectHangupFromVisi([this, self_weak = weak_from_this()](string_view dialog_id, string_view /*method*/)
	{
		if (self_weak.expired())
			return;
		m_strand.dispatch([this, self_weak, dialog_id = std::string(dialog_id)]()
		{
			auto self = self_weak.lock();
			if (!self)
				return;

#if GW_VERBOSE_LOGS
			dstream4 << LogPrefix(this) << "HangupFromVisi: dialog_id=" << dialog_id;
#endif

			m_parser->Hangup(dialog_id);
			if (auto channel = GetCSChannel())
				channel->TryStartWrite();
		});
	});

	info->FastUpdatePicConn = transcoder->ConnectFastUpdatePicture([this, self_weak = weak_from_this()](string_view dialog_id)
	{
		if (self_weak.expired())
			return;
		m_strand.dispatch([this, self_weak, dialog_id = std::string(dialog_id)]()
		{
			auto self = self_weak.lock();
			if (!self)
				return;

			m_parser->FastUpdatePicture(dialog_id);
			if (auto channel = GetCSChannel())
				channel->TryStartWrite();
		});
	});

	info->LoggedOutAsUserConn = transcoder->ConnectLoggedOutAsUser([self_weak = weak_from_this()](string_view dialog_id)
	{
		if (auto self = self_weak.lock())
			self->LoggedOutAsUser(dialog_id);
	});

	info->SetMediaChannelsConn = transcoder->ConnectSetMediaChannels([this, self_weak = weak_from_this()](string_view dialog_id, const std::vector<VS_MediaChannelInfo>& channels, int32_t bandw_rcv)
	{
		if (self_weak.expired())
			return;
		m_strand.dispatch([this, self_weak, dialog_id = std::string(dialog_id), channels, bandw_rcv]()
		{
			auto self = self_weak.lock();
			if (!self)
				return;

			m_parser->SetMediaChannels(dialog_id, channels, {}, bandw_rcv);
			if (auto channel = GetCSChannel())
				channel->TryStartWrite();
		});
	});

	info->InviteReplayConn = transcoder->ConnectInviteReply([this, self_weak = weak_from_this()](string_view dialog_id, VS_CallConfirmCode confirm_code, bool is_group_conf, string_view /*conf_name*/, string_view to_display_name)
	{
		if (self_weak.expired())
			return;
		m_strand.dispatch([this, self_weak, dialog_id = std::string(dialog_id), confirm_code, is_group_conf, to_display_name = std::string(to_display_name)]()
		{
			auto self = self_weak.lock();
			if (!self)
				return;

			m_parser->InviteReplay(dialog_id, confirm_code, is_group_conf, {}, to_display_name);
			if (auto channel = GetCSChannel())
				channel->TryStartWrite();
		});
	});

	info->InviteConn = transcoder->ConnectInvite([this, self_weak = weak_from_this()](string_view dialog_id, string_view from, string_view to, bool is_group_conf, bool /*is_public_conf*/, bool use_new_dialog_id, string_view dn_from_utf8)
	{
		if (self_weak.expired())
			return;
		m_strand.dispatch([self_weak, dialog_id = std::string(dialog_id), from = std::string(from), to = std::string(to), is_group_conf, use_new_dialog_id, dn_from_utf8 = std::string(dn_from_utf8)]() mutable
		{
			if (auto self = self_weak.lock())
				self->InviteFromVisi(std::move(dialog_id), std::move(from), std::move(to), is_group_conf, use_new_dialog_id, std::move(dn_from_utf8));
		});
	});

	info->ChatConn = transcoder->ConnectChat([this, self_weak = weak_from_this()](string_view dialog_id, string_view from, string_view to, string_view dn, const char* mess)
	{
		if (self_weak.expired())
			return;
		m_strand.dispatch([this, self_weak, dialog_id = std::string(dialog_id), from = std::string(from), to = std::string(to), dn = std::string(dn), mess = std::string(mess)]()
		{
			auto self = self_weak.lock();
			if (!self)
				return;

			m_parser->Chat(dialog_id, from, to, dn, mess.c_str());
			if (auto channel = GetCSChannel())
				channel->TryStartWrite();
		});
	});

	info->CommandConn = transcoder->ConnectCommand([this, self_weak = weak_from_this()](string_view dialog_id, string_view from, string_view command)
	{
		if (self_weak.expired())
			return;
		m_strand.dispatch([this, self_weak, dialog_id = std::string(dialog_id), from = std::string(from), command = std::string(command)]()
		{
			auto self = self_weak.lock();
			if (!self)
				return;

			m_parser->Command(dialog_id, from, command);
			if (auto channel = GetCSChannel())
				channel->TryStartWrite();
		});
	});

	if (!shared)
	{
		transcoder->SetConnectMeToTransceiver([this, self_weak = weak_from_this()](const std::string& dialog_id, const std::string& conf_id)
		{
			auto self = self_weak.lock();
			if (!self)
				return false;

			if (conf_id.empty())
				return false;
			auto transcoder = GetTranscoder(dialog_id);
			if (!transcoder)
				return false;

			return gw::InitWithRtpControl(m_transc_pool, transcoder, conf_id, vs::ignore<std::string>());
		});
	}

	info->is_my_transcoder = !shared;

	transcoder->SetDialogId(dialog_id);
	info->transcoder = std::move(transcoder);
	m_transcoders->insert_or_assign(std::string(dialog_id), std::move(info));
	m_inactivity_start_time = decltype(m_inactivity_start_time)();
}

void VS_TransportConnection::ScheduleTimer()
{
	m_timer.expires_from_now(std::chrono::milliseconds(500));
	m_timer.async_wait(m_strand.wrap([self_weak = weak_from_this()](const boost::system::error_code& ec) {
		if (ec == boost::asio::error::operation_aborted)
			return;
		auto self = self_weak.lock();
		if (!self)
			return;

		self->Timeout();
		self->ScheduleTimer();
	}));
}

void VS_TransportConnection::Timeout()
{
	assert(m_strand.running_in_this_thread());

	m_parser->Timeout();

	for (const auto& channel : m_channels)
		if (channel)
			channel->Timeout();

	if (m_inactivity_start_time == decltype(m_inactivity_start_time)())
	{
		if (!m_channels.empty() && m_transcoders->empty() && !m_parser->NeedPermanentConnection())
			m_inactivity_start_time = std::chrono::steady_clock::now();
	}
	else if (m_inactivity_start_time + std::chrono::seconds(30) > std::chrono::steady_clock::now())
		Close();
}

std::string VS_TransportConnection::PrepareCallToSIP(string_view to_sip_id, const VS_UserData* from_user, const VS_CallConfig& config)
{
	assert(m_strand.running_in_this_thread());

	dstream3 << LogPrefix(this) << "PrepareCallToSIP: to_sip_id=" << to_sip_id << ", resolved_sip_id=" << config.resolveResult.NewCallId;

	auto transcoder = FindLoggedinTranscoder(config.resolveResult.NewCallId);
	if (!transcoder)
	{
		auto dialog_id = m_parser->NewDialogID(config.resolveResult.NewCallId, config.resolveResult.dtmf, config, from_user->m_name.m_str);
		if (dialog_id.empty())
			return {};
		if (GetTranscoder(dialog_id))
		{
			dstream4 << LogPrefix(this) << "PrepareCallToSIP: failed: transcoder for dialog_id=" << dialog_id << " already exists";
			return {};
		}
		transcoder = NewTranscoder(dialog_id);
		if (!transcoder)
			return {};
	}

	std::string login;
	if (transcoder->PrepareForCall(config.resolveResult.NewCallId, to_sip_id, login, m_is_ipv4) != VS_CallInviteStatus::SUCCESS)
		return {};
	return login;
}

bool VS_TransportConnection::PrepareTranscportConnection(const net::address& address, net::port port, net::protocol protocols, VS_ChannelID channel_id)
{
	bool result = false;
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { done.set(); };

		m_parser->SetPeerCSAddress("", {address, port, protocols});
		if (channel_id == e_noChannelID)
			channel_id = m_parser->GetDefaultChannelID();

		auto channel = std::make_shared<Channel>(shared_from_this());
		m_channels.emplace_back(channel.get());
		channel->Connect(address, port, protocols, channel_id);
		result = true;
	});
	done.wait();
	return result;
}

void VS_TransportConnection::RegisterNewConnection(const net::address& address, net::port port, net::protocol protocols, const VS_ChannelID channel_id)
{
	++m_n_requested_connections;
	m_strand.dispatch([this, self = shared_from_this(), address, port, protocols, channel_id]() {
		--m_n_requested_connections;
		auto channel = std::make_shared<Channel>(shared_from_this());
		m_channels.emplace_back(channel.get());
		channel->Connect(address, port, protocols, channel_id);
	});
}

void VS_TransportConnection::SetUserDialogID(string_view login, string_view dialog_id)
{
	boost::shared_ptr<VS_ClientControlInterface> transcoder;
	m_transcoders.withLock([&](transcoders_map_t& transcoders) {
		auto it = transcoders.find(login);
		if (it == transcoders.end())
			return;
		transcoder = it->second->transcoder;
		transcoders[std::string(dialog_id)] = it->second;
	});
	if (transcoder)
		transcoder->SetDialogId(dialog_id);
}

void VS_TransportConnection::LoginUser(string_view dialog_id, string_view login, string_view password, std::chrono::steady_clock::time_point expire_time, string_view external_name, std::function<void (bool)> result_callback, std::function<void (void)> logout_cb, const std::vector<std::string>& aliases)
{
	// NOTE: Not using strand because class members are not accessed.

	auto transcoder = FindLoggedinTranscoder(login);

	if (expire_time <= std::chrono::steady_clock::now())
	{
		if (transcoder)
			transcoder->LogoutUser(std::move(logout_cb));
		else
			m_strand.dispatch([result_callback = std::move(result_callback)]() { result_callback(true); });
		return;
	}

	if (transcoder)
	{
		m_strand.dispatch([result_callback = std::move(result_callback)]() { result_callback(true); }); // It is regester timeout update
		return;
	}

	transcoder = NewTranscoder(dialog_id);
	if (!transcoder)
	{
		m_strand.dispatch([result_callback = std::move(result_callback)]() { result_callback(true); }); // There is no more free trancoders left.
		return;
	}

	// if caller connects to us in ipv4 protocol - then we open ipv4
	// multimedia connections, otherwise - we open ipv6 connections.
	transcoder->LoginUser(login, password, expire_time, external_name, std::move(result_callback), std::move(logout_cb), m_is_ipv4, aliases);
}

void VS_TransportConnection::SetUserEndpointAppInfo(string_view dialog_id, string_view app_name, string_view version)
{
	auto transcoder = GetTranscoder(dialog_id);
	if (transcoder)
		transcoder->SetUserEndpointAppInfo(app_name, version);
}

void VS_TransportConnection::SetRegistrationConfiguration(VS_CallConfig config)
{
	auto exec = [this](VS_CallConfig& config)
	{
		m_strand.post([self_weak = weak_from_this(), config = std::move(config)]() mutable {
			if (auto self = self_weak.lock())
				self->SetRegistrationConfiguration(std::move(config));
		});
	};
	if (!m_strand.running_in_this_thread())
	{
		exec(config);
		return;
	}
	m_parser->SetRegistrationConfiguration(std::move(config), std::move(exec));
	Timeout();
}

void VS_TransportConnection::UpdateStatusRegistration(std::function<void(const std::shared_ptr<VS_ParserInterface>&)>&& exec)
{
	if (m_strand.running_in_this_thread())
	{
		exec(m_parser);
		return;
	}
	m_strand.dispatch([exec = std::move(exec), parser = m_parser]() { exec(parser); });
}

void VS_TransportConnection::ResetAllConfigsStatus()
{
	m_strand.dispatch([parser = m_parser]() { parser->ResetAllConfigsStatus(); });
}

void VS_TransportConnection::AsyncInvite(string_view dialog_id, const gw::Participant & from, string_view to_id, const VS_ConferenceInfo& ci, std::function<void(bool redirect, ConferenceStatus status, const std::string& ip)> InviteResult, string_view dn_from_utf8, bool new_session, bool force_create)
{
	// NOTE: Not using strand because class members are not accessed.

	if (boost::iequals(to_id, string_view(DEFAULT_DESTINATION_CALLID))
	 || boost::iequals(to_id, string_view(DEFAULT_DESTINATION_CALLID_SIP))
	 || boost::iequals(to_id, string_view(DEFAULT_DESTINATION_CALLID_H323)))
	{
		// Process default destination call (like #sip:@server)
		InviteResult(false, InviteMethod(dialog_id, from.callID, to_id, ci, dn_from_utf8) ? ConferenceStatus::AVAILABLE : ConferenceStatus::UNDEFINED, {});
		return;
	}

	// Don't perform NetworkResolve if ip is specified
	auto at_pos = to_id.find('@');
	assert(at_pos != to_id.npos); // Old code effectively assumed this by passing a potentially null pointer to strncpy.
	auto server_name = to_id.substr(at_pos + 1);

	if (server_name.size() >= 4) // the shortest servername is "[::]"
	{
		if (server_name.front() == '[' && server_name.back() == ']')
		{
			server_name.remove_prefix(1);
			server_name.remove_suffix(1);
		}

		boost::system::error_code ec;
		auto ip = net::address::from_string(std::string(server_name), ec);
		if (!ec && !ip.is_unspecified())
		{
			InviteResult(false, InviteMethod(dialog_id, from.callID, to_id, ci, dn_from_utf8) ? ConferenceStatus::AVAILABLE : ConferenceStatus::UNDEFINED, {});
			return;
		}
	}

	if (s_concurrent_async_invite_threads >= 100)
	{
		InviteResult(false, ConferenceStatus::UNDEFINED, {});
		return;
	}

	std::function<void (UserStatusInfo&)> continuation([this, self_weak = weak_from_this(), InviteResult = std::move(InviteResult), dialog_id = std::string(dialog_id), from_id = from.callID, to_id = std::string(to_id), dn_from_utf8 = std::string(dn_from_utf8)](UserStatusInfo& status)
	{
		auto self = self_weak.lock();
		if (!self)
			return;

		auto allow_call = [&]()
		{
			auto transcoder = GetTranscoder(dialog_id);
			if (!transcoder)
			{
				InviteResult(false, ConferenceStatus::UNDEFINED, {});
				return;
			}

			if (!dn_from_utf8.empty())
				transcoder->UpdateDisplayName(dn_from_utf8, false);

			// if caller connects to us in ipv4 protocol - then we open ipv4
			// multimedia connections, otherwise - we open ipv6 connections.
			InviteResult(false, to_conference_status(transcoder->InviteMethod(from_id, to_id, VS_ConferenceInfo(false, false), m_is_ipv4)), {});
		};

		auto user_p = boost::get<UserStatusInfo::User>(&status.info);

		if (!user_p || user_p->status != USER_AVAIL)
		{
			// let it call anyway
			allow_call();
			return;
		}

		int32_t use_sip_redirect = 0;
		VS_RegistryKey(false, CONFIGURATION_KEY).GetValue(&use_sip_redirect, sizeof(use_sip_redirect), VS_REG_INTEGER_VT, "Use SIP Redirect");

		if (!use_sip_redirect || user_p->server == g_tr_endpoint_name)
		{
			allow_call();
			return;
		}

		auto cut_from = user_p->server.find_last_of('#');
		if (std::string::npos != cut_from)
			user_p->server.erase(cut_from);
		InviteResult(true, ConferenceStatus::UNDEFINED, user_p->server);
	});

	++s_concurrent_async_invite_threads;
	std::thread([self_weak = weak_from_this(), to_id = std::string(to_id), continuation = std::move(continuation)]() mutable
	{
		vs::SetThreadName("AsyncInviteH323");
		VS_SCOPE_EXIT { --s_concurrent_async_invite_threads; };
		auto self = self_weak.lock();
		if (!self)
			return;

		self->m_strand.post([continuation = std::move(continuation), status = self->m_get_user_status(to_id, false, true)]() mutable { continuation(status); });
	}).detach(); // Artem Boldarev (09.02.17): I believe that using thread pool would be a better solution.
}

void VS_TransportConnection::HangupOutcomingCall(string_view dialog_id)
{
	auto transcoder = GetTranscoder(dialog_id);
	if (transcoder)
		transcoder->HangupOutcomingCall();
}

void VS_TransportConnection::SetForRegisteredUser()
{
	m_strand.dispatch([self_weak = weak_from_this()]()
	{
		if (auto self = self_weak.lock())
			self->m_for_registred_user = true;
	});
}

void VS_TransportConnection::PutSharedTranscoder(string_view dialog_id, boost::shared_ptr<VS_ClientControlInterface> transcoder)
{
	if (dialog_id.empty())
		return;
	if (!transcoder)
		return;
	PutTranscoder(dialog_id, std::move(transcoder), true);
}

bool VS_TransportConnection::IsTCPKeepAliveAllowed()
{
	int32_t value = 0;
	VS_RegistryKey(false, CONFIGURATION_KEY).GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "TCP KeepAlive");
	return value != 0;
}

void VS_TransportConnection::CloseConnection(const net::address& address, net::port port, net::protocol protocol)
{
	m_strand.dispatch([this, self_weak = weak_from_this(), address, port, protocol]()
	{
		auto self = self_weak.lock();
		if (!self)
			return;

		for (const auto& channel : m_channels)
			if (channel && channel->RemoteAddress() == address && channel->RemotePort() == port && channel->ConnectionProtocol() == protocol)
				channel->Close();
	});
}
