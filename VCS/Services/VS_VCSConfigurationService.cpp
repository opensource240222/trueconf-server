#include "VS_VCSConfigurationService.h"
#include "VS_ConfMemStorage.h"
#include "../../ServerServices/VS_MediaBrokerStats.h"
#include "AppServer/Services/VS_Storage.h"
#include "../../AppServer/Services/VS_AppServerData.h"
#include "../../ServerServices/VS_ReadLicense.h"
#include "../../common/streams/Router/Router.h"
#include "../../common/net/EndpointRegistry.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"
#include "../../common/std/VS_RegServer.h"
#include "../../common/std/cpplib/VS_PerformanceMonitor.h"
#include "../../common/std/debuglog/VS_Debug.h"
#include <vector>

#define DEBUG_CURRENT_MODULE VS_DM_CONFIGS

bool VS_VCSConfigurationService::Init( const char *our_endpoint, const char *our_service, const bool permittedAll)
{

	VS_RegistryKey key(false, "AppProperties\\Config");

	key.GetValue(&m_width, sizeof(m_width), VS_REG_INTEGER_VT, "Width");
	key.GetValue(&m_height, sizeof(m_height), VS_REG_INTEGER_VT, "Height");
	key.GetValue(&m_bitrate, sizeof(m_bitrate), VS_REG_INTEGER_VT, "Bitrate");

	return true;
}

bool VS_VCSConfigurationService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	auto dbStorage = g_dbStorage;
	if (!dbStorage)
		return true;
	if (recvMess == 0)	// Skip
		return true;
	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
	// Write to log !!! Error !!!
		break;
// Store request
	case transport::MessageType::Request:
		m_recvMess = recvMess.get();
		{
			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
					dprint3("Processing %20s; cid:%s user:%s srv:%s \n", method, recvMess->SrcCID(), recvMess->SrcUser(), recvMess->SrcServer());
					// Process methods
					if (strcasecmp(method, UPDATECONFIGURATION_METHOD) == 0) {
						UpdateConfiguration_Method(cnt);
					}
					else if (strcasecmp(method, GETAPPPROPERTIES_METHOD) == 0) {
						const char* app_name = cnt.GetStrValueRef(NAME_PARAM);
						const char* ver = cnt.GetStrValueRef(CLIENTVERSION_PARAM);
						GetAppProperties_Method(app_name, ver);
					}
					else if (strcasecmp(method, SETPROPERTIES_METHOD) == 0) {
						SetEpProperties_Method(cnt);
					}
					else if (strcasecmp(method, ONAPPPROPSCHANGE_METHOD) == 0) {
						OnPropertiesChange_Method(cnt.GetStrValueRef(SESSION_PARAM));
					}
					else if (strcasecmp(method, POINTDETERMINEDIP_METHOD) == 0) {
						PointDeterminedIP_Handler(cnt.GetStrValueRef(USERNAME_PARAM),cnt.GetStrValueRef(IPCONFIG_PARAM));
					} else if (strcasecmp(method, POINTDISCONNECTED_METHOD) == 0) {
						PointDisconnected_Handler(cnt.GetStrValueRef(NAME_PARAM));
					}
				}
			}
		}
		break;
	case transport::MessageType::Reply:
		break;
	case transport::MessageType::Notify: // Later... may be...
		break;
	}
	m_recvMess = nullptr;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// UPDATECONFIGURATION_METHOD(IPCONFIG_PARAM, ...)
////////////////////////////////////////////////////////////////////////////////
void VS_VCSConfigurationService::UpdateConfiguration_Method(VS_Container &cnt)
{
	VS_UserData ud;
	const auto user_id = m_recvMess->SrcUser_sv();
	if (!g_storage->FindUser(user_id, ud))
		return;

	// Remove all IP
	ud.m_IPconfig.Empty();
	int IPCnt = 0;
	void *IP;
	size_t size;
	VS_Container rcnt;

	// Enumerate all configuration
	cnt.Reset();
	char local_ip[1024] = {0};
	while (cnt.Next() && IPCnt < 8) {
		if (strcasecmp(cnt.GetName(), IPCONFIG_PARAM) == 0) {
			IPCnt++;
			IP = (void *)cnt.GetBinValueRef(size);
			rcnt.AddValue(IPCONFIG_PARAM, IP, size);
			net::endpoint::ConnectTCP tcp;
			tcp.Deserialize(IP, size);
			sprintf(local_ip + strlen(local_ip), "%s:%d, ", tcp.host.c_str(), tcp.port);
		}
		if (strcasecmp(cnt.GetName(), IP6CONFIG_PARAM) == 0) {
			IPCnt++;
			IP = (void *)cnt.GetBinValueRef(size);
			rcnt.AddValue(IP6CONFIG_PARAM, IP, size);
			net::endpoint::ConnectTCP tcp;
			tcp.Deserialize(IP, size);
			sprintf(local_ip + strlen(local_ip), "%s:%d, ", tcp.host.c_str(), tcp.port);
		}
	}
	if (IPCnt && rcnt.SerializeAlloc(IP, size)) {
		ud.m_IPconfig.Set(IP, size);
		free(IP);
	}
	g_storage->UpdateUser(user_id, ud);
	// update ep props
	if (IPCnt) {
		int len = (int)strlen(local_ip);
		if (len > 2)
			local_ip[len-2] = 0;
		VS_Container cnt2;
		cnt2.AddValue(METHOD_PARAM, SETPROPERTIES_METHOD);
		cnt2.AddValue(HASH_PARAM, ud.m_appID);
		cnt2.AddValue(NAME_PARAM, "Local Ip");
		cnt2.AddValue(PROPERTY_PARAM, local_ip);

		SetEpProperties_Method(cnt2);
	}

	// update our ep config
	VS_Container rCnt;
	if (g_appServer && g_appServer->GetNetInfo(rCnt) && rCnt.IsValid())
		PostRequest(0, user_id.c_str(), rCnt);

	// Send user: WxH and Bitrate params
	VS_Container rCnt2;
	rCnt2.AddValue(METHOD_PARAM, CONFIGURATIONUPDATED_METHOD);
	//if (m_bitrate!=-1)
	//	rCnt2.AddValue(RESTRICTBITRATE_PARAM, m_bitrate);
	//if (m_width!=-1 && m_height!=-1) {
	//	VS_MediaFormat mf;
	//	mf.SetVideo(m_width, m_height);
	//	mf.SetAudio(16000, VS_ACODEC_SPEEX);
	//	rCnt2.AddValue(MEDIAFORMAT_PARAM, (void*)&mf, sizeof(VS_MediaFormat));
	//}
	long flag = VS_CheckLicense(LE_TRIAL);
	rCnt2.AddValueI32(RESTRICTFLAGS_PARAM, flag);
	PostReply(rCnt2);
}

void VS_VCSConfigurationService::GetAppProperties_Method(const char* app_name, const char* ver)
{
	dprint2("CID %s app %s %s \n", m_recvMess->SrcCID(), app_name ? app_name : "---", ver ? ver : "0.0.0.0");

	if (!app_name || !*app_name)
		return; //no default props
	auto dbStorage = g_dbStorage;
	VS_Container rCnt;
	int cnt = dbStorage->GetServerProperties(rCnt, dbStorage->PROP_GLOBALAPP);
	cnt += dbStorage->GetAppProperties(rCnt, app_name);
	if (rCnt.AddValue("server_protocol_version", SERVER_PROTOCOL_VERSION_STR))
		cnt++;
	if (cnt > 0) {
		rCnt.AddValue(METHOD_PARAM, SETPROPERTIES_METHOD);
		PostReply(rCnt);
	}
	if (!!m_recvMess->SrcCID())
		m_connected_cids[m_recvMess->SrcCID()] = app_name;
}

bool VS_VCSConfigurationService::OnPointDisconnected_Event(const VS_PointParams* prm)
{
	if (prm->type==VS_PointParams::PT_CLIENT) {
		if (prm->cid && *prm->cid) {
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, POINTDISCONNECTED_METHOD);
			cnt.AddValue(NAME_PARAM, prm->cid);
			PostRequest(OurEndpoint(), 0, cnt);
		}
	}
	return true;

}

void VS_VCSConfigurationService::SetEpProperties_Method(VS_Container &cnt)
{
	auto dbStorage = g_dbStorage;
	VS_UserData ud;
	if (!g_storage->FindUser(m_recvMess->SrcUser_sv(), ud))
		return;

	if (ud.m_client_type==CT_TRANSCODER)
		return;

	const char* app_id = (!!ud.m_appID)? ud.m_appID.m_str: cnt.GetStrValueRef(HASH_PARAM);
	if (!app_id)
		return;

	const char *name = 0, *prop = 0;
	cnt.Reset();
	while(cnt.Next()) {
		if (strcasecmp(cnt.GetName(), NAME_PARAM)==0) {
			name = cnt.GetStrValueRef();
		}
		else if (strcasecmp(cnt.GetName(), PROPERTY_PARAM)==0) {
			prop = cnt.GetStrValueRef();
			if (name && prop) {
				if (ud.m_protocolVersion <= 11)
					dbStorage->SetEndpointProperty(app_id, name, prop);
				else {
#ifdef _WIN32
					VS_WideStr wprop;
					wprop.AssignUTF8(prop);
					dbStorage->SetEndpointProperty(app_id, name, wprop);
#endif //_WIN32
				}
			}
			name = 0; prop = 0;
		}
	}
}

void VS_VCSConfigurationService::OnPropertiesChange_Method(const char* pass)
{
	if (!pass)
		return;

	auto dbStorage = g_dbStorage;
	dbStorage->OnPropertiesChange(pass);

	for(std::map<std::string,std::string>::iterator it=m_connected_cids.begin(); it!=m_connected_cids.end(); ++it)
	{
		const char* cid = it->first.c_str();
		VS_Container rCnt;
		int cnt =dbStorage->GetServerProperties(rCnt,dbStorage->PROP_GLOBALAPP);
		cnt+=dbStorage->GetAppProperties(rCnt,it->second.c_str());
		if(rCnt.AddValue("server_protocol_version", SERVER_PROTOCOL_VERSION_STR))
			cnt++;
		if(cnt>0)
		{
			rCnt.AddValue(METHOD_PARAM, SETPROPERTIES_METHOD);
			PostUnauth(cid, rCnt);
		}
	}
}

bool VS_VCSConfigurationService::Timer( unsigned long ticks)
{
	CountStats(ticks);
	return true;
}

bool VS_VCSConfigurationService::CountStats(unsigned long tickcount)
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
		std::vector<int> params;
		params.resize(13);

		memcpy(stats.m_Version, m_broker_ver.c_str(), (m_broker_ver.length() <= 511)? m_broker_ver.length(): 511);
		params[0] = stats.m_CPULoad = m_ProcessorLoad.GetCurr();
		params[1] = stats.m_Transport_NumEndpoints = m_tstat.endpoints;
		params[2] = stats.m_Transport_Bitrate_In = (int) (m_tstat.in_byterate/128.); // (*8/1024)
		params[3] = stats.m_Transport_Bitrate_Out = (int) (m_tstat.out_byterate/128.); // (*8/1024)
		params[4] = stats.m_Streams_NumStreams = m_sstat.streams;
		params[5] = stats.m_Streams_Bitrate_In = (int) (m_sstat.in_byterate/128.); // (*8/1024)
		params[6] = stats.m_Streams_Bitrate_Out = (int) (m_sstat.out_byterate/128.); // (*8/1024)
		params[7] = stats.m_OnlineUsers = g_storage->CountOnlineUsers();
		params[8] = stats.m_Confs = Confs;
		params[9] = stats.m_Participants = Parts;
		params[10] = g_storage->CountOnlineGuests();
		params[11] = g_storage->CountOnlineGateways();
		params[12] = g_storage->CountOnlineTerminalPro();

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, LOGSTATS_METHOD);
		rCnt.AddValueI32(TYPE_PARAM, 2); // 1 - for Average stats; 2 - for Current stats
		rCnt.AddValue(MEDIABROKERSTATS_PARAM, &stats, sizeof(stats));

		auto dbStorage = g_dbStorage;
		dbStorage->LogSystemParams(params);

#ifdef _SVKS_M_BUILD_
#else
		PostRequest(RegServerName, 0, rCnt, 0, REGISTRATION_SRV);
#endif

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

void VS_VCSConfigurationService::SendStats()
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
	rCnt.AddValueI32(TYPE_PARAM, 1);			// 1 - for Average stats; 2 - for Current stats
	rCnt.AddValue(MEDIABROKERSTATS_PARAM, &stats, sizeof(stats));
#ifdef _SVKS_M_BUILD_
#else
	PostRequest(RegServerName, 0, rCnt, 0, REGISTRATION_SRV);
#endif
}

bool VS_VCSConfigurationService::OnPointDeterminedIP_Event(const char* uid, const char* ip )
{
	if (!uid || !*uid)
		return true;

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, POINTDETERMINEDIP_METHOD);
	cnt.AddValue(USERNAME_PARAM, uid);
	cnt.AddValue(IPCONFIG_PARAM, ip);

	PostRequest(OurEndpoint(), 0, cnt);
	return true;
}

bool VS_VCSConfigurationService::PointDeterminedIP_Handler(const char* uid, const char* ip )
{
	VS_UserData ud;
	if (!uid || !g_storage->FindUser(uid, ud))
		return true;
	if (ud.m_client_type==CT_TRANSCODER)
		return true;
	auto dbStorage = g_dbStorage;
	dbStorage->SetEndpointProperty(ud.m_appID, WAN_IP_TAG, ip);
	return true;
}

bool VS_VCSConfigurationService::PointDisconnected_Handler(const char* cid)
{
	if (!cid || !*cid)
		return false;
	m_connected_cids.erase(cid);
	return true;
}

