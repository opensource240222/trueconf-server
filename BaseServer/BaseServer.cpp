#include "acs/Lib/VS_AcsLib.h"
#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "transport/Router/VS_TransportRouter.h"
#include "Bwt/VS_BwtHandler.h"
#include "tools/Server/VS_Server.h"
#include "statuslib/VS_ResolveServerFinder.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "CheckSrv/CheckSrv/VS_CheckSrvHandler.h"
#include "std/cpplib/ASIOThreadPool.h"
#include "std/debuglog/VS_Debug.h"

#include "version.h"
#include "VS_BaseServerServices.h"

#include <boost/make_shared.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

static VS_AccessConnectionSystem	*s_acs = nullptr;
static VS_TlsHandler                *tlsHandler = 0;
static VS_TransportRouter			*tr  = 0;
static VS_BaseServerServices		*rs  = 0;

static VS_BwtHandler				*bwtHandler = 0;
static VS_CheckSrvHandler			*checkSrvHandler = 0;

extern std::string g_tr_endpoint_name;


static const unsigned n_threads = std::max(4u, std::thread::hardware_concurrency())
+ 1/* VS_RoamingSettings */
+ 1/* VS_HttpHandler_v2 */
+ 1/* BSConferenceSRV */
;
static vs::ASIOThreadPool atp(n_threads);

VS_ACS_Response IsWsHandlerProtocol(const void *in_buffer, unsigned long *in_len)
{
	return vs_acs_connection_is_not_my;
}

bool VS_Server::Start() try
{
	VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
	if (!cfg_root.IsValid())
		throw std::runtime_error("Registry Configuration key not found");

	if (g_tr_endpoint_name.empty())
		throw std::runtime_error("Failed to set server endpoint");

	VS_SimpleStr ver = STRPRODUCTVER;



	puts( "\n\nSRV: Access Connections System is starting..." );
	if (!VS_AcsLibInitial())
		throw std::runtime_error("Access Connections System has failed to start");

	VS_Server::srv_components = rs = new VS_BaseServerServices;
	if (!rs || !rs->IsValid())
		throw std::runtime_error("Services has failed to start");

	s_acs = new VS_AccessConnectionSystem;
	tlsHandler = new VS_TlsHandler;
	if (!s_acs || !s_acs->Init())
		throw std::runtime_error("Access Connections System has failed to start");
	puts( "\tAccess Connections System was started." );

	puts( "SRV: Transport Router is starting..." );
	auto pWatchDog = VS_Server::GetWatchDog();
	assert(pWatchDog != nullptr);
	tr = new VS_TransportRouter(pWatchDog);
	if (!tr || !tr->Init(g_tr_endpoint_name.c_str(), s_acs, tlsHandler))
		throw std::runtime_error("Transport Router has failed to start");
	printf( "\tTransport Router for \"%s\" was started.\n", g_tr_endpoint_name.c_str());

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
		printf("SRV: ACS opened %u ports.\n", s_acs->AddListeners(g_tr_endpoint_name.c_str()));
		if (!pWatchDog->AddTestable(s_acs, 10))
			throw std::runtime_error("Failed to initialize watchdog");
	}
	atp.Start();
	VS_ResolveServerFinder::SetIOService(atp.get_io_service());
	if (!rs->Init(vs::RegistrationParams(VS_Server::reg_params), atp.get_io_service(), tr, pWatchDog, s_acs, tlsHandler, ver, g_tr_endpoint_name.c_str()))
		throw std::runtime_error("Services has failed to initialize");

	return true;
}
catch (const std::exception& e)
{
	dprint0("Error during the initialization. See Stdout[.log].\n%s\n", e.what());
	return false;
}

void VS_Server::Stop()
{
	if (tr)	{		tr->PrepareToDie();			}
	if (s_acs) {
		s_acs->PrepareToDie();
		s_acs->RemoveAllHandlers();
	}
	if (tlsHandler)
		tlsHandler->RemoveAllHandlers();
	delete rs; rs = 0;
	delete tr;		tr = 0;
	delete bwtHandler; bwtHandler = 0;
	delete checkSrvHandler; checkSrvHandler = 0;
	delete s_acs; s_acs = nullptr;
	delete tlsHandler; tlsHandler = 0;

	atp.get_io_service().stop(); // FIXME: Ideally we should be able stop without this.
	atp.Stop();
}

//@{ \name Server, Registry Key and Windows Service Functions.
const char* VS_Server::ShortName() { return VS_BASE_SERVER_WS_SERVICE_NAME; }
const char* VS_Server::LongName() { return VS_BASE_SERVER_WS_DISPLAY_NAME; }
const char* VS_Server::RegistryKey() {
	if (!!g_root_key)
		return g_root_key;
	return VS_BASE_SERVER_WS_ROOT_KEY_NAME;
}
const char* VS_Server::ServiceName() { return VS_BASE_SERVER_WS_SERVICE_NAME; }
const char* VS_Server::ProductVersion() { return STRPRODUCTVER; }
//@}

int main(int argc, char *argv[]) { return VS_Server::main(argc, argv); }
