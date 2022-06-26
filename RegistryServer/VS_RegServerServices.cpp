#include "VS_RegServerServices.h"

#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "http/handlers/VisicronServiceStatus.h"
#include "RegistryServer/Services/VS_ManagerService.h"
#include "RegistryServer/Services/VS_RegDBStorage.h"
#include "RegistryServer/Services/VS_RegistrationService.h"
#include "ServerServices/VS_HttpHandler_v2.h"
#include "ServerServices/VS_PingService.h"
#include "std-generic/compat/memory.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "tools/Server/VS_Server.h"

#include "time.h"
#include <boost/shared_ptr.hpp>

struct VS_RegServerServices_Implementation
{
	enum StartUpSequence
    {
        NONE_SS = 0,
        /*DIRSTORAGE_SS,
		DIRECTORY_SRV_SS,*/
		POOLTHREADS_SRV_SS,
        REGSTORAGE_SS,
        REGISTRATION_SRV_SS,
		MANAGER_SRV_SS,
				PING_SRV_SS
    };

    VS_RegServerServices_Implementation(boost::asio::io_service& _ios)
		: ios(_ios), tr(0), ss(NONE_SS)
	{
		rs = VS_RegistrationService::Make();
	}

    ~VS_RegServerServices_Implementation( void ) {     Destroy();      }

	boost::asio::io_service& ios;
	VS_TransportRouter* tr = nullptr;
	VS_AccessConnectionSystem* acs = nullptr;
	VS_TlsHandler* tlsHandler = nullptr;
	StartUpSequence   ss;

	VS_PoolThreadsService	pts;
	VS_RegStorage*			rstorage;
	boost::shared_ptr<VS_RegistrationService>	rs;
	VS_PingService			pings;
	VS_ManagerService		mgr;

	std::shared_ptr<http::Router>			m_http_router;
	std::unique_ptr<VS_HttpHandler_v2>		m_http_handler;


	inline void STARTMESS(const char *service)
	{
		time_t start_vaule(0);
		time(&start_vaule);
		printf("\t %-20s: at %s", service, ctime(&start_vaule));
	};
    inline bool Init( VS_TransportRouter *tr, VS_AccessConnectionSystem* acs,
                      VS_TlsHandler* tlsHandler, VS_RoutersWatchdog *watchdog)
	{
		if (ss != NONE_SS)      return false;
		VS_RegServerServices_Implementation::tr = tr;
		VS_RegServerServices_Implementation::acs = acs;
		VS_RegServerServices_Implementation::tlsHandler = tlsHandler;

		std::string user_agent = VS_Server::LongName();
		user_agent += " ";
		user_agent += VS_Server::ProductVersion();
		m_http_router = std::make_shared<http::Router>();
		m_http_router->AddHandler("GET /vsstatus", std::make_shared<http::handlers::VisicronServiceStatus>(user_agent));

		m_http_handler = vs::make_unique<VS_HttpHandler_v2>(ios);
		m_http_handler->SetHttpRouter(m_http_router);
		acs->AddHandler("Handler Of HTTP Tunneling Requests.", m_http_handler.get());
		tlsHandler->AddHandler("Handler Of HTTP Tunneling Requests.", m_http_handler.get());

        puts( "RGSRV: Services are starting..." );
        const char   sSd[] = "\t started successfully.", sFd[] = "\t failed.";
		VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
		if (!cfg_root.IsValid()) {
			puts("Configuration registry key not found");
			return false;
		}

		printf( "\tRegistration DB storage starting: " );
        rstorage=new VS_RegDBStorage();
        if (rstorage->error_code || !((VS_RegDBStorage*)rstorage)->Init())
         {
            printf( "failed with code %d\n",rstorage->error_code);
            return false;
         }

		if (!watchdog->AddTestable( rstorage, 13 ))
		{	puts( sFd );	return false;	}
        ss = REGSTORAGE_SS;    puts( sSd );
        printf( "\tRegistration Service starting: " );
				rs->SetStorage(rstorage);

		tr->SetServCertInfoInterface(rstorage);
        if (!tr->AddCallService( REGISTRATION_SRV, rs.get(),true ))
        {   puts( sFd );    return false;   }
        ss = REGISTRATION_SRV_SS;  puts( sSd );

	// --------------------------------------------------------------------------------

		STARTMESS(VS_POOL_THREADS_SERVICE_NAME);
		if (!tr->AddService(VS_POOL_THREADS_SERVICE_NAME, &pts) || !pts.SetThread()) {
			puts( sFd );	return false;
		}
		ss = POOLTHREADS_SRV_SS;	puts( sSd );
// --------------------------------------------------------------------------------


        if (!tr->AddCallService( MANAGER_SRV, &mgr ))
        {   puts( sFd );    return false;   }
        ss = MANAGER_SRV_SS;  puts( sSd );


		printf( "\tPing Service starting: " );
		if (!tr->AddCallService( PING_SRV, &pings ))
        {   puts( sFd );    return false;   }
        ss = PING_SRV_SS;  puts( sSd );

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

        const char   sF[] = "\t\t finished.";
        switch (ss)
        {
        case PING_SRV_SS :
            printf( "\tDestroying Ping Service: " );
            tr->RemoveService( PING_SRV );   puts( sF );
		case MANAGER_SRV_SS :
            printf( "\tDestroying Manager Service: " );
            tr->RemoveService( MANAGER_SRV );   puts( sF );
        case REGISTRATION_SRV_SS :
            printf( "\tDestroying Registration Service: " );
            tr->RemoveService( REGISTRATION_SRV );   puts( sF );
		case POOLTHREADS_SRV_SS:
			pts.ResetThread();
			printf( "\tDestroying Pool Threads service: " );
			tr->RemoveService(VS_POOL_THREADS_SERVICE_NAME); puts( sF );
		case REGSTORAGE_SS :
			tr->SetServCertInfoInterface(0);
			printf( "\tDestroying Registration Storage: " );
			delete rstorage;   rstorage=0;    puts( sF );
        }
        ss = NONE_SS;   tr = 0;
	}
};

VS_RegServerServices::VS_RegServerServices(boost::asio::io_service& ios)
{   imp = new VS_RegServerServices_Implementation(ios);     }

VS_RegServerServices::~VS_RegServerServices( void ) {     if (imp)    delete imp;     }

bool VS_RegServerServices::IsValid( void ) const
{   return imp;  }

bool VS_RegServerServices::Init( VS_TransportRouter *tr, VS_AccessConnectionSystem* acs,
	VS_TlsHandler* tlsHandler, VS_RoutersWatchdog *watchdog)
{   return !imp ? false : imp->Init( tr, acs, tlsHandler, watchdog); }

void VS_RegServerServices::Destroy( void ) {   if (imp)    imp->Destroy();     }
