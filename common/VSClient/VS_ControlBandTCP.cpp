/**
 **************************************************************************
 * \file VS_ControlBandTCP.cpp
 * (c) 2002-2006 Visicron Systems, Inc. All rights reserved.
 *									http://www.visicron.net/
 * \brief Control Bandwith by analising local and broker statistic
 * \b Project Client
 * \author SMirnovK
 * \date 08.12.2006
 *
 * $Revision: 6 $
 *
 * $History: VS_ControlBandTCP.cpp $
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 13.08.12   Time: 14:29
 * Updated in $/VSNA/VSClient
 * - processor load removed
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 16.07.12   Time: 19:59
 * Updated in $/VSNA/VSClient
 * - update SVC client to spatial
 * - update vpx libs to v1.1.0
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 16.02.12   Time: 17:00
 * Updated in $/VSNA/VSClient
 * - add SVC capability
 * - change MediaFormat structure
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 24.03.10   Time: 19:21
 * Updated in $/VSNA/VSClient
 * - were added calc statictics (bug 7127)
 * - were merged all calc statistics in sender and receiver
 * - were added jitter calculation in Nhp receiver
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 25.03.09   Time: 15:03
 * Updated in $/VSNA/VSClient
 * 5.5 PVC enhancments:
 * - added "adaptive data decode" capability
 * - new bitrate control for data
 *
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 8.02.07    Time: 20:11
 * Updated in $/VS2005/VSClient
 * - bitrate adjustment
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 26.01.07   Time: 16:11
 * Updated in $/VS/VSClient
 * - support h.264 decoder type
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 22.01.07   Time: 17:39
 * Updated in $/VS/VSClient
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 17.01.07   Time: 19:45
 * Updated in $/VS/VSClient
 * - added dynamic tracking for max out bitrate value
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 14.12.06   Time: 19:42
 * Updated in $/VS/VSClient
 * - fix bag in sent time calculatin
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 14.12.06   Time: 12:54
 * Updated in $/VS/VSClient
 * - changed command intraface for receiver
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 12.12.06   Time: 15:56
 * Created in $/VS/VSClient
 * - new bitrate control module
 * - comand interfase changed
 *
 ****************************************************************************/
#include "VS_StreamPacketManagment.h"
#include "VS_Dmodule.h"
#include "VS_TCPStream.h"
#include "../streams/Protocol.h"
#include "../streams/Statistics.h"
#include <math.h>


/**
 **************************************************************************
 ****************************************************************************/
void VS_ControlBandTCP::Clear()
{
	m_list.clear();
	m_prevBrokerTime = 0;
	m_prevRecalculateTime = 0;
	m_prevStatTime = 0;
	m_overflowTime = 0;
	m_prevAddPacket = 0;
	m_numIterates = 0;
	m_numTicks = 0;
	m_phSndBitrate = UNDEF_BANDWIDTH;
	m_calcSndBitrate = UNDEF_BANDWIDTH;
	m_lowPhBandwidth = UNDEF_BANDWIDTH;
	m_highPhBandwidth = UNDEF_BANDWIDTH;
	m_coefBitrate = 0;
	m_eState = PART_UNDEF;
	m_phBandwidthChangeTime = -1;
	m_coefLoad = 0;
	m_prevCalcBandwidth = 0;
	memset(m_Tracks, 0, sizeof(m_Tracks));
	memset(&m_conf_st, 0, sizeof(VS_ConferenceStatSnd));
	memset(m_instantQueueLen, 0, MAX_WND_SIZE * sizeof(int));
	memset(m_instantQueueBytes, 0, MAX_WND_SIZE * sizeof(int));
	for (int i = 0; i <= BT_ALL; i++) {
		memset(m_sndBitrate[i], 0, MAX_TICKS_SIZE * sizeof(double));
		m_avgSndBitrate[i] = 0.0;
	}
}


/**
 **************************************************************************
 ****************************************************************************/
void VS_ControlBandTCP::Add(DWORD time, USHORT size, USHORT track, USHORT stime, USHORT stimeonly, UCHAR slayer)
{
	if (track==1) track = 1;
	else if (track==2) track = 2;
	else if (track==5) track = 3;
	else track = 0;
	VS_SendPacketCharacter str = {time, size, track, stime, stimeonly};
	m_list.push_front(str);
	if (m_list.size() > 800)
		m_list.pop_back();
	if (!stimeonly) {
		m_sndBitrate[track][m_numTicks%MAX_TICKS_SIZE] += size;
		m_sndBitrate[BT_ALL][m_numTicks%MAX_TICKS_SIZE] += size;
	}
	// calculate conference statistics
	if (!stimeonly) {
		m_conf_st.bytes_cur[track] += size;
	}
}



/**
 **************************************************************************
 ****************************************************************************/
DWORD VS_ControlBandTCP::AnaliseBrokerStat(DWORD currTime)
{
	DWORD i;
	DWORD BuffOffs = m_Tracks[4].currBrokerBuffs/100;
	DWORD timeWindow = currTime - m_prevBrokerTime;
	DWORD size = m_list.size();
	if (timeWindow && size) {
		for (i = 0; i< 5; i++)
			m_Tracks[i].CurrBitrate = m_Tracks[i].CurrPktsSpeed = m_Tracks[i].CurrPktsTime = 0;
		i = BuffOffs; // shift back by buffers existed on broker
		DWORD t1 = currTime, t2 = currTime;
		i+=BuffOffs;
		if (i<size)
			t1 = t2 = m_list[i].time;
		while (i<size) {
			VS_SendPacketCharacter &str = m_list[i];
			if (t2 - str.time < timeWindow) {
				if (!str.stimeonly) {
					m_Tracks[str.track].CurrBitrate+=str.size;
					m_Tracks[str.track].CurrPktsSpeed+=1;
					m_Tracks[4].CurrBitrate+=str.size;
					m_Tracks[4].CurrPktsSpeed+=1;
				}
				m_Tracks[str.track].CurrPktsTime+=str.stime;
				m_Tracks[4].CurrPktsTime+=str.stime;
				t1 = str.time;
			}
			else
				break;
			i++;
		}
		timeWindow = t2-t1;
	}
	if (timeWindow) {
		float totalPktsSpeed = m_Tracks[4].CurrPktsSpeed*1000/timeWindow;
		for (i= 0; i< 5; i++) {
			VS_TrackInfo &Track = m_Tracks[i];
			Track.CurrBitrate = Track.CurrBitrate*(float)7.8125/timeWindow;
			if (Track.CurrPktsSpeed > 0) {
				Track.CurrPktsTime = Track.CurrPktsTime*1000/timeWindow;
				Track.CurrPktsSpeed = Track.CurrPktsSpeed*1000/timeWindow;
				float BrokerLoad = (float)Track.currBrokerBuffs/totalPktsSpeed;	// % of track packets in regard to total sent packets
				Track.BrokerLoad = BrokerLoad;
				if (abs(int(Track.currBrokerBuffs - Track.prevBrokerBuffs)) > 100 && BrokerLoad > 1) {
					// > 1% and > one averaged buffer, can not determine if less then one
					Track.lastExceedTime = currTime;
					int BrokerBand = int(Track.CurrBitrate*(Track.CurrPktsSpeed-Track.currBrokerBuffs/100.+Track.prevBrokerBuffs/100.)/Track.CurrPktsSpeed);
					if (BrokerBand < 0)
						BrokerBand = 0;
					if (Track.BrokerBand)
						Track.BrokerBand = (Track.BrokerBand*4 + BrokerBand)/5;
					else
						Track.BrokerBand = BrokerBand;
				}
				else if (Track.currBrokerBuffs==0 && Track.prevBrokerBuffs==0)
					Track.BrokerLoad = 0;
			}
			else {
				if (Track.currBrokerBuffs && Track.prevBrokerBuffs) {
					Track.BrokerLoad = Track.BrokerLoad*Track.currBrokerBuffs/Track.prevBrokerBuffs;
				}
				else {
					Track.BrokerLoad = 0;
				}
			}
			Track.prevBrokerBuffs = Track.currBrokerBuffs;
		}
	}
	m_prevBrokerTime = currTime;
	Calculate(currTime);
	return 0;
}


/**
 **************************************************************************
 ****************************************************************************/
void VS_ControlBandTCP::Calculate(DWORD currTime)
{
	for (DWORD i = 0; i<5; i++) {
		float BrokerLoad = m_Tracks[i].BrokerLoad;
		float CurrPktsTime = m_Tracks[i].CurrPktsTime;
		DWORD OldBand = m_Tracks[i].RecomendedBitrate > 0 ? m_Tracks[i].RecomendedBitrate : m_Tracks[i].MaxBitrate;
		DWORD newBand = OldBand;
		if		(BrokerLoad > 50) {
			if (OldBand > m_Tracks[i].BrokerBand*3/4)
				newBand = m_Tracks[i].BrokerBand*3/4;
		}
		else if (BrokerLoad > 25) {
			if (OldBand > m_Tracks[i].BrokerBand*7/8)
				newBand = m_Tracks[i].BrokerBand*7/8;
		}
		else if (BrokerLoad > 5) {
			if (OldBand > m_Tracks[i].BrokerBand*7/8)
				newBand = DWORD(OldBand/(1.+BrokerLoad/2/100.));
		}
		else if (CurrPktsTime > 120) {
			newBand = DWORD(OldBand*(1.-CurrPktsTime/2000));
		}
		else if (BrokerLoad > 1 || CurrPktsTime > 40)
			newBand = OldBand;
		else if (OldBand < m_Tracks[i].MaxBitrate*94/100) {
			if (BrokerLoad > 0 || CurrPktsTime > 1) {
				if (m_Tracks[i].BrokerBand && currTime-m_Tracks[i].lastExceedTime<33000) {
					if (OldBand < m_Tracks[i].BrokerBand) {
						newBand = std::min(DWORD(OldBand*1.06+1.), m_Tracks[i].BrokerBand);
					}
				}
				else
					newBand = DWORD(OldBand*1.06+1.);
			}
			else {
				if (m_Tracks[i].BrokerBand && currTime-m_Tracks[i].lastExceedTime<12000) {
					if (OldBand < m_Tracks[i].BrokerBand) {
						newBand = DWORD(OldBand*1.12+1.);
					}
				}
				else
					newBand = DWORD(OldBand*1.12+1.);
			}
		}
		if (newBand > m_Tracks[i].MaxBitrate)
			newBand = m_Tracks[i].MaxBitrate;
		if (newBand < 0.7*OldBand)
			newBand = DWORD(0.7*OldBand);
		if (newBand < 5)
			newBand = 5;
		if (m_Tracks[i].CurrPktsSpeed==0)
			newBand = OldBand;
		m_Tracks[i].RecomendedBitrate = newBand;
	}
	//DTRACE(VSTM_BTRC, "qlen = %6d, qbytes = %6d, btr = %7d",  m_Tracks[4].CurrQueueLen,  m_Tracks[4].CurrQueueBytes, m_Tracks[2].RecomendedBitrate);
	//DTRACE(VSTM_BTRC, "\n tot=|%4d|%4d|%5.1f|%4d|\n vid=|%4d|%4d|%5.1f|%4d|\n dat=|%4d|%4d|%5.1f|%4d|",
	//	m_Tracks[4].BrokerBand, m_Tracks[4].prevBrokerBuffs, m_Tracks[4].CurrPktsTime, m_Tracks[4].RecomendedBitrate,
	//	m_Tracks[2].BrokerBand, m_Tracks[2].prevBrokerBuffs, m_Tracks[2].CurrPktsTime, m_Tracks[2].RecomendedBitrate,
	//	m_Tracks[3].BrokerBand, m_Tracks[3].prevBrokerBuffs, m_Tracks[3].CurrPktsTime, m_Tracks[3].RecomendedBitrate);
	m_prevRecalculateTime = currTime;
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_ControlBandTCP::IsDataAllowed(int MaxBand)
{
	m_Tracks[3].MaxBitrate = MaxBand;
	DWORD t3=timeGetTime();
	DWORD t2=0, t1=0, i=0, t4=0;
	DWORD tmax = 0, count=0;
	DWORD video = 0, data = 0, size = m_list.size();
	while (i<size) {
		VS_SendPacketCharacter &str = m_list[i];
		if (t3-str.time < 1000) {
			if (!t4) t4 = str.time;		// fix last send time
			if (str.track==2)
				video+=str.size;
			else if (str.track==3) {
				if (!t1) t1 = str.time; // fix last data send time
				if (t2 && t2-str.time > tmax)
					tmax = t2-str.time; // fix the largest not data sent interval
				t2 = str.time;			// fix first data  send time
				data+=str.size;
				count++;
			}
			i++;
		}
		else
			break;
	}
	if (t1==0)							// data has not been sent in 1000 msec, allow
		return true;
	video = video/128;					// assume ~1000 msec
	DWORD brokerBand = GetBitrate(3);
	DWORD dt = t1-t2<=1 ? 1 : t1-t2;
	dt+=t3-t1;							// suppose last sent time is now
	dt=dt-tmax+(t1-t2)/count+1;			// sub max time to avoid long empty window, add avverage instead
	data = data*1000/dt/128;
	data = DWORD(data*(0.95 + 0.1/RAND_MAX*rand()));

	brokerBand = std::min(brokerBand, MaxBand-video);
	if (brokerBand < video/5)
		brokerBand = video/5;

	return data < brokerBand;
}

/**
 **************************************************************************
 ****************************************************************************/
DWORD VS_ControlBandTCP::GetBitrate(int type)
{
	unsigned int btr = m_Tracks[type].RecomendedBitrate;
	if (type == 2 || type == 3) {
		if (!m_direct_connection) btr = std::min<decltype(btr)>(btr, m_calcSndBitrate);
		else btr = m_calcSndBitrate;
	}
	return btr;
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_ControlBandTCP::CalculatePhBandwidth(unsigned int currTime)
{
	int coefLoad = 0;
	int band = m_phSndBitrate;
	int idx = (m_numIterates) ? (m_numIterates - 1) % MAX_WND_SIZE : 0;
	if (m_Tracks[4].CurrQueueBytes >= QBYTES_MIN) {
		coefLoad = m_instantQueueBytes[idx] * 100 / (m_avgSndBitrate[BT_ALL] * 128 + 1);
		if (coefLoad >= UPDATE_BANDWIDTH_PERCENT) {
			if (m_eState & (PART_UNDEF | PART_IDLE | PART_FREEZE)) {
				m_highPhBandwidth = 950 * m_phSndBitrate / 1000;
			}
			band = static_cast<int32_t>(m_avgSndBitrate[BT_ALL]);
			m_overflowTime = currTime;
		}
	}
	if (m_phSndBitrate != UNDEF_BANDWIDTH) {
		__int64 dt = currTime - m_overflowTime;
		if (dt >= STAT_BITRATEFREEZE_TM) {
			if (coefLoad >= FREEZE_BITRATE_PERCENT) {
				m_overflowTime = currTime - STAT_BITRATEFREEZE_TM;
				dt = currTime - m_overflowTime;
			}
		}
	}
	if (band < BANDWIDTH_MIN) band = BANDWIDTH_MIN;
	m_phSndBitrate = band;
	return coefLoad;
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_ControlBandTCP::CalculateSendBitrate(unsigned int currTime, int maxBand, int coefLoad)
{
	int lowThr = 60;
	int hiThr = 105;
	int deltaInc = 1;
	int tickUpperFirst = 12;
	int kInceaseIdle = 10250;
	int kInceaseOther = 10900;
	unsigned int increaseThr = 120;
	int kInceaseIdleRestrict = 10500;
	int kInceaseOtherRestrict = 11200;
	if (m_hw_encoder) {
		lowThr = 50;
		hiThr = 103;
		deltaInc = 2;
		tickUpperFirst = 36;
		kInceaseIdle = 10200;
		kInceaseOther = 10700;
		increaseThr = 130;
		kInceaseIdleRestrict = 10400;
		kInceaseOtherRestrict = 10900;
	}
	if (coefLoad >= UPDATE_BANDWIDTH_PERCENT) {
		bool bChange = (m_calcSndBitrate > m_phSndBitrate);
		m_phBandwidthRestrictTime = currTime + deltaInc * STAT_BITRATEFREEZE_TM;
		int setBtr = ((int)m_calcSndBitrate * 100) / (m_coefLoad + 50);
		if (setBtr < BANDWIDTH_MIN) setBtr = BANDWIDTH_MIN;
		if (setBtr > m_phSndBitrate) setBtr = m_phSndBitrate;
		m_calcSndBitrate = setBtr;
		m_lowPhBandwidth = setBtr;
		if (m_eState & PART_UNDEF) {
			m_highPhBandwidth = maxBand;
		}
		else if (m_eState & PART_FIRSTUPPER) {
			m_highPhBandwidth = hiThr * m_phSndBitrate / 100;
		}
		if (m_highPhBandwidth > maxBand) m_highPhBandwidth = maxBand;
		if (m_highPhBandwidth <= m_lowPhBandwidth) m_highPhBandwidth = hiThr * m_lowPhBandwidth / 100;
		m_eState = PART_RESTRICT;
	} else if (coefLoad >= FREEZE_BITRATE_PERCENT) {
		if (m_eState & (PART_FREEZE | PART_IDLE)) {
			m_eState = PART_FREEZE;
			m_phBandwidthChangeTime = currTime + 5 * STAT_BITRATEFREEZE_TM;
		}
	} else if (m_phSndBitrate != UNDEF_BANDWIDTH) {
		if (m_eState & PART_RESTRICT) {
			if (currTime > m_phBandwidthRestrictTime) {
				m_eState = PART_FIRSTUPPER;
				m_phBandwidthChangeTime = currTime;
				m_coefBitrate = 0;
			}
		} else if ((m_eState & PART_FREEZE) && (m_phBandwidthChangeTime <= currTime)) {
			m_eState = PART_IDLE;
			m_phBandwidthChangeTime = currTime;
			m_lowPhBandwidth = m_calcSndBitrate;
			m_coefBitrate = 0;
		} else if (m_eState & (PART_FIRSTUPPER | PART_SECONDUPPER | PART_THIRDUPPER)) {
			if (m_calcSndBitrate >= m_highPhBandwidth) {
				m_eState = PART_FREEZE;
				m_phBandwidthChangeTime = currTime + 5 * STAT_BITRATEFREEZE_TM;
			}
		}
	} else {
		m_eState = PART_UNDEF;
		m_calcSndBitrate = (unsigned short)(m_calcSndBitrate * 1.12 + 1.);
		if (m_calcSndBitrate > maxBand) m_calcSndBitrate = maxBand;
	}

	if (!(m_eState & PART_RESTRICT)) {
		if (m_eState & (PART_FIRSTUPPER | PART_SECONDUPPER | PART_THIRDUPPER | PART_IDLE)) {
			unsigned int newBitrate = m_calcSndBitrate;
			if (m_coefBitrate == 0) {
				int oldBitrate = newBitrate;
				int nmax = deltaInc * 36;
				if (m_eState & PART_FIRSTUPPER) nmax = tickUpperFirst;
				m_coefBitrate = (int)(pow(10.0, log10((double)m_highPhBandwidth / (double)m_lowPhBandwidth) / (double)nmax) * 10000);
				oldBitrate = oldBitrate * m_coefBitrate / 10000;
				if (oldBitrate == newBitrate) m_coefBitrate = 10100;
				if (m_coefBitrate > 10000 && m_coefBitrate < 10100) m_coefBitrate = 10100;
			} else {
				int k = (newBitrate <= 100 && m_coefBitrate < 10800) ? 10800 : m_coefBitrate;
				newBitrate = (newBitrate * k + 5000) / 10000;
				if (newBitrate >= m_highPhBandwidth) m_coefBitrate = (m_eState & PART_IDLE) ? kInceaseIdle : kInceaseOther;
				if (newBitrate >= (increaseThr * m_highPhBandwidth / 100)) m_coefBitrate = (m_eState & PART_IDLE) ? kInceaseIdleRestrict : kInceaseOtherRestrict;
				if (newBitrate > (unsigned int)maxBand) newBitrate = maxBand;
			}
			m_calcSndBitrate = newBitrate;
		}
	}

	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_ControlBandTCP::GetLoadQueue()
{
	return m_coefLoad;
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_ControlBandTCP::GetVideoBandwidth(int MaxBand)
{
	DWORD currTime = timeGetTime();
	m_Tracks[2].MaxBitrate = MaxBand;
	if (currTime - m_prevRecalculateTime > 3000) {
		AnaliseBrokerStat(currTime);
	}
	if (m_calcSndBitrate == UNDEF_BANDWIDTH) m_calcSndBitrate = MaxBand;
	if (m_prevStatTime == 0) m_prevStatTime = currTime;
	unsigned int dt = currTime - m_prevStatTime;
	if (dt > 500) {
		int idx = m_numIterates % MAX_WND_SIZE;
		m_instantQueueLen[idx] = m_sendFrameQueue->GetSize();
		m_instantQueueBytes[idx] = m_sendFrameQueue->GetBytes();
		int nCount = (m_numIterates > MAX_WND_SIZE) ? MAX_WND_SIZE : m_numIterates;
		m_Tracks[4].CurrQueueLen = 0;
		m_Tracks[4].CurrQueueBytes = 0;
		for (int i = 0; i < nCount; i++) {
			m_Tracks[4].CurrQueueLen += m_instantQueueLen[i];
			m_Tracks[4].CurrQueueBytes += m_instantQueueBytes[i];
		}
		m_Tracks[4].CurrQueueLen /= MAX_WND_SIZE;
		m_Tracks[4].CurrQueueBytes /= MAX_WND_SIZE;
		m_numIterates++;
		idx = m_numTicks % MAX_TICKS_SIZE;
		m_numTicks++;
		int new_idx = m_numTicks % MAX_TICKS_SIZE;
		for (int i = 0; i <= BT_ALL; i++) {
			m_sndBitrate[i][idx] = (m_sndBitrate[i][idx] * 7.8125) / (double)dt;
			m_avgSndBitrate[i] = 0.0;
			for (int j = 0; j < MAX_TICKS_SIZE; j++) m_avgSndBitrate[i] += m_sndBitrate[i][j];
			m_avgSndBitrate[i] /= (double)MAX_TICKS_SIZE;
			m_sndBitrate[i][m_numTicks%MAX_TICKS_SIZE] = 0.0;
		}
		m_prevStatTime = currTime;
	}
	if (m_prevCalcBandwidth == 0) m_prevCalcBandwidth = currTime + 3000;
	if ((__int64)currTime - (__int64)m_prevCalcBandwidth > 2000) {
		m_coefLoad = CalculatePhBandwidth(currTime);
		CalculateSendBitrate(currTime, MaxBand, m_coefLoad);
		m_prevCalcBandwidth = currTime;
		int idx = (m_numIterates) ? (m_numIterates - 1) % MAX_WND_SIZE : 0;
		DTRACE(VSTM_BTRC, "\n qlen = %6d, qbytes = %6d (avg qbytes = %6u), btr = %7.1f, bl = %7d (v = %7.1f, a = %4.1f, d = %7.1f), ph = %6d [lb = %6d, hb = %6d], calc = %6d, set = %6u, state = %#010x",
			m_instantQueueLen[idx], m_instantQueueBytes[idx], m_Tracks[4].CurrQueueBytes, m_avgSndBitrate[BT_ALL], m_coefLoad, m_avgSndBitrate[BT_VIDEO], m_avgSndBitrate[BT_AUDIO], m_avgSndBitrate[BT_DATA],
			m_phSndBitrate, m_lowPhBandwidth, m_highPhBandwidth, m_calcSndBitrate, m_Tracks[2].RecomendedBitrate, m_eState);
	}
	return GetBitrate(2);
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_ControlBandTCP::GetCurrBand(BandType type)
{
	return (type < BT_CMD || type > BT_ALL) ? 0 : int(m_avgSndBitrate[type] + 0.5);
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_ControlBandTCP::SetReceivedCommand(stream::Command& cmd)
{
	if (cmd.type == stream::Command::Type::BrokerStat) {
		auto* pStat = reinterpret_cast<stream::StreamStatistics*>(cmd.data);
		int i;
		for (i = 0; i< 5; i++)
			m_Tracks[i].currBrokerBuffs = 0;
		for (i = 0; i< pStat->ntracks && i<10; i++) { /// restrict to first 10 tracks info
			int DataType;
			auto& ts = pStat->tracks[i];
			if      (ts.track == stream::Track::audio) DataType = 1;
			else if (ts.track == stream::Track::video) DataType = 2;
			else if (ts.track == stream::Track::data)  DataType = 3;
			else					DataType = 0;
			m_Tracks[DataType].currBrokerBuffs = ts.nFramesBuffer;
		}
		m_Tracks[4].currBrokerBuffs = pStat->allFramesBuffer;
		DWORD currTime = timeGetTime();
		AnaliseBrokerStat(currTime);
		m_direct_connection = false;
		return true;
	}
	return false;
}
