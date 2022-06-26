#include "TrueGateway/VS_SignalConnectionsMgr.h"
#include "TrueGateway/CallConfig/VS_Indentifier.h"
#include "TrueGateway/interfaces/VS_ParserInterface.h"
#include "TrueGateway/VS_GatewayStarter.h"
#include "TrueGateway/VS_TransportConnection.h"
#include "acs_v2/Handler.h"
#include "acs_v2/Service.h"
#include "net/InterfaceInfo.h"
#include "net/UDPRouter.h"
#include "statuslib/VS_CallIDInfo.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/ignore.h"
#include "std/cpplib/MakeShared.h"
#include "std/cpplib/VS_Policy.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_UserData.h"
#include "std/debuglog/VS_Debug.h"

#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

static const auto MAX_COUNT_CONNS_BEFORE_CRASH = 1275;

class VS_SignalConnectionHandler : public acs::Handler
{
public:
	explicit VS_SignalConnectionHandler(VS_SignalConnectionsMgr* mgr)
		: m_mgr(mgr)
	{
	}

	acs::Response Protocol(const stream_buffer& buffer, unsigned channel_token) override;
	acs::Response Protocol(const packet_buffer& buffer, unsigned channel_token) override;
	void Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer) override;
	void Accept(net::UDPConnection&& connection, packet_buffer&& buffer) override;

private:
	VS_SignalConnectionsMgr* m_mgr;
};

acs::Response VS_SignalConnectionHandler::Protocol(const stream_buffer& buffer, unsigned /*channel_token*/)
{
	return m_mgr->Protocol(buffer.data(), buffer.size());
}

acs::Response VS_SignalConnectionHandler::Protocol(const packet_buffer& buffer, unsigned /*channel_token*/)
{
	if (buffer.Empty())
		return acs::Response::next_step;
	return m_mgr->Protocol(buffer.Front().Data(), buffer.Front().Size());
}

void VS_SignalConnectionHandler::Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer)
{
	m_mgr->Accept(std::move(socket), buffer.data(), buffer.size());
}

void VS_SignalConnectionHandler::Accept(net::UDPConnection&& connection, packet_buffer&& buffer)
{
	size_t size = 0;
	for (const auto& x : buffer)
		size += x.Size();
	auto data = vs::make_unique_default_init<unsigned char[]>(size);
	{
		auto p = data.get();
		for (const auto& x : buffer)
		{
			std::memcpy(p, x.Data(), x.Size());
			p += x.Size();
		}
		assert(p == data.get() + size);
	}
	m_mgr->Accept(std::move(connection), data.get(), size);
}

VS_SignalConnectionsMgr::VS_SignalConnectionsMgr(InitInfo&& info)
	: AbstractGatewayEventListener(std::move(info.peerConfig))
	, m_strand(info.strand)
	, m_timer(m_strand.get_io_service())
	, m_should_run(true)
	, m_stopped(true)
	, m_policy(boost::make_shared<VS_Policy>("SIP"))
	, m_checkDigest(std::move(info.checkDigest))
	, m_getUserStatus(std::move(info.getUserStatus))
	, m_ourEndpoint(info.ourEndpoint)
	, m_ourService(info.ourService)
	, m_postMes(std::move(info.postMes))
	, m_transcPool(info.transcPool)
	, m_logger(std::move(info.logger))
{
}

VS_SignalConnectionsMgr::~VS_SignalConnectionsMgr()
{
}

std::shared_ptr<VS_TransportConnection> VS_SignalConnectionsMgr::CreateTransportConnection(std::shared_ptr<VS_ParserInterface> parser)
{
	assert(m_strand.running_in_this_thread());

	std::shared_ptr<VS_TransportConnection> result;
	if (!m_should_run.load(std::memory_order_acquire))
		return nullptr;
	if (!parser)
		return nullptr;

	parser->SetPolicy(m_policy);
	parser->SetCallConfigStorage(m_peer_config);
	if (m_checkDigest)
		parser->SetDigestChecker(m_checkDigest);
	return vs::MakeShared<VS_TransportConnection>(m_strand, std::move(parser), m_getUserStatus, m_transcPool, m_logger);
}

acs::Response VS_SignalConnectionsMgr::Protocol(const void* data, size_t size)
{
	if (!m_should_run.load(std::memory_order_acquire))
		return acs::Response::not_my_connection;
	if (!data || size == 0)
		return acs::Response::next_step;
	return m_peer_config->GetCommonIdentifier()->Protocol(data, size);
}

void VS_SignalConnectionsMgr::Accept(boost::asio::ip::tcp::socket&& socket, const void* data, size_t size)
{
	Accept(std::move(socket), data, size, "TCP");
}

void VS_SignalConnectionsMgr::Accept(net::UDPConnection&& connection, const void* data, size_t size)
{
	Accept(std::move(connection), data, size, "UDP");
}

template <class Socket>
void VS_SignalConnectionsMgr::Accept(Socket&& socket, const void* data, size_t size, const char* proto_name)
{
	if (!m_should_run.load(std::memory_order_acquire))
		return;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		if (m_connections.size() > MAX_COUNT_CONNS_BEFORE_CRASH / 2)
		{
			dstream2 << "SignalConnectionsMgr: Rejecting " << proto_name << " connection from " << socket.remote_endpoint(vs::ignore<boost::system::error_code>()) << " due to limit";
			return;
		}

		boost::system::error_code ec;
		const auto ep = socket.remote_endpoint(ec);
		if (ec)
			return;

		auto conn = CreateTransportConnection(m_peer_config->GetCommonIdentifier()->CreateParser(m_strand, data, size, m_logger));
		if (!conn)
			return;

		conn->Accept(std::move(socket), data, size);

		m_connections.emplace(Address { ep.address(), ep.port() }, std::move(conn));
	});
	ready.wait();
}

void VS_SignalConnectionsMgr::ScheduleTimer()
{
	m_timer.expires_from_now(std::chrono::seconds(1));
	m_timer.async_wait(m_strand.wrap([this, self_weak = weak_from_this()](const boost::system::error_code& ec)
	{
		if (ec == boost::asio::error::operation_aborted)
			return;
		auto self = self_weak.lock();
		if (!self)
			return;
		VS_SCOPE_EXIT { ScheduleTimer(); };

		m_policy->Timeout();

		{
			for (auto it = m_connections.begin(); it != m_connections.end(); /**/)
			{
				if (it->second->IsClosed())
					it = m_connections.erase(it);
				else
					++it;
			}

			if (!m_should_run.load(std::memory_order_acquire) && m_connections.empty())
				m_stopped.set();
		}

		if (m_should_run.load(std::memory_order_acquire))
			ReinitializeConfiguration();
	}));
}

void VS_SignalConnectionsMgr::HandleEvent(const VS_GatewayService::ReloadConfiguration& reloadCfg)
{
	m_strand.post([this, self = shared_from_this(), reloadCfg]
	{
		AbstractGatewayEventListener::HandleEvent(reloadCfg);
	});
}


void VS_SignalConnectionsMgr::Close()
{
	if (!m_should_run.load(std::memory_order_acquire))
		return;

	m_strand.dispatch([this]() {
		dstream3 << "SignalConnectionsMgr: close";
		PushAllSubscribtionsInvalid();

		m_handler = nullptr;

		for (const auto& kv : m_connections)
			kv.second->Close();
		m_connections.clear();

		m_should_run.store(false, std::memory_order_release);
		m_stopped.set();
	});
	WaitForShutdown();
}

void VS_SignalConnectionsMgr::WaitForShutdown()
{
	const auto start_time = std::chrono::steady_clock::now();
	dstream3 << "SignalConnectionsMgr::WaitForShutdown";
	m_stopped.wait();
	const auto wait_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();
	dstream3 << "SignalConnectionsMgr::WaitForShutdown: finished, wait time: " << (wait_time_ms / 1000) << "s " << (wait_time_ms % 1000) << "ms";
}

bool VS_SignalConnectionsMgr::Start(acs::Service* acs)
{
	if (!VS_GatewayStarter::IsVCS())
		return false;

	AbstractGatewayEventListener::Start();

	m_handler = std::make_shared<VS_SignalConnectionHandler>(this);
	acs->AddHandler("H.323 Handler", m_handler);

	ScheduleTimer();

	return true;
}

std::string VS_SignalConnectionsMgr::ReserveOut(const std::string& to_sip_id, VS_UserData* from_user)
{
	std::string transcoder_id;

	if (!m_should_run.load(std::memory_order_acquire))
		return transcoder_id;

	VS_UserData fake;
	fake.m_name = "Administrator";
	if (!from_user || !from_user->m_name)
		from_user = &fake;

	VS_CallConfig config;
	if (!m_peer_config->Resolve(config, to_sip_id, from_user))
		return transcoder_id;

	if (config.resolveResult.NewCallId.empty())
		config.resolveResult.NewCallId = to_sip_id;

	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };

		auto ds = dstream3;
		ds << "SignalConnectionsMgr::ReserveOut: to_sip_id=" << to_sip_id;
		std::shared_ptr<VS_TransportConnection> conn;
		auto protocol = config.Address.protocol;
		if ((protocol & (net::protocol::TCP | net::protocol::UDP)) == net::protocol::none)
			protocol = net::protocol::TCP | net::protocol::UDP;

		for (auto range = m_connections.equal_range(Address{ config.Address.addr, config.Address.port });
			range.first != range.second; ++range.first)
		{
			if (static_cast<bool>(protocol & range.first->second->ConnectionProtocol()))
			{
				conn = range.first->second;
				conn->ResetConnectionTimeout();
				break;
			}
		}

		if (!conn)
		{
			for (const auto& kv : m_connections)
				if (kv.second->FindLoggedinTranscoder(to_sip_id) != nullptr)
				{
					conn = kv.second;
					break;
				}
		}

		if (!conn || conn->IsTrunkFull())
		{
			conn = CreateTransportConnection(m_peer_config->GetCommonIdentifier()->CreateParser(m_strand, to_sip_id, m_logger));
			if (!conn)
				return;
			if (!conn->PrepareTranscportConnection(config.Address.addr, config.Address.port, protocol))
				return;

			m_connections.emplace(Address{ config.Address.addr, config.Address.port }, conn);
		}
		transcoder_id = conn->PrepareCallToSIP(to_sip_id, from_user, config);
		ds << ", transcoder=" << transcoder_id;
	});
	ready.wait();
	return transcoder_id;
}

bool VS_SignalConnectionsMgr::Resolve(std::string& call_id, VS_CallIDInfo& ci, VS_UserData* from_user)
{
	if (!IsMyCallId(call_id.c_str()))
		return false;

	auto transcoder_id = ReserveOut(call_id, from_user);
	if (transcoder_id.empty())
	{
		ci.m_status = USER_INVALID;
		return false;
	}
	ci.m_status = USER_AVAIL;
	ci.m_serverID = m_ourEndpoint;
	// m_extStatus
	ci.m_realID = std::move(transcoder_id);
	//m_logginTime
	ci.m_homeServer = m_ourEndpoint;
	dstream3 << "SignalConnectionsMgr::Resolve: call_id=" << call_id << ", status=" << ci.m_status << ", real_id=" << ci.m_realID;
	return true;
}


void VS_SignalConnectionsMgr::Subscribe(const char *call_id)
{
	m_strand.post([this, self = shared_from_this(), call_id = std::string(call_id)]() mutable {
		dstream3 << "SignalConnectionsMgr::Subscribe: call_id=" << call_id;
		if (!m_should_run.load(std::memory_order_acquire))
			return;
		if (!IsMyCallId(call_id.c_str()))
			return;

		VS_Container status_cnt;
		status_cnt.AddValue(METHOD_PARAM, PUSHSTATUSDIRECTLY_METHOD);
		status_cnt.AddValue(CALLID_PARAM, call_id);
		status_cnt.AddValueI32(USERPRESSTATUS_PARAM, USER_AVAIL);
		status_cnt.AddValue(SERVER_PARAM, m_ourEndpoint);
		status_cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
		void* body;
		size_t bodySize;
		status_cnt.SerializeAlloc(body, bodySize);

		m_subscribes.insert(std::move(call_id));

		VS_RouterMessage* msg = new VS_RouterMessage(m_ourService, NULL, PRESENCE_SRV, 0, 0, m_ourEndpoint, m_ourEndpoint,
			VS_TransportRouterServiceHelper::default_timeout, body, bodySize);
		if (!m_postMes(msg))
			delete msg;
		free(body);
	});
}


bool VS_SignalConnectionsMgr::CanICall(VS_UserData* from_ude, const char* to_call_id, bool IsVCS)
{
	if (!from_ude || !to_call_id ||!*to_call_id)
		return false;
	if (IsVCS)
		return true;
	if (!(from_ude->m_rights&VS_UserData::UR_COMM_DIALER))
		return false;
	return true;
}


void VS_SignalConnectionsMgr::Unsubscribe(const char *call_id)
{
	m_strand.post([this, self = shared_from_this(), call_id = std::string(call_id)]() {
		dstream3 << "SignalConnectionsMgr::Unsubscribe: call_id=" << call_id;
		if (!IsMyCallId(call_id.c_str()))
			return;

		m_subscribes.erase(call_id);
	});
}


void VS_SignalConnectionsMgr::PushAllSubscribtionsInvalid()
{
	assert(m_strand.running_in_this_thread());
	for (const auto& call_id : m_subscribes)
	{
		VS_Container status_cnt;
		status_cnt.AddValue(METHOD_PARAM, PUSHSTATUSDIRECTLY_METHOD);
		status_cnt.AddValue(CALLID_PARAM, call_id);
		status_cnt.AddValueI32(USERPRESSTATUS_PARAM, USER_INVALID);
		status_cnt.AddValue(SERVER_PARAM, m_ourEndpoint);
		status_cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
		void* body;
		size_t bodySize;
		status_cnt.SerializeAlloc(body, bodySize);

		VS_RouterMessage* msg = new VS_RouterMessage(m_ourService, NULL, PRESENCE_SRV, 0, 0, m_ourEndpoint, m_ourEndpoint,
			VS_TransportRouterServiceHelper::default_timeout, body, bodySize);
		if (!m_postMes(msg))
			delete msg;
		free(body);
	}
	m_subscribes.clear();
}

bool VS_SignalConnectionsMgr::IsMyCallId(const char* call_id) const
{
	return m_peer_config->GetCommonIdentifier()->IsMyCallId(call_id);
}

void VS_SignalConnectionsMgr::NewPeerCfg(string_view call_id, const std::vector<VS_ExternalAccount>& v)
{
	if (call_id.empty())
		return;

	m_strand.post([this, self = shared_from_this(), call_id = std::string(call_id), v]() {
		dstream3 << "SignalConnectionsMgr::NewPeerCfg: call_id=" << call_id << ", num=" << v.size();
		m_peer_config->RemoveUser(call_id);

		if (!v.empty())
			m_peer_config->AddUser(call_id, v);
		auto &&locked_configs = m_peer_config->GetRegistrationSettings().lock();
		for (auto &registration_config : *locked_configs)
		{
			if (registration_config.TcId == call_id)
			{
				SetRegistrationConfiguration(registration_config);
			}
		}
	});
}

void VS_SignalConnectionsMgr::UpdateStatusRegistration(const net::address& address, net::port port,
	std::function<void(const std::shared_ptr<VS_ParserInterface>&)> &&exec)
{
	m_strand.post([this, self = shared_from_this(), address, port, exec = std::move(exec)]() mutable
	{
		auto conn = GetTransportConnectionByAddress({ address, port });
		if (conn)
		{
			conn->UpdateStatusRegistration(std::move(exec));
		}
	});
}

const char* VS_SignalConnectionsMgr::GetPeerName()
{
	return H323_PEERS_KEY;
}

void VS_SignalConnectionsMgr::ResetAllConfigsStatus()
{
	m_strand.post([this, self = shared_from_this()]()
	{
		for (const auto& x : m_connections)
			x.second->ResetAllConfigsStatus();
	});
}

std::shared_ptr<VS_TransportConnection> VS_SignalConnectionsMgr::GetTransportConnectionByAddress(const Address& a)
{
	assert(m_strand.running_in_this_thread());
	auto it = m_connections.find(a);
	return it != m_connections.end() ? it->second : nullptr;
}

// channel_id == e_noChannelID means 'use default parser's channel id'
void VS_SignalConnectionsMgr::SetRegistrationConfiguration(VS_CallConfig config)
{
	m_strand.post([this, self = shared_from_this(), config = std::move(config)]()
	{
		Address a{ config.Address.addr, config.Address.port };
		auto old_conn = GetTransportConnectionByAddress(a);
		if (old_conn)
		{
			old_conn->ResetInactivity();
			old_conn->SetRegistrationConfiguration(std::move(config));
		}
		else
		{
			auto conn = CreateTransportConnection(m_peer_config->GetCommonIdentifier()->CreateParser(m_strand, config, m_logger));
			if (!conn)
				return;
			if (!conn->PrepareTranscportConnection(config.Address.addr, config.Address.port, config.Address.protocol, (config.SignalingProtocol == VS_CallConfig::H323) ? e_RAS : e_noChannelID))
				return;
			conn->SetRegistrationConfiguration(std::move(config));
			m_connections.emplace(std::move(a), std::move(conn));
		}
	});
}

#undef DEBUG_CURRENT_MODULE
