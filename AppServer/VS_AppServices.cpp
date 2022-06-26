#include "VS_AppServices.h"

#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "FakeClient/VS_FakeClientManager.h"
#include "FakeClient/VS_FakeEndpoint.h"
#include "http/handlers/VisicronServiceStatus.h"
#include "ServerServices/VS_HttpHandler_v2.h"
#include "ServerServices/VS_PingService.h"
#include "ServerServices/VS_VerificationService.h"
#include "Services/VS_AppConfigurationService.h"
#include "Services/VS_AppConfRestrict.h"
#include "Services/VS_AppManagerService.h"
#include "Services/VS_AuthService.h"
#include "Services/VS_ChatService.h"
#include "Services/VS_HomeForwarderService.h"
#include "Services/VS_MultiConfService.h"
#include "Services/VS_PresenceService.h"
#include "Services/VS_ResolveService.h"
#include "statuslib/VS_ResolveServerFinder.h"
#include "std/cpplib/MakeShared.h"
#include "std/cpplib/VS_Policy.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "streams/Relay/VS_StreamsRelay.h"
#include "tools/Server/VS_Server.h"
#include "TransceiverLib/TransceiverConfiguration.h"
#include "TransceiverLib/TransceiverHandlerName.h"
#include "TransceiverLib/TransceiversPool.h"
#include "TransceiverLib/VS_TransceiverCircuitHandler.h"
#include "transport/Router/VS_PoolThreadsService.h"
#include "TrueGateway/VS_GatewayStarter.h"
#include "WebSocket/VS_WsHandler.h"
#include "AppServer/version.h"

#include <boost/make_shared.hpp>
#include <iostream>
#include "ServerServices/utils/VS_ASUtils.h"


VS_ACS_Response IsWsHandlerProtocol(const void *in_buffer, unsigned long *in_len)
{
	return VS_WsHandler::Protocol(in_buffer, in_len);
}

struct VS_AppServices_Implementation
{
	enum StartUpSequence {
		NONE_SS = 0,
		VERIFY_SRV_SS,
		POOLTHREADS_SRV_SS,
		LOG_SRV_SS,
		OFFLINECHAT_SRV_SS,
		PRESENCE_SRV_SS,
		MANAGER_SRV_SS,
		AUTH_SRV_SS,
		CONFIGURATION_SRV_SS,
		ADDRESSBOOK_SRV_SS,
		CONFERENCE_SRV_SS,
		CHAT_SRV_SS,
		PING_SRV_SS,
		RESOLVE_SRV_SS,
		TORRENT_SRV_SS,
		MULTIGATEWAY_SRV_SS
	};

	static const int RESTART_TIME	=5*60;//5 minutes

	VS_AppServices_Implementation( boost::asio::io_service& _ios ) :ios(_ios), tr(0), sr(0), acs(0), ss(NONE_SS), m_srv_start_mode(0)
		,proxiesPool(std::make_shared<ts::ProxiesPool>(ios, ts::GetMinAvailableTransceivers(), ts::GetMaxConferencesByOneTransceiver(), ts::GetMaxFreeTimeForTransceiver()))
		, transHandler(std::static_pointer_cast<VS_SetConnectionInterface>(proxiesPool)) {}
	~VS_AppServices_Implementation( void ) {Destroy();}

	boost::asio::io_service& ios;
	VS_TransportRouter *tr;
	stream::Router *sr;
	VS_AccessConnectionSystem	*acs;
	VS_TlsHandler	*tlsHandler;

	StartUpSequence	ss;

	VS_PoolThreadsService			pts;		//sync
	boost::shared_ptr<VS_PresenceService>	upss;		//async
	VS_AppManagerService			ms;			//async
	VS_AuthService					as;			//async
	VS_HomeForwarderService			fwd_abs,fwd_log,fwd_offline_chat,fwd_torrent;		//async
	VS_MultiConfService				cs;			//async
	VS_ChatService					chs;		//async
	VS_PingService					pngs;
	VS_ResolveService				rslvs;
	VS_AppConfigurationService		confs;
	std::unique_ptr<VS_VerificationService>			verifys;

	std::shared_ptr<http::Router>			m_http_router;
	std::unique_ptr<VS_HttpHandler_v2>		m_http_handler;
	VS_WsHandler							wsHandler;

	long m_srv_start_mode;

	std::shared_ptr<vs_relaymodule::VS_StreamsRelay>		streams_relay;
	std::shared_ptr<ts::ProxiesPool>					proxiesPool;
	VS_TransceiverCircuitHandler						transHandler;
	std::shared_ptr<VS_TranscoderLogin>               transcoder_login;


	inline bool Init(vs::RegistrationParams&& rp, VS_TransportRouter* tr, stream::Router* sr,
		VS_AccessConnectionSystem* acs, VS_TlsHandler* tlsHandler,
		VS_RoutersWatchdog* watchdog, const char* ver)
	{
		if (ss != NONE_SS)		return false;
		VS_AppServices_Implementation::tr = tr;
		VS_AppServices_Implementation::sr = sr;
		VS_AppServices_Implementation::acs = acs;
		VS_AppServices_Implementation::tlsHandler = tlsHandler;

		std::string user_agent = VS_Server::LongName();
		user_agent += " ";
		user_agent += VS_Server::ProductVersion();
		m_http_router = std::make_shared<http::Router>();
		m_http_router->AddHandler("GET /vsstatus", std::make_shared<http::handlers::VisicronServiceStatus>(user_agent));

		m_http_handler = vs::make_unique<VS_HttpHandler_v2>(ios);
		m_http_handler->SetHttpRouter(m_http_router);
		acs->AddHandler("Handler Of HTTP Tunneling Requests.", m_http_handler.get());
		tlsHandler->AddHandler("Handler Of HTTP Tunneling Requests.", m_http_handler.get());

		puts( "SRV: Services are starting..." );
		const char   sSd[] = "\t started successfully.", sFd[] = "\t failed.";
#define STARTMESS(x) printf("\t %-20s: ", (x))

		m_srv_start_mode = rp.mode;

		VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
		if (!cfg_root.IsValid()) {
			puts("Registry Configuration key not found");
			return false;
		}
		verifys = vs::make_unique<VS_VerificationService>(std::move(rp));
		if(m_srv_start_mode == 1 || m_srv_start_mode == 2)
		{
// --------------------------------------------------------------------------------
			// todo(kt): mode m_rp
			STARTMESS(VERIFY_SRV);
			verifys->SetComponents(watchdog, MANAGER_SRV);
			if (!tr->AddService(VERIFY_SRV, verifys.get()) || !verifys->SetThread()){
				puts( sFd ); return false;
			}
			ss = VERIFY_SRV_SS; puts(sSd);
// --------------------------------------------------------------------------------
		}
		else
		{
			acs->AddHandler("WebSock protocol", &wsHandler);
			tlsHandler->AddHandler("WebSock protocol", &wsHandler);

			VS_FakeClientManager::Init(ios);
			VS_FakeEndpointFactory::InitV1(tr->GetSetConnectionHandler(), tr->EndpointName());

			upss = boost::make_shared<VS_PresenceService>(ios);

			cs.SetStreamRouter(sr);
			cs.SetIOservice(ios);
			auto confResctrict = boost::make_shared<VS_AppConfRestrict>();
			{
				auto default_resolve_func = VS_ResolveServerFinder::GetResolverByDomainFunc();
				VS_ResolveServerFinder::SetResolverByDomainFunc([default_resolve_func](const std::string &domain, std::string &bs) -> bool
				{
					std::string value;
					if (VS_RegistryKey(false, CONFIGURATION_KEY, false, true).GetString(value, "Debug HomeBS") && !value.empty())
					{
						bs = std::move(value);
						return true;
					}
					return default_resolve_func(domain, bs);
				});
			}


			cs.SetConfRestrict(confResctrict);
		    rslvs.SetConfRestrict(confResctrict);
		    chs.SetConfRestrict(confResctrict);
			upss->SetConfRestrict(confResctrict);
			cs.SetPresenceService(upss);
			rslvs.SetPresenceService(upss);		rslvs.RegAtPresSRV();
//			chs.SetPresenceService(upss);
			ms.SetPresenceService(upss);


			// StreamsRelay init
			proxiesPool->Init(acs);
			cs.SetTransceiversPool(proxiesPool);
			chs.SetTransceiversPool(proxiesPool);
			if (!acs->AddHandler(CircuitsACSHandler, &transHandler))	return false;
			tlsHandler->AddHandler(CircuitsACSHandler, &transHandler);
			// Create VS_TranscoderLogin

			transcoder_login = vs::MakeShared<VS_TranscoderLogin>();

			// -------------------------


	// --------------------------------------------------------------------------------
			// todo(kt): mode m_rp
			STARTMESS(VERIFY_SRV);
			verifys->SetComponents(watchdog, MANAGER_SRV);
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
			printf( "\tStarting Storage " );
			g_storage=new VS_Storage(pts.OurEndpoint());
			g_storage->SetConfRestrict(confResctrict);
			int error_code=g_storage->Init();
			if (error_code)
			{
				printf( "failed with code %d\n",error_code);
				delete g_storage; g_storage = 0;
				return false;
			}

	// --------------------------------------------------------------------------------

			STARTMESS(LOG_SRV);
			if (!tr->AddService(LOG_SRV, &fwd_log) || !fwd_log.SetThread()) {
				puts(sFd); return false;
			}
			ss = LOG_SRV_SS; puts(sSd);

			STARTMESS(OFFLINECHAT_SRV);
			if (!tr->AddService(OFFLINECHAT_SRV, &fwd_offline_chat) || !fwd_offline_chat.SetThread()) {
				puts(sFd); return false;
			}
			ss = OFFLINECHAT_SRV_SS; puts(sSd);

			STARTMESS(PRESENCE_SRV);
			if (!tr->AddService(PRESENCE_SRV, upss.get()) || !upss->SetThread()) {
				puts(sFd); return false;
			}
			ss = PRESENCE_SRV_SS; puts(sSd);

			utils::init_gateway_ext_funcs_for_AS(upss);

			STARTMESS(MANAGER_SRV);
			ms.SetComponents(sr, watchdog, ver);
			if (!tr->AddService(MANAGER_SRV, &ms) || !ms.SetThread() || !watchdog->AddTestable(&ms, 11)) {
				puts(sFd);
				watchdog->Restart(RESTART_TIME);
				return false;
			}
			ss = MANAGER_SRV_SS; puts(sSd);

			STARTMESS(AUTH_SRV);
			if (!tr->AddService(AUTH_SRV, &as, true, true) || !as.SetThread()) {
				puts(sFd); return false;
			}
			ss = AUTH_SRV_SS; puts(sSd);

			STARTMESS(CONFIGURATION_SRV);
			if (!tr->AddService(CONFIGURATION_SRV, &confs, true, true) || !confs.SetThread()) {
				puts(sFd); return false;
			}
			ss = CONFIGURATION_SRV_SS; puts(sSd);

			STARTMESS(ADDRESSBOOK_SRV);
			if (!tr->AddService(ADDRESSBOOK_SRV, &fwd_abs, true, true) || !fwd_abs.SetThread()) {
				puts(sFd); return false;
			}
			ss = ADDRESSBOOK_SRV_SS; puts(sSd);

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

			STARTMESS(PING_SRV);
			if (!tr->AddCallService(PING_SRV, &pngs))
			{	puts( sFd );	return false;	}
			ss = PING_SRV_SS;	puts( sSd );

			STARTMESS(RESOLVE_SRV);
			if (!tr->AddService(RESOLVE_SRV, &rslvs) || !rslvs.SetThread()) {
				puts( sFd ); return false;
			}
			ss = RESOLVE_SRV_SS; puts(sSd);

			STARTMESS(TORRENT_SRV);
			if (!tr->AddService(TORRENT_SRV, &fwd_torrent) || !fwd_torrent.SetThread()) {
				puts(sFd); return false;
			}
			ss = TORRENT_SRV_SS; puts(sSd);

			std::vector<std::pair<const char*, const VS_TransportRouterServiceBase*>> srvs{
				{ VERIFY_SRV, verifys.get() },
				{ VS_POOL_THREADS_SERVICE_NAME, &pts },
				{ LOG_SRV, &fwd_log },
				{ OFFLINECHAT_SRV, &fwd_offline_chat },
				{ PRESENCE_SRV, upss.get() },
				{ MANAGER_SRV, &ms },
				{ AUTH_SRV, &as },
				{ CONFIGURATION_SRV, &confs },
				{ ADDRESSBOOK_SRV, &fwd_abs },
				{ CONFERENCE_SRV, &cs },
				{ CHAT_SRV, &chs },
//				{ (char*)PING_SRV, ((VS_TransportRouterSimpleService*)&pngs) },
				{ RESOLVE_SRV, &rslvs },
				{ TORRENT_SRV, &fwd_torrent }
			};
			printf("Service ThreadID:\n");
			for (const auto& p : srvs)
				std::cout << '\t' << p.first << ": " << p.second->GetProcessingThreadId() << '\n';

			unsigned long dont_start_gw(0);
			cfg_root.GetValue(&dont_start_gw,sizeof(dont_start_gw),VS_REG_INTEGER_VT,"dont_start_gateway");
			VS_GatewayStarter::SetIsAS(true);
			if(dont_start_gw == 0)
			{
				VS_GatewayStarter *gw_starter = VS_GatewayStarter::GetInstance();
				if (gw_starter)
				{
					const std::string serverInfo = VS_TRUECONF_WS_DISPLAY_NAME " " STRPRODUCTVER;
					std::function<bool(const std::string&, const std::string&)> empty_check_digest;
					if(!gw_starter->IsStarted())
						gw_starter->StartGateway(ios, tr, empty_check_digest, proxiesPool, transcoder_login, serverInfo);
					gw_starter->SetMaxTranscoders(VS_License::TC_INFINITY);
					if(acs)
					{
						std::string addr;
						acs->GetListeners(addr);
						gw_starter->SetServerAddresses(addr.c_str());
					}

					as.SetTransLogin(transcoder_login);
					as.m_OnUserLoggedIn.connect( boost::bind(&VS_GatewayStarter::OnUserLoginEnd_Event, gw_starter, _1, _2) );
					as.m_OnUserLoggedOut.connect( boost::bind(&VS_GatewayStarter::OnUserLogoff_Event, gw_starter, _1, _2) );
					as.m_OnNewPeerCfg.connect( boost::bind(&VS_GatewayStarter::OnNewPeerCfg_Event, gw_starter, _1, _2) );
				}
			}

			as.m_OnUserLoggedIn.connect( boost::bind(&VS_PresenceService::OnUserLoginEnd_Event, upss.get(), _1, _2) );
			as.m_OnUserLoggedOut.connect( boost::bind(&VS_PresenceService::OnUserLogoff_Event, upss.get(), _1, _2) );


		}
		return true;
	}

	inline void Destroy( void )
	{
		const char   sSl[] = " successful.";
#define ENDSEQ(x) printf("\t Destroying %-16s: ", (x)); tr->RemoveService((x)); puts(sSl);
		puts( "SRV: Services are exiting..." );
		if(1==m_srv_start_mode)
		{
			if (verifys)
				verifys->ResetThread();
			ENDSEQ(VERIFY_SRV);
		}
		else
		{
			as.m_OnUserLoggedIn.disconnect_all_slots();
			as.m_OnUserLoggedOut.disconnect_all_slots();
			as.m_OnNewPeerCfg.disconnect_all_slots();

			VS_GatewayStarter *gw_starter = VS_GatewayStarter::GetInstance();
			if (gw_starter)
				gw_starter->StopGateway();

			acs->RemoveHandler(m_http_handler->HandlerName());
			acs->RemoveHandler(wsHandler.HandlerName());
			acs->RemoveHandler(CircuitsACSHandler);

			tlsHandler->RemoveHandler(m_http_handler->HandlerName());
			tlsHandler->RemoveHandler(wsHandler.HandlerName());
			tlsHandler->RemoveHandler(CircuitsACSHandler);

			VS_FakeEndpointFactory::DeInit();
			VS_FakeClientManager::DeInit();
			proxiesPool->Stop();

			switch (ss)
			{
			case TORRENT_SRV_SS:
				fwd_torrent.ResetThread();
				ENDSEQ(TORRENT_SRV)
			case RESOLVE_SRV_SS:
				rslvs.ResetThread();
				ENDSEQ(RESOLVE_SRV)
			case PING_SRV_SS:
				ENDSEQ(PING_SRV)
			case CHAT_SRV_SS:
				chs.ResetThread();
				ENDSEQ(CHAT_SRV)
			case CONFERENCE_SRV_SS:
				cs.ResetThread();
				ENDSEQ(CONFERENCE_SRV)
			case ADDRESSBOOK_SRV_SS:
				fwd_abs.ResetThread();
				ENDSEQ(ADDRESSBOOK_SRV)
			case CONFIGURATION_SRV_SS:
				confs.ResetThread();
				ENDSEQ(CONFIGURATION_SRV)
			case AUTH_SRV_SS:
				as.ResetThread();
				ENDSEQ(AUTH_SRV)
			case MANAGER_SRV_SS:
				ms.ResetThread();
				ENDSEQ(MANAGER_SRV)
			case PRESENCE_SRV_SS:
				upss->ResetThread();
				ENDSEQ(PRESENCE_SRV)
			case OFFLINECHAT_SRV_SS:
				fwd_offline_chat.ResetThread();
				ENDSEQ(OFFLINECHAT_SRV)
			case LOG_SRV_SS:
				fwd_log.ResetThread();
				ENDSEQ(LOG_SRV)
			case POOLTHREADS_SRV_SS:
				pts.ResetThread();
				ENDSEQ(VS_POOL_THREADS_SERVICE_NAME)
			case VERIFY_SRV_SS:
				if (verifys)
					verifys->ResetThread();
				ENDSEQ(VERIFY_SRV);
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
		}
		ss = NONE_SS; sr = 0; tr = 0;
	}

	bool Test( void )
	{
		return g_storage!=0;
	};

};

//////////////////////////////////////////////////////////////////////////////////////////

VS_AppServices::VS_AppServices(boost::asio::io_service& ios)
{
	VS_Policy::SetIsVCS( false );
	imp = new VS_AppServices_Implementation(ios);
}

VS_AppServices::~VS_AppServices( void ) {		if (imp)	delete imp;		}

bool VS_AppServices::IsValid( void ) const
{	return imp;	}

stream::ConferencesConditions* VS_AppServices::GetConferencesConditions()
{	return !imp ? 0 : &imp->cs;		}

bool VS_AppServices::Init(vs::RegistrationParams&& rp, VS_TransportRouter* tr, stream::Router* sr, VS_AccessConnectionSystem* acs, VS_TlsHandler* tlsHandler, VS_RoutersWatchdog* watchdog, const char* ver)
{
	return !imp ? false : imp->Init(std::move(rp), tr, sr, acs, tlsHandler, watchdog, ver);
}

void VS_AppServices::Destroy( void ) {	if (imp)	imp->Destroy();		}

bool VS_AppServices::Test( void)
{	return imp?imp->Test():false; };

VS_AccessConnectionSystem* VS_AppServices::GetAcs() const
{
	return !imp ? 0 : imp->acs;
}
stream::Router *VS_AppServices::GetStreamsRouter() const
{
	return !imp ? 0 : imp->sr;
}
VS_TransportRouter *VS_AppServices::GetTransportRouter() const
{
	return !imp ? 0 : imp->tr;
}

std::shared_ptr<VS_TranscoderLogin> VS_AppServices::GetTranscoderLogin() const
{
	return imp->transcoder_login;
}

//////////////////////////////////////////////////////////////////////////////////////////
