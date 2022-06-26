#include "std/cpplib/VS_WorkThreadIOCP.h"
#include "VS_GatewayStarter.h"
#include "CallConfig/VS_CallConfigStorage.h"
#include "CallConfig/VS_Indentifier.h"
#include "CallConfig/VS_IndentifierSIP.h"
#include "clientcontrols/VS_ClientControlAllocatorInterface.h"
#ifdef _WIN32
#include "clientcontrols/VS_TranscodersDispatcher.h"
#endif
#include "h323/VS_H323GatekeeperStorage.h"
#include "TrueGateway/sip/SIPTransportLayer.h"
#include "net/EndpointRegistry.h"
#include "sip/VS_SIPCallResolver.h"
#include "sip/VS_SIPParser.h"
#include "sip/VS_SIPVisiProxy.h"
#include "sip/VS_TranscoderKeeper.h"
#include "sip/VS_VisiSIPProxy.h"
#include "statuslib/VS_ResolveServerFinder.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/ignore.h"
#include "std/cpplib/event.h"
#include "std/cpplib/MakeShared.h"
#include "std/cpplib/VS_Policy.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/Globals.h"
#include "transport/Router/VS_TransportRouter.h"
#include "VS_SignalConnectionsMgr.h"

#include <boost/asio/io_service.hpp>
#include <boost/signals2/deconstruct.hpp>
#include <boost/make_shared.hpp>
#include "acs_v2/Service.h"
#include "net/EndpointRegistry.h"
#include "net/DNSUtils/VS_DNS.h"
#include "net/InterfaceInfo.h"

#include "std/debuglog/VS_Debug.h"
#include "net/Logger/PcapLogger.h"
#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

#define INCLUDE_GATEWAY

VS_GatewayStarter *VS_GatewayStarter::m_instance = 0;
VS_Lock	*VS_GatewayStarter::m_lock_instance = new VS_Lock();
static const auto c_GWStopTimeout = std::chrono::seconds(30);
static const auto c_GWStartTimeout = std::chrono::seconds(5);

static constexpr uint8_t DEFAULT_MAX_FILE_SIZE = 100; /*MB*/
static constexpr char DEFAULT_INITIAL_FILE_NAME[] = "gateway.pcap";
static constexpr char DEFAULT_OLD_FILE_NAME[] = "gateway.old.pcap";

UserStatusFunction VS_GatewayStarter::m_get_user_status = [](string_view /*call_id*/, bool /*use_cache*/, bool/*do_ext_resolve*/) { return UserStatusInfo(); };
std::function<std::string(string_view)> VS_GatewayStarter::m_get_app_property = [](string_view) { return ""; };
std::function<std::string(string_view)> VS_GatewayStarter::m_get_web_manager_property = [](string_view) { return ""; };

VS_GatewayStarter* VS_GatewayStarter::GetInstance()
{
#ifndef INCLUDE_GATEWAY
	return 0;
#else
	VS_AutoLock lock(m_lock_instance);
	return !m_instance? (m_instance = new VS_GatewayStarter()) : m_instance;
#endif
}

VS_GatewayStarter::VS_GatewayStarter()
	: m_tr(0)
	, m_componentsCreated(false)
	, m_threadPool(1, "VS_Indentifier")
{
	if (!m_get_user_status)
	{
		m_get_user_status = [](string_view /*call_id*/, bool /*use_cache*/, bool /*do_ext_resolve*/) { return UserStatusInfo(); };
	}
}

VS_GatewayStarter::~VS_GatewayStarter()
{
	m_instance = 0;
}

std::string ToString(const std::set<net::Endpoint>& eps) {
	std::stringstream ss;
	for (const auto& ep : eps) {
		ss << ep << ',';
	}
	auto res = ss.str();
	if (!res.empty())
		res.pop_back();

	return res;
}

GWInitStatus VS_GatewayStarter::StartGateway(
	boost::asio::io_service& ios,
	transport::IRouter* tr,
	const std::function<bool(const std::string&, const std::string&)>& check_digest, const std::weak_ptr<ts::IPool>& transceiversPool,
	const std::weak_ptr<VS_TranscoderLogin>& transLogin,
	const std::string& serverInfo)
{
#ifndef INCLUDE_GATEWAY
	return GWInitStatus::notStarted;
#else
	VS_AutoLock lock(this);
	if(m_componentsCreated)
		return GWInitStatus::notStarted;
	puts( "MGW: Gateway is starting..." );
	const char   sSd[] = "\t started successfully.", sFd[] = "\t failed.";
	if (m_H323Protocol.get() || m_GatewayService.get() || !tr)
		return GWInitStatus::notStarted;
	m_strand.emplace(ios);
	m_sipStrand.emplace(ios);
	m_tr = tr;

	m_threadPool.Start();
	auto res = GWInitStatus::started;
	{
		STARTMESS(TRANSCODERSDISPATCHER_SRV);
		{
			VS_ClientControlAllocatorInterface::Init(transceiversPool, transLogin);
#ifdef _WIN32
			if (auto disp = dynamic_cast<VS_TranscodersDispatcher*>(VS_ClientControlAllocatorInterface::Instance()))
			{
				if (!m_tr->AddService(TRANSCODERSDISPATCHER_SRV, disp, true, true) || !disp->SetThread())
				{
					puts(sFd);
					return GWInitStatus::notStarted;
				}
			}
#endif
		}

		puts(sSd);
		std::set<net::Endpoint> bindedEps;
		bool hasSipListeners(true), hasH323Listeners(true);
		StartGatewayACS(ios, bindedEps, hasSipListeners, hasH323Listeners);
		if (!hasSipListeners) {
			dstream0 << "Endpoints for SIP weren't binded!";
			res &= ~GWInitStatus::sipBinded;
		}
		if (!hasH323Listeners) {
			dstream0 << "Endpoints for H323 weren't binded!";
			res &= ~GWInitStatus::h323Binded;
		}

		m_GatewayService = boost::make_shared<VS_GatewayService>();
		m_trKeeper = std::make_shared<VS_TranscoderKeeper>();

		std::shared_ptr<VS_CallConfigStorage> sip_call_config = vs::MakeShared<VS_CallConfigStorage>();
		boost::shared_ptr<VS_IndentifierSIP> sip_identifier = boost::static_pointer_cast<VS_IndentifierSIP>(
			VS_Indentifier::MakeIndentifierChain( {VS_CallConfig::SIP}, m_threadPool.get_io_service(), serverInfo));
		sip_call_config->RegisterProtocol(sip_identifier);

		int32_t res;
		VS_RegistryKey key{ false, CONFIGURATION_KEY };
		bool is_logging_enabled = !(key.GetValue(&res, sizeof(res), VS_REG_INTEGER_VT, "Enable gateway pcap") > 0 && res == 0);
		res = 0;
		uint64_t max_file_size = (key.GetValue(&res, sizeof(res), VS_REG_INTEGER_VT, "Max gateway pcap") > 0 && res > 0) ? res : DEFAULT_MAX_FILE_SIZE;
		max_file_size *= (1024 * 1024);
		if (is_logging_enabled)
		    m_logger = std::make_shared<net::PcapLogger>(max_file_size, vs::GetLogDirectory() + DEFAULT_INITIAL_FILE_NAME, vs::GetLogDirectory() + DEFAULT_OLD_FILE_NAME);
		auto sip_parser = vs::MakeShared<VS_SIPParser>(*m_sipStrand, serverInfo, m_logger);
		sip_parser->SetCallConfigStorage(sip_call_config);
		sip_parser->SetPolicy(boost::make_shared<VS_Policy>("SIP", VS_SIPParser::PolicySettings()));
		sip_parser->SetDigestChecker([check_digest](const std::string& a1, const std::string& a2)
		{
			return check_digest(a1, a2);
		});
		sip_parser->SetGetAppPropertyFunction(m_get_app_property);
		sip_parser->SetGetWebManagerPropertyFunction(m_get_web_manager_property);
		auto sip_tcp_listener = net::endpoint::ReadAcceptTCP(1, "SIP", false);
		auto sip_udp_listener = net::endpoint::ReadAcceptUDP(1, "SIP", false);
		m_sipTransport = vs::MakeShared<sip::TransportLayer>(m_sipStrand.get(), sip_parser, m_trKeeper,
			sip_call_config, sip_tcp_listener ? sip_tcp_listener->port : 5060, sip_udp_listener ? sip_udp_listener->port : 5060, m_logger);
		m_acsSrv->AddHandler("SIP", std::static_pointer_cast<acs::Handler>(m_sipTransport));


		{
			boost::system::error_code ec;
			vs::set<std::string, vs::str_less> bindAddrSet;
			for (const auto& endpoint: bindedEps)
			{
				if (endpoint.addr.is_v6())
				{
					std::string tmp = "[";
					tmp += endpoint.addr.to_string(ec);
					tmp += ']';
					bindAddrSet.emplace(std::move(tmp));
				} else
					bindAddrSet.emplace(endpoint.addr.to_string(ec));
			}
			size_t size = net::endpoint::GetCountConnectTCP("SIP", false);
			for (uint32_t i = 1; i <= size; ++i)
			{
				auto endpoint = net::endpoint::ReadConnectTCP(i, "SIP", false);
				if (endpoint)
				{
					if (net::is_ipv6(endpoint->host))
					{
						std::string tmp = "[";
						tmp += endpoint->host;
						tmp += ']';
						bindAddrSet.emplace(tmp);
					} else
						bindAddrSet.emplace(endpoint->host);
				}
			}
			VS_RegistryKey rKey(false, CONFIGURATION_KEY);
			std::unique_ptr<char, free_deleter> buffer = nullptr;
			size = rKey.GetValue(buffer, VS_REG_STRING_VT, SIP_FROM_HOST);
			if (size != 0)
				bindAddrSet.emplace(buffer.get());

			extern std::string g_tr_endpoint_name;
			size_t pos = g_tr_endpoint_name.find('#');
			if (pos != std::string::npos)
				bindAddrSet.emplace(g_tr_endpoint_name.substr(0, pos));
			else
				bindAddrSet.emplace(g_tr_endpoint_name);
			sip_identifier->InitBindAddrSet(std::move(bindAddrSet));
		}

		VS_VisiSIPProxy::InitInfo init_visi_sip;
		init_visi_sip.parser = sip_parser;
		init_visi_sip.trKeeper = m_trKeeper;
		init_visi_sip.peerConfig = sip_call_config;
		init_visi_sip.sipTransport = m_sipTransport;

		m_visiSIPProxy = vs::MakeShared<VS_VisiSIPProxy>(m_sipStrand.get(), std::move(init_visi_sip));

		m_trKeeper->SetVisiToSip(m_visiSIPProxy);

		VS_SIPCallResolver::InitInfo init_sip_call;
		init_sip_call.trKeeper = m_trKeeper;
		init_sip_call.parser = sip_parser;
		init_sip_call.peerConfig = sip_call_config;

		m_sipCallResolver = vs::MakeShared<VS_SIPCallResolver>(m_sipStrand.get(), std::move(init_sip_call));
		m_sipCallResolver->Start();

		m_GatewayService->SetSipListener(m_sipCallResolver);
		VS_ResolveServerFinder::Instance()->RegisterExternalPresence(m_sipCallResolver);

		m_trKeeper->SetSipCallResolver(m_sipCallResolver);

		VS_SIPVisiProxy::InitInfo info;
		info.trKeeper = m_trKeeper;
		info.sipTransport = m_sipTransport;
		info.getUserStatus = [stat = m_get_user_status](string_view callId, bool useCache, bool doExtResolve) //TODO: fix decorator
		{
			return stat(callId, useCache, doExtResolve);
		};

		info.sipCallResolver = m_sipCallResolver;
		info.transcPool = transceiversPool;
		m_sipVisiProxy = vs::MakeShared<VS_SIPVisiProxy>(m_sipStrand.get(), std::move(info));
		sip_parser->SetConfCallBack(m_sipVisiProxy);
		sip_parser->SetUserToDialogIdCallback([tr_keeper = m_trKeeper](string_view login, string_view dialogId)
		{
			tr_keeper->SetUserDialogID(login, dialogId);
		});
		/////////////
		puts(sSd);

		STARTMESS(GATEWAY_SRV);
		if(!m_tr->AddService(GATEWAY_SRV, m_GatewayService.get(),true, true) || !m_GatewayService->SetThread())
		{
			puts(sFd);
			return GWInitStatus::notStarted;
		}

		std::shared_ptr<VS_CallConfigStorage> call_config = vs::MakeShared<VS_CallConfigStorage>();
		VS_SignalConnectionsMgr::InitInfo smInitInfo(m_strand.get());
		smInitInfo.checkDigest = check_digest;
		smInitInfo.getUserStatus = [stat = m_get_user_status](string_view callId, bool useCache, bool doExtResolve) //TODO: fix decorator
		{
			return stat(callId, useCache, doExtResolve);
		};
		smInitInfo.ourEndpoint = m_GatewayService->OurEndpoint();
		smInitInfo.ourService = m_GatewayService->OurService();
		smInitInfo.postMes = [gw_service = m_GatewayService](VS_RouterMessage* rout)
		{
			return gw_service->PostMes(rout);
		};
		smInitInfo.peerConfig = call_config;
		smInitInfo.transcPool = transceiversPool;
		smInitInfo.logger = m_logger;
		m_H323Protocol = vs::MakeShared<VS_SignalConnectionsMgr>(std::move(smInitInfo));

		m_GatewayService->SetH323Listener(m_H323Protocol);
		VS_ResolveServerFinder::Instance()->RegisterExternalPresence(m_H323Protocol);

		if (IsVCS() || IsAS()) {
			call_config->RegisterProtocol(VS_Indentifier::MakeIndentifierChain({ VS_CallConfig::RTSP, VS_CallConfig::H323, VS_CallConfig::H225RAS }, m_threadPool.get_io_service(), serverInfo));
		}
		m_H323Protocol->Start(m_acsSrv.get());


		std::string bindAddrsess;
		if (!bindedEps.empty()) {
			bindAddrsess += ToString(bindedEps);
		}

		VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
		cfg_root.SetString(bindAddrsess.c_str(), CURRENT_GATEWAY_CONNECT_TAG);
		dstream0 << "Gateway Bind Address = " << bindAddrsess << '\n';

		STARTMESS(SIPCALL_SRV);
		if (!m_tr->AddService(SIPCALL_SRV, m_sipCallResolver.get(), true, true) || !m_sipCallResolver->SetThread())
		{
			puts(sFd);
			return GWInitStatus::notStarted;
		}
		puts(sSd);
	}
	m_componentsCreated = true;
	return res;
#endif
}
void VS_GatewayStarter::SetMaxTranscoders(const int max_transcoders)
{
#ifndef INCLUDE_GATEWAY
	return;
#else
	VS_AutoLock lock(this);
	if(!m_componentsCreated)
		return;
	if (auto disp = VS_ClientControlAllocatorInterface::Instance())
		disp->SetMaxTranscoders(max_transcoders);
#endif
}
void VS_GatewayStarter::SetServerAddresses(const char *addr)
{
#ifndef INCLUDE_GATEWAY
	return;
#elif defined(_WIN32)
	VS_AutoLock lock(this);
	if(!m_componentsCreated)
		return;
	if (auto disp = dynamic_cast<VS_TranscodersDispatcher*>(VS_ClientControlAllocatorInterface::Instance()))
		disp->SetServerAddresses(addr);
#endif
}

inline void VS_GatewayStarter::STARTMESS(const char *service)
{
	time_t start_vaule(0);
	time(&start_vaule);
	printf("\t %-20s: at %s", service, ctime(&start_vaule));
};
void VS_GatewayStarter::StopGateway()
{
#ifndef INCLUDE_GATEWAY
	return;
#else
	VS_AutoLock lock(this);
	VS_H323GatekeeperStorage::Instance().UnregisterAll();
	if(!m_componentsCreated)
		return;
	VS_ResolveServerFinder *resolve_finder = VS_ResolveServerFinder::Instance();
	if (resolve_finder) {
		resolve_finder->UnRegisterExternalPresence(m_H323Protocol);
		resolve_finder->UnRegisterExternalPresence(m_sipCallResolver);
	}
	if (m_sipTransport) {
		m_sipTransport->Shutdown();
	}

	StopSip();
#ifdef _WIN32
	{
		if (auto disp = dynamic_cast<VS_TranscodersDispatcher*>(VS_ClientControlAllocatorInterface::Instance()))
		{
			disp->ResetThread();
			disp->Shutdown();
			if(m_tr)
				m_tr->RemoveService(TRANSCODERSDISPATCHER_SRV);
			delete disp;
		}
	}
#endif
	StopGatewayACS();
	m_threadPool.Stop();

	VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
	cfg_root.RemoveValue(CURRENT_GATEWAY_CONNECT_TAG);
	m_componentsCreated = false;
#endif
}

void VS_GatewayStarter::StopSip() {
	assert(m_strand);
	vs::event ready(false);
	m_strand->get_io_service().dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		if (m_H323Protocol) {
			m_H323Protocol->Close();
			m_H323Protocol.reset();
		}
		if(m_GatewayService)
		{
			m_GatewayService->ResetThread();
			if(m_tr)
			{
			m_tr->RemoveService(GATEWAY_SRV);
	}
			m_GatewayService.reset();
		}
	if (m_sipCallResolver) {
		if (m_tr) {
			m_tr->RemoveService(SIPCALL_SRV);
		}
		m_sipCallResolver.reset();
	}
	if (m_sipTransport) {
		m_sipTransport.reset();
	}
	m_visiSIPProxy.reset();
	m_trKeeper.reset();
	m_sipVisiProxy.reset();
	});
	ready.wait();
}

std::string ToString(net::protocol p) {
	switch (p)
	{
	case net::protocol::TCP:
		return "TCP";
	case net::protocol::UDP:
		return "UDP";
	case net::protocol::TLS:
		return "TLS";
	case net::protocol::none:
	default:
		return "none";
	}
}

bool VS_GatewayStarter::StartGatewayACS(boost::asio::io_service & ios, std::set<net::Endpoint> &bindedEps, bool &hasSipListeners, bool &hasH323listeners)
{
	m_acsSrv = acs::Service::Create(ios);
	if (!m_acsSrv) {
		dstream0 << "Failed to create gw::acs::Service instance";
		return false;
	}

	auto startF = m_acsSrv->Start();
	if (startF.wait_for(c_GWStartTimeout) != std::future_status::ready) {
		dstream0 << "gw::acs::Service::Start took too long";
		return false;
	}
	if (!startF.valid() || !startF.get()) {
		dstream0 << "gw::acs::Service::Start failed";
		return false;
	}

	auto try_bind = [&](const net::address& address, net::port port, net::protocol protocol)
	{
		auto ec = m_acsSrv->AddListener(address, port, protocol, 0, false);
		if (ec)
			dstream1 << "gw::acs::AddListener fail to bind on " << ToString(protocol) << ":" << address << ". Error: " << ec.message();
		return !ec;
	};

	hasSipListeners = true;
	if (m_acsSrv->AddListeners("SIP", 0, false) == 0)
	{
		hasSipListeners = false;
		const auto interfaces = net::GetInterfaceInfo();
		for (const auto& info : *interfaces)
		{
			for (auto protocol : { net::protocol::TCP , net::protocol::UDP })
			{
				for (const auto& addr : info.addr_local_v4)
					hasSipListeners = try_bind(addr, 5060, protocol) || hasSipListeners;
				for (const auto& addr : info.addr_local_v6)
					hasSipListeners = try_bind(addr, 5060, protocol) || hasSipListeners;
			}
		}
	}

	hasH323listeners = true;
	if (m_acsSrv->AddListeners("H323", 0, false) == 0)
	{
		hasH323listeners = false;
		const auto interfaces = net::GetInterfaceInfo();
		for (const auto& info : *interfaces)
		{
			for (const auto& addr : info.addr_local_v4)
				hasH323listeners = (try_bind(addr, 1720, net::protocol::TCP) // H.225
				              && try_bind(addr, 1719, net::protocol::UDP) // RAS
				              && try_bind(addr, 1718, net::protocol::UDP) // RAS broadcast
				                ) || hasH323listeners;
			for (const auto& addr : info.addr_local_v6)
				hasH323listeners = (try_bind(addr, 1720, net::protocol::TCP) // H.225
				              && try_bind(addr, 1719, net::protocol::UDP) // RAS
				              && try_bind(addr, 1718, net::protocol::UDP) // RAS broadcast
				                ) || hasH323listeners;
		}
	}

	std::vector<std::pair<net::address, net::port>> listeners;
	m_acsSrv->GetListenerList(listeners, net::protocol::UDP);
	for (const auto& ep : listeners)
		bindedEps.emplace(ep.first, ep.second, net::protocol::UDP);
	listeners.clear();
	m_acsSrv->GetListenerList(listeners, net::protocol::TCP);
	for (const auto& ep : listeners)
		bindedEps.emplace(ep.first, ep.second, net::protocol::TCP);

	return true;
}

void VS_GatewayStarter::StopGatewayACS()
{
	auto stopF = m_acsSrv->Stop();
	if (stopF.wait_for(c_GWStopTimeout) != std::future_status::ready)
		dstream1 << "gw::acs::Service::Stop took too long";
	else if (!stopF.valid())
		dstream1 << "gw::acs::Service::Stop failed";
}

void VS_GatewayStarter::OnUserLoginEnd_Event(const VS_UserData& ud, const std::string &cid)
{
#ifndef INCLUDE_GATEWAY
	return false;
#else
	OnNewPeerCfg_Event(string_view{ ud.m_name.m_str, (size_t)ud.m_name.Length() }, ud.m_external_accounts);
#endif
}

void VS_GatewayStarter::OnUserLogoff_Event(const VS_UserData& ud, const std::string &cid)
{
#ifndef INCLUDE_GATEWAY
	return false;
#else
	if (!!ud.m_name)
		OnNewPeerCfg_Event(string_view{ ud.m_name.m_str, (size_t)ud.m_name.Length() }, std::vector<VS_ExternalAccount>());
#endif
}

void VS_GatewayStarter::OnNewPeerCfg_Event(string_view callId, const std::vector<VS_ExternalAccount>& v)
{
#ifndef INCLUDE_GATEWAY
	return false;
#else
	if (m_strand)
	{
		m_strand->get_io_service().post([this, callId = std::string(callId), v]() {
			if (m_H323Protocol)
				m_H323Protocol->NewPeerCfg(callId, v);
			if (m_sipCallResolver)
				m_sipCallResolver->NewPeerCfg(callId, v);
		});
	}
#endif
}

bool VS_GatewayStarter::m_isVcs = false;
bool VS_GatewayStarter::m_isAS = false;

void VS_GatewayStarter::SetIsVCS( bool isVcs )
{
	m_isVcs = isVcs;
}

bool VS_GatewayStarter::IsVCS()
{
	return m_isVcs;
}

void VS_GatewayStarter::SetIsAS(bool isAS)
{
	m_isAS = isAS;
}

bool VS_GatewayStarter::IsAS()
{
	return m_isAS;
}

void VS_GatewayStarter::SetGetUserStatusFunction(const UserStatusFunction &func)
{
	m_get_user_status = func;
}
void VS_GatewayStarter::SetGetAppPropertyFunciton(const std::function<std::string(string_view)> &func)
{
	m_get_app_property = func;
}
void VS_GatewayStarter::SetGetWebManagerPropertyFunciton(const std::function<std::string(string_view)> &func)
{
	m_get_web_manager_property = func;
}
bool VS_GatewayStarter::HasTelCfg() const
{
	return (m_sipCallResolver && m_sipCallResolver->HasTelCfg())
		|| (m_H323Protocol && m_H323Protocol->HasTelCfg())
		;
}