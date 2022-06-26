/**
 **************************************************************************
 * \file VS_ControlQueueNhp.cpp
 * (c) 2002-2006 Visicron Systems, Inc. All rights reserved.
 *									http://www.visicron.net/
 * \brief Implement Control module. Make desision about current video Bandwidth
 * \b Project Client
 * \author SMirnovK
 * \author AnufrievS
 * \date 09.11.2006
 *
 * $Revision: 12 $
 *
 * $History: VS_ControlQueueNhp.cpp $
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 18.07.12   Time: 18:40
 * Updated in $/VSNA/VSClient
 * - fix bitrate for udp
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 16.07.12   Time: 19:59
 * Updated in $/VSNA/VSClient
 * - update SVC client to spatial
 * - update vpx libs to v1.1.0
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 29.11.11   Time: 16:27
 * Updated in $/VSNA/VSClient
 * - fix nhp for group conf
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 24.11.11   Time: 15:14
 * Updated in $/VSNA/VSClient
 * - fix nhp bitrate
 * - fix nhp "old" packet
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 28.09.11   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - beta nhp revision
 * - fix fps on low bitrates
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 18.03.11   Time: 18:55
 * Updated in $/VSNA/VSClient
 * - max  packet len decreased to 4000
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 17.02.11   Time: 11:39
 * Updated in $/VSNA/VSClient
 * - limit packet size to 4096 bytes
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 24.03.10   Time: 19:21
 * Updated in $/VSNA/VSClient
 * - were added calc statictics (bug 7127)
 * - were merged all calc statistics in sender and receiver
 * - were added jitter calculation in Nhp receiver
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 11.12.09   Time: 19:23
 * Updated in $/VSNA/VSClient
 * - isIntercom defined for NHP classes (control queue, receiver); remove
 * stat/request send fo Intercom
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 11.12.09   Time: 18:32
 * Updated in $/VSNA/VSClient
 *  - were added constraint lenght for video packets (for TCP & NHP
 * connections)
 *  - were changed default and maximum lenght of video packets for NHP
 *  - were replaced constant video packet lenght (1200 bytes) with average
 * packet lenght in NHP receive module
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 20.07.09   Time: 13:54
 * Updated in $/VSNA/VSClient
 * - change priority video and data
 * - change nhp bitrate scheme  for data
 * - status bar bitrate include data
 * - fix memory allocation for nhp
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 20  *****************
 * User: Sanufriev    Date: 31.01.07   Time: 17:41
 * Updated in $/VS/VSClient
 * - change bitrate control when media data is absent
 *
 * *****************  Version 19  *****************
 * User: Sanufriev    Date: 30.01.07   Time: 16:40
 * Updated in $/VS/VSClient
 * - fixed bitrate control
 *
 * *****************  Version 18  *****************
 * User: Sanufriev    Date: 29.01.07   Time: 15:03
 * Updated in $/VS/VSClient
 * - change statistics output
 * - modified upper bitrate algorithm
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 26.01.07   Time: 16:11
 * Updated in $/VS/VSClient
 * - support h.264 decoder type
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 23.01.07   Time: 17:49
 * Updated in $/VS/VSClient
 * - change statistics output format
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 22.01.07   Time: 18:13
 * Updated in $/VS/VSClient
 * - fixed data, update bitrate control
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 22.01.07   Time: 8:55
 * Updated in $/VS/VSClient
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 17.01.07   Time: 17:19
 * Updated in $/VS/VSClient
 * - were added data bitrate control
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 11.01.07   Time: 17:41
 * Updated in $/VS/VSClient
 * - fixed bitrate control
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 28.12.06   Time: 17:56
 * Updated in $/VS/VSClient
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 28.12.06   Time: 10:04
 * Updated in $/VS/VSClient
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 22.12.06   Time: 11:28
 * Updated in $/VS/VSClient
 * - were change bitrate control
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 20.12.06   Time: 11:54
 * Updated in $/VS/VSClient
 * - added Notifay for packets added in ControlBand Module
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 20.12.06   Time: 11:36
 * Updated in $/VS/VSClient
 * - added DEBUG defines
 * - change bitrate control
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 16.12.06   Time: 13:48
 * Updated in $/VS/VSClient
 * - remove TIME_DELAY command analisys in SetReceivedCommand
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 14.12.06   Time: 13:15
 * Updated in $/VS/VSClient
 * - changed GetVideoBandwidth
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 14.12.06   Time: 12:54
 * Updated in $/VS/VSClient
 * - changed command intraface for receiver
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 13.12.06   Time: 18:33
 * Updated in $/VS/VSClient
 * - comands in NHP intrfaces
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 13.11.06   Time: 13:33
 * Updated in $/VS/VSClient
 * - warnings and simple errors removed
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 10.11.06   Time: 16:49
 * Created in $/VS/VSClient
 * - added NHP moules implementation
 *
 ****************************************************************************/

#include "VS_UDPStream.h"
#include "VS_Dmodule.h"
#include "VS_ApplicationInfo.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include <math.h>
#include "streams/VS_SendFrameQueueNhp.h"

#define BITRATE_TIME_BOOST_MS (60000)
#define BITRATE_TIME_DELAY_MS (10000)

/**
 **************************************************************************
 * \brief Constructor
 ****************************************************************************/
VS_ControlQueueNhp::VS_ControlQueueNhp(bool is_cmd_send)
{
	m_list.clear();
	m_last_bitrate = 0;
	m_bandwidth = -1;
	m_upper_bandwidth = -1;
	m_time_update_stat = 0;
	m_sttime_update_stat = 0;
	m_is_loss = false;
	m_is_cmd_send = is_cmd_send;
	m_upcoef0 = 0;
	m_upcoef1 = 0;
	m_last_rcv_pkts = 1;
	m_rcv_databytes = 0;
	m_is_dataloss = false;
	m_avg_bitrate_rcv = 0;
	m_reducecoef = 1.0;
	m_over_upcoef1 = BITRATE_TIME_BOOST_MS / 1000 / 2;
	m_num_stat = 0;

	m_LimitSize = VIDEODATASIZE_DEFAULT;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&m_LimitSize, sizeof(int), VS_REG_INTEGER_VT, "PacketLimit");
	if (m_LimitSize < VIDEODATASIZE_MIN) m_LimitSize = VIDEODATASIZE_MIN;
	if (m_LimitSize > VIDEODATASIZE_MAX) m_LimitSize = VIDEODATASIZE_MAX;

	memset(&m_conf_st, 0, sizeof(VS_ConferenceStatSnd));

	for (int i = 0; i < 5; i++)
		m_CurrBitrate[i] = 0;
	m_last_analyse = 0;

	m_BandAvg.Init(3);
	m_LoadAvg.Init(6);
	m_is_overflow = false;
}

/**
 **************************************************************************
 * \brief Add statistic for send packet
 ****************************************************************************/
void VS_ControlQueueNhp::Add(unsigned long time, unsigned short size, unsigned short track, unsigned short stime, unsigned short stimeonly, unsigned char slayer)
{
	if (!stimeonly) {
		if (track == 1) track = 1;
		else if (track == 2) track = 2;
		else if (track == 5) track = 3;
		else track = 0;
		VS_SendPacketCharacterNhp str = {time, size, track, stime, stimeonly};
		m_list.push_front(str);
		if (m_list.size() > 600)
			m_list.pop_back();
		if (time - m_last_analyse > 3000) AnalyseStat(time);
		// calculate conference statistics
		m_conf_st.bytes_cur[track] += size;
	}
}

/**
 **************************************************************************
 * \brief Deside put data into the queue or not
 ****************************************************************************/
bool VS_ControlQueueNhp::IsDataAllowed(int MaxBand)
{
	bool res = true;
	unsigned long t3 = timeGetTime();
	unsigned long t2 = 0, t1 = 0, i = 0, t4 = 0;
	unsigned long tmax = 0, count=0;
	unsigned long video = 0, data = 0, size = m_list.size(), dband = 0, min_band = 0;

	if (!m_is_cmd_send) {
		m_bandwidth = MaxBand;
		m_over_bandwidth = MaxBand;
	}

	while (i < size) {
		VS_SendPacketCharacterNhp &str = m_list[i];
		if (t3 - str.time < 1000) {
			if (!t4) t4 = str.time;				// fix last send time
			if (str.track == 2)
				video += str.size;
			else if (str.track == 3) {
				if (!t1) t1 = str.time;			// fix last data send time
				if (t2 && t2 - str.time > tmax)
					tmax = t2 - str.time;		// fix the largest not data sent interval
				t2 = str.time;					// fix first data  send time
				data += str.size;
				count++;
			}
			i++;
		}
		else
			break;
	}

	video = video / 128;							// assume ~1000 msec
	if (t1 != 0) {								// data has not been sent in 1000 msec, allow
		unsigned long dt = (t1 - t2 <= 1) ? 1 : t1 - t2;
		dt += t3 - t1;									// suppose last sent time is now
		dt = dt - tmax + (t1 - t2) / count + 1;			// sub max time to avoid long empty window, add avverage instead
		data = data * 1000 / dt / 128;
		data = (unsigned long)(data * (0.95 + 0.1 / RAND_MAX * rand()));

		if (data >= (unsigned long)MaxBand) {
			res = false;
		} else {
			dband = std::min(m_over_bandwidth, MaxBand);
			if (dband > video)
				dband -= video;
			else
				dband = 0;
			min_band = std::min((unsigned long)(m_rcv_databytes / 128), dband);
			if (video) {
				dband = std::max(dband, video / 5);
				res = data < dband;
			} else {
				if (m_is_dataloss) {
					res = data < min_band;
				} else {
					res = data < (unsigned long)((0.95 + 0.1 / RAND_MAX * rand()) * m_over_bandwidth);
				}
			}
		}
	}

	return res;
}

/**
 **************************************************************************
 * \brief Update bitrate state
 ****************************************************************************/
void VS_ControlQueueNhp::UpdateState(int updelay, int c0, int c1, int start_bitrate, int max_bitrate, unsigned int ctime)
{
	if (updelay >= 0) m_sttime_update_stat = ctime + updelay;
	if (c0 >= 0) {
		m_upcoef0 = max_bitrate - c0;
		m_over_bandwidth = max_bitrate;
	}
	if (c1 >= 0) m_upcoef1 = c1;
	m_start_bitrate = start_bitrate;
}

/**
 **************************************************************************
 * \brief Update current bandwidth
 ****************************************************************************/
void VS_ControlQueueNhp::UpdateBandwidth()
{
	unsigned int ctime = timeGetTime();

	m_is_dataloss = m_total_stat.typeStat[NHPH_DT_DATA].loss_pkts > (unsigned short)(0.05 * m_total_stat.typeStat[NHPH_DT_DATA].rcv_pkts + 1);
	m_rcv_databytes = m_total_stat.typeStat[NHPH_DT_DATA].rcv_bytes;
	m_is_loss = m_total_stat.lossByBitrate > 0;
	m_reducecoef = (m_total_stat.reserved == 0) ? 0.75 : 1.0;

	unsigned short loss_pkts = m_total_stat.typeStat[NHPH_DT_VIDEO].loss_pkts,
			       rcv_pkts = m_total_stat.typeStat[NHPH_DT_VIDEO].rcv_pkts;
	int rcv_bytes = m_total_stat.typeStat[NHPH_DT_VIDEO].rcv_bytes;
	int curr_bandwidth = (rcv_bytes * 8 + 512) / 1024;
	int avg_bandwidth = curr_bandwidth;
	int prev_btr = m_last_bitrate;
	int btr_load = 100;

	if (m_num_stat > 4) {
		AnalyseStat(ctime);
		m_BandAvg.Snap(curr_bandwidth);
		m_BandAvg.GetAverage(avg_bandwidth);
		m_bandwidth = avg_bandwidth + (m_rcv_databytes * 8 + 512) / 1024;
		if (m_is_loss) {
			//int dbtr = m_CurrBitrate[NHPH_DT_VIDEO] - curr_bandwidth;
			//double avg_pkts = (double)rcv_bytes / (double)rcv_pkts;
			//int dloss = (int)(loss_pkts * avg_pkts * 8.0 / 1024.0 + 0.5);
			//dbtr = std::max(dbtr, dloss);
			double avg_pkts = (double)rcv_bytes / (double)rcv_pkts;
			int dbtr = (int)(loss_pkts * avg_pkts * 8.0 / 1024.0 + 0.5);
			m_last_bitrate = std::max(m_last_bitrate, (int)m_CurrBitrate[NHPH_DT_VIDEO]);
			m_last_bitrate -= dbtr;
			m_last_bitrate = std::min(m_last_bitrate, avg_bandwidth); /// min of avg band & new send bitrate
			m_last_bitrate = std::max((prev_btr * 80) / 100, m_last_bitrate); /// reduce only 20%
			m_last_bitrate = std::max(m_last_bitrate, BITRATE_MIN);
			int max_btr = (int)((m_last_bitrate * 130.0) / 100.0);
			max_btr = std::min(max_btr, m_upper_bandwidth);
			UpdateState(BITRATE_TIME_DELAY_MS, m_last_bitrate, BITRATE_TIME_BOOST_MS, m_last_bitrate, max_btr, ctime);
		}
		btr_load = (int)(m_CurrBitrate[NHPH_DT_VIDEO] / (double)(avg_bandwidth + 1.0) * 100);
		m_LoadAvg.Snap(btr_load);
		m_LoadAvg.GetAverage(btr_load);
	} else {
		avg_bandwidth = (int)m_CurrBitrate[NHPH_DT_VIDEO];
		curr_bandwidth = avg_bandwidth;
		m_BandAvg.Snap((int)m_CurrBitrate[NHPH_DT_VIDEO]);
		m_BandAvg.GetAverage(avg_bandwidth);
		m_LoadAvg.Snap(btr_load);
		m_LoadAvg.GetAverage(btr_load);
	}

	m_num_stat++;
	//if (btr_load > 120 && !m_is_loss) {
	//	m_time_update_stat = ctime - 3000;
	//}

	DTRACE(VSTM_BTRC,  "vid=| %d| %3d| %3d| %7u| %5d| %4d| %4d| %4d| %4d| \n\t\t\t\t\t\t\tdat=| %d| %7d|",
						m_is_loss, loss_pkts, rcv_pkts, curr_bandwidth, avg_bandwidth, (int)m_CurrBitrate[NHPH_DT_VIDEO], btr_load, prev_btr, m_last_bitrate,
						m_is_dataloss, m_rcv_databytes);

}

/**
 **************************************************************************
 * \brief Calculate need video bandwidth
 ****************************************************************************/
int VS_ControlQueueNhp::GetVideoBandwidth(int MaxBand)
{
	int dt = 0;
	unsigned int ctime = timeGetTime();

	//if (m_time_update_stat == 0) m_time_update_stat = ctime - 1000;

	if ((ctime - m_time_update_stat) > 3 * QBITRATE_TIME_INTERVAL && m_is_cmd_send) {
		if (m_last_bitrate == 0) {
			m_upper_bandwidth = MaxBand;
			m_last_bitrate = (MaxBand + 1) / 2;
			if (MaxBand < BITRATE_MIN || m_last_bitrate < BITRATE_MIN) m_last_bitrate = BITRATE_MIN;
			UpdateState(0, m_last_bitrate, BITRATE_TIME_BOOST_MS, m_last_bitrate, MaxBand, ctime);
			m_avg_bitrate_rcv = m_last_bitrate;
		} else if (m_upper_bandwidth != MaxBand) {
			m_upper_bandwidth = MaxBand;
			int c0 = m_over_bandwidth - m_upcoef0;
			UpdateState(-1, c0, -1, m_last_bitrate, MaxBand, ctime);
		}

		int new_btr = m_last_bitrate;

		if (m_CurrBitrate[NHPH_DT_VIDEO] > 0 && m_num_stat > 5) {
			int max_btr = 0;
			int avg_rcvbtr = 0;
			int btr_load = 100;
			m_LoadAvg.GetAverage(btr_load);
			m_BandAvg.GetAverage(avg_rcvbtr);

			dt = ctime - m_sttime_update_stat;

			if (btr_load > 400 || (btr_load > 120 && dt > -3000)) {
				new_btr = (int)((avg_rcvbtr * 100.0) / (double)btr_load);
				new_btr = std::max((m_last_bitrate * 80) / 100, new_btr); /// reduce only 20%
				new_btr = std::max(new_btr, BITRATE_MIN);
				max_btr = (int)((new_btr * 130.0) / 100.0);
				max_btr = std::min(max_btr, MaxBand);
				UpdateState(BITRATE_TIME_DELAY_MS, new_btr, BITRATE_TIME_BOOST_MS, new_btr, max_btr, ctime);
			} else {
				if (dt > 0) {
					if (dt > m_upcoef1) dt = m_upcoef1;
					new_btr = (int)(m_reducecoef * (m_start_bitrate + m_upcoef0 * sin(1.570796327 * dt / m_upcoef1)));
					new_btr = std::min(new_btr, MaxBand);
					if (dt == m_upcoef1 && m_over_bandwidth < MaxBand) {
						max_btr = 3 * m_over_bandwidth / 2;
						max_btr = std::min(max_btr, MaxBand);
						UpdateState(0, new_btr, m_over_upcoef1 * 1000, new_btr, max_btr, ctime);
						//m_over_upcoef1 += 30;
					}
				}
			}
		}

		if (abs(new_btr - m_last_bitrate) < 5 && new_btr != MaxBand) new_btr = m_last_bitrate;

		DTRACE(VSTM_BTRC,  "get=| %7d| %7d| %7d| %7d| (stbtr = %d, c0 = %d, c1 = %d, dt = %d, over_band = %d)",
							(int)m_CurrBitrate[NHPH_DT_VIDEO], m_last_bitrate, new_btr, MaxBand,
							m_start_bitrate, m_upcoef0, m_upcoef1, dt, m_over_bandwidth);

		m_last_bitrate = new_btr;
		m_time_update_stat = ctime;
	} else if (!m_is_cmd_send) {
		m_last_bitrate = MaxBand;
	}

	return m_last_bitrate;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ControlQueueNhp::AnalyseStat(unsigned int ctime)
{
	unsigned long i = 0;
	unsigned long size = m_list.size();
	if (m_last_analyse == 0) m_last_analyse = ctime;
	unsigned int timeWindow = ctime - m_last_analyse;

	if (timeWindow > 0 && size > 0) {
		for (i = 0; i < 5; i++)
			m_CurrBitrate[i] = 0;
		timeWindow = 0;
		while (i < size) {
			VS_SendPacketCharacterNhp &str = m_list[i];
			if (ctime - str.time < 3000) {
				m_CurrBitrate[str.track] += str.size;
				timeWindow = ctime - str.time;
				i++;
			} else
				break;
		}
		if (timeWindow > 0) {
			for (i = 0; i < 5; i++)
				m_CurrBitrate[i] = (m_CurrBitrate[i] * 1000.0) / (timeWindow * 128.0);
		}
		m_last_analyse = ctime;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_ControlQueueNhp::GetCurrBand(BandType type)
{
	return (type < BT_CMD || type > BT_ALL) ? 0 : int(m_CurrBitrate[type] + 0.5);
}

/**
 **************************************************************************
 * \brief Analise incoming commands from stream
 ****************************************************************************/
bool VS_ControlQueueNhp::SetReceivedCommand(stream::Command& cmd)
{
	if (!m_is_cmd_send) return 0;

	switch (cmd.type)
	{
	case stream::Command::Type::Stat:
		memcpy(&m_total_stat, cmd.data, cmd.data_size);
		UpdateBandwidth();
		break;
	case stream::Command::Type::RequestPacket:
		{
			VS_RequestPacket *rpacket = reinterpret_cast<VS_RequestPacket*>(cmd.data);
			VS_SendFrameQueueNhp *queue = (VS_SendFrameQueueNhp *)m_sendFrameQueue;
			queue->ResendPackect(rpacket->id_packet, rpacket->type_data);
			NotifyQueueHandler();
		}
		break;
	default:
		return false;
	}
	return true;
}