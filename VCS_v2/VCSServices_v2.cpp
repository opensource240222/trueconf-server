#include "VCSServices_v2.h"
#include "std/debuglog/VS_Debug.h"

#include "TLSHandlerFake.h"
#include "ldap_core/common/VS_RegABStorage.h"
#include "newtransport/Router/ServiceV1Adaptor.h"
#include "AppServer/Services/VS_MultiConfService.h"
#include "AppServer/Services/VS_PresenceService.h"
#include "AppServer/Services/VS_ChatService.h"
#include "AppServer/Services/VS_ResolveService.h"
#include "AppServer/Services/VS_DSControlService.h"
#include "ServerServices/VS_ReadLicense.h"
#include "ServerServices/VS_RoamingSettings.h"
#include "ServerServices/VS_RTSPProxy.h"
#include "ServerServices/VS_CallConfigUpdaterService.h"
#include "ServerServices/VS_ServerConfiguratorService.h"
#include "ServerServices/gateway/VS_GatewayABStorage.h"
#include "ServerServices/VS_TorrentStarter_v2.h"
#include "ServerServices/chatv2/ChatV2Service.h"
#include "ServerServices/chatv2/ResolverForChat.h"
#include "VCS/Services/VS_VCSConfRestrict.h"
#include "VCS/Services/VS_RegistryStorage.h"
#include "VCS/Services/VCSAuthService.h"
#include "VCS/Services/VS_OfflineChatService.h"
#include "VCS/Services/VS_VCSConfigurationService.h"
#include "VCS/Services/VS_VCSLogService.h"
#include "VCS/Services/VS_VCSLocatorService.h"
#include "BaseServer/Services/storage/VS_DBStorageInterface.h"
#include "BaseServer/Services/VS_AddressBookService.h"
#include "std/cpplib/MakeShared.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_Utils.h"
#include "std/VS_RegServer.h"
#include "std-generic/cpplib/scope_exit.h"
#include "http/handlers/OnlineUsers.h"
#include "http/handlers/ServerConfigurator.h"
#include "http/handlers/RouterMonitor.h"
#include "http/handlers/TorrentAnnounce.h"
#include "http/handlers/VisicronServiceStatus.h"
#include "TransceiverLib/VS_TransceiverCircuitHandler.h"
#include "TransceiverLib/TransceiversPool.h"
#include "TransceiverLib/TransceiverHandlerName.h"
#include "TransceiverLib/TransceiverConfiguration.h"
#include "FakeClient/VS_FakeClientManager.h"
#include "FakeClient/VS_FakeEndpoint.h"
#include "ServerServices/VS_WebrtcPeerConnectionService.h"
#include "version.h"
#include "TrueGateway/VS_GatewayStarter.h"
#include "TrueGateway/clientcontrols/VS_TranscoderLogin.h"

#include "ldap_core/VS_LDAPCore.h"
#include "ldap_core/VS_LDAPFactory.h"
#include "ldap_core/CfgHelper.h"

#include <boost/range/adaptor/reversed.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>


#include "ServerServices/utils/VS_TCSUtils.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

template <class T>
std::shared_ptr<T> GetOldService(const std::map<std::string, std::shared_ptr<VS_TransportRouterServiceBase>>& services, const std::string& name)
{
	auto it = services.find(name);
	if (it == services.end())
		return nullptr;
	return std::dynamic_pointer_cast<T>(it->second);
}

void VCSServices_v2::OnTransceiverStarted(const std::string& transceiverName) const
{
	if (!g_storage) return;
	std::vector<VS_ConferenceDescription> to_restore;

	g_storage->GetCurrentConferences(to_restore);
	for (const auto& conf : to_restore) {
		if (!conf.m_name) continue;
		std::vector<part_start_info> parts_info;
		std::unordered_map<std::string, std::vector<part_start_info>> confs_users;

		g_storage->GetParticipants(conf.m_name.m_str, parts_info);
		if (parts_info.size() > 0) {
			confs_users[conf.m_name.m_str] = parts_info;
		}

		auto streams_circuit = m_proxiesPool->RestoreProxyByTransName(transceiverName, conf.m_name.m_str);
		if (!streams_circuit) return;

		streams_circuit->RestoreConferences(to_restore, confs_users);
	}

}

VCSServices_v2::VCSServices_v2(boost::asio::io_service& ios)
	: m_ios(ios)
	, m_proxiesPool(std::make_shared<ts::ProxiesPoolV2<> >(
		ios, ts::GetMinAvailableTransceivers(),
		ts::GetMaxConferencesByOneTransceiver(),
		ts::GetMaxFreeTimeForTransceiver())
	)
	, m_transHandler(std::make_shared<ts::Handler>(m_proxiesPool))
{
}

bool VCSServices_v2::Init(vs::RegistrationParams&& rp, const std::shared_ptr<transport::Router>& tr,
	const std::shared_ptr<stream::RouterV2>& sr, const std::shared_ptr<acs::Service>& acs_srv,
	VS_RoutersWatchdog* watchdog, string_view product_version)
{
	assert(tr);
	assert(sr);
	assert(acs_srv);

	auto t_all = std::chrono::steady_clock::now();
	VS_SCOPE_EXIT{
		dstream4 << "VCSServices_v2::Init() time " <<
		std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t_all).count() << "ms";
	};

	m_tr = tr;
	m_sr = sr;

	auto start_tr = [&tr, &acs_srv](void) -> void {
						std::cout << "Starting: Transport Router\n";
						tr->Start(acs_srv.get());
						std::cout << "Started:  Transport Router\n";
					};
	if (rp.mode == 1 || rp.mode == 2) {
		start_tr();
		m_verify_srv = vs::make_unique<VS_VerificationService>(std::move(rp));
		m_verify_srv->SetComponents(watchdog, REGISTRATION_SRV, std::string(product_version).c_str());
		bool res = transport::InstallV1Service(m_verify_srv.get(), VERIFY_SRV, true, tr);
		res = res && m_verify_srv->SetThread();
	} else {
		auto confRestrict = boost::make_shared<VS_VCSConfRestrict>();

		auto logs = std::make_shared<VS_VCSLogService>();
		const auto db_conn_str = VS_GetDBConnectionString();
		if (!db_conn_str.empty())
		{
			if (!logs->InitEventStorage(db_conn_str))
			{
				dstream1 << "EventLog init failed, connection string: " << db_conn_str;
				return false;
			}
		}
		else
			dstream0 << "EventLog not initialized: no connection string";
		m_old_services[LOG_SRV] = logs;

		auto PresSRV = boost::make_shared<VS_PresenceService>(m_ios);
		utils::init_gateway_ext_funcs_for_TCS(PresSRV);

		// by default it allows roaming connections only to the reg server
		tr->SetIsRoamingAllowedFunc([](const char *for_server_name) -> bool
		{
			if (!!for_server_name && strcasecmp(RegServerName, for_server_name) == 0)
				return true;
			return false;
		});
		start_tr();

		// todo(kt): at mode = 0 also add verify srv
		p_licWrap = new VS_LicensesWrap;
		m_transLogin = vs::MakeShared<VS_TranscoderLogin>();

		std::string acs_listeners_tcp;
		acs_srv->GetListenerList(acs_listeners_tcp, net::protocol::TCP);

		VS_Policy::SetIsVCS(true);
		VS_GatewayStarter::SetIsVCS(true);
		m_chck_lics = vs::make_unique<VS_CheckLicenseService>(m_ios, vs::make_unique<VS_TorrentStarter_v2>(m_ios, acs_srv));
		m_chck_lics->SetPresenceService(PresSRV);
		m_chck_lics->SetTransceiversPool(m_proxiesPool);
		m_chck_lics->SetComponents(watchdog, m_tr.get(), acs_listeners_tcp, product_version, m_transLogin);
		bool res = transport::InstallV1Service(m_chck_lics.get(), CHECKLIC_SRV, true, tr);
		res = res && m_chck_lics->SetThread();
		dstream0 << "AddServiceOld(" << CHECKLIC_SRV << ")=" << std::boolalpha << res;
		if (!res)
			return false;

		dstream4 << "Start wait checkLicEvent";
		auto reg_result = m_chck_lics->WaitForRegistrationEvent();
		dstream4 << "End wait checkLicEvent:" << reg_result;
		if (!reg_result)
		{
			// todo(kt): unreg from TR
			return false;
		}

		// load and use roaming settings
		m_roaming_settings = vs::MakeShared<VS_RoamingSettings>(m_ios);
		m_roaming_settings->Start();
		confRestrict->SetRoamingSettings(m_roaming_settings);
		tr->SetIsRoamingAllowedFunc([w = PresSRV->weak_from_this()](const char* sid) -> bool {
			auto pres = w.lock();
			if (pres)
				return pres->VS_PresenceService::IsRoamingAllowed(sid);
			return false;
		});



		//VS_License lic;
		//lic.m_error = 0;
		//lic.m_onlineusers = VS_License::TC_INFINITY;
		//lic.m_conferences = 6;
		//lic.m_symmetric_participants = 16;
		//lic.m_restrict |=
		//	VS_License::Flags::USER_GROUPS_ALLOWED |
		//	VS_License::Flags::LDAP_ALLOWED |
		//	VS_License::Flags::FILETRANSFER_ALLOWED |
		//	VS_License::Flags::DSHARING_ALLOWED |
		//	VS_License::Flags::VIDEORECORDING_ALLOWED |
		//	VS_License::Flags::WHITEBOARD_ALLOWED |
		//	VS_License::Flags::SLIDESHOW_ALLOWED |
		//	VS_License::Flags::ASYMMETRICCONF_ALLOWED |
		//	VS_License::Flags::ROLECONF_ALLOWED |
		//	VS_License::Flags::UDPMULTICAST_ALLOWED |
		//	VS_License::Flags::HD_ALLOWED |
		//	VS_License::Flags::WEBRTC_BROADCAST_ALLOWED |
		//	VS_License::Flags::ENABLE_WEBINARS;
		//*p_licWrap += lic;

		//g_dbStorage = std::make_shared<VS_RegistryStorage>(new VS_RegABStorage(), VS_CheckLicense(LE_USER_GROUPS_ALLOWED), g_tr_endpoint_name.c_str());
	 //

		g_storage = new VS_Storage(tr->EndpointName().c_str());
		g_storage->SetConfRestrict(confRestrict);
		g_storage->Init();

		auto init = [&tr](std::shared_ptr<transport::ServiceBase>& srv) -> bool
		{
			auto res = tr->AddService(srv.get());
			dstream0 << "AddService(" << srv->GetName() << ")=" << res;
			if (res)
				srv->SetTransportRouter(tr);
			return res;
		};
		for (auto& srv : m_services)
			if (!init(srv))
				return false;

		VS_RegistryKey cfg_root(false, CONFIGURATION_KEY);
		if (!cfg_root.IsValid())
			return false;

		PresSRV->SetConfRestrict(confRestrict);
		m_old_services_boost[PRESENCE_SRV] = PresSRV;

		auto vcs_as = vs::MakeShared<VS_VCSAuthService>(m_ios);
		vcs_as->SetPresenceService(PresSRV);
		m_old_services[AUTH_SRV] = vcs_as;

		auto ab_storage = boost::make_shared<VS_GatewayABStorage>();
		auto abook = std::make_shared<VS_AddressBookService>();
		abook->SetABStorage(ab_storage);
		abook->SetConfRestrict(confRestrict);
		m_old_services[ADDRESSBOOK_SRV] = abook;

		auto cfg_srv = std::make_shared<VS_VCSConfigurationService>();
		cfg_srv->SetComponents(sr.get(), product_version);
		m_old_services[CONFIGURATION_SRV] = cfg_srv;

		auto chat = std::make_shared<VS_ChatService>();
		chat->SetConfRestrict(confRestrict);
		chat->SetTransceiversPool(m_proxiesPool);
		m_old_services[CHAT_SRV] = chat;

		auto chat_v2 = std::make_shared<ChatV2Service>();
		if (ChatV2Service::IsChatV2Enabled())
		{
			m_old_services[CHATV2_SRV] = chat_v2;
		}

		auto offline_chat = std::make_shared<VS_OfflineChatService>();
		offline_chat->SetConfRestrict(confRestrict);
		m_old_services[OFFLINECHAT_SRV] = offline_chat;

		vcs_as->m_OnUserLoggedIn.connect(boost::bind(&VS_PresenceService::OnUserLoginEnd_Event, PresSRV.get(), _1, _2));
		vcs_as->m_OnUserLoggedOut.connect(boost::bind(&VS_PresenceService::OnUserLogoff_Event, PresSRV.get(), _1, _2));
		{
			auto *gateway = VS_GatewayStarter::GetInstance();
			vcs_as->m_OnUserLoggedIn.connect(boost::bind(&VS_GatewayStarter::OnUserLoginEnd_Event, gateway, _1, _2));
			vcs_as->m_OnUserLoggedOut.connect(boost::bind(&VS_GatewayStarter::OnUserLogoff_Event, gateway, _1, _2));
		}
		auto mcSrv = std::make_shared<VS_MultiConfService>();
		mcSrv->SetConfRestrict(confRestrict);
		mcSrv->SetPresenceService(PresSRV);
		mcSrv->SetStreamRouter(m_sr.get());
		m_sr->AddConferencesCondition(mcSrv.get());
		mcSrv->SetIOservice(m_ios);
		m_old_services[CONFERENCE_SRV] = mcSrv;

		m_old_services[LOCATE_SRV] = std::make_shared<VS_VCSLocatorService>();;

		auto resolve = std::make_shared<VS_ResolveService>();
		resolve->SetConfRestrict(confRestrict);
		resolve->SetPresenceService(PresSRV);
		m_old_services[RESOLVE_SRV] = resolve;

		auto call_cfg = std::make_shared<VS_CallConfigUpdaterService>();
		call_cfg->SetConfig(/*m_srv_start_mode*/0);			// normal mode, not register_server or offline_register_server
		m_old_services[CALL_CFG_UPDATER_SRV] = call_cfg;

		auto web_cfg = std::make_shared<VS_ServerConfiguratorService>();
		web_cfg->SetConfRestrict(confRestrict);
		m_old_services[SERVERCONFIGURATOR_SRV] = web_cfg;

		m_old_services[DS_CONTROL_SRV] = std::make_shared<VS_DSControlService>();;

		// We need this to quickly discard TLS connections to avoid timeouts in the client caused by other handlers waiting for more data.
		m_tls_handler = std::make_shared<vs::TLSHandlerFake>();
		acs_srv->AddHandler("Fake TLS handler", m_tls_handler);

		auto online_users_handler = std::make_shared<http::handlers::OnlineUsers>(PresSRV, /* todo(kt): ServerName Version string */ std::string());
		m_http_router = std::make_shared<http::Router>();
		auto router_monitor = std::make_shared<http::handlers::RouterMonitor>(tr, sr,acs_srv);
		m_http_router->AddHandler("GET /rm/tr", router_monitor);
		m_http_router->AddHandler("GET /rm/acs", router_monitor);
		m_http_router->AddHandler("GET /rm/sr", router_monitor);
		m_http_router->AddHandler("GET /s4/", online_users_handler);
		m_http_router->AddHandler("GET /s2/", online_users_handler);
		m_http_router->AddHandler("POST /", std::make_shared<http::handlers::ServerConfigurator>(web_cfg->GetPushDataSignalSlot(), /* todo(kt): ServerName Version string */ std::string()));
		m_http_router->AddHandler("GET /vsstatus", std::make_shared<http::handlers::VisicronServiceStatus>(std::string(VS_PRODUCT_NAME).append(" ").append(STRPRODUCTVER)));

		auto torrent_starter = m_chck_lics->GetTorrentStarter();
		confRestrict->SetTorrentStarter(torrent_starter);

		m_http_router->AddHandler("GET /announce", std::make_shared<http::handlers::TorrentAnnounce>(torrent_starter));

		m_http_handler = std::make_shared<http_v2::ACSHandler>(m_ios);
		m_http_handler->SetHttpRouter(m_http_router);
		acs_srv->AddHandler("HttpHandlerV2", m_http_handler);

		m_checksrv_handler = std::make_shared<checksrv::Handler>(m_tr);
		acs_srv->AddHandler("CheckSrvHandlerV2", m_checksrv_handler);

	    m_websock_handler = std::make_shared<ws::Handler>(m_ios);
		acs_srv->AddHandler("WebSock protocol", m_websock_handler);

	       VS_FakeClientManager::Init(m_ios);
		VS_FakeEndpointFactory::InitV2(tr);

	        m_proxiesPool->SetOnTransceiverReady([this](const std::string& transceiverName) {this->OnTransceiverStarted(transceiverName); });
	        m_proxiesPool->Init(acs_srv);
			mcSrv->SetTransceiversPool(m_proxiesPool);
	        if (acs_srv->AddHandler(CircuitsACSHandler, m_transHandler) != boost::system::errc::success)	return false;

		m_rtsp_proxy = std::make_shared<VS_RTSPProxy>(m_proxiesPool);
		acs_srv->AddHandler("RTSP proxy", m_rtsp_proxy);
		acs_srv->AddListeners("RTSP");

	        VS_FakeClientManager::Init(m_ios);
		VS_FakeEndpointFactory::InitV2(tr);

		m_webrtc_pc_srv = vs::MakeShared<VS_WebrtcPeerConnectionService>(m_proxiesPool);
		m_webrtc_pc_srv->Init();

		for (const auto& kv : m_old_services)
		{
			const auto& name = kv.first;
			const auto& srv = kv.second;
			auto t = std::chrono::steady_clock::now();
			bool res = transport::InstallV1Service(srv.get(), name/*srv->OurService()*/, true, tr);
			res = res && srv->SetThread();
			dstream0 << "AddServiceOld(" << name << ")=" << std::boolalpha << res << ", time "
				<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t).count() << "ms";
			if (!res)
				return false;
		}

		for (const auto& kv : m_old_services_boost)
		{
			const auto& name = kv.first;
			const auto& srv = kv.second;
			auto t = std::chrono::steady_clock::now();
			bool res = transport::InstallV1Service(srv.get(), name/*srv->OurService()*/, true, tr);
			res = res && srv->SetThread();
			dstream0 << "AddServiceOld(" << name << ")=" << std::boolalpha << res << ", time "
				<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t).count() << "ms";
			if (!res)
				return false;
		}
		if (ChatV2Service::IsChatV2Enabled())
		{
			auto resolver = vs::MakeShared<ResolverForChatImpl>(m_ios);
			resolver->SetPresenceService(PresSRV);
			chat_v2->ConstructChatLayout(resolver);
		}
	}

	return true;
}

void VCSServices_v2::Destroy()
{
	for (auto& srv : m_services | boost::adaptors::reversed)
	{
		const auto res = m_tr->RemoveService(srv->GetName());
		dstream0 << "RemoveService(" << srv->GetName() << ")=" << res;
	}
	m_services.clear();

	if (auto gw_starter = VS_GatewayStarter::GetInstance())
		gw_starter->StopGateway();

	if (m_rtsp_proxy)
		m_rtsp_proxy->Close();
	m_rtsp_proxy = nullptr;

	VS_FakeEndpointFactory::DeInit();
	VS_FakeClientManager::DeInit();
	if (m_proxiesPool)
	{
		m_proxiesPool->Stop();
		// m_proxiesPool = nullptr; // FIXME: This currently causes problems
	}
	if (m_webrtc_pc_srv)
	{
		m_webrtc_pc_srv->Shutdown();
		m_webrtc_pc_srv = nullptr;
	}

	for (const auto& kv : m_old_services_boost)
	{
		//const auto& name = kv.first;
		const auto& srv = kv.second;
		srv->ResetThread();
		// todo(kt): de-init old services
	}

	if (auto srv = GetOldService<VS_MultiConfService>(m_old_services, CONFERENCE_SRV))
	{
		m_sr->RemoveConferencesCondition(srv.get());
	}

	for (const auto& kv : m_old_services)
	{
		//const auto& name = kv.first;
		const auto& srv = kv.second;
		srv->ResetThread();
		// todo(kt): de-init old services
	}

	if (m_chck_lics)
		m_chck_lics->ResetThread();
	if (m_verify_srv)
		m_verify_srv->ResetThread();

	if (m_roaming_settings)
		m_roaming_settings->Stop();
	delete g_storage; g_storage = 0;
	g_dbStorage.reset();
}
