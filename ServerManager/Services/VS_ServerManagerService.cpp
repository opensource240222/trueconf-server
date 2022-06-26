/**************************************************
 * $Revision: 66 $
 * $History: VS_ServerManagerService.cpp $
 *
 * *****************  Version 66  *****************
 * User: Ktrushnikov  Date: 27.08.12   Time: 20:27
 * Updated in $/VSNA/Servers/ServerManager/Services
 * #13060
 * - if use ns.trueconf.ru as NameServer, SOA records we added (including
 * their A-records at "Additional records")
 *
 * *****************  Version 65  *****************
 * User: Ktrushnikov  Date: 23.03.12   Time: 10:57
 * Updated in $/VSNA/Servers/ServerManager/Services
 * #11358: DDNS update with ports 4307, 80, 443
 *
 * *****************  Version 64  *****************
 * User: Ktrushnikov  Date: 5.03.12    Time: 17:36
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - #11142: update A record
 * - many domain support (for conferendo.com)
 * - "Allowed AS" registry added
 *
 * *****************  Version 63  *****************
 * User: Ktrushnikov  Date: 6.12.11    Time: 18:27
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - ddns update vcs2.tcp with hosts (was vcs.tcp)
 * - skip list of servers added
 *
 * *****************  Version 62  *****************
 * User: Mushakov     Date: 5.10.11    Time: 21:36
 * Updated in $/VSNA/Servers/ServerManager/Services
 *  - ssl refactoring (SetCert interfaces)
 *
 * *****************  Version 61  *****************
 * User: Mushakov     Date: 20.05.11   Time: 17:33
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - new rs supported
 *
 * *****************  Version 60  *****************
 * User: Mushakov     Date: 6.05.11    Time: 20:43
 * Updated in $/VSNA/Servers/ServerManager/Services
 *  - new reg; new reg cert; cert chain supported in tc_server
 *
 * *****************  Version 59  *****************
 * User: Mushakov     Date: 23.02.11   Time: 4:05
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - SecureHandshake ver. 2 added
 *
 * *****************  Version 58  *****************
 * User: Mushakov     Date: 29-11-10   Time: 20:55
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - registration 3.1 (sm)
 * - auto update cert for as, bs, rs 3.1 (sm)
 *
 * *****************  Version 57  *****************
 * User: Mushakov     Date: 1.11.10    Time: 21:00
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - cert update added
 * - registration from server added
 * - authorization servcer added
 *
 * *****************  Version 56  *****************
 * User: Ktrushnikov  Date: 6.10.10    Time: 14:58
 * Updated in $/VSNA/Servers/ServerManager/Services
 * SM:
 * - RedirectAS() & LoadBalance() by domain[vzochat.com, v-port.net]
 *
 * *****************  Version 55  *****************
 * User: Mushakov     Date: 10.09.10   Time: 20:24
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Registration on SM added
 *
 * *****************  Version 54  *****************
 * User: Ktrushnikov  Date: 29.07.10   Time: 11:39
 * Updated in $/VSNA/Servers/ServerManager/Services
 * SM: support serveral DDNS update configurations
 *
 * *****************  Version 53  *****************
 * User: Ktrushnikov  Date: 26.07.10   Time: 12:59
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - 2 threads to update servers in different SRVs (split by server_name
 * *.vzochat.com)
 *
 * *****************  Version 52  *****************
 * User: Mushakov     Date: 22.07.10   Time: 15:16
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - locatorBS supported for old servers
 *
 * *****************  Version 51  *****************
 * User: Ktrushnikov  Date: 19.07.10   Time: 18:29
 * Updated in $/VSNA/Servers/ServerManager/Services
 * AS asks SM (not BS) for AppProps
 * - by SM connected
 * - by manual request from support web
 *
 * *****************  Version 50  *****************
 * User: Ktrushnikov  Date: 29.06.10   Time: 12:21
 * Updated in $/VSNA/Servers/ServerManager/Services
 * Arch 3.1: NamedConfs invitation:
 * - RESOLVEALL_METHOD to get server & status of users
 * - start resolve in separate thread (PoolThreads)
 * - ask SM for RS in BS
 * - access to RS name from all BS services via locks
 * - reconnect to RS
 * - calc is_time fixed
 *
 * *****************  Version 49  *****************
 * User: Mushakov     Date: 24.06.10   Time: 16:15
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - opt disabled when SECUREBEGIN_
 * - locator bs removed
 *
 * *****************  Version 48  *****************
 * User: Mushakov     Date: 15.06.10   Time: 15:32
 * Updated in $/VSNA/Servers/ServerManager/Services
 *  - sharding
 *
 * *****************  Version 47  *****************
 * User: Ktrushnikov  Date: 15.03.10   Time: 17:47
 * Updated in $/VSNA/Servers/ServerManager/Services
 * DDNS
 * - get list of DNS servers
 * - try registry, then auto-update (VCS)
 *
 * *****************  Version 46  *****************
 * User: Ktrushnikov  Date: 10.03.10   Time: 13:22
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - update of vcs.tcp.video-port.com
 *
 * *****************  Version 45  *****************
 * User: Mushakov     Date: 28.01.10   Time: 19:53
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - offline registration supported (VCS)
 *
 * *****************  Version 44  *****************
 * User: Mushakov     Date: 15.12.09   Time: 17:16
 * Updated in $/VSNA/Servers/ServerManager/Services
 *
 * *****************  Version 43  *****************
 * User: Smirnov      Date: 22.07.08   Time: 12:40
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - automatic load balance is switched off
 *
 * *****************  Version 42  *****************
 * User: Smirnov      Date: 7.06.08    Time: 15:17
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - take into account max_users limit on minimal user server while
 * balansing
 *
 * *****************  Version 41  *****************
 * User: Smirnov      Date: 3.06.08    Time: 21:55
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Redirect v.3
 *
 * *****************  Version 40  *****************
 * User: Ktrushnikov  Date: 22.05.08   Time: 21:39
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Implementation of REDIRECT state in AS commented
 * - Check if NetConfig Exist in AS for redirecting
 * - LoadBalance algo re-written
 * - Overload algo re-written
 *
 * *****************  Version 39  *****************
 * User: Ktrushnikov  Date: 21.05.08   Time: 17:43
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - CheckOverLoad() for users loggged in ASs at the SM
 *
 * *****************  Version 38  *****************
 * User: Dront78      Date: 19.05.08   Time: 15:30
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - redirect with no online servers fixed
 *
 * *****************  Version 37  *****************
 * User: Ktrushnikov  Date: 16.05.08   Time: 19:41
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - SM & AS: LoadBalance algorithm added as MSC_LOADBALANCE
 *
 * *****************  Version 36  *****************
 * User: Ktrushnikov  Date: 28.04.08   Time: 13:53
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - don't stop server if db init failed
 *
 * *****************  Version 35  *****************
 * User: Ktrushnikov  Date: 27.04.08   Time: 19:33
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Average statistics added
 *
 * *****************  Version 34  *****************
 * User: Dront78      Date: 24.04.08   Time: 20:31
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - DNS server thread rewritten
 *
 * *****************  Version 33  *****************
 * User: Ktrushnikov  Date: 15.04.08   Time: 21:29
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - ManageCommand: Redirect added
 *
 * *****************  Version 32  *****************
 * User: Ktrushnikov  Date: 14.04.08   Time: 16:44
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - "l_dns_states.UnLock()" called before
 * "m_dns->UpdateRecords(nfrecords, rfrecords)" to prevent lock for 15
 * seconds
 * - Update DNS in OnPointConnect/Disconnect implemented in Service thread
 * (not in Transport Thread)
 * - Processing BrokerEvents as responces from servers to ManageCommands
 * added
 *
 * *****************  Version 31  *****************
 * User: Ktrushnikov  Date: 12.04.08   Time: 17:29
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Manager commands for AS from SM added
 *
 * *****************  Version 30  *****************
 * User: Ktrushnikov  Date: 11.04.08   Time: 16:58
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - bugfix: set server start time only if BE_START received
 *
 * *****************  Version 29  *****************
 * User: Dront78      Date: 4.04.08    Time: 18:46
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - DDNS algorithms updated
 *
 * *****************  Version 28  *****************
 * User: Mushakov     Date: 25.03.08   Time: 17:50
 * Updated in $/VSNA/Servers/ServerManager/Services
 *  - SSL added
 *  - fixed bug: Connect to server with another name
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 21.03.08   Time: 19:42
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - support for Current stats  (not Average stats): send from AS to SM
 * and saved to Registry::Servers
 * - VS_FileTime: RUS_FMT added: dd.mm.yyyy hh:mm:ss
 * - struct VS_AppServerStats added
 *
 * *****************  Version 26  *****************
 * User: Ktrushnikov  Date: 22.02.08   Time: 12:18
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - don't check server type on endpoint connect, because it is
 * automatically check by m_storage.IsRegisteredAS(prm->uid)
 *
 * *****************  Version 25  *****************
 * User: Ktrushnikov  Date: 22.02.08   Time: 10:25
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Before DDNS_UPDATE check if connected AS is registered (in registry)
 *
 * *****************  Version 24  *****************
 * User: Ktrushnikov  Date: 21.02.08   Time: 16:34
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - bug fixes in Storage
 * - UPDATE_INFO only for registered servers
 * - Get BS/RS algo: 1st online BS and RS
 * - On SM start set servers statuses to zero
 *
 * *****************  Version 23  *****************
 * User: Ktrushnikov  Date: 20.02.08   Time: 16:34
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - new func added: VS_Utils::VS_GetServerType()
 * - SM:ManagerService:DDNS_UPDATE add only AS servers
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 19.02.08   Time: 17:13
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - bugfixes in onConnect callbacks
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 13.02.08   Time: 13:43
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - properties corrected
 * - sturtup sequence of server connects rewrited
 * - code redused
 *
 * *****************  Version 20  *****************
 * User: Dront78      Date: 12.02.08   Time: 15:40
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - unuseble functions removed
 * - updated DDNS multithreading code
 *
 * *****************  Version 19  *****************
 * User: Dront78      Date: 11.02.08   Time: 19:37
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Updated OnDisconnect_CleanUpDNS
 * - fixed wrong call in host to ip conversion
 *
 * *****************  Version 18  *****************
 * User: Dront78      Date: 11.02.08   Time: 17:19
 * Updated in $/VSNA/Servers/ServerManager/Services
 * Possible bug with AV fixed.
 *
 * *****************  Version 17  *****************
 * User: Dront78      Date: 11.02.08   Time: 17:17
 * Updated in $/VSNA/Servers/ServerManager/Services
 * DDNS in registry support added.
 *
 * *****************  Version 16  *****************
 * User: Ktrushnikov  Date: 23.01.08   Time: 15:06
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - init varibles with zero
 * - crash fixed
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 22.01.08   Time: 17:25
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - if no messages - delete endpoint
 * - ControlInquiry type in Forced disconnect endpoint was set to All (not
 * to delete one endpoint)
 * - OnPointConnect/Disconnet conditions added in AppServer
 *
 * *****************  Version 14  *****************
 * User: Ktrushnikov  Date: 16.01.08   Time: 19:15
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - OnPointConnect_Event() added
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 16.01.08   Time: 16:54
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - link errors
 *
 * *****************  Version 12  *****************
 * User: Ktrushnikov  Date: 16.01.08   Time: 16:36
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - DNS fix
 *
 * *****************  Version 11  *****************
 * User: Ktrushnikov  Date: 16.01.08   Time: 16:34
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - VS_DDNS: constant get out from library to SM Manager Service
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 25.12.07   Time: 18:28
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Disable ping on service level in SM
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 18.12.07   Time: 15:44
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - OnEndpointConnect/Disconnect event function params changed
 *
 * *****************  Version 8  *****************
 * User: Avlaskin     Date: 17.12.07   Time: 17:07
 * Updated in $/VSNA/Servers/ServerManager/Services
 *
 * *****************  Version 7  *****************
 * User: Ktrushnikov  Date: 15.12.07   Time: 12:29
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Ping Servers from Storage
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 11.12.07   Time: 17:44
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Use only dns-name (don't use port) in DDNS SRV Updates
 * - Handle DDNS on Connect/Disconnect of broker
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 11.12.07   Time: 15:44
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - DDNS functions support hostname (not just ip)
 * - Funcs in EndpointRegistry to parse buffer and return ConnectTCP
 * (don't write to registry)
 * - Send info about AS's ConnectTCP to SM
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 9.12.07    Time: 20:52
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - Cleanup, when stop server
 * - Async DNS Update support added
 * - Storage Lock fixed
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 5.12.07    Time: 14:25
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - storage update by timer
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 4.12.07    Time: 11:50
 * Updated in $/VSNA/Servers/ServerManager/Services
 * - VS_TransportMessage::Set(): checks fixed for new arch
 * - VS_AppManagerService: Init() overload fixed
 * - VS_AppManagerService: Message processing fixed
 *
 * *****************  Version 1  *****************
 * User: Ktrushnikov  Date: 30.11.07   Time: 11:08
 * Created in $/VSNA/Servers/ServerManager/Services
 * - ServerManagerService added for SM server
 * - Storage for ServerManagerService added
 *
 **************************************************/
#include "process.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../../common/std/cpplib/VS_Utils.h"
#include "../../common/std/cpplib/VS_Replace.h"
#include "../../common/commonlibs/std-generic/cpplib/hton.h"
#include "../../ServerServices/Common.h"
#include "VS_ServerManagerService.h"
#include "acs/Lib/VS_AcsLib.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/ThreadUtils.h"
#include "SecureLib/VS_CertificateIssue.h"
#include "SecureLib/VS_CryptoInit.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_SecureConstants.h"
#include "net/DNSUtils/VS_DNSUtils.h"
#include "net/DNSUtils/VS_DNSTools.h"

#include <memory>
#include <string>
#include <set>

#define DEBUG_CURRENT_MODULE VS_DM_REGS

#define DNS_SCAN_TIMEOUT	15*60*1000

#ifndef DDNS_USE_OFFLINE
//#define DDNS_USE_OFFLINE
#endif

VS_ServerManagerService::VS_ServerManagerService()
	: m_obj_deleted(false)
	, m_timer(CreateEvent(0, true, false, 0))
	, m_loadbalance_time(0)
	, m_redirectas_time(0)
	, m_tr(0)
{
	m_TimeInterval = std::chrono::seconds(15);
}

VS_ServerManagerService::~VS_ServerManagerService()
{

}

bool VS_ServerManagerService::Init(const char* our_endpoint, const char* our_service , const bool permittedAll)
{
	// SSL
	vs::InitOpenSSL();
	net::dns_tools::init_loaded_options();
	m_storage.CheckChangeRegistry();
	m_dbstorage.Init();

	VS_RegistryKey key(false, CONFIGURATION_KEY);
	key.GetString(m_skip_list, "DDNS Skip List");

	for(unsigned int i=0; i < 10; i++)
	{
		if (!ddns_thread[i].ddns.SetParamsFromReg(i+1))
			break;
		VS_SimpleStr key_name;		key_name.Resize(1024);
		sprintf_s(key_name.m_str, 1024, "Configuration\\DDNS%d\\Allowed AS", i+1);
		VS_RegistryKey key(false, key_name);
		if (!key.IsValid())
			break;
		key.ResetValues();
		std::unique_ptr<void, free_deleter> tmp;
		std::string server;
		while (key.NextValue(tmp, VS_REG_STRING_VT, server) > 0)
			m_dns_domains.emplace(std::move(server), &ddns_thread[i]);

		ddns_thread[i].service = this;
		ddns_thread[i].thread_handle = (HANDLE)_beginthreadex(0, 0, Thread, &(ddns_thread[i]), 0, 0);
	}
	////if(m_tr)
	////	m_tr->SetServCertInfoInterface(&m_storage);
	return true;
}


void VS_ServerManagerService::AsyncDestroy()
{
	m_obj_deleted = true;
	SetEvent(m_timer);
	if(m_tr)
		m_tr->SetServCertInfoInterface(0);
	for(unsigned int i=0; i < 10; i++)
	{
		if (!(ddns_thread[i].thread_handle))
			continue;
		WaitForSingleObject(ddns_thread[i].thread_handle, DNS_SCAN_TIMEOUT);
		CloseHandle(ddns_thread[i].thread_handle);
	}
}


bool VS_ServerManagerService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)  return true;
	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
		m_recvMess = recvMess.get();
		{
			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
				const char* method = 0;
				if ( (method = cnt.GetStrValueRef(METHOD_PARAM)) != 0 ) {
					dprint2("Method = %s\n", method);

					if ( _stricmp(method, UPDATEINFO_METHOD) == 0 ) {
						OnUpdateInfo(cnt, m_recvMess->SrcServer());

					} else if ( _stricmp(method, POINTCONNECTED_METHOD) == 0 ) {
						const char* param = 0;
						if ((param = cnt.GetStrValueRef(NAME_PARAM)) != 0 )
							OnConnect_UpdateDNS(param);

					} else if ( _stricmp(method, POINTDISCONNECTED_METHOD) == 0 ) {
						const char* param = 0;
						if ((param = cnt.GetStrValueRef(NAME_PARAM)) != 0 )
							OnDisconnect_CleanUpDNS(param);

					} else if ( _stricmp(method, LOGEVENT_METHOD) == 0 ) {
						int32_t be = BE_UNKNOWN;
						cnt.GetValue(TYPE_PARAM, be);
						if ( be == BE_START ) {
							m_storage.SetServerStat_StartTime( m_recvMess->SrcServer() );
							m_storage.SetRedirected(m_recvMess->SrcServer(), 0);

						} else if ( be == BE_STATESET ) {
							int32_t state = 0;		cnt.GetValue(SUBTYPE_PARAM, state);
							dprint1("StateSet(%d) for %s\n", state, m_recvMess->SrcServer());
							if ( state == SSTATE_REDIRECT )
								m_storage.SetRedirected(m_recvMess->SrcServer(), 1);

						} else if ( be == BE_PROPSREFRESHED ) {
							dprint1("Properties refreshed for %s\n", m_recvMess->SrcServer());
						}
					} else if ( _stricmp(method, LOGSTATS_METHOD) == 0 ) {
						OnLogStats_Method(cnt);
					}
					else if (_stricmp(method, REGISTERSERVER_METHOD) == 0)
					{
						RegisterServer_Method(cnt.GetStrValueRef(SERVERID_PARAM),
											cnt.GetStrValueRef(SERVERNAME_PARAM),
											cnt.GetStrValueRef(KEY_PARAM),
											cnt.GetStrValueRef(PASSWORD_PARAM),
											cnt.GetStrValueRef(CERT_REQUEST_PARAM),
											cnt.GetStrValueRef("Srv Version"));
					}
					else if(_stricmp(method, CERTIFICATEUPDATE_METHOD) == 0)
					{
						CertificateUpdate_Method(cnt);
					}
					else if (_stricmp(method, REGISTERSERVEROFFLINE_METHOD) == 0)
					{

						//long	productID(0);
						//if(!cnt.GetValue(PRODUCTID_PARAM,productID))
						//	productID = 0;

						//bool reg_ok = RegisterServer_Method(cnt.GetStrValueRef(ENDPOINT_PARAM),
						//					cnt.GetStrValueRef(SERVERNAME_PARAM),
						//					cnt.GetStrValueRef(PASSWORD_PARAM),
						//					cnt.GetStrValueRef(CERT_REQUEST_PARAM),
						//					productID, true);

					}else if (_stricmp(method,GETALLAPPPROPERTIES_METHOD) == 0){
						GetAllAppProperties_Method();
					}else if (_stricmp(method, GETASOFMYDOMAIN_METHOD) == 0){
						GetASOfMyDomain_Method(m_recvMess->SrcServer_sv());
					}
				}
			}
		}
		break;
	case transport::MessageType::Reply:
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return true;
}

bool VS_ServerManagerService::OnUpdateInfo(VS_Container &cnt, const char* from_sid)
{
	if ( !from_sid || !*from_sid )
		return false;

	auto server_type = VS_GetServerType(from_sid);

	VS_SimpleStr asdomain;

	if (!m_storage.IsRegisteredAS(from_sid, &asdomain) && server_type != ST_BS) // only for registered servers
		return false;

	VS_SimpleStr bs;
	VS_SimpleStr rs;

	m_storage.GetLocatorBS(bs);
	m_storage.GetRS(rs);

	VS_Container cnt2;
	cnt2.AddValue(METHOD_PARAM, UPDATEINFO_METHOD);
	cnt2.AddValue(LOCATORBS_PARAM, bs.m_str);
	cnt2.AddValue(RS_PARAM, rs.m_str);
	cnt2.AddValue(ASDOMAIN_PARAM, asdomain);

	if (server_type != ST_BS)		// add only for AS
	{
		auto m = m_storage.GetAllBS();
		for(auto const& p : m)
		{
			cnt2.AddValue("domain", p.first);
			for(auto const& i: p.second)
				cnt2.AddValue("BS", i);
		}
	}

	if (server_type == ST_AS)
		m_storage.GetRSByDomain(cnt2, asdomain);
	else if (server_type == ST_BS)
		m_storage.GetAllRS(cnt2);

	PostReply(cnt2);
	return true;
}
bool VS_ServerManagerService::RegisterServer_Method(const char *server_id,const char * server_name,const char* key,const char* pass,
													const char *cert_request, const char *version, bool IsOffline)
{
	if(!server_name || !*server_name)
		return false;

	bool	isServVerified = !!m_recvMess->SrcServer()&&IsAuthorized(m_recvMess->SrcServer())&&(!strcasecmp(m_recvMess->SrcServer(),server_name));


	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::unique_ptr<char, free_deleter> SM_Certificate;
	int SM_cert_sz = rkey.GetValue(SM_Certificate, VS_REG_BINARY_VT, SRV_CERT_KEY);

	if (SM_cert_sz && SM_Certificate.get()[SM_cert_sz-1])
	{
		char *tmp = (char*)malloc(SM_cert_sz + 1);
		memcpy(tmp, SM_Certificate.get(), SM_cert_sz);
		tmp[SM_cert_sz] = 0;
		SM_Certificate.reset(tmp);
		SM_cert_sz+=1;
	}

	long result(0);
	int regRes(0);
	dprint3("trying to register broker %s,serial='%s'\n",server_id,(const char*)pass);
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, (IsOffline)? REGISTERSERVEROFFLINE_METHOD: REGISTERSERVER_METHOD);
	VS_SimpleStr out_key;
	VS_SimpleStr cert;

	if(m_storage.RegisterServer(server_id,server_name,pass,key,cert_request,version,isServVerified,out_key,cert,regRes))
	{
		cnt.AddValue(KEY_PARAM,(const char*)out_key);
		cnt.AddValue(CERTIFICATE_PARAM,cert);
		cnt.AddValue(SM_CERTIFICATE_PARAM, SM_Certificate.get());
		cnt.AddValue(CERTIFICATE_CHAIN_PARAM, SM_Certificate.get());
	}
	result = regRes;
	cnt.AddValueI32(RESULT_PARAM, result);

	PostReply(cnt);
	return (result == 1);
}

bool VS_ServerManagerService::CertificateUpdate_Method(VS_Container &cnt)
{
	const char *cert_buf = cnt.GetStrValueRef(CERTIFICATE_PARAM);
	const char *cert_req = cnt.GetStrValueRef(CERT_REQUEST_PARAM);
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::unique_ptr<char, free_deleter> SM_Certificate;
	int SM_cert_sz = rkey.GetValue(SM_Certificate, VS_REG_BINARY_VT, SRV_CERT_KEY);
	if (SM_cert_sz && SM_Certificate.get()[SM_cert_sz-1])
	{
		char *tmp = (char*)malloc(SM_cert_sz + 1);
		memcpy(tmp, SM_Certificate.get(), SM_cert_sz);
		tmp[SM_cert_sz] = 0;
		SM_Certificate.reset(tmp);
		SM_cert_sz+=1;
	}

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM,CERTIFICATEUPDATE_METHOD);
	VS_SimpleStr new_cert;
	long result(0);
	VS_Certificate	cert;
	if(!cert.SetCert(cert_buf,strlen(cert_buf)+1,store_PEM_BUF))
		return false;
	if(!m_recvMess->SrcServer() || !IsAuthorized(m_recvMess->SrcServer()))
		result = 0;
	else if(m_storage.UpdateServerCertificate(m_recvMess->SrcServer(),cert_req, new_cert,result))
	{
		result = 1;
		rCnt.AddValue(CERTIFICATE_PARAM,new_cert);
		rCnt.AddValue(SM_CERTIFICATE_PARAM, SM_Certificate.get());
		rCnt.AddValue(CERTIFICATE_CHAIN_PARAM, SM_Certificate.get());
	}
	rCnt.AddValueI32(RESULT_PARAM, result);
	PostReply(rCnt);
	return result==1;
}

bool VS_ServerManagerService::OnPointConnected_Event(const VS_PointParams* prm)
{
	if (prm->reazon <= 0) {
		dprint1("Error {%d} while Connect to (%s)\n", prm->reazon, prm->uid);
	}
	else if (prm->type==VS_PointParams::PT_SERVER) {
		dprint2("Server Connect: uid=%s, reason: %2d\n", prm->uid, prm->reazon);

		if ( m_storage.IsRegisteredAS(prm->uid) )		// only for registered AS servers
		{
			// LocalRequest
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, POINTCONNECTED_METHOD);
			cnt.AddValue(NAME_PARAM, prm->uid);

			PostRequest(OurEndpoint(), 0, cnt);
		}
		m_storage.ServerStatus(prm->uid, 1);	// set online
	}
	else {
		dprint1("NOT SERVER Connect!: uid=%s, reason: %2d\n", prm->uid, prm->reazon);
	}
	return true;
}


bool VS_ServerManagerService::OnPointDisconnected_Event(const VS_PointParams* prm)
{
	if (prm->type==VS_PointParams::PT_SERVER) {
		dprint2("Server DisConnect: uid=%s, reason: %2d\n", prm->uid, prm->reazon);

		if ( m_storage.IsRegisteredAS(prm->uid) )
		{
			// LocalRequest
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, POINTDISCONNECTED_METHOD);
			cnt.AddValue(NAME_PARAM, prm->uid);

			PostRequest(OurEndpoint(), 0, cnt);
		}
		m_storage.ServerStatus(prm->uid, 0);	// set offline
	}
	return true;
}

bool VS_ServerManagerService::Timer(unsigned long tick)
{
	m_storage.CheckChangeRegistry();

	if (tick-m_redirectas_time > 4*60*1000 ) {
		// process domains with more than 2 online servers

		std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo> v;
		m_storage.GetAllAS(v);
		std::map<std::string, int32_t> dns;
		for (auto &it : v)
			if (it.m_status == 1 && !it.m_domain.IsEmpty())
				dns[it.m_domain.m_str]++;

		if (tick-m_loadbalance_time > 15*60*1000) { //at 4th time must be true, 4*4 > 15
			for (auto &it : dns)
				if (it.second > 1)
					LoadBalance(it.first.c_str());

			m_loadbalance_time = tick;
		}
		else {
			for (auto &it : dns)
				if (it.second > 1)
					RedirectAS(it.first.c_str());
		}
		m_redirectas_time = tick;
	}

	VS_SimpleStr sid;
	long mgr_cmd = MSC_NONE;
	long mgr_cmd_param = 0;

	m_storage.ResetManageCommandIndex();
	while ( m_storage.GetNextManageCommand(sid, mgr_cmd, mgr_cmd_param) )
	{
		dprint1("SendCommand(%ld, %ld) for %s\n", mgr_cmd, mgr_cmd_param, sid.m_str);
		SendCommandToServer(sid, mgr_cmd, mgr_cmd_param);
	}
	return true;
}

bool VS_ServerManagerService::OnDisconnect_CleanUpDNS(const char* endpoint_name)
{
	if (!endpoint_name || !*endpoint_name) { return false; }

	std::string ep = endpoint_name;
	if (m_skip_list.length() && m_skip_list.find(ep)!=std::string::npos)
		return false;

	std::multimap<std::string, UpdateThreadArgs*>::iterator it=m_dns_domains.find(ep);
	if (it==m_dns_domains.end())
		return false;
	std::string key = ep;
	for(; it!=m_dns_domains.upper_bound(key); it++)
	{
		UpdateThreadArgs* dns = it->second;

		VS_AutoLock lock(&dns->online_servers_lock);
		std::string srv = (dns->ddns.GetRequestName())? dns->ddns.GetRequestName(): "";
		if (srv.find(net::dns::VCS2_DDNS_SRV)!=std::string::npos)		// vcs2.tcp
		{
			if (ep.find('#', 0)!=std::string::npos)
				ep = ep.substr(0, ep.find('#', 0));
		}
		std::vector<std::string>::iterator _i = std::find(dns->online_servers.begin(), dns->online_servers.end(), ep);
		while (_i != dns->online_servers.end()) {
			dns->online_servers.erase(_i);
			_i = std::find(dns->online_servers.begin(), dns->online_servers.end(), ep);
		}
	}

	return true;
}


bool VS_ServerManagerService::OnConnect_UpdateDNS(const char *endpoint_name)
{
	if (!endpoint_name || !*endpoint_name) { return false; }

	std::string ep = endpoint_name;
	if (m_skip_list.length() && m_skip_list.find(ep)!=std::string::npos)
		return true;

	std::multimap<std::string, UpdateThreadArgs*>::iterator it=m_dns_domains.find(ep);
	if (it==m_dns_domains.end())
		return false;
	std::string key = ep;
	for(; it!=m_dns_domains.upper_bound(key); it++)
	{
		UpdateThreadArgs* dns = it->second;

		VS_AutoLock lock(&dns->online_servers_lock);
		std::string srv = (dns->ddns.GetRequestName())? dns->ddns.GetRequestName(): "";
		if (srv.find(net::dns::VCS2_DDNS_SRV)!=std::string::npos)		// vcs2.tcp
		{
			if (ep.find('#', 0)!=std::string::npos)
				ep = ep.substr(0, ep.find('#', 0));
		}
		std::vector<std::string>::iterator _i = std::find(dns->online_servers.begin(), dns->online_servers.end(), ep);
		if (_i == dns->online_servers.end()) { dns->online_servers.push_back(ep); }
	}

	return true;
}

bool VS_ServerManagerService::OnLogStats_Method(VS_Container &cnt)
{
	if ( !m_recvMess )
		return false;

	const char* sid = m_recvMess->SrcServer();
	if ( !sid )
		return false;

	int32_t type = 0;
	if ( !cnt.GetValue(TYPE_PARAM, type) )
		return false;

	if ( type == 1 ) {					// Average stats
		size_t tmp_sz = 0;
		VS_MediaBrokerStats* stats = (VS_MediaBrokerStats*) cnt.GetBinValueRef(MEDIABROKERSTATS_PARAM, tmp_sz);
		if ( !stats || (tmp_sz != sizeof(VS_MediaBrokerStats)) )
			return false;

		m_dbstorage.LogStats(m_recvMess->SrcServer(), stats);

	}else if ( type == 2 ) {			// Current stats
		size_t tmp_sz = 0;
		VS_AppServerStats* stats = (VS_AppServerStats*) cnt.GetBinValueRef(MEDIABROKERSTATS_PARAM, tmp_sz);
		if ( !stats || (tmp_sz != sizeof(VS_AppServerStats)) )
			return false;

		m_storage.SetServerStat(sid, stats);

	} else {				// Unknown stats
		return false;
	}

	return true;
}

void VS_ServerManagerService::UpdateServersThread(UpdateThreadArgs* arg)
{
	extern unsigned long DNS_SRV_TTL;

	dprint3("DNS: thread for %s started\n", arg->ddns.GetRequestName());

	std::vector<int> ports;
	ports.push_back(4307);
	//ports.push_back(80);
	//ports.push_back(443);

	while (!arg->service->m_obj_deleted) {
		WaitForSingleObject(arg->service->m_timer, DNS_SCAN_TIMEOUT);
		if (arg->service->m_obj_deleted) { break; }

		std::set<std::string> online_ips;
		{	// get IP of all AS-servers
			int net_interface = 5;
			char *ips[5];
			for(int i =0;i<net_interface;i++)
				ips[i] = new char[256];
			arg->online_servers_lock.Lock();
			for (std::vector<std::string>::iterator it=arg->online_servers.begin(); it!=arg->online_servers.end(); ++it)
			{
				VS_SimpleStr host = it->c_str();		// skip server_type (#as)
				char* pos = strchr(host, '#');
				if (pos)
					*pos = 0;

				unsigned long ip = 0;
				unsigned int ips_count = VS_GetHostsByName(host,ips,net_interface,255);
				if (!ips_count)
					continue;
				for(unsigned int i=0; i<ips_count; i++)
					online_ips.insert(ips[i]);
			}
			arg->online_servers_lock.UnLock();
			for(int i = 0;i<net_interface;i++)
				delete [] ips[i];
		}

		std::vector<std::string> _dnsservers;
		const char* pReq = arg->ddns.GetRequestName();

		{
			auto res = net::dns::make_srv_lookup(pReq).get();
			if (!res.second) //no err
			{
				for (auto &item : res.first)
				{
					if (item.priority == arg->ddns.GetPriority() && item.weight == arg->ddns.GetWeight()) {
						_dnsservers.emplace_back(std::move(item.host));
					}
				}
			}
		}

		dprint3("DNS SRV: %s records count = %zu\n", pReq, _dnsservers.size());

		std::vector<std::string> current_srv_records;
		if (arg->ddns.GetUpdateSRVType() == VS_DDNS::e_hostname || arg->ddns.GetUpdateSRVType() == VS_DDNS::e_none) {
			VS_AutoLock lock(&arg->online_servers_lock);
			current_srv_records = arg->online_servers;
		} else if (arg->ddns.GetUpdateSRVType() == VS_DDNS::e_ip) {
			current_srv_records.assign(online_ips.begin(), online_ips.end());
		}

		std::vector<std::string> _addservers;
		std::vector<std::string> _killservers;

		// check records to remove
		for (size_t i = 0; i < _dnsservers.size(); ++i) {
			std::vector<std::string>::iterator _ikill = std::find(current_srv_records.begin(), current_srv_records.end(), _dnsservers.at(i));
			if (_ikill == current_srv_records.end()) { _killservers.push_back(_dnsservers.at(i)); }
		}
		dprint3("DNS SRV: %zu records to kill\n", _killservers.size());

		// check records to add
		for (size_t i = 0; i < current_srv_records.size(); ++i) {
			std::vector<std::string>::iterator _iadd = std::find(_dnsservers.begin(), _dnsservers.end(), current_srv_records.at(i));
			if (_iadd == _dnsservers.end() ||
				(std::count(_dnsservers.begin(), _dnsservers.end(), current_srv_records.at(i)) < ports.size()))		// not all ports present in DNS
			{ _addservers.push_back(current_srv_records.at(i)); }
		}
		dprint3("DNS SRV: %zu records to add\n", _addservers.size());

 		// start dns update
		DNS_RECORDA record_SRV = {0, arg->ddns.GetRequestName(), DNS_TYPE_SRV, sizeof(DNS_RECORDA), {}, DNS_SRV_TTL, 0, {}};
		record_SRV.Data.SRV.wPriority = arg->ddns.GetPriority();
		record_SRV.Data.SRV.wWeight = arg->ddns.GetWeight();

 		const size_t bsize = 2048;
 		auto buff = std::make_unique<char[]>(bsize);
		auto cstr = std::make_unique<char[]>(bsize);

		for(std::vector<int>::iterator it=ports.begin(); it!=ports.end(); ++it)
		{
			record_SRV.Data.SRV.wPort = *it;

			for (size_t i = 0; i < _killservers.size(); ++i) {
				strncpy(cstr.get(), _killservers.at(i).c_str(), bsize);
				record_SRV.Data.SRV.pNameTarget = cstr.get();
				size_t pcksize = arg->ddns.eMakeDnsSrvPacket(record_SRV, buff.get(), bsize, true);
				bool b = arg->ddns.eUpdateDnsRecord(buff.get(), pcksize);
				dprint1("DNS SRV: kill server %s:%d from %s : %s\n", cstr.get(), *it, arg->ddns.GetRequestName(), (b) ? "OK" : "FAILED");
			}

			for (size_t i = 0; i < _addservers.size(); ++i) {
				strncpy(cstr.get(), _addservers.at(i).c_str(), bsize);
				record_SRV.Data.SRV.pNameTarget = cstr.get();
				size_t pcksize = arg->ddns.eMakeDnsSrvPacket(record_SRV, buff.get(), bsize, false);
				bool b = arg->ddns.eUpdateDnsRecord(buff.get(), pcksize);
				dprint1("DNS SRV: add server %s:%d to %s : %s\n", cstr.get(), *it, arg->ddns.GetRequestName(), (b) ? "OK" : "FAILED");
			}
		}

 		_dnsservers.clear();
 		_addservers.clear();
 		_killservers.clear();

		{	// update A records
			// Get current A record
			std::string request_name = arg->ddns.GetRequestName();
			if (request_name[request_name.length() - 1] == '.')
				request_name = request_name.substr(0, request_name.length() - 1);

			const char* req = arg->ddns.GetRequestName();

			{
				auto res = net::dns::make_a_lookup(req).get();
				if (!res.second)
				{
					net::dns::hostent_reply &host = res.first;
					if (host.name == request_name)
					{
						for (auto &addr : host.addrs)
						{
							_dnsservers.push_back(addr.to_string());
						}
					}
				}
			}

			dprint3("DNS A: %s records count = %zu\n", arg->ddns.GetRequestName(), _dnsservers.size());

			// check A-records to remove
			for (std::vector<std::string>::iterator it=_dnsservers.begin(); it!=_dnsservers.end(); ++it) {
				if (online_ips.find(*it)==online_ips.end()){ _killservers.push_back(*it); }
			}
			dprint3("DNS A: %zu records to kill\n", _killservers.size());

			// check A-records to add
			for (std::set<std::string>::iterator it=online_ips.begin(); it!=online_ips.end(); ++it) {
				if (std::find(_dnsservers.begin(),_dnsservers.end(),*it)==_dnsservers.end()) { _addservers.push_back(*it); }
			}
			dprint3("DNS A: %zu records to add\n", _addservers.size());


			DNS_RECORDA record_A = {0, arg->ddns.GetRequestName(), DNS_TYPE_A, sizeof(DNS_RECORDA), {}, DNS_SRV_TTL, 0, {}};

			for (size_t i = 0; i < _killservers.size(); ++i) {
				unsigned long ip(0);
				VS_GetIpByHost(_killservers.at(i).c_str(), &ip);
				record_A.Data.A.IpAddress = vs_htonl(ip);
				bool b = arg->ddns.UpdateRecords(0, &record_A, true);
				dprint1("DNS A: kill server %s from %s : %s\n", _killservers.at(i).c_str(), arg->ddns.GetRequestName(), (b)?"OK": "FAILED");
			}

			for (size_t i = 0; i < _addservers.size(); ++i) {
				unsigned long ip(0);
				VS_GetIpByHost(_addservers.at(i).c_str(), &ip);
				record_A.Data.A.IpAddress = vs_htonl(ip);
				bool b = arg->ddns.UpdateRecords(&record_A, 0, true);
				dprint1("DNS A: add server %s to %s : %s\n", _addservers.at(i).c_str(), arg->ddns.GetRequestName(), (b)?"OK": "FAILED");
			}
		}
	}

	dprint3("DNS: thread for %s finished\n", arg->ddns.GetRequestName());
}

unsigned VS_ServerManagerService::Thread(void *arg)
{
	vs::SetThreadName("SM_UpdateSrvs");
	UpdateThreadArgs* a = (UpdateThreadArgs*) arg;
	a->service->UpdateServersThread(a);
	return 0;
}

void VS_ServerManagerService::SendCommandToServer(const char* sid, long mgr_cmd, long mgr_cmd_param, const char* redir_server)
{
	if ( !sid )
		return ;

	VS_Container cnt;
	if (redir_server)
		cnt.AddValue(SERVER_PARAM, redir_server);
	SendCommandToServer(sid, mgr_cmd, mgr_cmd_param, cnt);
}

void VS_ServerManagerService::SendCommandToServer(const char* sid, long mgr_cmd, long mgr_cmd_param, VS_Container &cnt)
{
	if ( !sid )
		return ;
	cnt.AddValue(METHOD_PARAM, MANAGESERVER_METHOD);
	cnt.AddValueI32(TYPE_PARAM, mgr_cmd);
	cnt.AddValueI32(SUBTYPE_PARAM, mgr_cmd_param);
	PostRequest(sid, 0, cnt, NULL, MANAGER_SRV);
}


void VS_ServerManagerService::LoadBalance(string_view domain)
{
	std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo> v;
	m_storage.GetAllAS(domain, v);
	if ( !v.size() )
		return ;

	// skip Offline & Redirected servers
	std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo>::iterator it = v.begin();
	while(it != v.end()) {
		if (it->m_status <= 0 || it->m_redirected)
			it = v.erase(it);
		else
			it++;
	}
	if ( !v.size() )
		return ;

	// Сформировать первую группу [online <= max - 50]
	// ЭТО ТЕ, НА КОТОРЫЕ МОЖНО РЕДИРЕКТИТЬ
	std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo> v1;
	for(it = v.begin(); it != v.end(); it++) {
		if (it->m_max_users == -1 || (long)(it->m_max_users - it->m_online_users) > 50 )
			v1.push_back(*it);
	}
	if ( !v1.size() )		// нет доступных для редиректа серверов
		return ;

	// Редирект в первой группе
	if ( v1.size() > 1 ) {		// we need more then 1 server
		// get max & min
		std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo>::iterator max = v1.begin();
		std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo>::iterator min = v1.begin();
		for(it = v1.begin(); it != v1.end(); it++) {
			if ( max->m_online_users < it->m_online_users )
				max = it;
			if ( min->m_online_users > it->m_online_users )
				min = it;
		}

		if (max != min) {
			unsigned long delta = (max->m_online_users - min->m_online_users)/2;
			if (delta > min->m_max_users - min->m_online_users)
				delta = min->m_max_users - min->m_online_users;
			if (delta > 50) {
				//dprint2("Balance: Redirect %d users from %s to %s\n", delta, max->m_dns_name, min->m_dns_name);
				//SendCommandToServer(max->m_dns_name, MSC_LOADBALANCE, delta, min->m_dns_name);
			}
		}
	}

	// Сформировать вторую группу [online >= max + 50]
	// ЭТО ТЕ, КОТОРЫЕ ПЕРЕГРУЖЕНЫ И НУЖДАЮТСЯ В РЕДИРЕКТЕ
	std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo> v2;
	for(it = v.begin(); it != v.end(); it++) {
		if ( (it->m_max_users != -1) && ((long)(it->m_online_users - it->m_max_users) > 50) )
			v2.push_back(*it);
	}

	// Редирект во второй группе
	for(unsigned int i=0; i < v2.size(); i++) {
		unsigned long n_users = v2[i].m_online_users - v2[i].m_max_users;
		VS_Container cnt;
		dprint2("Overload: Redirect %ld users from %s to %zu servers:\n", n_users, v2[i].m_dns_name.m_str, v1.size());

		for(unsigned int f = 0; f < v1.size(); f++) {
			dprint2("\t<%s>\n", v1[f].m_dns_name.m_str);
			cnt.AddValue(SERVER_PARAM, v1[f].m_dns_name);
		}
		SendCommandToServer(v2[i].m_dns_name, MSC_LOADBALANCE, n_users, cnt);
	}
}

void VS_ServerManagerService::RedirectAS(string_view domain)
{
	std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo> v;
	m_storage.GetAllAS(domain, v);
	if ( !v.size() )
		return ;

	// Сформировать первую группу [online <= max - 50]
	// v1 - ЭТО ТЕ, НА КОТОРЫЕ МОЖНО РЕДИРЕКТИТЬ
	// r  - КОТОРЫЕ НУЖНО РЕДИРЕКТИТЬ
	std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo>::iterator it = v.begin();
	std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo> v1;
	std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo> r;
	for(it = v.begin(); it != v.end(); it++) {
		if ( it->m_status <= 0 )
			continue;

		if ( !it->m_redirected ) {
			if (it->m_max_users == -1 || (long)(it->m_max_users - it->m_online_users) > 50)
				v1.push_back(*it);
		}
		else {
			if (it->m_online_users)
				r.push_back(*it);
		}
	}

	if ( !v1.size() || !r.size() )		// нет доступных для редиректа серверов или нуждающихся в нем
		return ;

	// Редирект из серверов в состоянии redirect на первую группу
	for(unsigned int i=0; i < r.size(); i++) {
		VS_Container cnt;
		dprint2("Redirect: Redirect %ld users from %s to %zu servers:\n", r[i].m_online_users, r[i].m_dns_name.m_str, v1.size());

		for(unsigned int f = 0; f < v1.size(); f++) {
			dprint2("\t<%s>\n", v1[f].m_dns_name.m_str);
			cnt.AddValue(SERVER_PARAM, v1[f].m_dns_name);
		}
		SendCommandToServer(r[i].m_dns_name, MSC_LOADBALANCE, r[i].m_online_users, cnt);
	}
}

void VS_ServerManagerService::GetAllAppProperties_Method()
{
	VS_AppPropertiesMap prop_map;
	if(m_dbstorage.GetAllAppProperties(prop_map))
	{
		VS_Container *cnt(0);
		for(VS_AppPropertiesMap::iterator i = prop_map.begin();i!=prop_map.end();i++)
		{
			cnt = i->second;
			cnt->AddValue(METHOD_PARAM, SETPROPERTIES_METHOD);
			PostReply(*cnt);
			delete cnt;
		}
		prop_map.clear();
	}
}

void VS_ServerManagerService::GetASOfMyDomain_Method(string_view src_server)
{
	if (src_server.empty() || VS_GetServerType(src_server) != ST_BS)
		return;

	// get domain of this BS
	string_view bs_domain;
	auto m = m_storage.GetAllBS();
	for (auto const& p : m)
		for (auto const& i : p.second)
			if (i == src_server)
			{
				bs_domain = p.first.m_str;
				break;
			}
	if (bs_domain.empty())
	{
		dstream4 << "GetMyDomainAS: bs_domain not found for " << src_server;
		return;
	}

	std::vector<VS_ServerManagerStorage::VS_ServerManager_ServerInfo> v;
	m_storage.GetAllAS(bs_domain, v);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, GETASOFMYDOMAIN_METHOD);
	for (auto const& i : v)
		rCnt.AddValue(SERVER_PARAM, i.m_dns_name);
	PostReply(rCnt);
}

VS_ServCertInfoInterface::get_info_res
VS_ServerManagerService::GetPublicKey(
	const VS_SimpleStr& server_name,
	VS_SimpleStr &pub_key,uint32_t &vcs_ver)
{
	return m_storage.GetPublicKey(server_name,pub_key,vcs_ver);
}
VS_ServCertInfoInterface::get_info_res
VS_ServerManagerService::GetServerCertificate(
	const VS_SimpleStr &server_name, VS_SimpleStr &cert)
{
	return m_storage.GetServerCertificate(server_name,cert);
}