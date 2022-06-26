/**************************************************
 * $Revision: 67 $
 * $History: VS_AppManagerService.cpp $
 *
 * *****************  Version 67  *****************
 * User: Mushakov     Date: 18.07.12   Time: 20:28
 * Updated in $/VSNA/Servers/AppServer/Services
 * - call by alias when users are on separate servers
 *
 * *****************  Version 66  *****************
 * User: Smirnov      Date: 17.10.11   Time: 21:57
 * Updated in $/VSNA/Servers/AppServer/Services
 * - add service name in config
 *
 * *****************  Version 65  *****************
 * User: Mushakov     Date: 20.05.11   Time: 17:33
 * Updated in $/VSNA/Servers/AppServer/Services
 * - new rs supported
 *
 * *****************  Version 64  *****************
 * User: Ktrushnikov  Date: 25.10.10   Time: 12:21
 * Updated in $/VSNA/Servers/AppServer/Services
 * offline chat messages
 * - send to CHAT_SRV using LOCATE_SRV (not directly)
 * - return not only one BS at GetFirstBS
 *
 * *****************  Version 63  *****************
 * User: Mushakov     Date: 15.10.10   Time: 16:27
 * Updated in $/VSNA/Servers/AppServer/Services
 * - defaultDomain added
 *
 * *****************  Version 62  *****************
 * User: Mushakov     Date: 29.09.10   Time: 18:12
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - video-port.com - bs doman by default
 *
 * *****************  Version 61  *****************
 * User: Ktrushnikov  Date: 24.08.10   Time: 20:00
 * Updated in $/VSNA/Servers/AppServer/Services
 * - deadlock fix (AS at car2)
 *
 * *****************  Version 60  *****************
 * User: Mushakov     Date: 30.07.10   Time: 14:07
 * Updated in $/VSNA/Servers/AppServer/Services
 * locatorBS is not vzo.me
 *
 * *****************  Version 59  *****************
 * User: Mushakov     Date: 29.07.10   Time: 19:28
 * Updated in $/VSNA/Servers/AppServer/Services
 * - bs for vzo.me in LOCATORBS_PARAM
 *
 * *****************  Version 58  *****************
 * User: Ktrushnikov  Date: 21.07.10   Time: 15:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - the same as previous
 *
 * *****************  Version 57  *****************
 * User: Ktrushnikov  Date: 21.07.10   Time: 15:35
 * Updated in $/VSNA/Servers/AppServer/Services
 * - set from_service to recieve replay from SM in right place
 *
 * *****************  Version 56  *****************
 * User: Ktrushnikov  Date: 19.07.10   Time: 18:29
 * Updated in $/VSNA/Servers/AppServer/Services
 * AS asks SM (not BS) for AppProps
 * - by SM connected
 * - by manual request from support web
 *
 * *****************  Version 55  *****************
 * User: Mushakov     Date: 30.06.10   Time: 14:47
 * Updated in $/VSNA/Servers/AppServer/Services
 * av fixed
 *
 * *****************  Version 54  *****************
 * User: Mushakov     Date: 24.06.10   Time: 16:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - opt disabled when SECUREBEGIN_
 * - locator bs removed
 *
 * *****************  Version 53  *****************
 * User: Mushakov     Date: 21.06.10   Time: 15:36
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - doman -> domain
 *
 * *****************  Version 52  *****************
 * User: Mushakov     Date: 15.06.10   Time: 15:32
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - sharding
 *
 * *****************  Version 51  *****************
 * User: Smirnov      Date: 3.06.08    Time: 21:55
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Redirect v.3
 *
 * *****************  Version 50  *****************
 * User: Mushakov     Date: 2.06.08    Time: 20:12
 * Updated in $/VSNA/Servers/AppServer/Services
 * - fixed AV in MSC_Redirect
 *
 * *****************  Version 49  *****************
 * User: Ktrushnikov  Date: 22.05.08   Time: 21:39
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Implementation of REDIRECT state in AS commented
 * - Check if NetConfig Exist in AS for redirecting
 * - LoadBalance algo re-written
 * - Overload algo re-written
 *
 * *****************  Version 48  *****************
 * User: Ktrushnikov  Date: 21.05.08   Time: 17:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * - CheckOverLoad() for users loggged in ASs at the SM
 *
 * *****************  Version 47  *****************
 * User: Ktrushnikov  Date: 16.05.08   Time: 19:41
 * Updated in $/VSNA/Servers/AppServer/Services
 * - SM & AS: LoadBalance algorithm added as MSC_LOADBALANCE
 *
 * *****************  Version 46  *****************
 * User: Smirnov      Date: 15.05.08   Time: 21:19
 * Updated in $/VSNA/Servers/AppServer/Services
 * - fixed bug when BS or RS connected first
 *
 * *****************  Version 45  *****************
 * User: Ktrushnikov  Date: 14.05.08   Time: 21:25
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added ktrushnikov_Redirect
 *
 * *****************  Version 44  *****************
 * User: Ktrushnikov  Date: 6.05.08    Time: 19:59
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Redirect to N online AS in cycle (not to first online AS)
 *
 * *****************  Version 43  *****************
 * User: Ktrushnikov  Date: 27.04.08   Time: 19:33
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Average statistics added
 *
 * *****************  Version 42  *****************
 * User: Smirnov      Date: 25.04.08   Time: 19:25
 * Updated in $/VSNA/Servers/AppServer/Services
 * - RS and BS online UP fixed
 *
 * *****************  Version 41  *****************
 * User: Ktrushnikov  Date: 14.04.08   Time: 16:45
 * Updated in $/VSNA/Servers/AppServer/Services
 * - BrokerEvent-responces added for ManageCommands from SM
 *
 * *****************  Version 40  *****************
 * User: Ktrushnikov  Date: 12.04.08   Time: 17:33
 * Updated in $/VSNA/Servers/AppServer/Services
 * - fix: dprint5 changed to dprint4
 *
 * *****************  Version 39  *****************
 * User: Ktrushnikov  Date: 12.04.08   Time: 17:29
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Manager commands for AS from SM added
 *
 * *****************  Version 38  *****************
 * User: Smirnov      Date: 11.04.08   Time: 17:32
 * Updated in $/VSNA/Servers/AppServer/Services
 * - protocol cleaning
 *
 * *****************  Version 37  *****************
 * User: Smirnov      Date: 4.04.08    Time: 19:53
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Server Disconnect procedure
 *
 * *****************  Version 36  *****************
 * User: Smirnov      Date: 31.03.08   Time: 20:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * - event based connects to server
 *
 * *****************  Version 35  *****************
 * User: Ktrushnikov  Date: 21.03.08   Time: 19:42
 * Updated in $/VSNA/Servers/AppServer/Services
 * - support for Current stats  (not Average stats): send from AS to SM
 * and saved to Registry::Servers
 * - VS_FileTime: RUS_FMT added: dd.mm.yyyy hh:mm:ss
 * - struct VS_AppServerStats added
 *
 * *****************  Version 34  *****************
 * User: Smirnov      Date: 11.03.08   Time: 13:16
 * Updated in $/VSNA/Servers/AppServer/Services
 * - query for BS
 *
 * *****************  Version 33  *****************
 * User: Ktrushnikov  Date: 23.02.08   Time: 13:23
 * Updated in $/VSNA/Servers/AppServer/Services
 * - SetRS to zero if RS is down and can't connect to it. Ask SM for
 * another RS
 *
 * *****************  Version 32  *****************
 * User: Ktrushnikov  Date: 23.02.08   Time: 10:52
 * Updated in $/VSNA/Servers/AppServer/Services
 * - use dprint()
 *
 * *****************  Version 31  *****************
 * User: Ktrushnikov  Date: 23.02.08   Time: 10:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * - SetBS(0) to ask SM for new BS if old one goes down
 *
 * *****************  Version 30  *****************
 * User: Ktrushnikov  Date: 22.02.08   Time: 16:47
 * Updated in $/VSNA/Servers/AppServer/Services
 * - bugfix: set correct status in UpdateServerStatusInVector()
 *
 * *****************  Version 29  *****************
 * User: Smirnov      Date: 20.02.08   Time: 19:33
 * Updated in $/VSNA/Servers/AppServer/Services
 * - hardware key
 *
 * *****************  Version 28  *****************
 * User: Smirnov      Date: 19.02.08   Time: 13:30
 * Updated in $/VSNA/Servers/AppServer/Services
 * - bugfix with servers connect
 *
 * *****************  Version 27  *****************
 * User: Smirnov      Date: 18.02.08   Time: 20:41
 * Updated in $/VSNA/Servers/AppServer/Services
 * - simple redirect
 *
 * *****************  Version 26  *****************
 * User: Smirnov      Date: 18.02.08   Time: 16:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added net config for servers
 *
 * *****************  Version 25  *****************
 * User: Smirnov      Date: 15.02.08   Time: 21:18
 * Updated in $/VSNA/Servers/AppServer/Services
 * - safe access to g_AppDAta in services
 * - new broker info structure
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 14.02.08   Time: 17:21
 * Updated in $/VSNA/Servers/AppServer/Services
 * - auto connect
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 13.02.08   Time: 13:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * - properties corrected
 * - sturtup sequence of server connects rewrited
 * - code redused
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:20
 * Updated in $/VSNA/Servers/AppServer/Services
 * SeApptProperties realized
 *
 * *****************  Version 21  *****************
 * User: Mushakov     Date: 11.02.08   Time: 16:24
 * Updated in $/VSNA/Servers/AppServer/Services
 * AppConfigurationService added in AppServer
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 31.01.08   Time: 22:14
 * Updated in $/VSNA/Servers/AppServer/Services
 * - conditions rewrited, added to AUTH_SRV_
 * - logoff user on point disconnect
 *
 * *****************  Version 19  *****************
 * User: Ktrushnikov  Date: 25.01.08   Time: 12:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - code cleanup from OnPointConnect/Disconnect tests
 *
 * *****************  Version 18  *****************
 * User: Ktrushnikov  Date: 24.01.08   Time: 17:25
 * Updated in $/VSNA/Servers/AppServer/Services
 * - delete calls to ConnectPoint from AS
 *
 * *****************  Version 17  *****************
 * User: Dront78      Date: 23.01.08   Time: 17:51
 * Updated in $/VSNA/Servers/AppServer/Services
 * _malloca redefinition warning fixed.
 *
 * *****************  Version 16  *****************
 * User: Ktrushnikov  Date: 23.01.08   Time: 15:06
 * Updated in $/VSNA/Servers/AppServer/Services
 * - init varibles with zero
 * - crash fixed
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 22.01.08   Time: 17:25
 * Updated in $/VSNA/Servers/AppServer/Services
 * - if no messages - delete endpoint
 * - ControlInquiry type in Forced disconnect endpoint was set to All (not
 * to delete one endpoint)
 * - OnPointConnect/Disconnet conditions added in AppServer
 *
 * *****************  Version 14  *****************
 * User: Ktrushnikov  Date: 16.01.08   Time: 19:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - OnPointConnect_Event() added
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 11.01.08   Time: 21:38
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Login procedure corrected
 *
 * *****************  Version 12  *****************
 * User: Ktrushnikov  Date: 26.12.07   Time: 16:58
 * Updated in $/VSNA/Servers/AppServer/Services
 * - TestCallBack changed
 * - MakeAPing() don't process message to router
 *
 * *****************  Version 11  *****************
 * User: Ktrushnikov  Date: 25.12.07   Time: 18:28
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Disable ping on service level in SM
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 19.12.07   Time: 15:55
 * Updated in $/VSNA/Servers/AppServer/Services
 * - shutdown if no BS only (don't care about RS)
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 19.12.07   Time: 15:49
 * Updated in $/VSNA/Servers/AppServer/Services
 * - shutdown server if no response from SM
 *
 * *****************  Version 8  *****************
 * User: Ktrushnikov  Date: 11.12.07   Time: 17:44
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Use only dns-name (don't use port) in DDNS SRV Updates
 * - Handle DDNS on Connect/Disconnect of broker
 *
 * *****************  Version 7  *****************
 * User: Ktrushnikov  Date: 11.12.07   Time: 15:44
 * Updated in $/VSNA/Servers/AppServer/Services
 * - DDNS functions support hostname (not just ip)
 * - Funcs in EndpointRegistry to parse buffer and return ConnectTCP
 * (don't write to registry)
 * - Send info about AS's ConnectTCP to SM
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 10.12.07   Time: 18:41
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed RS set to myself
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 5.12.07    Time: 12:01
 * Updated in $/VSNA/Servers/AppServer/Services
 * - quick ping RS on new one arrived from SM
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 4.12.07    Time: 11:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * - VS_TransportMessage::Set(): checks fixed for new arch
 * - VS_AppManagerService: Init() overload fixed
 * - VS_AppManagerService: Message processing fixed
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 30.11.07   Time: 10:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * - request UPDATEINFO_METHOD added with specific seq_id
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 28.11.07   Time: 16:48
 * Updated in $/VSNA/Servers/AppServer/Services
 * - rename VS_MagaerService from AS
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 27.11.07   Time: 19:18
 * Updated in $/VSNA/Servers/AppServer/Services
 * changed service name
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 27.11.07   Time: 18:54
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 26.11.07   Time: 19:55
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 26.11.07   Time: 15:17
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 22.11.07   Time: 20:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * new iteration
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 20.11.07   Time: 16:13
 * Updated in $/VSNA/Servers/AppServer/Services
 * renamed services
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 15.11.07   Time: 15:28
 * Updated in $/VSNA/Servers/AppServer/Services
 * updates
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 13.11.07   Time: 17:32
 * Created in $/VSNA/Servers/AppServer/Services
 * new services
 **************************************************/

#ifdef _WIN32


#include "../../common/streams/Router/Router.h"
#include "CheckSrv/CheckSrv/VS_ClientCheckSrv.h"
#include "acs/ConnectionManager/VS_ConnectionManager.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../../common/std/cpplib/VS_Map.h"
#include "../../common/std/cpplib/VS_SimpleStr.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/VS_RegServer.h"
#include "../../common/net/EndpointRegistry.h"
#include "../../common/tools/Server/VS_Server.h"
#include "../../ServerServices/Common.h"
#include "ServerServices/VS_MediaBrokerStats.h"
#include "../../common/std/cpplib/VS_PerformanceMonitor.h"
#include "CheckSrv/CheckSrv/VS_ClientCheckSrv.h"
#include "acs/ConnectionManager/VS_ConnectionManager.h"

#include "AppServer/Services/VS_Storage.h"
#include "VS_AppManagerService.h"
#include "std/debuglog/VS_Debug.h"
//#include "../../../transport/lib/VS_TransportLib.h"

#define DEBUG_CURRENT_MODULE VS_DM_CLUSTS

#define LBALANS_MIN_ENDP	50
#define LBALANS_ENDP_DIFF	30



//////// VS_AppManagerService ////////////
bool VS_AppManagerService::Init(const char *our_endpoint, const char *our_service, const bool permittedAll)
{
	if (!our_endpoint || !*our_endpoint)
		return false;
	dprint1("\t\t using server name '%s'\n", our_endpoint);

	// set server manager
	char buff[256] = {0};
	VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
	if (cfg.GetValue(buff, 256, VS_REG_STRING_VT, SERVER_MANAGER_TAG) > 0 && *buff) {
		Set(buff, false, ST_SM);
		m_SM = buff;
		// m_SM always is constant, no collisions
	}
	else {
		dprint0("invalid server manager specified \n");
		return false;
	}

	tu::TimeToLStr(std::chrono::system_clock::now(), buff, sizeof buff);
	dprint3("\t start time:%s", buff);

	ConnectServer(m_SM);

	VS_Container cnt1;
	cnt1.AddValue(METHOD_PARAM, LOGEVENT_METHOD);
	cnt1.AddValueI32(TYPE_PARAM, BE_START);
	cnt1.AddValue(FIELD1_PARAM, (char*)m_broker_ver);
	PostRequest(m_SM, 0, cnt1, NULL, MANAGER_SRV);

	m_state = SSTATE_RUNNING;
	g_appServer = this;
	VS_InstallConnectionManager("checksrv");
	return true;
};

void VS_AppManagerService::AsyncDestroy()
{
	SendStats();

	char state_str[16] = {};
	sprintf(state_str, "%u", static_cast<unsigned int>(m_state));

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGEVENT_METHOD);
	cnt.AddValueI32(TYPE_PARAM, BE_SHUTDOWN);
	cnt.AddValue(FIELD1_PARAM, state_str);
	PostRequest(m_SM, 0, cnt, NULL, MANAGER_SRV);

	FullDisconnectAllEndpoints();
	VS_Server::PauseDestroy(1000);
	Sleep(1000);
}

bool VS_AppManagerService::Timer( unsigned long ticks)
{
	if (m_state==SSTATE_RUNNING || m_state==SSTATE_REDIRECT) {
		CountStats(ticks);
		if (ticks - m_DirPingTime > DirPingPeriod) {
			CheckServers();
			m_DirPingTime = ticks;
		}
	}

	VS_SimpleStr bs;
	{
		std::lock_guard<std::recursive_mutex> lock(m_lock);
		for (std::map<VS_SimpleStr,std::list<std::string>>::iterator i = m_bs_by_dns.begin(); i!=m_bs_by_dns.end() && !bs; i++) {
			for (std::list<std::string>::iterator ii = i->second.begin(); ii!=i->second.end(); ii++)	{
				if (IsPingDistance(ii->c_str(), ticks)) {
					bs = ii->c_str();
					break;
				}
			}
		}
	}
	if (!!bs) {
		VS_LocatorCheck check(bs);
		check.Run(2000);
		unsigned long ptime = check.m_res;
		SetDistance(bs, ptime, ticks);
	}

	return true;
}

void VS_AppManagerService::UpdateInfo_Method(VS_Container &cnt)
{

	SetBSList(cnt);
	const char * asdomain = cnt.GetStrValueRef(ASDOMAIN_PARAM);
	SetNetInfo(OurEndpoint(), asdomain);
	std::map<VS_SimpleStr,std::list<std::string>>	tmp_bs_by_dns;
	{
		std::lock_guard<std::recursive_mutex> lock(m_lock);
		tmp_bs_by_dns = m_bs_by_dns;
	}
	for(std::map<VS_SimpleStr,std::list<std::string>>::iterator i = tmp_bs_by_dns.begin(); i!=tmp_bs_by_dns.end(); i++)
	{
		for(std::list<std::string>::iterator ii = i->second.begin(); ii!=i->second.end(); ii++)
		{
			if (!GetState(ii->c_str()))
				ConnectServer(ii->c_str());
		}
	}
}

bool VS_AppManagerService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)  return true;
	switch (recvMess->Type()) {
	case transport::MessageType::Invalid:   // Skip
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		m_recvMess = recvMess.get();
		{
			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0)
				{
					if (strcasecmp(method, UPDATEINFO_METHOD) == 0) {
						UpdateInfo_Method(cnt);
					}
					else if (strcasecmp(method, MANAGESERVER_METHOD) == 0) {
						int32_t mgr_cmd = MSC_NONE;
						int32_t mgr_cmd_param = 0;

						cnt.GetValue(TYPE_PARAM, mgr_cmd);
						cnt.GetValue(SUBTYPE_PARAM, mgr_cmd_param);

						dprint4("ManageServer_Method: %d, %d\n", mgr_cmd, mgr_cmd_param);
						ManageServer_Method(mgr_cmd, mgr_cmd_param, &cnt);
					}
				}
			}
		}
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return true;
}


bool VS_AppManagerService::CountStats(unsigned long tickcount)
{
	if ( tickcount-m_RefTime > RefreshPeriod )
	{
		GetStatistics(&m_tstat);

		if (m_sr)
			m_sr->GetStatistics(m_sstat);
		else memset(&m_sstat, 0, sizeof(stream::RouterStatistics));

		float bitrate = (m_tstat.in_byterate + m_tstat.out_byterate + m_sstat.in_byterate + m_sstat.out_byterate)/128.f;

		int OUsers, Confs, Parts;
		g_storage->CountOfAll(OUsers, Confs, Parts);

		auto cpuload = VS_PerformanceMonitor::Instance().GetTotalProcessorTime();

		m_ProcessorLoad.AddVal(cpuload);
		m_NetworkLoad.AddVal(bitrate);
		m_Enpoints.AddVal((float)m_tstat.endpoints);
		m_Users.AddVal((float)OUsers);
		m_Confs.AddVal((float)Confs);
		m_Parts.AddVal((float)Parts);

		m_TotalUsers = std::max(m_TotalUsers, OUsers);
		m_TotalConfs = std::max(m_TotalConfs, Confs);
		m_TotalEndpoints = std::max(m_TotalEndpoints, (int)m_tstat.endpoints);

		VS_AppServerStats stats;
		memcpy(stats.m_Version, m_broker_ver, (m_broker_ver.Length() <= 511)? m_broker_ver.Length(): 511);
		stats.m_CPULoad = m_ProcessorLoad.GetCurr();
		stats.m_Transport_NumEndpoints = m_tstat.endpoints;
		stats.m_Transport_Bitrate_In = (int) (m_tstat.in_byterate/128.); // (*8/1024)
		stats.m_Transport_Bitrate_Out = (int) (m_tstat.out_byterate/128.); // (*8/1024)
		stats.m_Streams_NumStreams = m_sstat.streams;
		stats.m_Streams_Bitrate_In = (int) (m_sstat.in_byterate/128.); // (*8/1024)
		stats.m_Streams_Bitrate_Out = (int) (m_sstat.out_byterate/128.); // (*8/1024)
		stats.m_OnlineUsers = OUsers;
		stats.m_Confs = Confs;
		stats.m_Participants = Parts;

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, LOGSTATS_METHOD);
		rCnt.AddValueI32(TYPE_PARAM, 2); // 1 - for Average stats; 2 - for Current stats
		rCnt.AddValue(MEDIABROKERSTATS_PARAM, &stats, sizeof(stats));
		PostRequest(m_SM, 0, rCnt, 0, MANAGER_SRV);

		m_RefTime = tickcount;
	}

	if (tickcount-m_SendTime > SendPeriod)
	{
		SendStats();
		m_SendTime = tickcount;
		EmptyUsageStat();
	}
	return true;
}

void VS_AppManagerService::SendStats()
{
	dprint3("Sending stats\n");
	VS_MediaBrokerStats stats;

	stats.m_AvConfs = m_Confs.GetAvg();
	stats.m_AvEndpts = m_Enpoints.GetAvg();
	stats.m_AvNetLoad = m_NetworkLoad.GetAvg();
	stats.m_AvParts = m_Parts.GetAvg();
	stats.m_AvPrLoad = m_ProcessorLoad.GetAvg();
	stats.m_AvUsers = m_Users.GetAvg();
	stats.Now();
	int a, b; float c;
	stats.m_periodOfAveraging = m_NetworkLoad.GetValues(a, b, c);
	stats.m_TotalConfs = m_TotalConfs;
	stats.m_TotalUsers = m_TotalUsers;
	stats.m_TotalEndpoints = m_TotalEndpoints;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, LOGSTATS_METHOD);
	rCnt.AddValueI32(TYPE_PARAM, 1); // 1 - for Average stats; 2 - for Current stats
	rCnt.AddValue(MEDIABROKERSTATS_PARAM, &stats, sizeof(stats));
	PostRequest(m_SM, 0, rCnt, 0, MANAGER_SRV);
}

bool VS_AppManagerService::OnPointConnected_Event(const VS_PointParams* prm)
{
	if (prm->reazon <= 0) {
		dprint1("Error {%d} while Connect to (%s)\n", prm->reazon, prm->uid);
	}
	else if (prm->type==VS_PointParams::PT_SERVER) {
		dprint2("Server Connect: uid=%s, reason: %2d\n", prm->uid, prm->reazon);
		if (!GetState(prm->uid)) {
			// offline
			if (IsBSList(prm->uid)) {
				dprint1("BS (%s) is UP\n", prm->uid);
				Set(prm->uid, true, ST_BS);
				DefinedServerStatusChanged(prm->uid, true, ST_BS);
			}
			else if (m_SM == prm->uid) {
				dprint1("SM (%s) is UP\n", prm->uid);
				Set(prm->uid, true, ST_SM);
				DefinedServerStatusChanged(prm->uid, true, ST_SM);
			}
			else {
				VS_ServerTypes type = DetermineType(prm->uid);
				dprint1("Server (%s) typeof |%d| is UP\n", prm->uid, type);
				Set(prm->uid, true, type);
			}
			VS_BinBuff cfg;
			if (GetNetConfig(OurEndpoint(), cfg)) {
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, UPDATECONFIGURATION_METHOD );
				rCnt.AddValue(NAME_PARAM, OurEndpoint());
				rCnt.AddValue(EPCONFIG_PARAM, cfg.Buffer(), cfg.Size());
				PostRequest(prm->uid, 0, rCnt, 0, CONFIGURATION_SRV);
			}
		}
		else {
			dprint1("Server (%s) Reconnect, reason: %2d\n", prm->uid, prm->reazon);
		}
	}
	else {
		AddCid(prm->cid);
	}

	return true;
}

bool VS_AppManagerService::OnPointDisconnected_Event(const VS_PointParams* prm)
{
	if (prm->type==VS_PointParams::PT_SERVER) {
		dprint2("Server DisConnect: uid=%s, reason: %2d\n", prm->uid, prm->reazon);
		if (IsBSList(prm->uid)) {
			dprint1("BS (%s) is DOWN\n", prm->uid);
			Set(prm->uid, false, ST_BS);
			DefinedServerStatusChanged(prm->uid, false, ST_BS);
		}
		else if (m_SM == prm->uid) {
			dprint1("SM (%s) is DOWN\n", prm->uid);
			Set(prm->uid, false, ST_SM);
			DefinedServerStatusChanged(prm->uid, false, ST_SM);
		}
		else {
			VS_ServerTypes type = DetermineType(prm->uid);
			dprint1("Server (%s) typeof |%d| is DOWN\n", prm->uid, type);
			Set(prm->uid, false, type);
		}
	}
	else {
		DelCid(prm->cid);
	}
	return true;
}

void VS_AppManagerService::DefinedServerStatusChanged(const char* server, bool status, VS_ServerTypes type)
{
	dprint1("DefinedServerStatusChanged: %s, type <%d> - set to %d\n", server, type, status);
	switch (type)
	{
	case ST_SM:
		if (status) {
			CheckServers();

			VS_Container cnt1;
			cnt1.AddValue(METHOD_PARAM, GETALLAPPPROPERTIES_METHOD);
			PostRequest(m_SM, 0, cnt1, 0, MANAGER_SRV, default_timeout, CONFIGURATION_SRV);
		}
		break;
	case ST_BS:
		break;
	case ST_RS:
		break;
	}
}


void VS_AppManagerService::CheckServers()
{
	const auto bs = CheckAllBS();
	if (GetState(m_SM)) {
		if (!bs) {
			dprint1("Query for BS\n");
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, UPDATEINFO_METHOD);
			PostRequest(m_SM, 0, cnt, NULL, MANAGER_SRV);
		}
	}
	else {
		ConnectServer(m_SM);
	}
}

bool VS_AppManagerService::CheckAllBS()
{
	std::vector<std::string> v;
	{
		std::lock_guard<std::recursive_mutex> lock(m_lock);
		if (m_bs_by_dns.empty())
			return false;
		for (std::map<VS_SimpleStr, std::list<std::string>>::iterator i = m_bs_by_dns.begin(); i != m_bs_by_dns.end(); i++)
		{
			for (std::list<std::string>::iterator ii = i->second.begin(); ii != i->second.end(); ii++)
			{
				if (!GetState(ii->c_str()))
				{
					v.push_back(*ii);
				}
			}
		}
	}

	for (std::vector<std::string>::iterator it=v.begin(); it != v.end(); ++it)
		ConnectServer(it->c_str());

	return (v.size()==0)? true: false;
}

void VS_AppManagerService::ManageServer_Method(long cmd, long param, const void* param2)
{
	if ( m_SM != m_recvMess->SrcServer() )
	{
		dprint1("Unknown server %s requested command(%ld, %ld), but is not SM (%s)\n", m_recvMess->SrcServer(), cmd, param, m_SM.m_str);
		return;
	}

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGEVENT_METHOD);

	if ( cmd == MSC_SETSTATE ) {
		cnt.AddValueI32(TYPE_PARAM, BE_STATESET);

		if ( param == SSTATE_OFFLINE ) {
			dprint0("shutdown method called from SM\n");
			m_state = SSTATE_OFFLINE;

			cnt.AddValueI32(SUBTYPE_PARAM, SSTATE_OFFLINE);
			PostRequest(m_SM, 0, cnt, NULL, MANAGER_SRV);

			m_watchdog->Shutdown();

		} else if ( param == SSTATE_RESTART ) {
			dprint0("restart method called from SM\n");
			m_state = SSTATE_RESTART;

			cnt.AddValueI32(SUBTYPE_PARAM, SSTATE_RESTART);
			PostRequest(m_SM, 0, cnt, NULL, MANAGER_SRV);

			m_watchdog->Restart(200);

		} else if ( param == SSTATE_REDIRECT ) {
			dprint0("redirect method called from SM\n");
			m_state = SSTATE_REDIRECT;

			cnt.AddValueI32(SUBTYPE_PARAM, SSTATE_REDIRECT);
			PostRequest(m_SM, 0, cnt, NULL, MANAGER_SRV);

		} else {
			dprint0("unknown param for SetState(%ld) method called from SM\n", param);
		}
	} else if ( cmd == MSC_REFRESH_PROPS ) {
		dprint0("RefreshProperties method called from SM\n");

		VS_Container cnt1;
		cnt1.AddValue(METHOD_PARAM, GETALLAPPPROPERTIES_METHOD);
		PostRequest(m_SM, 0, cnt1, NULL, MANAGER_SRV, default_timeout, CONFIGURATION_SRV);

		cnt.AddValueI32(TYPE_PARAM, BE_PROPSREFRESHED);
		PostRequest(m_SM, 0, cnt, NULL, MANAGER_SRV);

	} else if ( cmd == MSC_LOADBALANCE ) {
		dprint0("LoadBalance method called from SM\n");

		if ( param ) {
			VS_Container &cnt = *(VS_Container *)param2;
			cnt.Reset();
			const char* servers[100];
			int i = 0;
			while (cnt.Next() && i<100)
				if (strcasecmp(cnt.GetName(), SERVER_PARAM)==0)
					servers[i++] = cnt.GetStrValueRef();
			MSC_Redirect(param, servers, i);
		}

	} else {
		dprint0("unknown method (%ld) called with param (%ld) from SM\n", cmd, param);
	}
}

void VS_AppManagerService::MSC_Redirect(const unsigned int N, const char** sids, int NumOfSid)
{
	if (!N || !sids || NumOfSid==0) {
		dprint1("Redirect params are empty!\n");
		return;
	}

	typedef std::pair<const char*, VS_BinBuff> Tsi;
	std::vector<Tsi> ss;
	for (int i = 0; i<NumOfSid; i++) {
		Tsi si;
		si.first = sids[i];
		if (GetNetConfig(si.first, si.second) && si.second.IsValid())
			ss.push_back(si);
	}
	if (ss.size()==0) {
		dprint1("No servers to redirect!\n");
		return;
	}

	vs_user_id* users = 0;
	auto NumOfUsers = g_storage->GetUsers(users); // delete after use
	if (NumOfUsers==0) {
		dprint1("No users to redirect!\n");
		return;
	}

	VS_ParticipantDescription	tmp_pd;

	unsigned int i = 0;
	double sum = 0.0;
	double step = (double) NumOfUsers / N;
	if ( step < 1.0 )
		step = 1.0;

	dprint2("Redirect %d users (total: %d) to %zu servers\n", N, NumOfUsers, ss.size());
	while(sum < NumOfUsers) {
		vs_user_id &user = users[int(sum)];
		if (!g_storage->FindParticipant(SimpleStrToStringView(user), tmp_pd) && !(m_presenceService->GetStatus(user).m_status >= USER_BUSY)) {
			int index = i++ % ss.size();			// server to redirect
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, REDIRECTTO_METHOD);
			rCnt.AddValue(NAME_PARAM, ss[index].first);
			rCnt.AddValue(EPCONFIG_PARAM, ss[index].second.Buffer(), ss[index].second.Size());
			PostRequest(0, user, rCnt, 0, CONFIGURATION_SRV);
			dprint3("->Redirect <%s> to %s\n", (char*)user, (char*)ss[index].first);
		}
		sum += step;
	}
	dprint2("Redirected %d users (from %d)\n", i, N);

	delete [] users;
	ss.clear();
}
#endif