#include "VS_ServerManagerServices.h"

#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "http/handlers/VisicronServiceStatus.h"
#include "ServerServices/VS_HttpHandler_v2.h"
#include "ServerServices/VS_PingService.h"
#include "Services/VS_ServerManagerService.h"
#include "std-generic/compat/memory.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "tools/Server/VS_Server.h"

struct VS_ServerManagerServices_Implementation
{
	enum StartUpSequence {
		NONE_SS = 0,
		MANAGER_SRV_SS,
		PING_SRV_SS
	};

	static const int RESTART_TIME	=5*60;//5 minutes

	VS_ServerManagerServices_Implementation(boost::asio::io_service& _ios) : ios(_ios), tr(0), ss(NONE_SS)	{}
	~VS_ServerManagerServices_Implementation( void ) {Destroy();}

	boost::asio::io_service& ios;
	VS_TransportRouter* tr = nullptr;
	VS_AccessConnectionSystem* acs = nullptr;
	VS_TlsHandler* tlsHandler = nullptr;
	StartUpSequence	ss;
	VS_ServerManagerService			ms;			//async
	VS_PingService					pngs;

	std::shared_ptr<http::Router>			m_http_router;
	std::unique_ptr<VS_HttpHandler_v2>		m_http_handler;

	inline bool Init( VS_TransportRouter *tr, VS_AccessConnectionSystem* acs, VS_TlsHandler* tlsHandler, VS_RoutersWatchdog* watchdog, const char* ver)
	{
		if (ss != NONE_SS)		return false;
		VS_ServerManagerServices_Implementation::tr = tr;
		VS_ServerManagerServices_Implementation::acs = acs;
		VS_ServerManagerServices_Implementation::tlsHandler = tlsHandler;

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

		VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
		if (!cfg_root.IsValid()) {
			puts("Registry Configuration key not found");
			return false;
		}

// --------------------------------------------------------------------------------

		STARTMESS(MANAGER_SRV);
		tr->SetServCertInfoInterface(&ms);
		if (!tr->AddService(MANAGER_SRV, &ms,true, true)
			|| !ms.SetThread()
			|| !watchdog->AddTestable(&ms, 11))
		{
			puts(sFd);
			watchdog->Restart(RESTART_TIME);
			return false;
		}
		ss = MANAGER_SRV_SS; puts(sSd);

		STARTMESS(PING_SRV);
		if (!tr->AddCallService(PING_SRV, &pngs))
		{	puts( sFd );	return false;	}
		ss = PING_SRV_SS;	puts( sSd );
		return true;
	}

	inline void Destroy( void )
	{
		if (acs != nullptr)
		{
			acs->RemoveHandler(m_http_handler->HandlerName());
		}

		if (tlsHandler != nullptr)
			tlsHandler->RemoveHandler(m_http_handler->HandlerName());

		const char   sSl[] = " successful.";
#define ENDSEQ(x) printf("\t Destroying %-16s: ", (x)); tr->RemoveService((x)); puts(sSl);
		puts( "SRV: Services are exiting..." );
		switch (ss)
		{
		case PING_SRV_SS:
			ENDSEQ(PING_SRV)
		case MANAGER_SRV_SS:
			ms.ResetThread();
			ENDSEQ(MANAGER_SRV)
		}
		ss = NONE_SS; tr = 0;
	}

	bool Test( void )
	{
		return true;//g_storage!=0 && g_storage->Test();
	};

};

//////////////////////////////////////////////////////////////////////////////////////////

VS_ServerManagerServices::VS_ServerManagerServices(boost::asio::io_service& ios)
{	imp = new VS_ServerManagerServices_Implementation(ios);		}

VS_ServerManagerServices::~VS_ServerManagerServices( void ) {		if (imp)	delete imp;		}

bool VS_ServerManagerServices::IsValid( void ) const
{	return imp;	}

bool VS_ServerManagerServices::Init(VS_TransportRouter *tr, VS_AccessConnectionSystem* acs,
	VS_TlsHandler* tlsHandler, VS_RoutersWatchdog* watchdog, const char* ver)
{	return !imp ? false : imp->Init( tr, acs, tlsHandler, watchdog, ver);	}

void VS_ServerManagerServices::Destroy( void ) {	if (imp)	imp->Destroy();		}

bool VS_ServerManagerServices::Test( void)
{	return imp?imp->Test():false; };

//////////////////////////////////////////////////////////////////////////////////////////
