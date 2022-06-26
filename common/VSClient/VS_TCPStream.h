/**
 **************************************************************************
 * \file VS_TCPStream.h
 * (c) 2002-2006 Visicron Systems, Inc. All rights reserved.
 *									http://www.visicron.net/
 * \brief Contain structures and interfaces for TCP stream modules
 * \b Project Client
 * \author SMirnovK
 * \date 08.12.2006
 *
 * $Revision: 5 $
 *
 * $History: VS_TCPStream.h $
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
 * User: Sanufriev    Date: 14.03.11   Time: 14:27
 * Updated in $/VSNA/VSClient
 * - change VS_MediaFormat - were added dwFps
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
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 12.12.06   Time: 15:56
 * Created in $/VS/VSClient
 * - new bitrate control module
 * - comand interfase changed
 *
 ****************************************************************************/
#ifndef _VS_TCP_STREAM_H_
#define _VS_TCP_STREAM_H_

#include "../std/cpplib/VS_Map.h"
#include "std-generic/cpplib/VS_Container.h"
#include <windows.h>
#include <deque>

/**
**************************************************************************
* \brief Used to store sent packet info
****************************************************************************/
struct VS_SendPacketCharacter
{
	DWORD	time;				///< send time
	USHORT	size;				///< size of send packet
	USHORT	track;				///< track of media
	USHORT	stime;				///< sending time
	USHORT	stimeonly;			///< flag signaled when packet has not been send
};

/**
**************************************************************************
* \brief Used to store statistic about one type of media
****************************************************************************/
struct VS_TrackInfo
{
	DWORD	lastExceedTime;		///< last time when broker buffs was overrided
	DWORD	prevBrokerBuffs;	///< prev bufers from broker
	DWORD	currBrokerBuffs;	///< current bufers from broker
	DWORD	BrokerBand;			///< calculated broker band
	DWORD	RecomendedBitrate;	///< last queried bitrate
	DWORD	MaxBitrate;			///< bitrate restriction
	float	BrokerLoad;			///< calculated broker load
	float	CurrBitrate;		///< sent bitrate
	float	CurrPktsSpeed;		///< number of sent packets per second
	float	CurrPktsTime;		///< sent time per packet
	DWORD	CurrQueueLen;
	DWORD	CurrQueueBytes;
};

/**
**************************************************************************
* \brief Analise Broker Statisitic and clculate band for every media
****************************************************************************/

#define MAX_WND_SIZE				(6)
#define MAX_TICKS_SIZE				(5)
#define UNDEF_BANDWIDTH				(unsigned short)(-1)
#define QBYTES_MIN					(4096) /// 32 kbps
#define BANDWIDTH_MIN				(10)
#define STAT_BITRATEFREEZE_TM		(6000)
#define STAT_BITRATEFREEZE_TM		(6000)
#define UPDATE_BANDWIDTH_PERCENT 	(50)
#define FREEZE_BITRATE_PERCENT 		(10)
#define DETECT_ERASE_PERCENT		(40)
#define STAT_BITRATEUP_TM			(12000)
#define DETECT_COEF_LIMIT			(2)
#define DETECT_BANDWIDTH_REDUCE_TM 	(2000)

enum eBitrateState
{
	PART_UNDEF			= 0x00000000,
	PART_IDLE			= 0x00000001,
	PART_RESTRICT		= 0x00000002,
	PART_FIRSTUPPER		= 0x00000004,
	PART_SECONDUPPER	= 0x00000008,
	PART_THIRDUPPER		= 0x00000010,
	PART_FREEZE			= 0x00000020,
};

class VS_ControlBandTCP: public VS_ControlBandBase
{
	std::deque<VS_SendPacketCharacter>	m_list;					///< store sent packets
	VS_TrackInfo						m_Tracks[5];			///< store calculated tracks statistic
	DWORD								m_prevBrokerTime;		///< last time when broker stat arrived
	DWORD								m_prevRecalculateTime;	///< last time when stat calculated
	DWORD								m_prevStatTime;
	DWORD								m_prevAddPacket;
	DWORD								m_prevCalcBandwidth;
	__int64								m_overflowTime;
	__int64								m_phBandwidthChangeTime;
	__int64								m_phBandwidthRestrictTime;
	int									m_numIterates;
	__int64								m_numTicks;
	int									m_coefLoad;
	int									m_instantQueueLen[MAX_WND_SIZE], m_instantQueueBytes[MAX_WND_SIZE];
	double								m_sndBitrate[BT_ALL+1][MAX_TICKS_SIZE];
	double								m_avgSndBitrate[BT_ALL+1];
	unsigned short						m_phSndBitrate, m_calcSndBitrate, m_lowPhBandwidth, m_highPhBandwidth;
	unsigned int						m_eState;
	int									m_coefBitrate;
	void Calculate(DWORD currTime);
	void Clear();
	DWORD GetBitrate(int type);
	DWORD AnaliseBrokerStat(DWORD currTime);
	int CalculatePhBandwidth(unsigned int currTime);
	int CalculateSendBitrate(unsigned int currTime, int maxBand, int coefLoad);
public:
	VS_ControlBandTCP() {Clear();}
	void Add(DWORD time, USHORT	size, USHORT track, USHORT stime, USHORT stimeonly, UCHAR slayer);
	bool IsDataAllowed(int MaxBand);
	int GetVideoBandwidth(int MaxBand);
	int GetCurrBand(BandType type);
	bool SetReceivedCommand(stream::Command& cmd);
	int GetLoadQueue();
};

#endif /*_VS_TCP_STREAM_H_*/