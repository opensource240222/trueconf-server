/****************************************************************************
 * Includes
 ****************************************************************************/
#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "acs/Lib/VS_AcsLib.h"
#include "AppServer/Services/VS_AppServerData.h"
#include "AppServer/Services/VS_PresenceService.h"
#include "Bwt/VS_BwtHandler.h"
#include "CheckSrv/CheckSrv/VS_CheckSrvHandler.h"
#include "MDNS_Responder/ResponderWrapperOldACS.h"
#include "net/EndpointRegistry.h"
#include "ProtectionLib/Protection.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_SecureHandshake.h"
#include "ServerServices/VS_CheckCert.h"
#include "ServerServices/VS_ReadLicense.h"
#include "std-generic/cpplib/IntConv.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/ASIOThreadPool.h"
#include "std/cpplib/ForEachHost.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_IntConv.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_Utils.h"
#include "std/debuglog/VS_Debug.h"
#include "std/VS_RegServer.h"
#include "streams/Router/VS_StreamsRouter.h"
#include "streams/VS_UdpStreamRouter.h"
#include "tools/Server/VS_Server.h"
#include "transport/Client/VS_DNSFunction.h"
#include "transport/Router/VS_TransportRouter.h"

#include "version.h"
#include "VCSServices.h"
#include "TrueGateway/VS_GatewayStarter.h"

#include <process.h>

#include "std/cpplib/VS_MemoryLeak.h"

extern std::string g_tr_endpoint_name;


#define DEBUG_CURRENT_MODULE VS_DM_ALL_

static VS_AccessConnectionSystem	*s_acs = nullptr;
static VS_TransportRouter			*tr  = 0;
static VS_StreamsRouter				*sr  = 0;
static VS_VCSServices				*bs  = 0;
static VS_UdpStreamRouter           *uds = 0;
static VS_DDNS						*ddns = 0;
static mdns::ResponderWrapperOldACS *responderWrapper = 0;

static VS_BwtHandler				*bwtHandler = 0;
static VS_CheckSrvHandler			*checkSrvHandler = 0;
static VS_TlsHandler                *tlsHandler = 0;

static const char   sIma[] = "Internal mistake of initialization of application.",
					sEns[] = "The endpoint name of the broker is not specified.";
//VS_MemoryLeak leak;


static const unsigned n_threads = std::max(4u, std::thread::hardware_concurrency())
	+ 1/* VS_RoamingSettings */
	+ 1/* VS_HttpHandler_v2 */
	;
static vs::ASIOThreadPool atp(n_threads);

/**
 **************************************************************************
 * \brief Starting sequence
 ****************************************************************************/
void TryUpdateDDNS_OldSRV(VS_DDNS* ddns)
{
	ddns->SetSRV(ddns->GetDNSDomain(), net::dns::VCS_DDNS_SRV);

	if (!ddns->IsValid())
		return ;

	dprint3("DDNS OldSRV cleanup start...\n");
	bool IsCleanUpOK = ddns->CleanUp();
	dprint3("DDNS OldSRV cleanup for %s is %s\n", ddns->GetRequestName(), IsCleanUpOK? "ok": "failed");

	const size_t bsize = 2048;
	char *buff = new char[bsize];

	// start dns update
	dprint3("DDNS OldSRV update start...\n");
	DNS_RECORDA record =
	{
		0, ddns->GetRequestName(), DNS_TYPE_SRV, sizeof(DNS_RECORDA), {}, DNS_SRV_TTL, 0, {}
	};

	record.Data.SRV.pNameTarget = (char*)g_tr_endpoint_name.c_str();
	record.Data.SRV.wPort = 4307;

	size_t pcksize = ddns->eMakeDnsSrvPacket(record, buff, bsize, false);
	bool IsUpdateOK = (pcksize && ddns->eUpdateDnsRecord(buff, pcksize));
	dprint3("DDNS OldSRV update for %s with %s is %s\n", ddns->GetRequestName(), g_tr_endpoint_name.c_str(), IsUpdateOK? "ok": "failed");

	dprint3("DDNS OldSRV update end\n");

	delete [] buff;
}

void TryUpdateDDNS(VS_DDNS* ddns, const char* ip_str)
{
	if (!ddns->IsValid())
		return ;

	dprint3("DDNS SRV cleanup start...\n");
	bool IsCleanUpOK = ddns->CleanUp();
	dprint3("DDNS SRV cleanup for %s is %s\n", ddns->GetRequestName(), IsCleanUpOK? "ok": "failed");

	const size_t bsize = 2048;
	char *buff = new char[bsize];

	// start dns update
	dprint3("DDNS SRV update start...\n");
	DNS_RECORDA record =
	{
		0, ddns->GetRequestName(), DNS_TYPE_SRV, sizeof(DNS_RECORDA), {}, DNS_SRV_TTL, 0, {}
	};

	// перебрать все вывешанные порты
	{
		char* token_ctx = 0;
		char* token = 0;
		char* p = 0;
		unsigned int port = 0;
		char seps[] = ",";
		VS_SimpleStr tmp(ip_str);

		token = strtok_s(tmp, seps, &token_ctx);
		while( token != NULL )
		{
			p = strstr(token, ":");
			p[0] = 0;		p++;
			port = atoi(p);

			record.Data.SRV.pNameTarget = token;
			record.Data.SRV.wPort = port;

			size_t pcksize = ddns->eMakeDnsSrvPacket(record, buff, bsize, false);
			bool IsUpdateOK = (pcksize && ddns->eUpdateDnsRecord(buff, pcksize));
			dprint3("DDNS SRV update for %s with %s:%d is %s\n", ddns->GetRequestName(), token, port, IsUpdateOK? "ok": "failed");

			token = strtok_s( NULL, seps, &token_ctx );
		}
		dprint3("DDNS SRV update end\n");
	}

	delete [] buff;
}

struct ArgsUpdateDDNS
{
	VS_SimpleStr m_ip_str;
	VS_SimpleStr m_endpoint;
};

void __cdecl ThreadUpdateDDNS(void* p)
{
	vs::SetThreadName("UpdateDDNS");

	ArgsUpdateDDNS* args = (ArgsUpdateDDNS*) p;

	// Init DDNS params
	ddns = new VS_DDNS;
	if (!ddns->SetParamsFromReg())
	{
		if (ddns->InitDNSServers())
		{
			VS_SimpleStr server(args->m_endpoint);
			char* p = strchr(server.m_str,'#');
			if (p)
				*p = 0;
			if (!!server && server.m_str[server.Length()] != '.')
				server += ".";
			ddns->SetParams(0, server);
		}
	}

	// Try DDNS update
	TryUpdateDDNS(ddns, args->m_ip_str);
	TryUpdateDDNS_OldSRV(ddns);

	delete args;
}

void UpdateDDNS(const char* endpoint, const char* ip_str)
{
	if (!endpoint || !*endpoint || !ip_str || !*ip_str)
		return;

	ArgsUpdateDDNS* args = new ArgsUpdateDDNS;
	args->m_endpoint = endpoint;
	args->m_ip_str = ip_str;

	uintptr_t thr = _beginthread(ThreadUpdateDDNS, 0, args);
	if (!thr || thr==-1L)
		delete args;
}


bool VS_Server::Start( void )
{
	/******/
	if(VS_ArmDetectedVM())
		dprint0("VIRTUAL MACHINE IS DETECTED!\n\n\n\n");
	/******/

	const int net_interface = 5;
	char *ips_my[net_interface] = {0};

	bool   ret = false;
	VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
	VS_Container	UCAZA;
	p_licWrap = new VS_LicensesWrap;
	VS_SimpleStr ver=STRPRODUCTVER;
	try
	{
		puts("\n\nASRV: Access Connections System is starting...");
		if (!VS_AcsLibInitial()) { puts(sIma);	throw "\nVS_AcsLibInitial failed.\n"; }

		if (g_tr_endpoint_name.empty()) { puts(sIma);	throw "\nServer name was not set.\n"; }
		printf("server name = %s\n", g_tr_endpoint_name.c_str());

		if (!cfg_root.IsValid())
		{
			puts("Registry Configuration key not found"); throw "\n!cfg_root.IsValid().\n" ;
		}

		//Check Cert
		if (1 == VS_Server::reg_params.mode)
		{
			/**
			удалить сертификат и приватныей ключ, если нет доступа, то выйти
			*/
			if (cfg_root.HasValue(SRV_CERT_KEY) && !cfg_root.RemoveValue(SRV_CERT_KEY))
			{
				puts("Can't remove cert or private key");
				throw "";
			}
			if (cfg_root.HasValue(SRV_PRIVATE_KEY) && !cfg_root.RemoveValue(SRV_PRIVATE_KEY))
			{
				puts("Can't remove cert or private key");
				throw "";
			}
		}
		else if (0 == VS_Server::reg_params.mode)
		{
			if (!VS_CheckCert())
			{
				puts("Certificate not valid");
				throw "";
			}
		}

		VS_Server::srv_components = bs = new VS_VCSServices(atp.get_io_service());
		if (!bs || !bs->IsValid()) { puts(sIma);	throw "\n!bs || !bs->IsValid().\n"; }

		s_acs = new VS_AccessConnectionSystem;
		tlsHandler = new VS_TlsHandler;
		if (!s_acs || !s_acs->Init()) { puts(sIma);	throw "\n!acs || !acs->Init().\n"; }
		int n_listeners = 0;
		if (0 == VS_Server::reg_params.mode || (1 == VS_Server::reg_params.mode && VS_Server::reg_params.offline_reg_file.empty()))
		{
			if (net::endpoint::GetCountAcceptTCP(g_tr_endpoint_name, false) > 0)
			{
				n_listeners = s_acs->AddListeners(g_tr_endpoint_name.c_str());
			}
			else {
				// Accept on ANY_IP:4307
				char host[512] = { 0 };
				if (!VS_GetDefaultHostName(host, 512))
					throw "\nVS_GetDefaultHostName failed.\n";

				unsigned ips_count = 0;
				for (int i = 0; i < net_interface; i++)
					ips_my[i] = new char[64];

				if (!(ips_count = VS_GetHostsByName(host, ips_my, net_interface, 64)))
					throw "\nVS_GetHostsByName failed.\n";

				if (ips_count == 0 || (ips_count == 1 && strcasecmp(ips_my[0], VS_LocHost) == 0))
					throw "\nThe computer does not have any active network interfaces.\n";

				for (unsigned int i = 0; i < ips_count; i++)
					s_acs->AddListenerByIP(ips_my[i], 4307);
			}
		}
		puts("\tAccess Connections System was started.");

		puts("ASRV: Transport Router is starting...");
		auto pWd = VS_Server::GetWatchDog();
		assert(pWd != nullptr);
		tr = new VS_TransportRouter(pWd);
		int TransportPingPeriod = 8000;
		cfg_root.GetValue(&TransportPingPeriod, 4, VS_REG_INTEGER_VT, "TransportPingPeriod");
		TransportPingPeriod = TransportPingPeriod < 3000 ? 3000 : TransportPingPeriod > 10000 ? 10000 : TransportPingPeriod;
		if (!tr || !tr->Init(g_tr_endpoint_name.c_str(), s_acs, tlsHandler, 200, 20200, 1000, 30000, TransportPingPeriod))
		{
			puts(sIma);	throw "\n Transport Router init failed.\n";
		}
		printf("\tTransport Router for \"%s\" was started.\n", g_tr_endpoint_name.c_str());

		if (0 == VS_Server::reg_params.mode)
		{
			puts("ASRV: Stream Router is starting...");
			sr = VS_StreamsRouter::Create();
			if (!sr || !sr->Init(g_tr_endpoint_name.c_str(), s_acs, tlsHandler, bs->GetConferencesConditions(), CONFERENCES_MAX_CNT))
			{
				puts(sIma);	throw "\nStream router init failed.\n";

			}
			else printf("\tStream Router for \"%s\" was started.\n", g_tr_endpoint_name.c_str());

			puts("ASRV: UDP Stream Router is starting...");
			uds = new VS_UdpStreamRouter;

			if (!uds || !uds->Init(new VS_UdpStreamRouter_Impl, (char*)g_tr_endpoint_name.c_str(), true) ||
				!uds->Start(g_tr_endpoint_name.c_str()))
			{
				puts("...has failed to start. Check UDP ports.");
				dprint0("UDP stream router has failed to start.\n");
				dprint0("Probably UDP ports are binded by another application. Check it.\n");
				///Stas recomended. UDP router is option. Watchdog
			}
			else
			{
				printf("\tUdp Stream Router for \"%s\" was started.\n", g_tr_endpoint_name.c_str());
			}

			VS_RegistryKey rKey(false, CONFIGURATION_KEY);
			int32_t value = 0;
//			If TLS is disabled, don't add tlsHandler to ACS, rendering it inactive
			if (rKey.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "ServerTLSEnabled") <= 0
					|| value != 0)
				s_acs->AddHandler("Handler of TLS connections", tlsHandler);

			bwtHandler = new VS_BwtHandler;
			s_acs->AddHandler("Handler Of Bandwidth Tests.", bwtHandler);
			tlsHandler->AddHandler("Handler Of Bandwidth Tests.", bwtHandler);

			checkSrvHandler = new VS_CheckSrvHandler;
			checkSrvHandler->SetTransportRouter(tr);
			s_acs->AddHandler("Handler of check server.", checkSrvHandler);
			tlsHandler->AddHandler("Handler of check server.", checkSrvHandler);
		}

		if (!VS_ReadLicense(g_tr_endpoint_name.c_str(), VS_License::GRACE_PERIOD, UCAZA))
		{
			puts("License not valid"); throw "\nReadLicense failed.\n";
		};

		// Get IP:Port that we have already binded
		std::string ip_str;
		n_listeners = s_acs->GetListeners(ip_str);
		if (n_listeners && !ip_str.empty())
		{
			cfg_root.SetString(ip_str.c_str(), CURRENT_CONNECT_TAG);
		}

		bool NeedAddConnectTCP = !ip_str.empty()
			&& net::endpoint::GetCountConnectTCP(g_tr_endpoint_name, false) == 0
			&& net::endpoint::GetCountAcceptTCP(g_tr_endpoint_name, false) == 0;
		if (0 == VS_Server::reg_params.mode)
		{
			if (NeedAddConnectTCP)
				ForEachHost(ip_str, [] (string_view host, string_view port)
				{
					net::endpoint::AddConnectTCP({ host, vs::atoi_sv(port), net::endpoint::protocol_tcp }, g_tr_endpoint_name.c_str(), false);
				});

			if (!g_appServer)
				g_appServer = new VS_AppServerData;

			g_appServer->Set(g_tr_endpoint_name.c_str(), false, ST_VCS);		// add our server to list
			g_appServer->SetNetInfo(g_tr_endpoint_name.c_str(), 0);			// Save our ConnectTCP to g_appServer

			if (NeedAddConnectTCP)
				net::endpoint::ClearAllConnectTCP(g_tr_endpoint_name.c_str(), false);
		}

		atp.Start();
		VS_ResolveServerFinder::SetIOService(atp.get_io_service());

		if (VS_Server::reg_params.mode == 0 && mdns::ResponderWrapperOldACS::EnabledInRegistry())
		{
			responderWrapper = new mdns::ResponderWrapperOldACS;
			if (responderWrapper->Init(atp.get_io_service(), s_acs))
				responderWrapper->responder->start();
			else
				dstream2 << "mDNS responder failed to initialize!\n";
		}

		if (!bs->Init(vs::RegistrationParams(VS_Server::reg_params), tr, sr, s_acs, tlsHandler, pWd, ver, UCAZA))
			throw "\nVS_VCSServices init failed.\n";

		auto accept_tcp = [](const std::string& host, net::port p) {
			return s_acs->AddListener(host.c_str(), p, true);
		};

		//endpoint for configurator
		if (!net::endpoint::ReadOrMakeAcceptTCP(1, VS_Server::endpoint_for_config, accept_tcp, "127.0.0.1", 4307))
			dprint0("\nTrying to accept to local endpoint is failed.\n");

		printf("ASRV: ACS opened %u ports.\n", n_listeners);
		ret = pWd->AddTestable(s_acs, 10);

		if (0 == VS_Server::reg_params.mode)
			UpdateDDNS(g_tr_endpoint_name.c_str(), ip_str.c_str());
	}
	catch (const char* descr)
	{
		if (!!descr&&*descr!=0)
			dprint0(descr);
	}
	if (!ret)	dprint0( "Error during the initialization. See Stdout[.log].\n");
	for(int i = 0;i<net_interface;i++)
		if  (ips_my[i])
			delete [] ips_my[i];
	return ret;
}

/**
 **************************************************************************
 * \brief Ending sequence
 ****************************************************************************/

void VS_Server::Stop( void )
{
	VS_Server::srv_components = 0;
	VS_GatewayStarter *gw_starter = VS_GatewayStarter::GetInstance();
	if(gw_starter)
		gw_starter->StopGateway();

	if (responderWrapper != nullptr) // means we initialized it before
	{
		responderWrapper->Stop();
		delete responderWrapper;
	}
	if (tr)	{		tr->PrepareToDie();			}
	PauseDestroy(1000);
	Sleep(1000);
	if (s_acs) {
		s_acs->PrepareToDie();
		s_acs->RemoveAllHandlers();
	}
	if (tlsHandler)
	{
		tlsHandler->RemoveAllHandlers();
	}
	delete bs;		bs = 0;
	if (tr) {
		tr->DisconnectAllByCondition([](string_view cid, string_view ep, unsigned char hops)
		{
			return ep == RegServerName;
		});
		PauseDestroy(500);
		Sleep(500);
	}

	atp.get_io_service().stop(); // FIXME: Ideally we should be able stop without this.
	atp.Stop();

	delete sr;		sr = 0;
	delete tr;		tr = 0;
	delete bwtHandler; bwtHandler = 0;
	delete checkSrvHandler; checkSrvHandler = 0;
	delete s_acs; s_acs = nullptr;
	delete tlsHandler; tlsHandler = 0;
	if (uds) {		uds->Stop();	delete uds; uds = 0; }

	VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
	cfg_root.RemoveValue(CURRENT_CONNECT_TAG);
}


//@{ \name Server, Registry Key and Windows Service Functions.
const char* VS_Server::ShortName() { return VS_TRUECONF_WS_SERVICE_NAME; }
const char* VS_Server::LongName() { return VS_TRUECONF_WS_DISPLAY_NAME; }
const char* VS_Server::RegistryKey() {
	if (!!g_root_key)
		return g_root_key;
	return VS_TRUECONF_WS_ROOT_KEY_NAME;
}
const char* VS_Server::ServiceName() { return VS_TRUECONF_WS_SERVICE_NAME; }
const char* VS_Server::ProductVersion() { return STRPRODUCTVER; }
//@}

int main(int argc, char *argv[]) { return VS_Server::main(argc, argv); }
