#ifdef _WIN32	// not ported
#include "VS_ManagerService.h"
#include "../../ServerServices/VS_MediaBrokerStats.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/cpplib/VS_PerformanceMonitor.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_CLUSTS

bool VS_ManagerService::Init( const char *our_endpoint, const char *our_service, const bool permitAll )
{
	char buff[256] = {0};
	VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
	if (cfg.GetValue(buff, 256, VS_REG_STRING_VT, SERVER_MANAGER_TAG) > 0 && *buff) {
		m_SM = buff;
	}
	else {
		dprint0("invalid server manager specified \n");
		return false;
	}

	VS_Container cnt1;
	cnt1.AddValue(METHOD_PARAM, LOGEVENT_METHOD);
	cnt1.AddValueI32(TYPE_PARAM, BE_START);
	cnt1.AddValue(FIELD1_PARAM, (char*)m_server_version);
	PostRequest(m_SM, 0, cnt1, NULL, MANAGER_SRV);
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, UPDATEINFO_METHOD);
	PostRequest(m_SM, 0, cnt, NULL, MANAGER_SRV);

	return true;
}

bool VS_ManagerService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
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
					if (_stricmp(method, MANAGESERVER_METHOD) == 0) {
						int32_t mgr_cmd = MSC_NONE;
						int32_t mgr_cmd_param = 0;

						cnt.GetValue(TYPE_PARAM, mgr_cmd);
						cnt.GetValue(SUBTYPE_PARAM, mgr_cmd_param);

						dprint4("ManageServer_Method: %d, %d\n", mgr_cmd, mgr_cmd_param);
						ManageServer_Method(mgr_cmd, mgr_cmd_param);
					} else if (_stricmp(method, UPDATEINFO_METHOD) == 0) {

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

void VS_ManagerService::ManageServer_Method(long cmd, long param)
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

			cnt.AddValueI32(SUBTYPE_PARAM, SSTATE_OFFLINE);
			PostRequest(m_SM, 0, cnt, NULL, MANAGER_SRV);

			m_watchdog->Shutdown();

		} else if ( param == SSTATE_RESTART ) {
			dprint0("restart method called from SM\n");

			cnt.AddValueI32(SUBTYPE_PARAM, SSTATE_RESTART);
			PostRequest(m_SM, 0, cnt, NULL, MANAGER_SRV);

			m_watchdog->Restart();
		} else {
			dprint0("unknown param for SetState(%ld) method called from SM\n", param);
		}
	} else {
		dprint0("unknown method (%ld) called with param (%ld) from SM\n", cmd, param);
	}
}

bool VS_ManagerService::Timer(unsigned long tickcount)
{
	return CountStats(tickcount);
}

bool VS_ManagerService::CountStats(unsigned long tickcount)
{
	if ( tickcount-m_RefTime > RefreshPeriod )
	{
		GetStatistics(&m_tstat);

		float bitrate = (m_tstat.in_byterate + m_tstat.out_byterate)/128.f;

		auto cpuload = VS_PerformanceMonitor::Instance().GetTotalProcessorTime();

		m_ProcessorLoad.AddVal(cpuload);
		m_NetworkLoad.AddVal(bitrate);
		m_Enpoints.AddVal((float)m_tstat.endpoints);

		VS_AppServerStats stats;
		memcpy(stats.m_Version, m_server_version, (m_server_version.Length() <= 511)? m_server_version.Length(): 511);
		stats.m_CPULoad = m_ProcessorLoad.GetCurr();
		stats.m_Transport_NumEndpoints = m_tstat.endpoints;
		stats.m_Transport_Bitrate_In = (int) (m_tstat.in_byterate/128.); // (*8/1024)
		stats.m_Transport_Bitrate_Out = (int) (m_tstat.out_byterate/128.); // (*8/1024)

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, LOGSTATS_METHOD);
		rCnt.AddValueI32(TYPE_PARAM, 2); // 1 - for Average stats; 2 - for Current stats
		rCnt.AddValue(MEDIABROKERSTATS_PARAM, &stats, sizeof(stats));
		PostRequest(m_SM, 0, rCnt, 0, MANAGER_SRV);

		m_RefTime = tickcount;
	}
	return true;
}
bool VS_ManagerService::OnPointConnected_Event(const VS_PointParams* prm)
{
	return true;
}

bool VS_ManagerService::OnPointDisconnected_Event(const VS_PointParams* prm)
{
	return true;
}
#endif