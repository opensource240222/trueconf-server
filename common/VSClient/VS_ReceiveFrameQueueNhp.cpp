/**
 **************************************************************************
 * \file VS_RecieveFrameQueueNhp.cpp
 * (c) 2002-2006 Visicron Systems, Inc. All rights reserved.
 *									http://www.visicron.net/
 * \brief Implement Receiver module. Coolect NHP packets in frames of data,
 *		  monitor loss in network.
 * \b Project Client
 * \author SMirnovK
 * \author AnufrievS
 * \date 09.11.2006
 *
 * $Revision: 13 $
 *
 * $History: VS_ReceiveFrameQueueNhp.cpp $
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 24.07.12   Time: 13:07
 * Updated in $/VSNA/VSClient
 * - udp : request loss packets for data
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 20.07.12   Time: 12:57
 * Updated in $/VSNA/VSClient
 * - fix bitrate loss in udp rcv
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 15.05.12   Time: 16:19
 * Updated in $/VSNA/VSClient
 * - enhancement UDP Multicast:
 * a) decrease key-frame interval
 * b) change decoder for udp: can decode after loss - only for vpx & h.264
 * c) vpx encoder without svc for udp
 * - can use any codec in groupconf
 * - SVC client capability is ON only for vpx
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 24.11.11   Time: 15:14
 * Updated in $/VSNA/VSClient
 * - fix nhp bitrate
 * - fix nhp "old" packet
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 28.09.11   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - beta nhp revision
 * - fix fps on low bitrates
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 18.03.11   Time: 18:55
 * Updated in $/VSNA/VSClient
 * - max  packet len decreased to 4000
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 20.05.10   Time: 15:49
 * Updated in $/VSNA/VSClient
 * - bugfix in nhp receiver (coupling incorrect frame)
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 24.03.10   Time: 19:21
 * Updated in $/VSNA/VSClient
 * - were added calc statictics (bug 7127)
 * - were merged all calc statistics in sender and receiver
 * - were added jitter calculation in Nhp receiver
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 23.12.09   Time: 19:18
 * Updated in $/VSNA/VSClient
 * - division by zero (udp)
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 23.12.09   Time: 19:13
 * Updated in $/VSNA/VSClient
 * - fix first packet id != 0 (udp)
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 11.12.09   Time: 19:23
 * Updated in $/VSNA/VSClient
 * - isIntercom defined for NHP classes (control queue, receiver); remove
 * stat/request send fo Intercom
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 11.12.09   Time: 18:32
 * Updated in $/VSNA/VSClient
 *  - were added constraint lenght for video packets (for TCP & NHP
 * connections)
 *  - were changed default and maximum lenght of video packets for NHP
 *  - were replaced constant video packet lenght (1200 bytes) with average
 * packet lenght in NHP receive module
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 22  *****************
 * User: Sanufriev    Date: 31.01.07   Time: 17:41
 * Updated in $/VS/VSClient
 * - change bitrate control when media data is absent
 *
 * *****************  Version 21  *****************
 * User: Sanufriev    Date: 30.01.07   Time: 16:40
 * Updated in $/VS/VSClient
 * - fixed bitrate control
 *
 * *****************  Version 20  *****************
 * User: Sanufriev    Date: 26.01.07   Time: 16:11
 * Updated in $/VS/VSClient
 * - support h.264 decoder type
 *
 * *****************  Version 19  *****************
 * User: Sanufriev    Date: 17.01.07   Time: 17:20
 * Updated in $/VS/VSClient
 * - fixed push audio & data packets for large drop
 *
 * *****************  Version 18  *****************
 * User: Sanufriev    Date: 12.01.07   Time: 15:28
 * Updated in $/VS/VSClient
 *  - added frame buffer optimal forming
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 11.01.07   Time: 17:44
 * Updated in $/VS/VSClient
 * - added frame buffer optimal forming
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 28.12.06   Time: 18:15
 * Updated in $/VS/VSClient
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 28.12.06   Time: 17:56
 * Updated in $/VS/VSClient
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 28.12.06   Time: 10:04
 * Updated in $/VS/VSClient
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 22.12.06   Time: 11:28
 * Updated in $/VS/VSClient
 * - were change bitrate control
 * - were change buffer lenght update for video
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 20.12.06   Time: 16:33
 * Updated in $/VS/VSClient
 * - fixed audio packets processing
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 20.12.06   Time: 11:38
 * Updated in $/VS/VSClient
 * - added DEBUG defines
 * - were change interface PushQuery, remove AnalisysQuery  in receive
 * module
 * - fixed errors in PushVideoPackets, PushOtherPackets
 * - fixed calculate statistics
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 14.12.06   Time: 12:54
 * Updated in $/VS/VSClient
 * - changed command intraface for receiver
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 14.12.06   Time: 11:37
 * Updated in $/VS/VSClient
 * - change processing commands in stream
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 13.12.06   Time: 18:33
 * Updated in $/VS/VSClient
 * - comands in NHP intrfaces
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 13.11.06   Time: 14:32
 * Updated in $/VS/VSClient
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 13.11.06   Time: 14:05
 * Updated in $/VS/VSClient
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 13.11.06   Time: 14:01
 * Updated in $/VS/VSClient
 * - added video & audio header for returning data in
 * VS_ReceiveFrameQueueNhp
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 13.11.06   Time: 13:33
 * Updated in $/VS/VSClient
 * - warnings and simple errors removed
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 13.11.06   Time: 12:48
 * Updated in $/VS/VSClient
 * - change parameters type in functions in VS_ReceiveFrameQueueNhp
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 13.11.06   Time: 12:22
 * Updated in $/VS/VSClient
 * - warnings removed
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 10.11.06   Time: 16:49
 * Created in $/VS/VSClient
 * - added NHP moules implementation
 *
 ****************************************************************************/

#include "streams/VS_SendFrameQueueNhp.h"
#include "VS_UDPStream.h"
#include "VS_Dmodule.h"

class AlgFindPacket
{
	unsigned short	m_uId;
public:
	AlgFindPacket(unsigned short id) { m_uId = id; }
	bool operator () (VS_ReceivePacketInfo &packet)
	{
		return (m_uId == packet.uId);
	};
};

/**
 **************************************************************************
 * \brief Constructor
 ****************************************************************************/
VS_ReceiveFrameQueueNhp::VS_ReceiveFrameQueueNhp(bool is_request, int type_decoding, bool is_cmd_send)
{
	m_iTypeDecoding = type_decoding;
	m_bRequest = is_request;
	m_bCmdSend = is_cmd_send;
	m_bRequestLag = true;
	m_iGrabIdFrame = -1;
	m_iNetLagFake = 100;
	m_iNetLag = m_iNetLagFake;
	m_iRealBandwidth = 0;
	m_eWaitKeyFrame = WAIT_FROM_STREAM;
	m_iNumBadFrames = 0;
	m_uLastTimeQKey = 0;
	m_uLastTimeQLag = 0;
	m_iLastTimeVideo = -1;
	m_iLastTimeAudio = -1;
	m_iTimeStatAudio = -1;
	m_uIdQLag = 0;
	m_iDBuffer = 0;
	m_uTimestampFrame = 0;
	m_bLossByBitrate = false;

	memset(m_listIdQlag, 0, 20);
	m_ReqPacket.byResending = 1;

	for (int i = 0; i < 4; i++) {
		m_DePacketizer[i].iLastId = -1;
		m_DePacketizer[i].dqPacket.clear();
		m_AvgStat[i].uAvgFPS = 0;
		m_AvgStat[i].uAvgPacketSize = 0;
		m_AvgStat[i].uLossPackets = 0;
		m_AvgStat[i].uRcvBytes = 0;
		m_AvgStat[i].uRcvPackets = 0;
		memset(&(m_ConfStat[i]), 0, sizeof(VS_ConferenceStatRcv));
		m_iPrevLossId[i] = -1;
	}
	m_mapFrame.clear();

	m_uStartStat = 0;
	m_uStartAnalyse = 0;
	memset(&m_TotalStat, 0, sizeof(VS_NhpTotalStat));
	m_AudioStat.Init(1000);
	m_LossAvg.Init(60);
	m_LossStat.Init(30);
	m_BandAvg.Init(60);
	m_FpsAvg.Init(60);
	m_iNumLoss = 0;
	m_iNumByBtrLoss = 0;
	m_iNumRcv = 0;
	m_iNumByBtrAllLoss = 0;

	srand(timeGetTime());
}

/**
 **************************************************************************
 * \brief Destructor
 ****************************************************************************/
VS_ReceiveFrameQueueNhp::~VS_ReceiveFrameQueueNhp()
{
	for (int i = 0; i < 4; i++) {
		m_DePacketizer[i].dqPacket.clear();
		memset(&(m_ConfStat[i]), 0, sizeof(VS_ConferenceStatRcv));
		m_iPrevLossId[i] = -1;
	}
	m_mapFrame.clear();
	memset(&m_TotalStat, 0, sizeof(VS_NhpTotalStat));
}

/**
 **************************************************************************
 * \brief Parsing incoming stream packets
 ****************************************************************************/
bool VS_ReceiveFrameQueueNhp::Add(unsigned char* buff, int size)
{
	bool ret = false;
	unsigned int cur_time = timeGetTime();

	if (size > 0) {
		int size_header = 0;
		unsigned char *pPacket = buff;
		VS_HeaderPacket pHdrPacket;
		size_header = DecodeHeader(pPacket, &pHdrPacket);

		/* /// Emulate loss packets
		if (pHdrPacket.fHeader.dataType == NHPH_DT_VIDEO) {
			if (rand() < 165) {
				DTRACE(VSTM_NHP_VIDEO, "rnd loss: v (%d, %d, %d, %d, %d)",
					   pHdrPacket.fHeader.resendNum, pHdrPacket.fHeader.SeqId, pHdrPacket.fHeader.typeOfPart, pHdrPacket.vHeader.FrameId, pHdrPacket.vHeader.FrameType);
				return 0;
			}
		}
		*/

		if (size_header >= 0 && size >= size_header) {
			pPacket += size_header;
			size -= size_header;
			(this->*PushPacket)(pPacket, size, &pHdrPacket, cur_time);
			ret = true;
		}
	}

	RequestTimeDelayPacket(cur_time);

	return ret;
}

/**
 **************************************************************************
 * \brief Decode packet header
 ****************************************************************************/
int VS_ReceiveFrameQueueNhp::DecodeHeader(unsigned char *pBuff, VS_HeaderPacket *pHeader)
{
	int size_header = 0;

	memset(pHeader, 0, sizeof(VS_HeaderPacket));
	memcpy(&(pHeader->fHeader), pBuff, sizeof(VS_NhpFirstHeader));
	size_header += sizeof(VS_NhpFirstHeader);

	switch (pHeader->fHeader.dataType) {
		case NHPH_DT_VIDEO:
			{
				memcpy(&(pHeader->vHeader), pBuff + size_header, sizeof(VS_NhpVideoHeader));
				size_header += sizeof(VS_NhpVideoHeader);
				PushPacket = &VS_ReceiveFrameQueueNhp::PushVideoPacket;
				break;
			}
		case NHPH_DT_AUDIO:
			{
				memcpy(&(pHeader->aHeader), pBuff + size_header, sizeof(VS_NhpAudioHeader));
				PushPacket = &VS_ReceiveFrameQueueNhp::PushAudioPacket;
				break;
			}
		case NHPH_DT_CMND: case NHPH_DT_DATA:
			{
				PushPacket = &VS_ReceiveFrameQueueNhp::PushOtherPacket;
				break;
			}
		default :
			{
				size_header = -1;
				break;
			}
	}
	return size_header;
}

/**
 **************************************************************************
 * \brief Analyse packets queue
 ****************************************************************************/
bool VS_ReceiveFrameQueueNhp::AnalyseDePacketizerQueue(unsigned char *pPacket, int size, VS_NhpFirstHeader *pHeader, unsigned char type, unsigned int timestamp, unsigned int ctime)
{
	char type2name[4][16] = {"cmd", "a", "v", "d"};
	int module_dbg[4] = {VSTM_NHP_OTHER, VSTM_NHP_AUDIO, VSTM_NHP_VIDEO, VSTM_NHP_OTHER};
	unsigned char bResending = pHeader->resendNum;
	unsigned short uPacketId = pHeader->SeqId;
	unsigned char uPacketType = pHeader->typeOfPart;
	bool bRepeated = false;
	int iNum = 0, i = 0;

	VS_ReceivePacketInfo packet;
	packet.SetPacket(pPacket, size, uPacketType, uPacketId, bResending, ctime, timestamp);
	DEPACKETIZER *pDePacket = &(m_DePacketizer[type].dqPacket);
	if (type == NHPH_DT_VIDEO || type == NHPH_DT_AUDIO || type == NHPH_DT_DATA) {
		unsigned short uLastId = m_DePacketizer[type].iLastId;
		unsigned short dId = (unsigned short)(uPacketId - uLastId);
		iNum = dId - 1;
		iterPacketInfo itPacket = find_if(pDePacket->begin(), pDePacket->end(), AlgFindPacket(uPacketId));
		if (itPacket != pDePacket->end() || bResending != 0 || (iNum > 30000 && itPacket == pDePacket->end())) {
			/// зарезервировано место для пакета с данным id
			if (itPacket != pDePacket->end() && itPacket->byResending != 0) {
				itPacket->SetPacket(pPacket, size, uPacketType, uPacketId, 0, ctime, timestamp);
			} else {
				DTRACE(module_dbg[type], "ignore repeated %s packet' : t = %u, (%d, %d, %d), l = %d",
					   type2name[type], timestamp, bResending, uPacketId, uPacketType, size);
				bRepeated = true;
			}
			iNum = AnalyseResendQueue(uPacketId, 0, type, ctime);
		} else {
			/// пакета с данным id нет в очереди
			if (iNum >= 0) {
				/// резервирование места для перезапрошенных пакетов
				for (i = 0; i < iNum; i++) {
					m_ReqPacket.iRcvTime = ctime;
					m_ReqPacket.iTimestamp = timestamp;
					m_ReqPacket.uId = (unsigned short)(uLastId + 1 + i);
					pDePacket->push_back(m_ReqPacket);
				}
			}
			iNum = AnalyseResendQueue(uLastId, iNum, type, ctime);
			pDePacket->push_back(packet);
			m_DePacketizer[type].iLastId = uPacketId;
		}
	} else {
		pDePacket->push_back(packet);
	}
	AnalyseStream(type, ctime, timestamp, iNum, size);

	return bRepeated;
}

/**
 **************************************************************************
 * \brief Push video packet to queue
 ****************************************************************************/
void VS_ReceiveFrameQueueNhp::PushVideoPacket(unsigned char *pPacket, int size, VS_HeaderPacket *pHeader, unsigned int ctime)
{
	unsigned int uTimestamp = pHeader->vHeader.TimeStamp;
	unsigned char bResending = pHeader->fHeader.resendNum;
	unsigned short uPacketId = pHeader->fHeader.SeqId;
	unsigned char uPacketType = pHeader->fHeader.typeOfPart;

	if (m_eWaitKeyFrame == WAIT_FROM_STREAM) {
		unsigned short dId = (unsigned short)(uPacketId - m_DePacketizer[NHPH_DT_VIDEO].iLastId);
		bool bKey = pHeader->vHeader.FrameType == NHPVH_FT_IFRAME;
		bool bOldPacket = ((bResending == 0 && !bKey) || bKey) && (dId > 30000);
		/// если ждем ключевой кадр из стрима
		if (!bKey) {
			if (bResending == 0 && !bOldPacket) m_DePacketizer[NHPH_DT_VIDEO].iLastId = uPacketId;
			AnalyseStream(NHPH_DT_VIDEO, ctime, uTimestamp, 0, size);
			if (!bOldPacket) {
				DTRACE(VSTM_NHP_VIDEO, "ignore v packet, wait i-frame' : t = %u, v (%d, %d, %d, %d, %d), l = %d, ct = %u",
					   uTimestamp, bResending, uPacketId, uPacketType, pHeader->vHeader.FrameId, pHeader->vHeader.FrameType, size, ctime);
			} else {
				DTRACE(VSTM_NHP_VIDEO, "ignore old v packet : t = %u, v (%d, %d, %d, %d, %d), l = %d, ct = %u",
						uTimestamp, bResending, uPacketId, uPacketType, pHeader->vHeader.FrameId, pHeader->vHeader.FrameType, size, ctime);
			}
			RequestKeyFrame(ctime);
			return;
		} else {
			m_eWaitKeyFrame = WAIT_FROM_BUFFER;
			m_iGrabIdFrame = pHeader->vHeader.FrameId;
			m_iPrevLossId[NHPH_DT_VIDEO] = -1;
			DTRACE(VSTM_NHP_VIDEO, "Receive I Frame : %d", m_iGrabIdFrame);
		}
	}

	DTRACE(VSTM_NHP_VIDEO, "receive v packet : t = %u, v (%d, %d, %d, %d, %d), l = %d, ct = %u, d = %d",
		   uTimestamp, bResending, uPacketId, uPacketType, pHeader->vHeader.FrameId, pHeader->vHeader.FrameType,
		   size, ctime, uPacketId - m_DePacketizer[NHPH_DT_VIDEO].iLastId);

	bool bRepeated = AnalyseDePacketizerQueue(pPacket, size, &(pHeader->fHeader), NHPH_DT_VIDEO, uTimestamp, ctime);

	if (!bRepeated) {
		VS_ReceiveFrameInfo *pFrame;
		VS_ReceiveFrameInfo frInfo;
		frInfo.uId = pHeader->vHeader.FrameId;
		frInfo.uType = pHeader->vHeader.FrameType;
		frInfo.uFirstId = 0;
		frInfo.uLastId = 0;
		frInfo.uState = 0;
		frInfo.uSize = 0;
		frInfo.uNumPackets = 0;

		pairFrameInsert retInsert = m_mapFrame.emplace(uTimestamp, frInfo);
		itFrameInfo itFrame = retInsert.first;
		pFrame = &(itFrame->second);

		if (retInsert.second == true || pHeader->fHeader.resendNum == 0) pFrame->iRcvTime = ctime;

		if (pFrame->uState != 4) {
			pFrame->uState += uPacketType;
			pFrame->uSize += size;
			pFrame->uNumPackets++;
			if (uPacketType == NHPH_TP_FIRST || uPacketType == NHPH_TP_ONEONLY) pFrame->uFirstId = uPacketId;
			if (uPacketType == NHPH_TP_LAST || uPacketType == NHPH_TP_ONEONLY) pFrame->uLastId = uPacketId;
			if (pFrame->uState == 3) {
				int iNum = (unsigned short)(pFrame->uLastId - pFrame->uFirstId) + 1;
				if (iNum == pFrame->uNumPackets) {
					DTRACE(VSTM_NHP_VIDEO, "set full frame state' : t = %u, (%d, %d), l = %d, p = %d)",
						   uTimestamp, pFrame->uId, pFrame->uType, pFrame->uSize, pFrame->uNumPackets);
					pFrame->uState = 4;
				}
			}
		}
	}
}

/**
 **************************************************************************
 * \brief Push audio packet to queue
 ****************************************************************************/
void VS_ReceiveFrameQueueNhp::PushAudioPacket(unsigned char *pPacket, int size, VS_HeaderPacket *pHeader, unsigned int ctime)
{
	unsigned int uTimestamp = pHeader->aHeader.TimeStamp;
	DTRACE(VSTM_NHP_AUDIO, "receive a packet : t = %u, a (%d, %d), l = %d, ct = %u", uTimestamp, pHeader->fHeader.resendNum, pHeader->fHeader.SeqId, size, ctime);
	bool bRepeated = AnalyseDePacketizerQueue(pPacket, size, &(pHeader->fHeader), NHPH_DT_AUDIO, uTimestamp, ctime);
}

/**
 **************************************************************************
 * \brief Push cmd or data packet to queue
 ****************************************************************************/
void VS_ReceiveFrameQueueNhp::PushOtherPacket(unsigned char *pPacket, int size, VS_HeaderPacket *pHeader, unsigned int ctime)
{
	unsigned char uType = pHeader->fHeader.dataType;
	DTRACE(VSTM_NHP_OTHER, "receive %s packet : %d, l = %d, ct = %u", (uType == NHPH_DT_CMND) ? "cmd\0" : "data\0", pHeader->fHeader.SeqId, size, ctime);
	bool bRepeated = AnalyseDePacketizerQueue(pPacket, size, &(pHeader->fHeader), uType, 0, ctime);
}

/**
 **************************************************************************
 * \brief Calculate jitter
 ****************************************************************************/
void VS_ReceiveFrameQueueNhp::CalculateJitter(unsigned int ctime, unsigned char type, unsigned int timestamp, int dId)
{
	VS_ConferenceStatRcv *pStat = &(m_ConfStat[type]);
	if (pStat->started == 1) pStat->loss_packets += dId;
	// calculate jitter RFC 3550
	if (timestamp != 0) {
		double dt, dr, J = 0;
		if (pStat->inc_jitter > 1) {
			dt = (double)ctime - (double)pStat->prev_rcvtime;
			dr = (double)timestamp - (double)pStat->prev_timestamp;
			J = dt - dr;
			J = (15.0 * pStat->prev_jitter) / 16.0 + fabs(J) / 16.0;
			pStat->sum_jitters += J;
		} else {
			if (pStat->inc_jitter > 0) {
				dt = (double)ctime - (double)pStat->prev_rcvtime;
				dr = (double)timestamp - (double)pStat->prev_timestamp;
				J = dt - dr;
			}
		}
		pStat->prev_jitter = J;
		pStat->prev_rcvtime = ctime;
		pStat->prev_timestamp = timestamp;
		pStat->inc_jitter++;
	}
	pStat->started = 1;
}

/**
 **************************************************************************
 * \brief Analyse loss by bitrate
 ****************************************************************************/
bool VS_ReceiveFrameQueueNhp::AnalyseBitrateLoss(unsigned int ctime, int num_loss, unsigned char type)
{
	/// identify loss by bitrate
	if (m_uStartStat == 0) m_uStartStat = ctime;
	int dt = ctime - m_uStartStat;
	bool bLossBtr = false;

	if ((dt > 10 * QBITRATE_TIME_INTERVAL) && num_loss > 0 && type == NHPH_DT_VIDEO) {
		bool res;
		double a0 = 0.0, a1 = 0.0;
		int predicted, d, avg_d = 0;
		int new_loss = num_loss;
		int lossBitrate = (int)((m_AvgStat[type].uAvgPacketSize * num_loss * 8.0) / 1000.0 + 0.5);
		m_LossStat.GetA(a0, a1);
		predicted = (int)(a1 * dt + a0 + 0.5);
		d = predicted - (int)m_iNumLoss;
		m_LossAvg.Snap(d);
		res = m_LossAvg.GetAverage(avg_d);
		if (res && (100 * lossBitrate > 5 * m_iRealBandwidth)) {
			d = abs(avg_d) - num_loss;
			if (d < -2) {
				new_loss = abs(avg_d) + 1;
				DTRACE(VSTM_NHP_STAT, "bitrate loss : avg = %2d, ndl = %d, dl = %d, lossBtr = %d, band = %d", avg_d, new_loss, num_loss, lossBitrate, m_iRealBandwidth);
				bLossBtr = true;
			}
		}
		m_iNumLoss += new_loss;
		m_iNumByBtrLoss += num_loss - new_loss;
		m_LossStat.Snap(dt, (double)m_iNumLoss);
	}

	return bLossBtr;
}

/**
 **************************************************************************
 * \brief Analyse resend queue
 ****************************************************************************/
int VS_ReceiveFrameQueueNhp::AnalyseResendQueue(unsigned short firstId, int num, unsigned char type, unsigned int ctime)
{
	int module_dbg[4] = {VSTM_NHP_OTHER, VSTM_NHP_AUDIO, VSTM_NHP_VIDEO, VSTM_NHP_OTHER};
	bool bRequest = true;
	int i = 0, num_loss = std::max(num, 0) + int(m_iPrevLossId[type] != -1);
	/// receive wait-delay packet
	if (m_iPrevLossId[type] == firstId) {
		m_iPrevLossId[type] = -1;
		num_loss--;
	}
	/// don,t request for large net lag
	if (!m_bCmdSend || !m_bRequestLag) {
		m_iPrevLossId[type] = -1;
		if (num_loss > 0) DTRACE(module_dbg[type], "skip request (large net lag, cmd not send) : lag = %d, cmd = %d", m_iNetLag, m_bCmdSend);
		bRequest = false;
	}
	/// delay for case = 1 packet loss
	if (m_iPrevLossId[type] != -1) {
		if (bRequest) RequestPacket(m_iPrevLossId[type], type);
		m_iPrevLossId[type] = -1;
	}
	if (num == 1 && (type == NHPH_DT_VIDEO || type == NHPH_DT_DATA)) {
		m_iPrevLossId[type] = firstId + 1;
		DTRACE(module_dbg[type], "wait-delay packet: id = %d, t = %d", firstId + 1, type);
		num_loss--;
		bRequest = false;
	}
	if (num > 1 && (type == NHPH_DT_AUDIO || type == NHPH_DT_DATA)) bRequest = false;
	m_bLossByBitrate = AnalyseBitrateLoss(ctime, num_loss, type);
	if (bRequest) {
		/// analyse request for loss by bitrate
		int lossBitrate = 0;
		if (m_bLossByBitrate) lossBitrate = (int)((m_AvgStat[type].uAvgPacketSize * num * 8.0) / 1000.0 + 0.5);
		if (lossBitrate * 100 > m_iRealBandwidth * 15) {
			DTRACE(module_dbg[type], "skip request (overflow bandwidth) : B = %d, L = %d", m_iRealBandwidth, lossBitrate);
			return num_loss;
		}
		/// request for resend
		for (i = firstId + 1; i <= firstId + num; i++) {
			RequestPacket(i, type);
		}
	}
	return num_loss;
}

/**
 **************************************************************************
 * \brief Calculate statistics
 ****************************************************************************/
void VS_ReceiveFrameQueueNhp::AnalyseStream(unsigned char type, unsigned int ctime, unsigned int timestamp, int dId, int size)
{
	int i = 0;
	VS_ReceiveAverageStat *pStat;

	if (m_TotalStat.lossByBitrate == 0) m_TotalStat.lossByBitrate = (int)m_bLossByBitrate;

	/// calc avg stat
	for (i = 0; i < 4; i++) {
		pStat = &(m_AvgStat[i]);
		if (!pStat->qTimestamp.empty()) {
			int dt = ctime - pStat->qTimestamp.front() - QBITRATE_TIME_INTERVAL;
			while (dt >= 0) {
				pStat->uRcvPackets--;
				pStat->uRcvBytes -= pStat->qRcvBytes.front();
				pStat->uLossPackets -= pStat->qLossPacket.front();
				pStat->qTimestamp.pop();
				pStat->qRcvBytes.pop();
				pStat->qLossPacket.pop();
				if (pStat->qTimestamp.empty()) break;
				dt = ctime - pStat->qTimestamp.front() - QBITRATE_TIME_INTERVAL;
			}
		}
	}
	pStat = &(m_AvgStat[type]);
	if (type == NHPH_DT_VIDEO || type == NHPH_DT_AUDIO) {
		CalculateJitter(ctime, type, timestamp, dId);
	}
	pStat->uRcvPackets++;
	pStat->uRcvBytes += size;
	pStat->uAvgPacketSize = (int)((double)pStat->uRcvBytes / (double)pStat->uRcvPackets + 1.0);
	pStat->uLossPackets += dId;
	pStat->qTimestamp.push(ctime);
	pStat->qRcvBytes.push(size);
	pStat->qLossPacket.push(dId);

	/// audio buffer stat
	if (type == NHPH_DT_AUDIO) {
		if (m_iTimeStatAudio == -1) m_iTimeStatAudio = timestamp;
		int dt = (int)(timestamp - (unsigned int)m_iTimeStatAudio);
		int d = 0;
		if (m_AudioStat.GetAverage(d)) {
			dt = (dt > 3 * d) ? d : dt;
		}
		m_AudioStat.Snap(dt);
		m_iTimeStatAudio = timestamp;
	}

	/// send stat
	if (m_uStartAnalyse == 0) m_uStartAnalyse = ctime + 1000; /// 1 sec first stat delay
	if (ctime - m_uStartAnalyse > QBITRATE_TIME_INTERVAL) {
		m_iRealBandwidth = (int)((m_AvgStat[NHPH_DT_VIDEO].uRcvBytes * 8.0) / 1000.0);
		m_BandAvg.Snap(m_iRealBandwidth);
		m_BandAvg.GetAverage(m_iRealBandwidth);
		m_LossStat.Snap(ctime - m_uStartStat, (double)m_iNumLoss);
		m_TotalStat.reserved = int(m_bRequest && m_bRequestLag);
		for (i = 0; i < 4; i++) {
			VS_NhpTypeStat *pNhpStat = &(m_TotalStat.typeStat[i]);
			pNhpStat->rcv_pkts = m_AvgStat[i].uRcvPackets;
			pNhpStat->rcv_bytes = m_AvgStat[i].uRcvBytes;
			if (i == NHPH_DT_VIDEO) {
				unsigned char lb = m_TotalStat.lossByBitrate;
				double pl = (m_AvgStat[i].uLossPackets * 100.0) / ((double)m_AvgStat[i].uRcvPackets + 1);
				if (m_TotalStat.lossByBitrate == 1) {
					pNhpStat->loss_pkts = (int)(double(m_iNumByBtrLoss * QBITRATE_TIME_INTERVAL) / double(ctime - m_uStartAnalyse));
				} else {
					if (pl > 33.0) m_TotalStat.lossByBitrate = 1;
					m_iNumByBtrLoss = m_AvgStat[i].uLossPackets;
					pNhpStat->loss_pkts = m_AvgStat[i].uLossPackets;
				}
				DTRACE(VSTM_NHP_STAT, "send stat (video) : lbtr = %d, lp = %d, rp = %d, rb = %d | %d, %3.1f",
						m_TotalStat.lossByBitrate, pNhpStat->loss_pkts, pNhpStat->rcv_pkts, pNhpStat->rcv_bytes, lb, pl);
			} else {
				pNhpStat->loss_pkts = m_AvgStat[i].uLossPackets;
			}
		}
		PushQuery(stream::Command::Type::Stat, &m_TotalStat);
		memset(&m_TotalStat, 0, sizeof(VS_NhpTotalStat));
		m_uStartAnalyse = ctime;
		m_iNumRcv += m_AvgStat[NHPH_DT_VIDEO].uRcvPackets;
		m_iNumByBtrAllLoss += m_iNumByBtrLoss;
		DTRACE(VSTM_NHP_STAT, "loss stat : btr = %4.2f, rnd = %4.2f, btr_curr = %4.2f, rcv = %I64d, btr = %d",
							  (m_iNumByBtrAllLoss * 100.0) / (m_iNumRcv + 1.0), (m_iNumLoss * 100.0) / (m_iNumRcv + 1.0), (m_iNumByBtrLoss * 100.0) / (m_iNumRcv + 1.0), m_iNumRcv, m_iRealBandwidth);
		m_iNumByBtrLoss = 0;
	}
}

/**
 **************************************************************************
 * \brief Adjust audio buffer if need
 ****************************************************************************/
bool VS_ReceiveFrameQueueNhp::AdjustBuffers(int dtime)
{
	bool bRes = true;

	DEPACKETIZER *pAudio = &(m_DePacketizer[NHPH_DT_AUDIO].dqPacket);

	if (dtime < 0 && !pAudio->empty()) {
		/// убираем из очереди лишние аудио пакеты
		dtime = abs(dtime);
		__int64 timestamp = pAudio->front().iTimestamp;
		if (m_iLastTimeAudio == -1) m_iLastTimeAudio = timestamp;
		__int64 ltime = m_iLastTimeAudio;
		while (timestamp - m_iLastTimeAudio <= dtime) {
			ltime = timestamp;
			pAudio->pop_front();
			DTRACE(VSTM_NHP_AUDIO, "skip audio packet for adjust : t = %I64d, cdb = %I64d, db = %d", timestamp, timestamp - m_iLastTimeAudio, dtime);
			if (pAudio->empty()) break;
			timestamp = pAudio->front().iTimestamp;
		}
		m_iLastTimeAudio = ltime;
	}

	return bRes;
}

/**
**************************************************************************
* \brief Extract H264 (part of) frame
****************************************************************************/
void VS_ReceiveFrameQueueNhp::ExtractRTPData(unsigned char *packet, int insize, unsigned char *outbuff, int &outsize)
{
	unsigned char *in = packet;
	unsigned char* const pout = outbuff;

	unsigned int size, ExtraBytes;
	unsigned char*	curPtr, *endPtr, *outPtr;
	unsigned char const uIDC = 1;
	NAL_Unit_Type const uUnitType = (NAL_Unit_Type)(*in&0x1f);
	in++; insize--; /// uUnitType already read from in

	// get current RBSP compressed size
	size = insize;
	ExtraBytes = 0;

	// Set Pointers
	endPtr = in + size - 1;	// Point at Last byte with data in it.
	curPtr = in;
	outPtr = pout;

	// Write Start Codes, and NAL Header byte
	if ((uUnitType >= NAL_UT_SEI) && (uUnitType <= NAL_UT_PD)) {
		*outPtr++ = 0;	// Write an Extra zero_byte
		ExtraBytes = 1;
	}

	*outPtr++ = 0;
	*outPtr++ = 0;
	*outPtr++ = 1;
	*outPtr++ = (uIDC << 5) | uUnitType;
	ExtraBytes += 4;

	while (curPtr < endPtr - 1) {	// Copy all but the last 2 bytes
		*outPtr++ = *curPtr;
		// Check for start code emulation
		if ((*curPtr++ == 0) && (*curPtr == 0) && (!(*(curPtr+1) & 0xfc))) {
			/// SMirnovK: Polycom alredy insert this byte
			//*outPtr++ = *curPtr++;
			//*outPtr++ = 0x03;	// Emulation Prevention Byte
			//ExtraBytes++;
		}
	}

	if (curPtr < endPtr) {
		*outPtr++ = *curPtr++;
	}
	// copy the last byte
	*outPtr = *curPtr;

	outsize = size + ExtraBytes;
}

/**
 **************************************************************************
 * \brief Get video frame from queue
 ****************************************************************************/
bool VS_ReceiveFrameQueueNhp::TryGetVideoFrame(unsigned char *buff, int &size, unsigned int ctime)
{
	size = 0;

	if (m_mapFrame.empty()) return false;

	DEPACKETIZER *pDePacket = &(m_DePacketizer[NHPH_DT_VIDEO].dqPacket);
	bool bRes = false, bGrab, bTimeOut, bFullFrame, bNextFrame, bWaitKey = false;
	unsigned int i, timestamp;
	unsigned char dId;
	__int64 dt_grab;

	while (!m_mapFrame.empty()) {
		VS_ReceiveFrameInfo *pFrame = &(m_mapFrame.begin()->second);
		timestamp = m_mapFrame.begin()->first;
		int tmstmp = 100;
		m_FpsAvg.GetAverage(tmstmp);
		dt_grab = (__int64)ctime - (pFrame->iRcvTime + 2 * (tmstmp + ((m_bRequestLag) ? m_iNetLag : m_iNetLagFake)) + STUFF_DELAY);
		dId = (unsigned char)(pFrame->uId - m_iGrabIdFrame);

		bTimeOut = dt_grab >= 0;
		bFullFrame = (pFrame->uState == 4);
		bNextFrame = (dId == 1 || dId == 0);

		if (m_eWaitKeyFrame == WAIT_FROM_BUFFER) {
			bWaitKey = true;
			if (pFrame->uType == NHPVH_FT_IFRAME) {
				bWaitKey = false;
				m_eWaitKeyFrame = WAIT_NOT;
				DTRACE(VSTM_NHP_VIDEO, "find full i-frame in buffer : %d", pFrame->uId);
			}
		}

		bGrab = !bWaitKey && ((bTimeOut && m_mapFrame.size() > 1) || (bFullFrame && bNextFrame));
		i = 0;

		if (bGrab) {
			VS_NhpVideoHeader vh;
			vh.FrameId = pFrame->uId;
			vh.FrameType = pFrame->uType;
			vh.TimeStamp = timestamp;

			if (m_uTimestampFrame == 0) m_uTimestampFrame = timestamp;
			tmstmp = timestamp - m_uTimestampFrame;
			tmstmp = std::max(0, std::min(tmstmp, 1000));
			m_uTimestampFrame = timestamp;
			m_FpsAvg.Snap(tmstmp);

			if (bFullFrame && (bNextFrame || (bTimeOut && pFrame->uId == NHPVH_FT_IFRAME))) {
				/// полный кадр
				memcpy(buff, &vh, sizeof(VS_NhpVideoHeader));
				size = sizeof(VS_NhpVideoHeader);
				while (i < pFrame->uNumPackets) {
					iterPacketInfo it = pDePacket->begin();
					if (it->byResending > 0 || it->iTimestamp != timestamp) {
						DTRACE(VSTM_NHP_VIDEO, "skip video packet : %d, %d, %u", it->uId, it->byResending, timestamp);
					} else {
						if (m_iTypeDecoding == 0 || m_iTypeDecoding == 2) {
							memcpy(buff + size, it->Buff.Buffer(), it->uSize);
						} else {
							int outsize = 0;
							ExtractRTPData((unsigned char*)it->Buff.Buffer(), it->uSize, buff + size, outsize);
							it->uSize = outsize;
						}
						size += it->uSize;
						i++;
					}
					pDePacket->pop_front();
				}
				m_iNumBadFrames = 0;
				bRes = true;
				DTRACE(VSTM_NHP_VIDEO, "get video, full frame : (%d, %d), l = %d, ct = %u, dt = %I64d", pFrame->uId, pFrame->uType, size, ctime, dt_grab);
				if ((m_iTypeDecoding == 0 || m_iTypeDecoding == 2) && (size != pFrame->uSize + sizeof(VS_NhpVideoHeader)))
					DTRACE(VSTM_NHP_VIDEO, "incorrect size video frame");
			} else if (bTimeOut) {
				/// вышел таймаут "отдачи" кадра
				if (m_iTypeDecoding == 1) {
					/// если в декодере есть защита от ошибок
					memcpy(buff, &vh, sizeof(VS_NhpVideoHeader));
					size = sizeof(VS_NhpVideoHeader);
					while (i < pFrame->uNumPackets) {
						iterPacketInfo it = pDePacket->begin();
						if (it->byResending > 0 || it->iTimestamp != timestamp) {
							DTRACE(VSTM_NHP_VIDEO, "skip video packet : %d, %d, %u", it->uId, it->byResending, timestamp);
						} else {
							if (m_iTypeDecoding == 2) {
								memcpy(buff + size, it->Buff.Buffer(), it->uSize);
							} else {
								int outsize = 0;
								ExtractRTPData((unsigned char*)it->Buff.Buffer(), it->uSize, buff + size, outsize);
								it->uSize = outsize;
							}
							size += it->uSize;
							i++;
						}
						pDePacket->pop_front();
					}
					bRes = true;
					DTRACE(VSTM_NHP_VIDEO, "get video, not full frame : (%d, %d), l = %d, bad (%d, %d), ct = %u, dt = %I64d",
											pFrame->uId, pFrame->uType, size, dId, m_iNumBadFrames, ctime, dt_grab);
					m_iNumBadFrames += dId;
				} else if (m_iTypeDecoding == 0/* || m_iNumBadFrames >= 10*/) {
					m_eWaitKeyFrame = WAIT_FROM_BUFFER;
					DTRACE(VSTM_NHP_VIDEO, "get video, wait key from buffer : bad = %d, dec = %d, ct = %u, dt = %I64d, (rt = %I64d, fps = %d, lag = %d)",
											m_iNumBadFrames, m_iTypeDecoding, ctime, dt_grab, pFrame->iRcvTime, tmstmp, (m_bRequestLag) ? m_iNetLag : m_iNetLagFake);
				} else if (m_iTypeDecoding == 2) {
					DTRACE(VSTM_NHP_VIDEO, "get video, skip not full frame : (%d, %d), l = %d, bad (%d, %d), ct = %u, dt = %I64d",
											pFrame->uId, pFrame->uType, size, dId, m_iNumBadFrames, ctime, dt_grab);
					RequestKeyFrame(ctime);
				}
			}
		}

		if (bWaitKey || bGrab) {
			DTRACE(VSTM_NHP_VIDEO, "get video, erase frame : (%d, %d), ct = %u, wait key = %d", pFrame->uId, pFrame->uType, ctime, bWaitKey);
			m_iGrabIdFrame = pFrame->uId;
			m_iLastTimeVideo = timestamp;
			if (!bRes) {
				while (!pDePacket->empty() && pDePacket->front().iTimestamp <= (__int64)timestamp) {
					pDePacket->pop_front();
				}
			}
			m_mapFrame.erase(timestamp);
		} else {
			return false;
		}

		if (bRes) break;
	}

	if (!bRes) {
		DTRACE(VSTM_NHP_VIDEO, "get video, not enough data : ct = %u", ctime);
		RequestKeyFrame(ctime);
		m_eWaitKeyFrame = WAIT_FROM_STREAM;
	}

	return bRes;
}

/**
 **************************************************************************
 * \brief Get audio frame from queue
 ****************************************************************************/
bool VS_ReceiveFrameQueueNhp::TryGetAudioFrame(unsigned char *buff, int &size, unsigned int ctime)
{
	bool bRes = false;
	size = 0;
	DEPACKETIZER *pAudio = &(m_DePacketizer[NHPH_DT_AUDIO].dqPacket);

	while (!pAudio->empty()) {
		int tmstmp = 100, avg = 0, dt = 0;
		m_AudioStat.GetAverage(avg);
		if (m_iDBuffer > 0) {
			m_FpsAvg.GetAverage(tmstmp);
			dt = 2 * (tmstmp + ((m_bRequestLag) ? m_iNetLag : m_iNetLagFake));
		}
		__int64 timestamp = pAudio->front().iTimestamp;
		__int64 grabtime = pAudio->front().iRcvTime + dt + STUFF_DELAY;
		__int64 dgrab = (__int64)ctime - grabtime;
		bool bTimeOut = (dgrab >= 0 || (pAudio->front().byResending == 0 && dgrab >= -avg));

		if (!bTimeOut) {
			DTRACE(VSTM_NHP_AUDIO, "get audio, not timeout : %d, l = %d, ct = %u, dt = %I64d, avg = %d",
									pAudio->front().uId, size, ctime, dgrab, avg);
			break;
		}
		m_iDBuffer = 0;
		if (pAudio->front().byResending == 0) {
			size = pAudio->front().uSize;
			memcpy(buff, pAudio->front().Buff.Buffer(), size);
			bRes = true;
			DTRACE(VSTM_NHP_AUDIO, "get audio, full packet : %d, l = %d, ct = %u, dt = %I64d, avg = %d, qs = %d",
									pAudio->front().uId, size, ctime, dgrab, avg, pAudio->size());
		} else {
			DTRACE(VSTM_NHP_AUDIO, "get audio, empty packet : %d, ct = %u, dt = %I64d, avg = %d",
									pAudio->front().uId, size, dgrab, avg);
		}
		pAudio->pop_front();
		m_iLastTimeAudio = timestamp;

		if (bRes) break;
	}

	return bRes;
}

/**
 **************************************************************************
 * \brief Get cmd or data frame from queue
 ****************************************************************************/
bool VS_ReceiveFrameQueueNhp::TryGetOtherFrame(unsigned char *buff, int &size, unsigned int ctime, unsigned char type)
{
	bool bRes = false;
	DEPACKETIZER *pDePacket = &(m_DePacketizer[type].dqPacket);

	if (type == NHPH_DT_DATA) {
		while (!pDePacket->empty()) {
			int tmstmp = 100;
			m_FpsAvg.GetAverage(tmstmp);
			__int64 dgrab = (__int64)ctime - (pDePacket->front().iRcvTime + 2 * (tmstmp + ((m_bRequestLag) ? m_iNetLag : m_iNetLagFake)) + STUFF_DELAY);
			bool bTimeOut = dgrab >= 0;

			if (!bTimeOut) {
				DTRACE(VSTM_NHP_OTHER, "get data, not timeout : %d, l = %d, ct = %u, dt = %I64d",
										pDePacket->front().uId, size, ctime, dgrab);
				break;
			}
			if (pDePacket->front().byResending == 0) {
				size = pDePacket->front().uSize;
				memcpy(buff, pDePacket->front().Buff.Buffer(), size);
				bRes = true;
				DTRACE(VSTM_NHP_OTHER, "get data, full packet : %d, l = %d, ct = %u, dt = %I64d, qs = %d",
										pDePacket->front().uId, size, ctime, dgrab, pDePacket->size());
			} else {
				DTRACE(VSTM_NHP_OTHER, "get data, empty packet : %d, ct = %u, dt = %I64d",
										pDePacket->front().uId, size, dgrab);
			}
			pDePacket->pop_front();

			if (bRes) break;
		}
	} else {
		if (!pDePacket->empty()) {
			size = pDePacket->front().uSize;
			memcpy(buff, pDePacket->front().Buff.Buffer(), size);
			DTRACE(VSTM_NHP_OTHER, "get %s packet : %d, l = %d", (type == NHPH_DT_CMND) ? "cmd\0" : "d\0", pDePacket->front().uId, size);
			pDePacket->pop_front();
			bRes = true;
		}
	}

	return bRes;
}

/**
 **************************************************************************
 * \brief Get frames from queue
 ****************************************************************************/
bool VS_ReceiveFrameQueueNhp::Get(unsigned char *buff, int &size, int &type)
{
	bool bRes = false;
	unsigned int ctime = timeGetTime();

	/// проверяем буферы с данными
	/// пробуем заграбить команду
	type = NHPH_DT_CMND;
	bRes = TryGetOtherFrame(buff, size, ctime, type);
	if (!bRes) {
		/// пробуем заграбить аудио
		type = NHPH_DT_AUDIO;
		bRes = TryGetAudioFrame(buff, size, ctime);
		if (!bRes) {
			/// пробуем заграбить видео
			type = NHPH_DT_VIDEO;
			bRes = TryGetVideoFrame(buff, size, ctime);
			if (!bRes) {
				/// пробуем заграбить данные
				type = NHPH_DT_DATA;
				bRes = TryGetOtherFrame(buff, size, ctime, type);
			}
		}
	}

	return bRes;
}

/**
 **************************************************************************
 * \brief Reset Buffers
 ****************************************************************************/
void VS_ReceiveFrameQueueNhp::ResetBuffer()
{
	DTRACE(VSTM_NHP_VIDEO, "Reset buffers : change media format");
	m_DePacketizer[NHPH_DT_VIDEO].dqPacket.clear();
	m_mapFrame.clear();
	m_eWaitKeyFrame = WAIT_FROM_STREAM;
	m_uTimestampFrame = 0;
	RequestKeyFrame(timeGetTime());
}

/**
 **************************************************************************
 * \brief Analyse incoming commands from stream
 ****************************************************************************/
bool VS_ReceiveFrameQueueNhp::SetReceivedCommand(stream::Command& cmd)
{
	if (!m_bCmdSend) return false;

	switch (cmd.type)
	{
	case stream::Command::Type::TimeDelay:
		if (cmd.sub_type == stream::Command::Request) {
			cmd.MakeReply();
			CommandToSend(cmd);
		} else if (cmd.sub_type == stream::Command::Reply) {
			unsigned int ctime = timeGetTime(),
						 id = *reinterpret_cast<uint32_t*>(cmd.data);
			unsigned int stime = m_listIdQlag[id%20];
			int dt = ctime - stime;

			if (cmd.type == stream::Command::Type::TimeDelay) {
				int lag = (m_bRequestLag) ? m_iNetLag : m_iNetLagFake;
				if (id == 0) {
					m_iNetLag = dt / 2;
				} else {
					m_iNetLag = (int)((m_iNetLag * 3.0 +  dt * 7.0 / 2.0) / 10.0);
				}
				m_iNetLag = ((m_iNetLag + 4) / 5) * 5;
				if (m_iNetLag < 10) m_iNetLag = 10;
				if (m_iNetLag > QKEY_TIME_INTERVAL) {
					m_iDBuffer = 2 * (m_iNetLagFake - lag);
					m_bRequestLag = false;
				} else {
					m_iDBuffer = 2 * (m_iNetLag - lag);
					m_bRequestLag = true;
				}
				AdjustBuffers(m_iDBuffer);
				DTRACE(VSTM_NHP_STAT, "net latency : %d, %d, %d, %d", lag, m_iNetLag, m_iDBuffer, m_bRequestLag);
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

/**
 **************************************************************************
 * \brief Push query in queue
 ****************************************************************************/
bool VS_ReceiveFrameQueueNhp::PushQuery(stream::Command::Type cmnd_type, void *query)
{
	if (!m_bCmdSend) return false;

	stream::Command cmnd;
	switch (cmnd_type)
	{
	case stream::Command::Type::TimeDelay:
		cmnd.TimeDelay(*(unsigned long*)query);
		break;
	case stream::Command::Type::RequestKeyFrame:
		cmnd.RequestKeyFrame();
		DTRACE(VSTM_NHP_STAT, "i-frame query!");
		break;
	case stream::Command::Type::Stat:
		cmnd.Stat(query, sizeof(VS_NhpTotalStat));
		break;
	case stream::Command::Type::RequestPacket:
		cmnd.RequestPacket(query, sizeof(VS_RequestPacket));
		DTRACE(VSTM_NHP_STAT, "request packet query : tp = %d, id = %d", ((VS_RequestPacket*)query)->type_data, ((VS_RequestPacket*)query)->id_packet);
		break;
	default:
		return false;
	}
	CommandToSend(cmnd);

	return true;
}

/**
 **************************************************************************
 * \brief Key-frame query generation
 ****************************************************************************/
void VS_ReceiveFrameQueueNhp::RequestKeyFrame(unsigned int ctime)
{
	int limit_key_request = std::max(QKEY_TIME_INTERVAL, m_iNetLag);
	int dt = (m_uLastTimeQKey == 0) ? limit_key_request + 1 : ctime - m_uLastTimeQKey;
	if (dt > limit_key_request) {
		DTRACE(VSTM_NHP_STAT, "Receiver NHP 'try i-frame query...' : ct = %u, dt = %d", ctime, dt);
		PushQuery(stream::Command::Type::RequestKeyFrame, NULL);
		m_uLastTimeQKey = ctime;
	}
}

/**
 **************************************************************************
 * \brief Time-delay packet query generation
 ****************************************************************************/
void VS_ReceiveFrameQueueNhp::RequestTimeDelayPacket(unsigned int ctime)
{
	int dt = (m_uLastTimeQLag == 0) ? QLAG_TIME_INTERVAL + 1 : ctime - m_uLastTimeQLag;
	if (dt > QLAG_TIME_INTERVAL) {
		PushQuery(stream::Command::Type::TimeDelay, &m_uIdQLag);
		m_uLastTimeQLag = ctime;
		m_listIdQlag[m_uIdQLag%20] = ctime;
		m_uIdQLag++;
	}
}

/**
 **************************************************************************
 * \brief Request packet query generation
 ****************************************************************************/
void VS_ReceiveFrameQueueNhp::RequestPacket(unsigned short id, unsigned char type)
{
	VS_RequestPacket p;
	p.id_packet = id;
	p.type_data = type;
	PushQuery(stream::Command::Type::RequestPacket, &p);
}

/**
 **************************************************************************
 * \brief Get Statistics Rcv
 ****************************************************************************/
void VS_ReceiveFrameQueueNhp::GetStatistics(int &jitter, int &loss)
{
	jitter = int((m_ConfStat[NHPH_DT_VIDEO].sum_jitters + m_ConfStat[NHPH_DT_AUDIO].sum_jitters) /
		         (double)(m_ConfStat[NHPH_DT_VIDEO].inc_jitter + m_ConfStat[NHPH_DT_AUDIO].inc_jitter + 1) + 0.5);
	loss = m_ConfStat[NHPH_DT_VIDEO].loss_packets + m_ConfStat[NHPH_DT_AUDIO].loss_packets;
}
