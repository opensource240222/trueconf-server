/****************************************************************************
 * (c) 2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: MediaBrokerServices
 *
 * Created: 09.12.03     by  SMirnovK
 *
 * $History: VS_MediaBrokerStats.h $
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 27.04.08   Time: 19:33
 * Updated in $/VSNA/Servers/ServerServices
 * - Average statistics added
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 21.04.08   Time: 21:14
 * Updated in $/VSNA/Servers/ServerServices
 * - file VS_UsageStatService.h placed from AS to ServerServices
 * - return types for stat funcs changed from bool to void
 * - stat class is inited by -1 (not by 0)
 * - new light struct for stat: VS_UsageStatOther (not so big like for AS)
 * - Avarage & Current statistics added to BS and RS (like it is in AS)
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 21.03.08   Time: 19:42
 * Updated in $/VSNA/Servers/ServerServices
 * - support for Current stats  (not Average stats): send from AS to SM
 * and saved to Registry::Servers
 * - VS_FileTime: RUS_FMT added: dd.mm.yyyy hh:mm:ss
 * - struct VS_AppServerStats added
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 26.11.07   Time: 19:55
 * Updated in $/VSNA/Servers/ServerServices
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:50
 * Created in $/VSNA/Servers/ServerServices
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/ServerServices
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 10.12.03   Time: 15:13
 * Updated in $/VS/Servers/ServerServices
 * removes stat service to cluster service
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 9.12.03    Time: 19:25
 * Updated in $/VS/Servers/ServerServices
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 9.12.03    Time: 18:59
 * Created in $/VS/Servers/ServerServices
 * send broker stat
 *
 ****************************************************************************/

/****************************************************************************
 * \file VS_MediaBrokerStats.h
 ****************************************************************************/
#ifndef VS_MEDIABROKERSTATS_H
#define VS_MEDIABROKERSTATS_H
/****************************************************************************
 * Includes
 ****************************************************************************/

#include "std-generic/cpplib/TimeUtils.h"

#include <cstdint>
#include <chrono>

/****************************************************************************
 * Media Broker Statistic structure.
 * add new values at and of structure.
 ****************************************************************************/
#pragma pack(push, 4)
struct VS_MediaBrokerStats
{
	VS_MediaBrokerStats(){memset(this, 0, sizeof(VS_MediaBrokerStats));}
	int32_t				m__reserved;
	// Note that it is not possible to replace it with something like std::chrono::*::time_point
	// without breaking compatibility with existing registry server
	// because this struct is directly (without serialisation) sent to the registry server from time to time.
	// It seem that raplacing it with uint64_t which stores NT timestamp is the most compatible solution.
	uint64_t		m_currTime;
	int32_t				m_periodOfAveraging;
	int32_t				m_TotalConfs;
	int32_t				m_TotalUsers;
	int32_t				m_TotalEndpoints;
	float			m_AvPrLoad;
	float			m_AvNetLoad;
	float			m_AvEndpts;
	float			m_AvUsers;
	float			m_AvConfs;
	float			m_AvParts;

	void Now(void)
	{
		m_currTime = static_cast<uint64_t>(tu::UnixSecondsToWindowsTicks(std::chrono::system_clock::now()));
	}
};
#pragma pack(pop)

class VS_AppServerStats
{
public:
	VS_AppServerStats():	m_CPULoad(-1),
							m_Transport_NumEndpoints(-1), m_Transport_Bitrate_In(-1), m_Transport_Bitrate_Out(-1),
							m_Streams_NumStreams(-1), m_Streams_Bitrate_In(-1),	m_Streams_Bitrate_Out(-1),
							m_OnlineUsers(-1), m_Confs(-1), m_Participants(-1)
	{
		memset(m_Version, 0, 512);
	}
	~VS_AppServerStats()
	{	}

	char			m_Version[512];		// 511 + "\0"
	int32_t			m_CPULoad;
	int32_t			m_Transport_NumEndpoints;
	int32_t			m_Transport_Bitrate_In;
	int32_t			m_Transport_Bitrate_Out;
	int32_t			m_Streams_NumStreams;
	int32_t			m_Streams_Bitrate_In;
	int32_t			m_Streams_Bitrate_Out;
	int32_t			m_OnlineUsers;
	int32_t			m_Confs;
	int32_t			m_Participants;
};

#endif
