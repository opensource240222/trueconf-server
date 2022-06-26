#include "acs/Lib/VS_AcsLib.h"
#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "transport/Router/VS_TransportRouter.h"
#include "Bwt/VS_BwtHandler.h"
#include "CheckSrv/CheckSrv/VS_CheckSrvHandler.h"
#include "tools/Server/VS_Server.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/scope_exit.h"

#include "version.h"
#include "VS_RoutingServices.h"

#include "../common/std/cpplib/VS_MemoryLeak.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

static VS_AccessConnectionSystem	*s_acs = nullptr;
static VS_TlsHandler                *tlsHandler = 0;
static VS_TransportRouter			*tr  = 0;
static VS_RoutingServices			*rs  = 0;

static VS_BwtHandler				*bwtHandler = 0;
static VS_CheckSrvHandler			*checkSrvHandler = 0;
extern std::string g_tr_endpoint_name;

static const char   sIma[] = "Internal mistake of initialization of application.",
					sEns[] = "The endpoint name of the broker is not specified.";

VS_ACS_Response IsWsHandlerProtocol(const void *in_buffer, unsigned long *in_len)
{
	return vs_acs_connection_is_not_my;
}

bool VS_Server::Start()
{
	bool   ret = false;
	VS_SCOPE_EXIT {
		if (!ret)
			printf("Error during the initialization. See Stdout[.log].\n");
	};
	VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);

	VS_SimpleStr ver=STRPRODUCTVER;

	puts( "\n\nSRV: Access Connections System is starting..." );
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

	rs = new VS_RoutingServices;
	if (!rs || !rs->IsValid())
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

	puts( "SRV: Transport Router is starting..." );
	auto pWatchDog = VS_Server::GetWatchDog();
	assert(pWatchDog != nullptr);
	tr = new VS_TransportRouter(pWatchDog);
	if (!tr || !tr->Init(g_tr_endpoint_name.c_str(), s_acs, tlsHandler))
	{
		puts(sIma);
		return ret;
	}
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

	if (!rs->Init(vs::RegistrationParams(VS_Server::reg_params), tr, pWatchDog, ver))
		return ret;

	if (0 == VS_Server::reg_params.mode)
	{
		printf("SRV: ACS opened %u ports.\n", s_acs->AddListeners(g_tr_endpoint_name.c_str()));
		ret = pWatchDog->AddTestable(s_acs, 10);
	}
	else
		ret = true;

	return ret;
}
void VS_Server::Stop()
{
	if (tr)	{		tr->PrepareToDie();			}
	if (s_acs)
		s_acs->PrepareToDie();

	if ( rs ) { delete rs; rs = 0; }

	if (tr) { delete tr; tr = 0; }

	if (s_acs)
		s_acs->RemoveAllHandlers();
	if (tlsHandler)
	{
		tlsHandler->RemoveAllHandlers();
		delete tlsHandler;
		tlsHandler = 0;
	}
		if ( bwtHandler ) { delete bwtHandler; bwtHandler = 0; }
		if ( checkSrvHandler ) { delete checkSrvHandler; checkSrvHandler = 0; }
		delete s_acs; s_acs = nullptr;
}

//@{ \name Server, Registry Key and Windows Service Functions.
const char* VS_Server::ShortName() { return VS_ROUTING_SERVER_WS_SERVICE_NAME; }
const char* VS_Server::LongName() { return VS_ROUTING_SERVER_WS_DISPLAY_NAME; }
const char* VS_Server::RegistryKey() {
	if (!!g_root_key)
		return g_root_key;
	return VS_ROUTING_SERVER_WS_ROOT_KEY_NAME;
}
const char* VS_Server::ServiceName() { return VS_ROUTING_SERVER_WS_SERVICE_NAME; }
const char* VS_Server::ProductVersion() { return STRPRODUCTVER; }
//@}

int main(int argc, char *argv[]) { return VS_Server::main(argc, argv); }
