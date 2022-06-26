#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "acs/Lib/VS_AcsLib.h"
#include "RegistryServer/VS_RegServerServices.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/ASIOThreadPool.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "tools/Server/VS_Server.h"
#include "transport/Router/VS_TransportRouter.h"
#include "version.h"

#include <time.h>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

static VS_AccessConnectionSystem	*s_acs = nullptr;
static VS_TlsHandler                *tlsHandler = 0;
static VS_TransportRouter			*tr = 0;
static VS_RegServerServices			*rs = 0;
extern std::string g_tr_endpoint_name;

static const char   sIma[] = "Internal mistake of initialization of application.",
					sEns[] = "The endpoint name of the broker is not specified.";

static const unsigned n_threads = std::max(4u, std::thread::hardware_concurrency())
	+ 1/* VS_HttpHandler_v2 */
	;
static vs::ASIOThreadPool atp(n_threads);

VS_ACS_Response IsWsHandlerProtocol(const void *in_buffer, unsigned long *in_len)
{
	return vs_acs_connection_is_not_my;
}

/**
 **************************************************************************
 * \brief Starting sequence
 ****************************************************************************/
bool VS_Server::Start( void )
{
	bool   ret = false;
	VS_SCOPE_EXIT
	{
		if (!ret)
			printf( "Error during the initialization. See Stdout[.log].\n");
	};
	VS_RoutersWatchdog *pWatchDog = nullptr;

	puts( "\n\nSRV: Access Connections System is starting..." );
	if (!VS_AcsLibInitial())
	{
		puts( sIma );
		return ret;
	}

	if (g_tr_endpoint_name.empty())
	{
		puts( sIma );
		return ret;
	}

	rs = new VS_RegServerServices(atp.get_io_service());
	if (!rs || !rs->IsValid())
	{
		puts( sIma );
		return ret;
	}

	s_acs = new VS_AccessConnectionSystem;
	tlsHandler = new VS_TlsHandler;
	if (!s_acs || !s_acs->Init())
	{
		puts( sIma );
		return ret;
	}
	puts( "\tAccess Connections System was started." );

	VS_RegistryKey rKey(false, CONFIGURATION_KEY);
	int32_t value = 0;
//	If TLS is disabled, don't add tlsHandler to ACS, rendering it inactive
	if (rKey.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "ServerTLSEnabled") <= 0
			|| value != 0)
		s_acs->AddHandler("Handler of TLS connections", tlsHandler);

	puts( "SRV: Transport Router is starting..." );
	pWatchDog = VS_Server::GetWatchDog();
	assert(pWatchDog != nullptr);
	tr = new VS_TransportRouter(pWatchDog);
	if (!tr || !tr->Init(g_tr_endpoint_name.c_str(), s_acs, tlsHandler))
	{
		puts( sIma );
		return ret;
	}
	printf( "\tTransport Router for \"%s\" was started.\n", g_tr_endpoint_name.c_str());

	atp.Start();

	if (!rs->Init(tr, s_acs, tlsHandler, pWatchDog))
		return ret;

	printf("SRV: ACS opened %u ports.\n", s_acs->AddListeners(g_tr_endpoint_name.c_str()));
	ret = pWatchDog->AddTestable(s_acs, 10);

	return ret;
}

/**************************************************************************
* \brief Ending sequence
****************************************************************************/
void VS_Server::Stop( void )
{
	if (s_acs) {
		s_acs->PrepareToDie();
		s_acs->RemoveAllHandlers();
	}
	if (tlsHandler)
		tlsHandler->RemoveAllHandlers();
	delete rs;		rs = 0;
	delete tr;		tr = 0;
	delete s_acs; s_acs = nullptr;
	delete tlsHandler; tlsHandler = 0;

	atp.get_io_service().stop(); // FIXME: Ideally we should be able stop without this.
	atp.Stop();

	const time_t   tm = time( 0 );
	printf( "Registry Server finished at %s\n", ctime( &tm ));
}

 //@{ \name Server, Registry Key and Windows Service Functions.
const char* VS_Server::ShortName() { return VS_REGISTRY_SERVER_WS_SERVICE_NAME; }
const char* VS_Server::LongName() { return VS_REGISTRY_SERVER_WS_DISPLAY_NAME; }
const char* VS_Server::RegistryKey() {
	if (!!g_root_key)
		return g_root_key;
	return VS_REGISTRY_SERVER_WS_ROOT_KEY_NAME;
}
const char* VS_Server::ServiceName() { return VS_REGISTRY_SERVER_WS_SERVICE_NAME; }
const char* VS_Server::ProductVersion() { return STRPRODUCTVER; }

int main(int argc, char *argv[]) { return VS_Server::main(argc, argv); }
