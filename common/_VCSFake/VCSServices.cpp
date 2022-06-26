#ifdef _WIN32 // not ported
#include "VCSServices.h"

#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs_v2/Service.h"
#include "AppServer/Services/VS_ChatService.h"
#include "AppServer/Services/VS_MultiConfService.h"
#include "AppServer/Services/VS_PresenceService.h"
#include "AppServer/Services/VS_ResolveService.h"
#include "BaseServer/Services/VS_AddressBookService.h"
#include "FakeClient/VS_FakeClientManager.h"
#include "FakeClient/VS_FakeEndpoint.h"
#include "http/handlers/OnlineUsers.h"
#include "http/handlers/ServerConfigurator.h"
#include "http/handlers/VisicronServiceStatus.h"
#include "ldap_core/common/VS_RegABStorage.h"
#include "net/InterfaceInfo.h"
#include "ServerServices/chatv2/ChatV2Service.h"
#include "ServerServices/chatv2/ResolverForChat.h"
#include "ServerServices/gateway/VS_GatewayABStorage.h"
#include "ServerServices/VS_CallConfigUpdaterService.h"
#include "ServerServices/VS_HttpHandler_v2.h"
#include "ServerServices/VS_PingService.h"
#include "ServerServices/VS_ReadLicense.h"
#include "ServerServices/VS_RoamingSettings.h"
#include "ServerServices/VS_RTSPProxy.h"
#include "ServerServices/VS_ServerConfiguratorService.h"
#include "ServerServices/VS_TorrentStarter.h"
#include "ServerServices/VS_VerificationService.h"
#include "ServerServices/VS_WebrtcPeerConnectionService.h"
#include "VCS/Services/VCSAuthService.h"
#include "VCS/Services/VS_CheckLicenseService.h"
#include "VCS/Services/VS_OfflineChatService.h"
#include "VCS/Services/VS_RegistryStorage.h"
#include "VCS/Services/VS_VCSABStorage.h"
#include "VCS/Services/VS_VCSConfigurationService.h"
#include "VCS/Services/VS_VCSConfRestrict.h"
#include "VCS/Services/VS_VCSLocatorService.h"
#include "VCS/Services/VS_VCSLogService.h"
#include "ServerServices/utils/VS_TCSUtils.h"
#include "std-generic/compat/memory.h"
#include "std/cpplib/MakeShared.h"
#include "std/cpplib/VS_Policy.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_Utils.h"
#include "std/VS_RegServer.h"
#include "TransceiverLib/TransceiverConfiguration.h"
#include "TransceiverLib/TransceiverHandlerName.h"
#include "TransceiverLib/TransceiversPool.h"
#include "TransceiverLib/VS_TransceiverCircuitHandler.h"
#include "TrueGateway/CallConfig/VS_CallConfigCorrector.h"
#include "TrueGateway/clientcontrols/VS_TranscoderLogin.h"
#include "TrueGateway/VS_GatewayStarter.h"
#include "WebSocket/VS_WsHandler.h"

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

#include <ctime>
#include <unordered_map>

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

VS_ACS_Response IsWsHandlerProtocol(const void *in_buffer, unsigned long *in_len)
{
	return VS_WsHandler::Protocol(in_buffer, in_len);
}

//////////////
struct VS_VCSServices_Implementation
{
	enum StartUpSequence {
		NONE_SS = 0,
		POOLTHREADS_SRV_SS,
		LOG_SRV_SS,
		SMTPMAILER_SRV_SS,
		LOCATOR_SRV_SS,
		PRESENCE_SRV_SS,
		AUTH_SRV_SS,
		CONFIGURATION_SRV_SS,
		ADDRESSBOOK_SRV_SS,
		CONFERENCE_SRV_SS,
		CHAT_SRV_SS,
		CHATV2_SRV_SS,
		OFFLINECHAT_SRV_SS,
		PING_SRV_SS,
		CHECKLIC_SRV_CC,
		VERIFY_SRV_SS,
		RESOLVE_SRV_SS,
		SERVCONFIGURATOR_SRV_SS,
		WEBRTC_PEERCONNECTION_SRV_SS,
		CALL_CFG_UPDATER_SS
	};

	static const int RESTART_TIME = 5 * 60;//5 minutes

	VS_VCSServices_Implementation(boost::asio::io_service& _ios) : ios(_ios), tr(0), sr(0), ss(NONE_SS),m_srv_start_mode(0), acs(0), tlsHandler(0)
		, proxiesPool(std::make_shared<ts::ProxiesPool>(ios, ts::GetMinAvailableTransceivers(),ts::GetMaxConferencesByOneTransceiver(), ts::GetMaxFreeTimeForTransceiver()))
		, webrtc_pc_srv(vs::MakeShared<VS_WebrtcPeerConnectionService>(proxiesPool))
		, transHandler(std::static_pointer_cast<VS_SetConnectionInterface>(proxiesPool)), IsWebRtcSRV(false) {}
	~VS_VCSServices_Implementation( void ) {Destroy();}

	boost::asio::io_service& ios;
	VS_TransportRouter *tr;
	stream::Router *sr;
	VS_AccessConnectionSystem *acs;
	std::shared_ptr<acs::Service> acs_srv;
	VS_TlsHandler *tlsHandler;
	std::shared_ptr<VS_RoamingSettings> m_roaming_settings;
	StartUpSequence	ss;
	VS_PoolThreadsService pts; //sync
	boost::shared_ptr<VS_PresenceService> upss; //async
	std::shared_ptr<VS_VCSAuthService> vcs_as; //async
	VS_VCSLogService logs;
	VS_SmtpMailerService sms; //sync
	VS_VCSLocatorService locs; //sync
	VS_AddressBookService abs; //sync
	VS_MultiConfService cs; //async
	VS_ChatService chs; //async
	ChatV2Service                   chat_v2_srv;
	VS_OfflineChatService ochs;
	VS_PingService pngs;
	VS_ResolveService rslvs;
	VS_VCSConfigurationService vcs_cfg;
	std::unique_ptr<VS_CheckLicenseService> chck_lics;
	std::unique_ptr<VS_VerificationService>	verifys;
	VS_ServerConfiguratorService configSrv;
	VS_CallConfigUpdaterService callCfgUpdaterSrv;
	long m_srv_start_mode;
	std::shared_ptr<ts::ProxiesPool> proxiesPool;
	std::shared_ptr<VS_WebrtcPeerConnectionService>	webrtc_pc_srv;
	std::shared_ptr<http::Router> m_http_router;
	std::unique_ptr<VS_HttpHandler_v2> m_http_handler;
	VS_WsHandler wsHandler;
	VS_TransceiverCircuitHandler transHandler;
	std::shared_ptr<VS_TranscoderLogin> transcoder_login;
	std::shared_ptr<VS_RTSPProxy> rtspProxy;
	bool IsWebRtcSRV;
	bool m_isChatV2Enabled = false;

	inline void STARTMESS(const char *service)
	{
		time_t start_vaule(0);
		time(&start_vaule);
		printf("\t %-20s: at %s", service, ctime(&start_vaule));
	};

	inline bool Init(vs::RegistrationParams&& rp, VS_TransportRouter* tr, stream::Router* sr, VS_AccessConnectionSystem* acs,
		VS_TlsHandler* tlsHandler, VS_RoutersWatchdog* watchdog,  const char* ver, VS_Container& UCAZA)
	{
		if (ss != NONE_SS || !acs)		return false;
		VS_VCSServices_Implementation::tr = tr;
		VS_VCSServices_Implementation::sr = sr;
		VS_VCSServices_Implementation::acs = acs;
		VS_VCSServices_Implementation::tlsHandler = tlsHandler;
		puts( "SRV: Services are starting..." );
		const char   sSd[] = "\t started successfully.", sFd[] = "\t failed.";

		m_srv_start_mode = rp.mode;

		VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
		if (!cfg_root.IsValid()) {
			puts("Registry Configuration key not found");
			return false;
		}

		verifys = vs::make_unique<VS_VerificationService>(std::move(rp));
		if(m_srv_start_mode == 1 || m_srv_start_mode == 2)
		{
			STARTMESS(VERIFY_SRV);
			verifys->SetComponents(watchdog, REGISTRATION_SRV, ver);
			verifys->SetCallCfgCorrectorDataHandler([](const char *dumb_terminals_data) {
				if (VS_CallConfigCorrector::IsValidData(dumb_terminals_data))
				{
					VS_CallConfigCorrector::UpdateDataInRegistry(dumb_terminals_data);
					VS_CallConfigCorrector::GetInstance().UpdateCorrectorData(dumb_terminals_data);
				}
			});
			if (!tr->AddService(VERIFY_SRV, verifys.get()) || !verifys->SetThread()){
				puts( sFd ); return false;
			}
			ss = VERIFY_SRV_SS; puts(sSd);
		}
		else
		{
			VS_Policy::SetIsVCS( true );
			VS_GatewayStarter::SetIsVCS( true );

			{
				acs_srv = acs::Service::Create(ios);
				if (!acs_srv)
				{
					dstream0 <<  "Failed to create acs::Service instance";
					return false;
				}
				auto start_f = acs_srv->Start();
				if (start_f.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
				{
					dstream0 << "acs::Service::Start took too long";
					return false;
				}
				if (!start_f.valid() || !start_f.get())
				{
					dstream0 << "acs::Service::Start failed";
					return false;
				}

				if (acs_srv->AddListeners("RTSP") == 0)
				{
					auto try_bind = [&](const net::address& address)
					{
						auto ec = acs_srv->AddListener(address, 554, net::protocol::TCP);
						if (ec)
							dstream1 << "acs::AddListener failed to bind on TCP:" << address << ":554, error: " << ec.message();
						return !ec;
					};

					bool has_listeners = false;
					boost::system::error_code ec;
					const auto interfaces = net::GetInterfaceInfo();
					for (const auto& info : *interfaces)
					{
						for (const auto& addr : info.addr_local_v4)
							has_listeners = try_bind(addr) || has_listeners;
						for (const auto& addr : info.addr_local_v6)
							has_listeners = try_bind(addr) || has_listeners;
					}
					if (!has_listeners)
					{
						dstream0 << "Failed to bind to any address on port 554";
						return false;
					}
				}
			}

			chck_lics = vs::make_unique<VS_CheckLicenseService>(ios, vs::make_unique<VS_TorrentStarter>(ios));
			m_http_handler = vs::make_unique<VS_HttpHandler_v2>(ios);
			acs->AddHandler("Handler Of HTTP Tunneling Requests.", m_http_handler.get());
			tlsHandler->AddHandler("Handler Of HTTP Tunneling Requests.", m_http_handler.get());
			acs->AddHandler("WebSock protocol", &wsHandler);
			tlsHandler->AddHandler("WebSock protocol", &wsHandler);

			VS_FakeClientManager::Init(ios);
			VS_FakeEndpointFactory::InitV1(tr->GetSetConnectionHandler(), tr->EndpointName());

			upss = boost::make_shared<VS_PresenceService>(ios);
			vcs_as = vs::MakeShared<VS_VCSAuthService>(ios);

			tr->SetIsRoamingAllowedFunc(std::bind(&VS_PresenceService::IsRoamingAllowed, upss.get(), std::placeholders::_1));

			cs.SetStreamRouter(sr);
			cs.SetIOservice(ios);

			auto confResctrict = boost::make_shared<VS_VCSConfRestrict>();
			m_roaming_settings = vs::MakeShared<VS_RoamingSettings>(ios);
			m_roaming_settings->Start();
			confResctrict->SetRoamingSettings(m_roaming_settings);
			auto torrent_starter = chck_lics->GetTorrentStarter();
			confResctrict->SetTorrentStarter(torrent_starter);
			cs.SetConfRestrict(confResctrict);
			rslvs.SetConfRestrict(confResctrict);
			chs.SetConfRestrict(confResctrict);
			ochs.SetConfRestrict(confResctrict);
			upss->SetConfRestrict(confResctrict);
			configSrv.SetConfRestrict(confResctrict);
			abs.SetConfRestrict(confResctrict);
			cs.SetPresenceService(upss);
			rslvs.SetPresenceService(upss);		rslvs.RegAtPresSRV();
			chck_lics->SetPresenceService(upss);
			chat_v2_srv.SetPresenceService(upss);
			vcs_as->SetPresenceService(upss);
//			chs.SetPresenceService(upss);


			// StreamsRelay init
			proxiesPool->SetOnTransceiverReady([this](const std::string& transceiverName) {OnTransceiverStarted(transceiverName); });
			proxiesPool->Init(acs);
			cs.SetTransceiversPool(proxiesPool);
			chck_lics->SetTransceiversPool(proxiesPool);
			chs.SetTransceiversPool(proxiesPool);
			if (!acs->AddHandler(CircuitsACSHandler, &transHandler))	return false;
			tlsHandler->AddHandler(CircuitsACSHandler, &transHandler);
			// --------------------------------------------------------------------------------

			rtspProxy = std::make_shared<VS_RTSPProxy>(proxiesPool);
			acs_srv->AddHandler("RTSP proxy", rtspProxy);

			transcoder_login = vs::MakeShared<VS_TranscoderLogin>();

			utils::init_gateway_ext_funcs_for_TCS(upss);

			std::string user_agent = VS_Server::LongName();
			user_agent += " ";
			user_agent += VS_Server::ProductVersion();
			auto online_users_handler = std::make_shared<http::handlers::OnlineUsers>(upss, user_agent);
			m_http_router = std::make_shared<http::Router>();
			m_http_router->AddHandler("GET /s4/", online_users_handler);
			m_http_router->AddHandler("GET /s2/", online_users_handler);
			m_http_router->AddHandler("POST /", std::make_shared<http::handlers::ServerConfigurator>(configSrv.GetPushDataSignalSlot(), user_agent));
			m_http_router->AddHandler("GET /announce", std::make_shared<http::handlers::TorrentAnnounce>(torrent_starter));
			m_http_router->AddHandler("GET /vsstatus", std::make_shared<http::handlers::VisicronServiceStatus>(user_agent));
			m_http_handler->SetHttpRouter(m_http_router);

			STARTMESS(LOG_SRV);
			bool log_inited = true;
			const auto db_conn_str = VS_GetDBConnectionString();
			if (!db_conn_str.empty())
			{
				if (!logs.InitEventStorage(db_conn_str))
				{
					dstream1 << "EventLog init failed, connection string: " << db_conn_str;
					log_inited = false;
				}
			}
			else
				dstream0 << "EventLog not initialized: no connection string";
			if (!log_inited || !tr->AddService(LOG_SRV, &logs) || !logs.SetThread()) {
				puts(sFd); return false;
			}
			ss = LOG_SRV_SS; puts(sSd);

			STARTMESS(CHECKLIC_SRV);
			std::string listeners;
			if (acs)
				acs->GetListeners(listeners);
			chck_lics->SetComponents(watchdog, tr, listeners, ver, transcoder_login);
			if (!tr->AddService(CHECKLIC_SRV, chck_lics.get()) || !chck_lics->SetThread()) {
				puts( sFd ); return false;
			}
			if (!chck_lics->WaitForRegistrationEvent()) {
				chck_lics->ResetThread();
				tr->RemoveService(CHECKLIC_SRV);
				puts( sFd ); return false;
			}
			ss = CHECKLIC_SRV_CC; puts(sSd);

			// --------------------------------------------------------------------------------
			STARTMESS(VERIFY_SRV);
			verifys->SetComponents(watchdog, REGISTRATION_SRV, ver);
			if (!tr->AddService(VERIFY_SRV, verifys.get()) || !verifys->SetThread()){
				puts( sFd ); return false;
			}
			ss = VERIFY_SRV_SS; puts(sSd);

			// --------------------------------------------------------------------------------

			STARTMESS(VS_POOL_THREADS_SERVICE_NAME);
			if (!tr->AddService(VS_POOL_THREADS_SERVICE_NAME, &pts) || !pts.SetThread()) {
				puts( sFd );	return false;
			}
			ss = POOLTHREADS_SRV_SS;	puts( sSd );

			// --------------------------------------------------------------------------------
			STARTMESS("Starting Storage");
			g_storage=new VS_Storage(pts.OurEndpoint());
			g_storage->SetConfRestrict(confResctrict);
			int error_code=g_storage->Init();
			if (error_code)
			{
				printf( "failed with code %d\n",error_code);
				delete g_storage; g_storage = 0;
				return false;
			}
			puts(sSd);
			// --------------------------------------------------------------------------------
			STARTMESS(VS_SMTP_MAILER_SERVICE_NAME);
			if (!tr->AddService( VS_SMTP_MAILER_SERVICE_NAME, &sms ) || !sms.SetThread())
			{	puts( sFd );	return false;	}
			ss = SMTPMAILER_SRV_SS;		puts( sSd );

			STARTMESS(PRESENCE_SRV);
			if (!tr->AddService(PRESENCE_SRV, upss.get()) || !upss->SetThread()) {
				puts(sFd); return false;
			}
			ss = PRESENCE_SRV_SS; puts(sSd);

			STARTMESS(AUTH_SRV);
			if (!tr->AddService(AUTH_SRV, vcs_as.get(), true, true) || !vcs_as->SetThread()) {
				puts(sFd); return false;
			}
			ss = AUTH_SRV_SS; puts(sSd);


			STARTMESS(CONFIGURATION_SRV);
			vcs_cfg.SetComponents(sr, ver);
			if (!tr->AddService(CONFIGURATION_SRV, &vcs_cfg, true, true) || !vcs_cfg.SetThread()) {
				puts(sFd); return false;
			}
			ss = CONFIGURATION_SRV_SS; puts(sSd);

			STARTMESS(ADDRESSBOOK_SRV);

			boost::shared_ptr<VS_ABStorageInterface> ab_storage(new VS_GatewayABStorage());
			abs.SetABStorage(ab_storage);
			if (!tr->AddService(ADDRESSBOOK_SRV, &abs, true, true) || !abs.SetThread()) {
				puts(sFd); return false;
			}
			ss = ADDRESSBOOK_SRV_SS; puts(sSd);

			IsWebRtcSRV = VS_CheckLicense(LE_WEBRTC_BROADCAST);
			if (IsWebRtcSRV)
			{
				STARTMESS(WEBRTC_PEERCONNECTION_SRV);
				if (!webrtc_pc_srv->Init()) {
					puts(sFd); return false;
				}
				ss = WEBRTC_PEERCONNECTION_SRV_SS;	puts(sSd);
			}

			STARTMESS(CONFERENCE_SRV);
			if (!tr->AddService(CONFERENCE_SRV, &cs) || !cs.SetThread()) {
				puts( sFd ); return false;
			}
			ss = CONFERENCE_SRV_SS; puts(sSd);


			STARTMESS(CHAT_SRV);
			if (!tr->AddService(CHAT_SRV, &chs) || !chs.SetThread()) {
				puts( sFd ); return false;
			}
			ss = CHAT_SRV_SS; puts(sSd);

			if(ChatV2Service::IsChatV2Enabled())
			{
				STARTMESS(CHATV2_SRV);
				if (!tr->AddService(CHATV2_SRV, &chat_v2_srv) || !chat_v2_srv.SetThread()) {
					puts(sFd); return false;
				}
				ss = CHATV2_SRV_SS; puts(sSd);
				auto chatResolver = vs::MakeShared<ResolverForChatImpl>(ios);
				chatResolver->SetPresenceService(upss);
				chat_v2_srv.ConstructChatLayout(chatResolver);
			}

			STARTMESS(OFFLINECHAT_SRV);
			if (!tr->AddService(OFFLINECHAT_SRV, &ochs) || !ochs.SetThread()) {
				puts( sFd ); return false;
			}
			ss = OFFLINECHAT_SRV_SS; puts(sSd);

			STARTMESS(PING_SRV);
			if (!tr->AddCallService(PING_SRV, &pngs))
			{	puts( sFd );	return false;	}
			ss = PING_SRV_SS;	puts( sSd );

			STARTMESS(LOCATE_SRV);
			if (!tr->AddService(LOCATE_SRV, &locs) || !locs.SetThread()) {
				puts( sFd ); return false;
			}
			ss = LOCATOR_SRV_SS; puts(sSd);

			STARTMESS(RESOLVE_SRV);
			if (!tr->AddService(RESOLVE_SRV, &rslvs) || !rslvs.SetThread()) {
				puts( sFd ); return false;
			}
			ss = RESOLVE_SRV_SS; puts(sSd);

			STARTMESS(SERVERCONFIGURATOR_SRV);
			if( !tr->AddService(SERVERCONFIGURATOR_SRV,&configSrv) || !configSrv.SetThread() )
			{
				puts( sFd ); return false;
			}
			ss = SERVCONFIGURATOR_SRV_SS; puts(sSd);

			STARTMESS(CALL_CFG_UPDATER_SRV);
			callCfgUpdaterSrv.SetConfig(m_srv_start_mode);
			if (!tr->AddService(CALL_CFG_UPDATER_SRV, &callCfgUpdaterSrv) || !callCfgUpdaterSrv.SetThread()) {
				puts(sFd); return false;
			}
			ss = CALL_CFG_UPDATER_SS; puts(sSd);

			std::vector<std::pair<const char*, const VS_TransportRouterServiceBase*>> srvs{
				{ CHECKLIC_SRV, chck_lics.get() },
				{ VERIFY_SRV, verifys.get() },
				{ VS_POOL_THREADS_SERVICE_NAME, &pts },
				{ LOG_SRV, &logs },
				{ PRESENCE_SRV, upss.get() },
				{ AUTH_SRV, vcs_as.get() },
				{ CONFIGURATION_SRV, &vcs_cfg },
				{ CALL_CFG_UPDATER_SRV, &callCfgUpdaterSrv},
				{ ADDRESSBOOK_SRV, &abs },
				{ CONFERENCE_SRV, &cs },
				{ CHAT_SRV, &chs },
				{ CHATV2_SRV, &chat_v2_srv },
				{ OFFLINECHAT_SRV, &ochs },
//				{ (char*)PING_SRV, ((VS_TransportRouterServiceBase*)&pngs) },
				{ LOCATE_SRV, &locs },
				{ RESOLVE_SRV, &rslvs },
				{ SERVERCONFIGURATOR_SRV, &configSrv },
                { CALL_CFG_UPDATER_SRV, &callCfgUpdaterSrv }
			};
			printf("Service ThreadID:\n");
			for (const auto& p : srvs)
				std::cout << '\t' << p.first << ": " << p.second->GetProcessingThreadId() << '\n';
		}

		if(vcs_as)
		{
			VS_GatewayStarter *gateway = VS_GatewayStarter::GetInstance();

			vcs_as->m_OnUserLoggedIn.connect(boost::bind(&VS_PresenceService::OnUserLoginEnd_Event, upss.get(), _1, _2) );
			vcs_as->m_OnUserLoggedOut.connect(boost::bind(&VS_PresenceService::OnUserLogoff_Event, upss.get(), _1, _2));
			vcs_as->m_OnUserLoggedIn.connect(boost::bind(&VS_GatewayStarter::OnUserLoginEnd_Event, gateway, _1, _2) );
			vcs_as->m_OnUserLoggedOut.connect(boost::bind(&VS_GatewayStarter::OnUserLogoff_Event, gateway, _1, _2));

		}

		// Load ConfigCorrector data from registry on startup
		VS_CallConfigCorrector::LoadDataFromRegistry();

		tr->DisconnectAllByCondition(
			[](string_view cid, string_view ep, unsigned char hops)
				{
					return hops == 0 || ep != RegServerName;
				});

		if (0 == rp.mode)
		{
			void *buf(0);
			size_t sz(0);
			UCAZA.SerializeAlloc(buf, sz);
			VS_RouterMessage *mess = new VS_RouterMessage(g_tr_endpoint_name.c_str(), CHECKLIC_SRV, 0, g_tr_endpoint_name.c_str(), CHECKLIC_SRV, ~0, buf, sz);
			if (!upss || !upss->PostMes(mess))
				delete mess;
			free(buf);
		}

		return true;
	}
	inline void Destroy( void )
	{

		const char   sSl[] = " successful.";
#define ENDSEQ(x) printf("\t Destroying %-16s: ", (x)); tr->RemoveService((x)); puts(sSl);
		puts( "SRV: Services are exiting..." );
		if(1==m_srv_start_mode || 2 == m_srv_start_mode)
		{
			if (verifys)
				verifys->ResetThread();
			ENDSEQ(VERIFY_SRV);
		}
		else
		{
			if (vcs_as)
			{
				vcs_as->m_OnUserLoggedIn.disconnect_all_slots();
				vcs_as->m_OnUserLoggedOut.disconnect_all_slots();
			}

			if (acs != nullptr)
			{
				acs->RemoveHandler(m_http_handler->HandlerName());
				acs->RemoveHandler(wsHandler.HandlerName());
				acs->RemoveHandler(CircuitsACSHandler);
			}
			if (tlsHandler != nullptr)
			{
				tlsHandler->RemoveHandler(m_http_handler->HandlerName());
				tlsHandler->RemoveHandler(wsHandler.HandlerName());
				tlsHandler->RemoveHandler(CircuitsACSHandler);
			}

			rtspProxy->Close();
			rtspProxy = nullptr;

			VS_FakeEndpointFactory::DeInit();
			VS_FakeClientManager::DeInit();
			proxiesPool->Stop();

			switch (ss)
			{
			case CALL_CFG_UPDATER_SS:
				callCfgUpdaterSrv.ResetThread();
				ENDSEQ(CALL_CFG_UPDATER_SRV)
			case SERVCONFIGURATOR_SRV_SS:
				configSrv.ResetThread();
				ENDSEQ(SERVERCONFIGURATOR_SRV);
			case RESOLVE_SRV_SS:
				rslvs.ResetThread();
				ENDSEQ(RESOLVE_SRV)
			case LOCATOR_SRV_SS:
				locs.ResetThread();
				ENDSEQ(LOCATE_SRV)
			case PING_SRV_SS:
				ENDSEQ(PING_SRV)
			case OFFLINECHAT_SRV_SS:
				ochs.ResetThread();
				ENDSEQ(OFFLINECHAT_SRV)
			case CHAT_SRV_SS:
				chs.ResetThread();
				ENDSEQ(CHAT_SRV)
			case CHATV2_SRV_SS:
				if(chat_v2_srv.IsStarted())
				{
					chat_v2_srv.ResetThread();
					ENDSEQ(CHATV2_SRV);
				}
			case CONFERENCE_SRV_SS:
				cs.ResetThread();
				ENDSEQ(CONFERENCE_SRV)
			case WEBRTC_PEERCONNECTION_SRV_SS:
				if (IsWebRtcSRV)
				{
					webrtc_pc_srv->Shutdown();
					ENDSEQ(WEBRTC_PEERCONNECTION_SRV);
				}
			case ADDRESSBOOK_SRV_SS:
				abs.ResetThread();
				ENDSEQ(ADDRESSBOOK_SRV)
			case CONFIGURATION_SRV_SS:
				vcs_cfg.ResetThread();
				ENDSEQ(CONFIGURATION_SRV)
			case AUTH_SRV_SS:
				vcs_as->ResetThread();
				ENDSEQ(AUTH_SRV)
			case PRESENCE_SRV_SS:
				upss->ResetThread();
				ENDSEQ(PRESENCE_SRV)
			case LOG_SRV_SS:
				logs.ResetThread();
				ENDSEQ(LOG_SRV)
			case SMTPMAILER_SRV_SS:
				ENDSEQ(VS_SMTP_MAILER_SERVICE_NAME);
			case POOLTHREADS_SRV_SS:
				pts.ResetThread();
				ENDSEQ(VS_POOL_THREADS_SERVICE_NAME)
			case VERIFY_SRV_SS:
				if (verifys)
					verifys->ResetThread();
				ENDSEQ(VERIFY_SRV);
			case CHECKLIC_SRV_CC:
				chck_lics->ResetThread();
				ENDSEQ(CHECKLIC_SRV)
			default:
				if (pts.IsValid())
				{
					printf( "\tDestroying Phread was" );
					pts.ResetThread();
					puts( sSl );
				}
				if (g_storage) {
					printf( "\tDestroying Storage was" );
					delete g_storage; g_storage = 0;
					puts( sSl );
				}
			}

			{
				auto stop_f = acs_srv->Stop();
				if (stop_f.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
					dstream1 << "acs::Service::Stop took too long";
				else if (!stop_f.valid())
					dstream1 << "acs::Service::Stop failed";
			}

			if (m_roaming_settings)
				m_roaming_settings->Stop();

		}
		ss = NONE_SS; sr = 0; tr = 0;
	}

	bool Test( void )
	{
		return g_storage!=0;
	};

	void OnTransceiverStarted(const std::string& transceiverName) const{
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

			auto streams_circuit = proxiesPool->RestoreProxyByTransName(transceiverName, conf.m_name.m_str);
			if (!streams_circuit) return;

			streams_circuit->RestoreConferences(to_restore, confs_users);
		}
	}
};

VS_VCSServices::VS_VCSServices(boost::asio::io_service& ios)
{
	imp = new VS_VCSServices_Implementation(ios);
}
VS_VCSServices::~VS_VCSServices()
{
	if(imp)
		delete imp;
}

bool VS_VCSServices::IsValid() const
{
	return imp;
}

stream::ConferencesConditions* VS_VCSServices::GetConferencesConditions()
{
	return !imp ? 0 : &imp->cs;
}

bool VS_VCSServices::Init(vs::RegistrationParams&& rp, VS_TransportRouter* tr, stream::Router* sr, VS_AccessConnectionSystem* acs,
	VS_TlsHandler* tlsHandler, VS_RoutersWatchdog* watchdog, const char* ver, VS_Container& UCAZA)
{
	return !imp ? false : imp->Init(std::move(rp), tr, sr, acs, tlsHandler, watchdog, ver, UCAZA);
}
void VS_VCSServices::Destroy( void )
{
	if (imp)
		imp->Destroy();
}

bool VS_VCSServices::Test( void)
{
	return imp?imp->Test():false;
};
VS_AccessConnectionSystem* VS_VCSServices::GetAcs() const
{
	return !imp ? 0 : imp->acs;
}
stream::Router *VS_VCSServices::GetStreamsRouter() const
{
	return !imp ? 0 : imp->sr;
}
VS_TransportRouter *VS_VCSServices::GetTransportRouter() const
{
	return !imp ? 0 : imp->tr;
}
std::shared_ptr<VS_TranscoderLogin> VS_VCSServices::GetTranscoderLogin() const
{
	return imp->transcoder_login;
}

#endif
