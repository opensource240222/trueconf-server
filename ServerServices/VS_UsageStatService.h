/****************************************************************************
 * (c) 2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: MediaBrokerServices
 *
 * Created: 09.12.03     by  SMirnovK
 *
 * $History: VS_UsageStatService.h $
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 27.04.08   Time: 19:33
 * Updated in $/VSNA/Servers/ServerServices
 * - Average statistics added
 *
 * *****************  Version 1  *****************
 * User: Ktrushnikov  Date: 21.04.08   Time: 21:14
 * Created in $/VSNA/Servers/ServerServices
 * - file VS_UsageStatService.h placed from AS to ServerServices
 * - return types for stat funcs changed from bool to void
 * - stat class is inited by -1 (not by 0)
 * - new light struct for stat: VS_UsageStatOther (not so big like for AS)
 * - Avarage & Current statistics added to BS and RS (like it is in AS)
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 28.11.07   Time: 16:48
 * Updated in $/VSNA/Servers/AppServer/Services
 * - rename VS_MagaerService from AS
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 26.11.07   Time: 19:55
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 26.11.07   Time: 16:08
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed storage
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 22.11.07   Time: 20:34
 * Created in $/VSNA/Servers/AppServer/Services
 * new iteration
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 1.08.06    Time: 12:29
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * - bitrate precision increased in server stat
 *
 * *****************  Version 8  *****************
 * User: Akrutenyuk   Date: 6.12.04    Time: 15:57
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 9.03.04    Time: 17:02
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * perf Mon removed to Main
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 26.02.04   Time: 17:39
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * Clear Usage Stat every hour
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 25.12.03   Time: 21:22
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added registration service and storage
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 10.12.03   Time: 15:13
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * removes stat service to cluster service
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 9.12.03    Time: 18:59
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * send broker stat
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 9.12.03    Time: 12:58
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * rewrited stat
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 8.12.03    Time: 19:59
 * Created in $/VS/Servers/MediaBroker/BrokerServices
 * common stat fo mediabroker
 *
 ****************************************************************************/

/**
 ****************************************************************************
 * \file VS_UsageStatService.h
 ****************************************************************************/

#ifndef VS_USAGE_H
#define VS_USAGE_H

#include "../common/streams/Statistics.h"

/****************************************************************************
 * Store value ant time
 ****************************************************************************/
struct VS_StatSnap
{
	float		val;
	std::chrono::system_clock::time_point time;

	VS_StatSnap(): val(0) {}
	void Clean(){ val = 0; time = std::chrono::system_clock::time_point(); }
	void Snap(float Val, std::chrono::system_clock::time_point Time){ time = Time; val = Val;}
};

/****************************************************************************
 * Collect and store min, max, avg(hour) valuses
 ****************************************************************************/
class VS_Stat
{
	int				m_count;
	double			m_total;
	std::chrono::system_clock::time_point		m_TimeH;
	std::chrono::system_clock::time_point		m_currTime;

	VS_StatSnap		m_min;
	VS_StatSnap		m_max;
	VS_StatSnap		m_avg;
	VS_StatSnap		m_curr;
public:
	VS_Stat() { Clean(); }
	void Clean() {
		m_count = 0; m_total = 0.; m_TimeH = m_currTime = std::chrono::system_clock::time_point();
		m_min.Snap(10e10, m_TimeH); m_max.Clean(); m_avg.Clean(); m_curr.Clean();
	}
	void AddVal(double val) {
		if (m_TimeH == std::chrono::system_clock::time_point()) m_TimeH = std::chrono::system_clock::now();
		m_total+=val;
		m_count++;
		m_currTime = std::chrono::system_clock::now();
		if (m_min.val>val) m_min.Snap((float)val, m_currTime);
		if (m_max.val<val) m_max.Snap((float)val, m_currTime);
		m_avg.Snap((float)m_total/m_count, m_currTime);
		m_curr.Snap((float)val, m_currTime);
	}

	long long GetValues(int &Min, int &Max, float &Val) {
		Min = m_count ? (int)m_min.val : 0;
		Max = (int)m_max.val;
		Val = m_avg.val;
		return std::chrono::duration_cast<std::chrono::seconds>(m_currTime - m_TimeH).count();
	}
	float GetAvg(){return m_avg.val;}
	int GetCurr(){return (int)m_curr.val;}
};

/****************************************************************************
 * Collection of need stat parametrs
 ****************************************************************************/
class VS_UsageStatAS
{
public:
	VS_TransportRouterStatistics	m_tstat;
	stream::RouterStatistics		m_sstat;

	int			m_TotalConfs;
	int			m_TotalUsers;
	int			m_TotalEndpoints;
	VS_Stat		m_ProcessorLoad;
	VS_Stat		m_NetworkLoad;
	VS_Stat		m_Enpoints;
	VS_Stat		m_Users;
	VS_Stat		m_Confs;
	VS_Stat		m_Parts;

	//-------------------------------------------
	VS_UsageStatAS(){
		EmptyUsageStat();
	}
	void EmptyUsageStat() {
		memset(&m_tstat, 0, sizeof(VS_TransportRouterStatistics));
		memset(&m_sstat, 0, sizeof(stream::RouterStatistics));
		m_TotalConfs = m_TotalUsers = m_TotalEndpoints = 0;
		m_ProcessorLoad.Clean(); m_NetworkLoad.Clean(); m_Enpoints.Clean();
		m_Users.Clean(); m_Confs.Clean(), m_Parts.Clean();
	}
};

class VS_UsageStatOther
{
public:
	VS_TransportRouterStatistics	m_tstat;

	VS_Stat		m_ProcessorLoad;
	VS_Stat		m_NetworkLoad;
	VS_Stat		m_Enpoints;

	//-------------------------------------------
	VS_UsageStatOther(){
		EmptyUsageStat();
	}
	void EmptyUsageStat() {
		memset(&m_tstat, 0, sizeof(VS_TransportRouterStatistics));
		m_ProcessorLoad.Clean(); m_NetworkLoad.Clean(); m_Enpoints.Clean();
	}
};

#endif //VS_USAGE_H