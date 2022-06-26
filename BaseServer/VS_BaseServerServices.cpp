#ifdef _WIN32 // not ported
#include "VS_BaseServerServices.h"

#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "acs/VS_AcsDefinitions.h"
#include "SmtpMail/SmtpMailer/VS_SmtpMailer.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std-generic/compat/memory.h"
#include "transport/Router/VS_PoolThreadsService.h"
#include "ServerServices/VS_SmtpMailerService.h"
#include "ServerServices/VS_VerificationService.h"
#include "ServerServices/VS_TorrentStarter.h"
#include "Services/storage/VS_DBStorage_Conferendo.h"
#include "Services/storage/VS_DBStorage_TrueConf.h"
#include "Services/VS_BSConfRestrict.h"
#include "Services/VS_BSLogService.h"
#include "Services/VS_BaseAuthService.h"
#include "Services/VS_ConfigurationService.h"
#include "Services/VS_AddressBookService.h"
#include "Services/VS_LocatorService.h"
#include "Services/VS_BasePresenceService.h"
#include "Services/VS_ChatService.h"
#include "Services/VS_ManagerService.h"
#include "Services/storage/VS_BSABStorage.h"
#include "Services/VS_BaseConferenceService.h"
#include "Services/storage/VS_LocatorStorage.h"
#include "Services/storage/VS_LocatorStorageImplMemory.h"
#include "Services/storage/VS_LocatorStorageImplAdoDB.h"
#include "Services/storage/VS_TRStorageAdo.h"

#include "http/Router.h"
#include "http/handlers/TorrentAnnounce.h"
#include "http/handlers/VisicronServiceStatus.h"
#include "ServerServices/VS_HttpHandler_v2.h"

#include <boost/make_shared.hpp>

#include <iostream>

struct VS_BaseServerServices_Implementation
{
	enum StartUpSequence {
		NONE_SS = 0,
		POOLTHREADS_SRV_SS,
		SMTPMAILER_SRV_SS,
		MANAGER_SRV_SS,
		LOG_SRV_SS,
		PRESENCE_SRV_SS,
		AUTH_SRV_SS,
		OFFLINECHAT_SRV_SS,
		CONFIGURATION_SRV_SS,
		ADDRESSBOOK_SRV_SS,
		CONFERENCE_SRV_SS,
		VERIFY_SRV_SS,
		LOCATOR_SRV_SS,
		TORRENT_SRV_SS
	};
	long m_srv_start_mode;

	static const int RESTART_TIME	=5*60;//5 minutes

	VS_BaseServerServices_Implementation( void ) : tr(0), ss(NONE_SS),m_srv_start_mode(0)	{}
	~VS_BaseServerServices_Implementation( void ) {Destroy();}

	VS_AccessConnectionSystem *acs;
	VS_TlsHandler *tlsHandler;
	VS_TransportRouter *tr;
	StartUpSequence	ss;
	VS_PoolThreadsService	pts;		//sync
	VS_SmtpMailerService	sms;		//sync
	boost::shared_ptr<VS_BSLogService>			logs;		//sync
	boost::shared_ptr<VS_BasePresenceService>	pres;		//sync
	VS_BaseAuthService		auths;		//sync
	boost::shared_ptr<VS_BSChatService>			chs;		//async
	VS_ConfigurationService	cfg;		//sync
	VS_AddressBookService	abs;		//sync
	std::shared_ptr<VS_BaseConferenceService> confs;		//sync
	VS_LocatorService		locs;		//sync
	VS_ManagerService		ms;
	std::unique_ptr<VS_VerificationService>	verifys;
	std::shared_ptr<VS_TorrentStarter>			torrent_starter;
	std::shared_ptr<http::Router>				m_http_router = std::make_shared<http::Router>();
	std::unique_ptr<VS_HttpHandler_v2>			m_http_handler;

	inline bool Init(vs::RegistrationParams&& rp, boost::asio::io_service& ios, VS_TransportRouter *tr,
		VS_RoutersWatchdog* watchdog, VS_AccessConnectionSystem *acs, VS_TlsHandler* tlsHandler,
		const char* ver, const char* server_id)
	{
		torrent_starter = std::make_shared<VS_TorrentStarter>(ios);

		if (ss != NONE_SS || !server_id)		return false;
		VS_BaseServerServices_Implementation::acs = acs;
		VS_BaseServerServices_Implementation::tlsHandler = tlsHandler;
		VS_BaseServerServices_Implementation::tr = tr;
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
			auto confRestrict = boost::make_shared<VS_BSConfRestrict>();
			confRestrict->SetTorrentStarter(torrent_starter);

			std::string user_agent = VS_Server::LongName();
			user_agent += " ";
			user_agent += VS_Server::ProductVersion();
			m_http_router->AddHandler("GET /announce", std::make_shared<http::handlers::TorrentAnnounce>(torrent_starter));
			m_http_router->AddHandler("GET /vsstatus", std::make_shared<http::handlers::VisicronServiceStatus>(user_agent));
			m_http_handler = std::make_unique<VS_HttpHandler_v2>(ios);
			m_http_handler->SetHttpRouter(m_http_router);

			acs->AddHandler("HttpHandlerV2", m_http_handler.get());
			tlsHandler->AddHandler("HttpHandlerV2", m_http_handler.get());

			// storage ------------------------------------------------------------------------
			printf("\t DB Storage ");
			unsigned long IsConferendoBS = false;
			cfg_root.GetValue(&IsConferendoBS, sizeof(IsConferendoBS), VS_REG_INTEGER_VT, "IsConferendoBS");
			std::shared_ptr<VS_DBStorage> storage;
			if (IsConferendoBS)
				storage = std::make_shared<VS_DBStorage_Conferendo>();
			else
				storage = std::make_shared<VS_DBStorage_TrueConf>();
			g_dbStorage = storage;
			if (!g_dbStorage->Init(server_id) || g_dbStorage->error_code != 0) {
				printf("failed with code %d\n", g_dbStorage->error_code);
				g_dbStorage.reset();
				watchdog->Restart(RESTART_TIME);
				return false;
			}
			VS_LocatorStorageImplAdoDB::SetDBOPool(storage->dbo_pool());
			if (!watchdog->AddTestable(g_dbStorage.get(), 12)) {
				puts(sFd); return false;
			}
			puts(sSd);
			// --------------------------------------------------------------------------------

			{
				std::string resolve_server;
				if (cfg_root.GetString(resolve_server, "Debug Locator") && !resolve_server.empty())
				{
					VS_LocatorStorage::Instance(std::make_unique<VS_LocatorStorageImplMemory>(resolve_server.c_str()));
					VS_ResolveServerFinder::SetResolverByDomainFunc([resolve_server](const std::string &domain, std::string &bs) -> bool
					{
						bs = resolve_server;
						return true;
					});
				}
				else
					VS_LocatorStorage::Instance(); // default locator
			}
			pres = boost::make_shared<VS_BasePresenceService>();
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

			STARTMESS(VS_SMTP_MAILER_SERVICE_NAME);
			if (!tr->AddService( VS_SMTP_MAILER_SERVICE_NAME, &sms ) || !sms.SetThread())
			{	puts( sFd );	return false;	}
			ss = SMTPMAILER_SRV_SS;		puts( sSd );

			STARTMESS(LOG_SRV);

			logs = boost::signals2::deconstruct<VS_BSLogService>();
			if (!tr->AddService(LOG_SRV, logs.get()) || !logs->SetThread()) {
				puts(sFd); return false;
			}
			ss = LOG_SRV_SS; puts(sSd);

			STARTMESS(MANAGER_SRV);
			ms.SetComponents(watchdog, ver);
			if (!tr->AddService(MANAGER_SRV, &ms) || !ms.SetThread() || !watchdog->AddTestable(&ms, 11)) {
				puts(sFd);
				watchdog->Restart(RESTART_TIME);
				return false;
			}
			ss = MANAGER_SRV_SS; puts(sSd);

			STARTMESS(PRESENCE_SRV);
			if(!tr->AddService(PRESENCE_SRV,pres.get()) || !pres->SetThread()){
				puts(sFd); return false;
			}
			ss = PRESENCE_SRV_SS; puts(sSd);
			tr->DisconnectAllEndpoints(); // force reconnect for getting all user statuses and subscriptions
			g_BasePresenceService = pres;

			if (torrent_starter) {
				boost::weak_ptr<VS_BasePresenceService> presence_weak = pres;
				torrent_starter->SetGetServerID([presence_weak](const char *to_user) -> std::string {
					auto presense = presence_weak.lock();
					if (!presense || !to_user) return "";

					auto id_info = presense->GetStatus(to_user);
					if (id_info.m_status > USER_LOGOFF) {
						return id_info.m_serverID;
					}
					return "";
				});
			}

			STARTMESS(AUTH_SRV);
			if (!tr->AddService(AUTH_SRV, &auths) || !auths.SetThread()) {
				puts(sFd); return false;
			}
			ss = AUTH_SRV_SS; puts(sSd);

			STARTMESS(OFFLINECHAT_SRV);
			chs = boost::make_shared<VS_BSChatService>();
			if (!tr->AddService(OFFLINECHAT_SRV, chs.get()) || !chs->SetThread()) {
				puts(sFd); return false;
			}
			ss = OFFLINECHAT_SRV_SS; puts(sSd);


			STARTMESS(CONFIGURATION_SRV);
			if (!tr->AddService(CONFIGURATION_SRV, &cfg) || !cfg.SetThread()) {
				puts(sFd); return false;
			}
			ss = CONFIGURATION_SRV_SS; puts(sSd);

			STARTMESS(ADDRESSBOOK_SRV);
			boost::shared_ptr<VS_ABStorageInterface> ab_storage(new VS_BSABStorage());
			abs.SetABStorage(ab_storage);
			abs.SetConfRestrict(confRestrict);
			if (!tr->AddService(ADDRESSBOOK_SRV, &abs) || !abs.SetThread()) {
				puts( sFd ); return false;
			}
			ss = ADDRESSBOOK_SRV_SS; puts(sSd);

			STARTMESS(CONFERENCE_SRV);
			confs = std::make_shared<VS_BaseConferenceService>(ios);
			if (!tr->AddService(CONFERENCE_SRV, confs.get()) || !confs->SetThread()) {
					puts( sFd ); return false;
			}
			ss = CONFERENCE_SRV_SS; puts(sSd);

			STARTMESS(LOCATE_SRV);
			if (!tr->AddService(LOCATE_SRV, &locs)
				|| !locs.SetThread()) {
				puts( sFd ); return false;
			}
			ss = LOCATOR_SRV_SS; puts(sSd);

			std::vector<std::pair<const char*, const VS_TransportRouterServiceBase*>> srvs{
				{ VERIFY_SRV, verifys.get() },
				{ VS_POOL_THREADS_SERVICE_NAME, &pts},
				{ LOG_SRV, logs.get()},
				{ MANAGER_SRV, &ms },
				{ PRESENCE_SRV, pres.get() },
				{ AUTH_SRV, &auths },
				{ OFFLINECHAT_SRV, chs.get() },
				{ CONFIGURATION_SRV, &cfg },
				{ ADDRESSBOOK_SRV, &abs },
				{ CONFERENCE_SRV, confs.get() },
				{ LOCATE_SRV, &locs }
			};
			printf("Service ThreadID:\n");
			for (const auto& p : srvs)
				std::cout << '\t' << p.first << ": " << p.second->GetProcessingThreadId() << '\n';

			auto db = std::dynamic_pointer_cast<VS_DBStorage>(g_dbStorage);
			if (db) {
				std::shared_ptr<VS_TRStorageInterface> tr_storage;
				try {
					tr_storage = std::make_shared<VS_TRStorageAdo>(db->dbo_pool());
				} catch (std::runtime_error &e) {
					printf("Error Creating VS_TRStorageAdo: %s\n", e.what());
				}
				if (!torrent_starter || !torrent_starter->Start(tr, tr_storage)) {
					return false;
				}
				ss = TORRENT_SRV_SS;
			}
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
			if (acs)
			{
				if (m_http_handler)
				{
					acs->RemoveHandler(m_http_handler->HandlerName());
					tlsHandler->RemoveHandler(m_http_handler->HandlerName());
				}
			}

			switch (ss)
			{
			case TORRENT_SRV_SS: {
				if (torrent_starter) {
					torrent_starter->Stop(tr);
				}
			}
			case LOCATOR_SRV_SS:
				locs.ResetThread();
				ENDSEQ(LOCATE_SRV)

			case CONFERENCE_SRV_SS:
				if (confs)
					confs->ResetThread();
				ENDSEQ(CONFERENCE_SRV)

			case ADDRESSBOOK_SRV_SS:
				abs.ResetThread();
				ENDSEQ(ADDRESSBOOK_SRV)

			case CONFIGURATION_SRV_SS:
				cfg.ResetThread();
				ENDSEQ(CONFIGURATION_SRV)

			case OFFLINECHAT_SRV_SS:
				if (chs)
					chs->ResetThread();
				ENDSEQ(OFFLINECHAT_SRV)

			case AUTH_SRV_SS:
				auths.ResetThread();
				ENDSEQ(AUTH_SRV)

			case PRESENCE_SRV_SS:
				pres->ResetThread();
				g_BasePresenceService.reset();
				ENDSEQ(PRESENCE_SRV)

					/**
					destroy
					*/
			case MANAGER_SRV_SS:
				ms.ResetThread();
				ENDSEQ(MANAGER_SRV)

			case LOG_SRV_SS:
				logs->ResetThread();
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

			default:
				if (pts.IsValid())
				{
					printf( "\tDestroying Phread was" );
					pts.ResetThread();
					puts( sSl );
				}
			}
		}
		ss = NONE_SS; tr = 0;
	}

	bool Test( void )
	{
		return !!g_dbStorage && g_dbStorage->Test();
	};

};

//////////////////////////////////////////////////////////////////////////////////////////

VS_BaseServerServices::VS_BaseServerServices( void )
{	imp = new VS_BaseServerServices_Implementation;		}

VS_BaseServerServices::~VS_BaseServerServices( void ) {		if (imp)	delete imp;		}

bool VS_BaseServerServices::IsValid( void ) const
{	return imp; }

stream::ConferencesConditions* VS_BaseServerServices::GetConferencesConditions()
{	return !imp ? 0 : /*&imp->cs;*/	nullptr;	}

bool VS_BaseServerServices::Init(vs::RegistrationParams&& rp, boost::asio::io_service& ios,
	VS_TransportRouter *tr, VS_RoutersWatchdog* watchdog, VS_AccessConnectionSystem *acs,
	VS_TlsHandler* tlsHandler, const char* ver, const char* server_id)
{
	return !imp ? false : imp->Init(std::move(rp), ios, tr, watchdog, acs, tlsHandler, ver, server_id);
}

void VS_BaseServerServices::Destroy( void ) {	if (imp)	imp->Destroy();		}

bool VS_BaseServerServices::Test( void)
{	return imp?imp->Test():false; };

//VS_ServerComponentsInterface
VS_AccessConnectionSystem * VS_BaseServerServices::GetAcs() const {
	return imp->acs;
}

stream::Router * VS_BaseServerServices::GetStreamsRouter() const {
	return nullptr;
}

VS_TransportRouter * VS_BaseServerServices::GetTransportRouter() const {
	return imp->tr;
}

std::shared_ptr<VS_TranscoderLogin> VS_BaseServerServices::GetTranscoderLogin() const {
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
#endif