/**
 **************************************************************************
 * \file MediaBroker.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Startup file for 3 broker products
 *
 * \b Project AppServer
 * \author stass
 * \date 03.10.07
 *
 * $Revision: 10 $
 *
 * $History: AppServer.cpp $
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 10.12.10   Time: 21:24
 * Updated in $/VSNA/Servers/AppServer
 *  - don't open ports in registration mode
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 6-12-10    Time: 17:42
 * Updated in $/VSNA/Servers/AppServer
 * - registration from command line vs_appSrv /mode:1 /ServerName:name
 * [/ServerID:id] [/Serial:serial]
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 1.11.10    Time: 21:00
 * Updated in $/VSNA/Servers/AppServer
 * - cert update added
 * - registration from server added
 * - authorization servcer added
 *
 * *****************  Version 7  *****************
 * User: Ktrushnikov  Date: 16.01.09   Time: 15:52
 * Updated in $/VSNA/Servers/AppServer
 * - save endpoints in map: alfa-release!
 * - increase endpoints max from 10200 to 20200
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 24.10.08   Time: 21:12
 * Updated in $/VSNA/Servers/AppServer
 * - transport ping increased
 * - loging in conference corrected
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 17.10.08   Time: 14:43
 * Updated in $/VSNA/Servers/AppServer
 * - new time for transport ping
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 25.03.08   Time: 17:50
 * Updated in $/VSNA/Servers/AppServer
 *  - SSL added
 *  - fixed bug: Connect to server with another name
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 29.11.07   Time: 18:03
 * Updated in $/VSNA/Servers/AppServer
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 27.11.07   Time: 18:54
 * Updated in $/VSNA/Servers/AppServer
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 13.11.07   Time: 17:32
 * Created in $/VSNA/Servers/AppServer
 * new services
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:49
 * Created in $/VSNA/Servers/MediaBroker
 *
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "acs/Lib/VS_AcsLib.h"
#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "transport/Router/VS_TransportRouter.h"
#include "streams/Router/VS_StreamsRouter.h"
#include "streams/VS_UdpStreamRouter.h"
#include "Bwt/VS_BwtHandler.h"
#include "CheckSrv/CheckSrv/VS_CheckSrvHandler.h"
#include "tools/Server/VS_Server.h"
#include "statuslib/VS_ResolveServerFinder.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/ASIOThreadPool.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/cpplib/scope_exit.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"

#include "version.h"
#include "VS_AppServices.h"

#include "../common/std/cpplib/VS_MemoryLeak.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

static VS_AccessConnectionSystem	*s_acs = nullptr;
static VS_TlsHandler                *tlsHandler = 0;
static VS_TransportRouter			*tr  = 0;
static VS_StreamsRouter				*sr  = 0;
static VS_AppServices				*bs  = 0;
static VS_UdpStreamRouter           *uds = 0;

static VS_BwtHandler				*bwtHandler = 0;
static VS_CheckSrvHandler			*checkSrvHandler = 0;

extern std::string g_tr_endpoint_name;

static const char   sIma[] = "Internal mistake of initialization of application.",
					sEns[] = "The endpoint name of the broker is not specified.";

static const unsigned n_threads = std::max(4u, std::thread::hardware_concurrency())
	;
static vs::ASIOThreadPool atp(n_threads);

/**
 **************************************************************************
 * \brief Starting sequence
 ****************************************************************************/
bool VS_Server::Start( void )
{
	bool   ret = false;
	VS_SCOPE_EXIT {
		if (!ret)
			dprint0("Error during the initialization. See Stdout[.log].\n");
	};
	VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);

	VS_SimpleStr ver=STRPRODUCTVER;

	puts( "\n\nASRV: Access Connections System is starting..." );
	if (!VS_AcsLibInitial())
	{
		puts(sIma);
		return ret;
	}

	if (g_tr_endpoint_name.empty())
	{
		puts(sIma);
		return ret;
	}

	if (!cfg_root.IsValid())
	{
		puts("Registry Configuration key not found");
		return ret;
	}

	VS_Server::srv_components = bs = new VS_AppServices(atp.get_io_service());
	if (!bs || !bs->IsValid())
	{
		puts(sIma);
		return ret;
	}

	s_acs = new VS_AccessConnectionSystem;
	tlsHandler = new VS_TlsHandler;
	if (!s_acs || !s_acs->Init())
	{
		puts(sIma);
		return ret;
	}
	puts( "\tAccess Connections System was started." );

	puts( "ASRV: Transport Router is starting..." );
	auto pWatchDog = VS_Server::GetWatchDog();
	assert(pWatchDog != nullptr);
	tr = new VS_TransportRouter(pWatchDog);
	int TransportPingPeriod = 8000;
	cfg_root.GetValue(&TransportPingPeriod, 4, VS_REG_INTEGER_VT, "TransportPingPeriod");
	TransportPingPeriod = TransportPingPeriod < 3000 ? 3000 : TransportPingPeriod > 10000 ? 10000 : TransportPingPeriod;
	if (!tr || !tr->Init(g_tr_endpoint_name.c_str(), s_acs, tlsHandler, 200, 20200, 1000, 30000, TransportPingPeriod))
	{
		puts(sIma);
		return ret;
	}
	printf( "\tTransport Router for \"%s\" was started.\n", g_tr_endpoint_name.c_str());

	puts( "ASRV: Stream Router is starting..." );
	sr = VS_StreamsRouter::Create();
	if (!sr || !sr->Init(g_tr_endpoint_name.c_str(), s_acs, tlsHandler, bs->GetConferencesConditions(), CONFERENCES_MAX_CNT))
	{
		puts(sIma);
		return ret;
	}
	else printf( "\tStream Router for \"%s\" was started.\n", g_tr_endpoint_name.c_str());

    puts( "ASRV: UDP Stream Router is starting..." );
    uds = new VS_UdpStreamRouter;

    if (!uds || !uds->Init(new VS_UdpStreamRouter_Impl, (char*)g_tr_endpoint_name.c_str()) ||
        !uds->Start(g_tr_endpoint_name.c_str()))
    {
		puts("...has failed to start. Check UDP ports.");// goto go_return;
		dprint0("UDP stream router has failed to start.\n");
		dprint0("Probably UDP ports are binded by another application. Check it.\n");
		///Stas recomended. UDP router is option. Watchdog
	} else
	{
		dprint4( "Udp Stream Router for \"%s\" was started.\n", g_tr_endpoint_name.c_str());
	}

	VS_RegistryKey rKey(false, CONFIGURATION_KEY);
	int32_t value = 0;
//	If TLS is disabled, don't add tlsHandler to ACS, rendering it inactive
	if (rKey.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "ServerTLSEnabled") <= 0
			|| value != 0)
		s_acs->AddHandler("Handler of TLS connections", tlsHandler);

	bwtHandler = new VS_BwtHandler;
	s_acs->AddHandler("Handler Of Bandwidth Tests.", bwtHandler);
	tlsHandler->AddHandler( "Handler Of Bandwidth Tests.", bwtHandler );

	checkSrvHandler = new VS_CheckSrvHandler;
	checkSrvHandler->SetTransportRouter(tr);
	s_acs->AddHandler("Handler of check server.",checkSrvHandler);
	tlsHandler->AddHandler("Handler of check server.",checkSrvHandler);

	if (0 == VS_Server::reg_params.mode)
	{
		printf("ASRV: ACS opened %u ports.\n", s_acs->AddListeners(g_tr_endpoint_name.c_str()));
		ret = pWatchDog->AddTestable(s_acs, 10);
	}
	else
		ret = true;

	atp.Start();
	VS_ResolveServerFinder::SetIOService(atp.get_io_service());
	ret = bs->Init(vs::RegistrationParams(VS_Server::reg_params), tr, sr, s_acs, tlsHandler, pWatchDog, ver);

	return ret;
}


/**
 **************************************************************************
 * \brief Ending sequence
 ****************************************************************************/
void VS_Server::Stop( void )
{
	VS_Server::srv_components = 0;

	if (tr)	{		tr->PrepareToDie();			}
	if (s_acs) {
		s_acs->PrepareToDie();
		s_acs->RemoveAllHandlers();
	}
	if (tlsHandler)
	{
		tlsHandler->RemoveAllHandlers();
	}
	delete bs;		bs = 0;
	delete sr;		sr = 0;
	delete tr;		tr = 0;
	delete bwtHandler; bwtHandler = 0;
	delete checkSrvHandler; checkSrvHandler = 0;
	delete s_acs; s_acs = nullptr;
	delete tlsHandler; tlsHandler = 0;
	if (uds) {		uds->Stop();	delete uds; uds = 0; }

	atp.get_io_service().stop(); // FIXME: Ideally we should be able stop without this.
	atp.Stop();
}


//@{ \name Server, Registry Key and Windows Service Functions.
const char* VS_Server::ShortName() { return VS_APP_SERVER_WS_SERVICE_NAME; }
const char* VS_Server::LongName() { return VS_APP_SERVER_WS_DISPLAY_NAME; }
const char* VS_Server::RegistryKey() {
	if (!!g_root_key)
		return g_root_key;
	return VS_APP_SERVER_WS_ROOT_KEY_NAME;
}
const char* VS_Server::ServiceName() { return VS_APP_SERVER_WS_SERVICE_NAME; }
const char* VS_Server::ProductVersion() { return STRPRODUCTVER; }
//@}

int main(int argc, char *argv[]) { return VS_Server::main(argc, argv); }
