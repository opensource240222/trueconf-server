/**
 **************************************************************************
 * \file VS_UDPStream.h
 * (c) 2002-2006 Visicron Systems, Inc. All rights reserved.
 *									http://www.visicron.net/
 * \brief Contain structures and interfaces for Sender/Receiver Modules
 * \b Project Client
 * \author SMirnovK
 * \author AnufrievS
 * \date 09.11.2006
 *
 * $Revision: 12 $
 *
 * $History: VS_UDPStream.h $
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 24.07.12   Time: 13:07
 * Updated in $/VSNA/VSClient
 * - udp : request loss packets for data
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 16.07.12   Time: 19:59
 * Updated in $/VSNA/VSClient
 * - update SVC client to spatial
 * - update vpx libs to v1.1.0
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 16.02.12   Time: 17:00
 * Updated in $/VSNA/VSClient
 * - add SVC capability
 * - change MediaFormat structure
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
 * User: Sanufriev    Date: 14.03.11   Time: 14:27
 * Updated in $/VSNA/VSClient
 * - change VS_MediaFormat - were added dwFps
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 24.03.10   Time: 19:21
 * Updated in $/VSNA/VSClient
 * - were added calc statictics (bug 7127)
 * - were merged all calc statistics in sender and receiver
 * - were added jitter calculation in Nhp receiver
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 23.12.09   Time: 19:13
 * Updated in $/VSNA/VSClient
 * - fix first packet id != 0 (udp)
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
 * *****************  Version 19  *****************
 * User: Sanufriev    Date: 29.01.07   Time: 15:03
 * Updated in $/VS/VSClient
 * - change statistics output
 * - modified upper bitrate algorithm
 *
 * *****************  Version 18  *****************
 * User: Sanufriev    Date: 26.01.07   Time: 16:11
 * Updated in $/VS/VSClient
 * - support h.264 decoder type
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 22.01.07   Time: 18:13
 * Updated in $/VS/VSClient
 * - fixed data, update bitrate control
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 17.01.07   Time: 17:19
 * Updated in $/VS/VSClient
 * - were added data bitrate control
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 12.01.07   Time: 15:28
 * Updated in $/VS/VSClient
 *  - added frame buffer optimal forming
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 11.01.07   Time: 17:44
 * Updated in $/VS/VSClient
 * - added frame buffer optimal forming
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 28.12.06   Time: 10:04
 * Updated in $/VS/VSClient
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 22.12.06   Time: 11:28
 * Updated in $/VS/VSClient
 * - were change bitrate control
 * - were change buffer lenght update for video
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 20.12.06   Time: 11:54
 * Updated in $/VS/VSClient
 * - added Notifay for packets added in ControlBand Module
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 20.12.06   Time: 11:35
 * Updated in $/VS/VSClient
 * - added DEBUG defines
 * - were change interface PushQuery, remove AnalisysQuery  in receive
 * module
 * - change bitrate control
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 14.12.06   Time: 13:15
 * Updated in $/VS/VSClient
 * - changed GetVideoBandwidth
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 14.12.06   Time: 12:54
 * Updated in $/VS/VSClient
 * - changed command intraface for receiver
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 14.12.06   Time: 11:37
 * Updated in $/VS/VSClient
 * - change processing commands in stream
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 13.12.06   Time: 18:33
 * Updated in $/VS/VSClient
 * - comands in NHP intrfaces
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 12.12.06   Time: 15:56
 * Updated in $/VS/VSClient
 * - new bitrate control module
 * - comand interfase changed
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 13.11.06   Time: 12:48
 * Updated in $/VS/VSClient
 * - change parameters type in functions in VS_ReceiveFrameQueueNhp
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 13.11.06   Time: 12:22
 * Updated in $/VS/VSClient
 * - warnings removed
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 13.11.06   Time: 11:43
 * Updated in $/VS/VSClient
 * - replace pointer to reference in function GetFrame()
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 10.11.06   Time: 16:49
 * Created in $/VS/VSClient
 * - added NHP moules implementation
 *
 ****************************************************************************/

#ifndef  __VS_UDPSTREAM_H__
#define __VS_UDPSTREAM_H__

#include "VS_NhpHeaders.h"
#include "VSAudioUtil.h"
#include "VS_StreamPacketManagment.h"
#include "../streams/Command.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/VS_TimeDeviation.h"

//#include <windows.h>
#include <time.h>
#include <math.h>
#include <queue>
#include <list>
#include <map>
#include <set>

#pragma pack (push, 1)

/**
 **************************************************************************
 * \brief Data for request packet
 ****************************************************************************/
struct VS_RequestPacket
{
	unsigned short id_packet;
	unsigned char  type_data;
};

/**
**************************************************************************
* \brief Used to store sent packet info
****************************************************************************/
struct VS_SendPacketCharacterNhp
{
	unsigned long	time;				///< send time
	unsigned short	size;				///< size of send packet
	unsigned short	track;				///< track of media
	unsigned short	stime;				///< sending time
	unsigned short	stimeonly;			///< flag signaled when packet has not been send
};

#pragma pack (pop)

/**
 **************************************************************************
 * \brief Make desision about current video Bandwidth
 ****************************************************************************/
class VS_ControlQueueNhp: public VS_ControlBandBase
{
	std::deque<VS_SendPacketCharacterNhp> m_list;
	int								 m_last_bitrate, m_bandwidth, m_last_rcv_pkts, m_over_bandwidth, m_rcv_databytes, m_start_bitrate, m_avg_bitrate_rcv, m_num_stat;
	int								 m_upcoef0, m_upcoef1, m_upper_bandwidth, m_over_upcoef1;
	double							 m_reducecoef;
	unsigned int					 m_time_update_stat, m_sttime_update_stat;
	bool							 m_is_loss, m_is_dataloss, m_is_cmd_send, m_is_overflow;
	VS_NhpTotalStat					 m_total_stat;
	int								 m_LimitSize;
	int								 m_last_analyse;
	double							 m_CurrBitrate[5];
	VS_TimeDeviation<int>			 m_BandAvg, m_LoadAvg;
	void							 UpdateBandwidth();
	void							 UpdateState(int updelay, int c0, int c1, int last_bitrate, int max_bitrate, unsigned int ctime);
	void							 AnalyseStat(unsigned int ctime);
public:
	VS_ControlQueueNhp(bool is_cmd_send = true);
	int GetVideoBandwidth(int MaxBand);
	bool SetReceivedCommand(stream::Command& cmd);
	bool IsDataAllowed(int MaxBand);
	void Add(unsigned long time, unsigned short size, unsigned short track, unsigned short stime, unsigned short stimeonly, unsigned char slayer);
	int GetCurrBand(BandType type);
};

/**
 **************************************************************************
 * \brief Coolect NHP packets in frames of data, monitor loss in network
 ****************************************************************************/
struct VS_HeaderPacket
{
	VS_NhpFirstHeader fHeader;
	VS_NhpVideoHeader vHeader;
	VS_NhpAudioHeader aHeader;
};

struct VS_ReceivePacketInfo
{
	int				iState;
	VS_BinBuff		Buff;
	unsigned int	uSize;
	unsigned char	uType;
	unsigned short	uId;
	unsigned char	byResending;
	__int64			iRcvTime;	/// for audio
	__int64			iTimestamp; /// for audio
	void SetPacket(unsigned char *pBuff, unsigned int size, unsigned char type, unsigned short id, unsigned char resend,
					__int64 ctime = -1, __int64 timestamp = -1)
	{
		Buff.Set(pBuff, size);
		uSize = size;
		uType = type;
		uId = id;
		byResending = resend;
		iRcvTime = ctime;
		iTimestamp = timestamp;
		iState = 0;
	};
};

struct VS_ReceiveFrameInfo
{
	unsigned char		uType;
	unsigned char		uId;
	unsigned int		uState;
	unsigned int		uSize;
	unsigned short		uFirstId;
	unsigned short		uLastId;
	unsigned int		uNumPackets;
	__int64				iRcvTime;
};

struct VS_ReceiveAverageStat
{
	std::queue<unsigned int> qTimestamp;
	std::queue<unsigned int> qRcvBytes;
	std::queue<unsigned int> qRcvFPS;
	std::queue<unsigned int> qLossPacket;
	unsigned int uRcvBytes;
	unsigned int uRcvPackets;
	unsigned int uLossPackets;
	unsigned int uAvgPacketSize;
	unsigned int uAvgFPS;
};

#define DEPACKETIZER std::deque<VS_ReceivePacketInfo>

struct VS_DePacketizer
{
	int				iLastId;
	DEPACKETIZER	dqPacket;
};

struct CompareFrameInfo
{
  bool operator() (const unsigned int& key0, const unsigned int& key1) const
  {
	  return key0 < key1;
  }
};

typedef DEPACKETIZER::iterator													iterPacketInfo;
typedef std::pair<unsigned int, VS_ReceiveFrameInfo>							pairFrameInfo;
typedef std::map<unsigned int, VS_ReceiveFrameInfo, CompareFrameInfo>::iterator	itFrameInfo;
typedef std::pair<itFrameInfo, bool>											pairFrameInsert;

class VS_ReceiveFrameQueueNhp: public VS_NhpBuffBase
{
private:

	enum NAL_Unit_Type {
		NAL_UT_RESERVED	 = 0x00, // Reserved
		NAL_UT_SLICE	 = 0x01, // Coded Slice - slice_layer_no_partioning_rbsp
		NAL_UT_DPA		 = 0x02, // Coded Data partition A - dpa_layer_rbsp
		NAL_UT_DPB		 = 0x03, // Coded Data partition A - dpa_layer_rbsp
		NAL_UT_DPC		 = 0x04, // Coded Data partition A - dpa_layer_rbsp
		NAL_UT_IDR_SLICE = 0x05, // Coded Slice of a IDR Picture - slice_layer_no_partioning_rbsp
		NAL_UT_SEI		 = 0x06, // Supplemental Enhancement Information - sei_rbsp
		NAL_UT_SPS		 = 0x07, // Sequence Parameter Set - seq_parameter_set_rbsp
		NAL_UT_PPS		 = 0x08, // Picture Parameter Set - pic_parameter_set_rbsp
		NAL_UT_PD		 = 0x09, // Picture Delimiter - pic_delimiter_rbsp
		NAL_UT_FD		 = 0x0a  // Filler Data - filler_data_rbsp
	};

	enum eStateKey {
		WAIT_NOT		 = 0,
		WAIT_FROM_BUFFER = 1,
		WAIT_FROM_STREAM = 2
	};

	eStateKey m_eWaitKeyFrame;
	int m_iGrabIdFrame;
	int	m_iTypeDecoding;
	bool m_bCmdSend, m_bRequest, m_bRequestLag;
	int	m_iNetLag, m_iNetLagFake;
	int	m_iDBuffer;
	int	m_iRealBandwidth;
	int m_iNumBadFrames;
	bool m_bLossByBitrate;
	unsigned int m_uLastTimeQKey;
	unsigned int m_uLastTimeQLag, m_uIdQLag;
	unsigned int m_uTimestampFrame;
	__int64 m_iLastTimeAudio, m_iLastTimeVideo;
	__int64 m_iTimeStatAudio;
	unsigned int m_listIdQlag[20];

	/// statistics
	unsigned int					m_uStartStat;
	unsigned int					m_uStartAnalyse;
	VS_NhpTotalStat					m_TotalStat;
	VS_ConferenceStatRcv			m_ConfStat[4];
	VS_ReceiveAverageStat			m_AvgStat[4];
	VS_TimeDeviation<int>			m_AudioStat;
	VS_TimeDeviation<int>			m_BandAvg;
	VS_TimeDeviation<int>			m_FpsAvg;
	VS_TimeDeviation<int>			m_LossAvg;
	VS_FreqDeviation				m_LossStat;
	__int64							m_iNumLoss, m_iNumRcv, m_iNumByBtrLoss, m_iNumByBtrAllLoss;
	int								m_iPrevLossId[4];

	/// containers
	VS_ReceivePacketInfo										m_ReqPacket;
	VS_DePacketizer												m_DePacketizer[4];
	std::map<unsigned int, VS_ReceiveFrameInfo, CompareFrameInfo>	m_mapFrame;

protected:
	void					ExtractRTPData(unsigned char *packet, int insize, unsigned char *outbuff, int &outsize);
	int						DecodeHeader(unsigned char *pBuff, VS_HeaderPacket *pHeader);
	/// push packets
	void					PushVideoPacket(unsigned char *pPacket, int size, VS_HeaderPacket *pHeader, unsigned int ctime);
	void					PushAudioPacket(unsigned char *pPacket, int size, VS_HeaderPacket *pHeader, unsigned int ctime);
	void					PushOtherPacket(unsigned char *pPacket, int size, VS_HeaderPacket *pHeader, unsigned int ctime);
	void					(VS_ReceiveFrameQueueNhp::*PushPacket)(unsigned char *pPacket, int size, VS_HeaderPacket *pHeader, unsigned int ctime);
	/// get frames
	bool					TryGetVideoFrame(unsigned char *buff, int &size, unsigned int ctime);
	bool					TryGetAudioFrame(unsigned char *buff, int &size, unsigned int ctime);
	bool					TryGetOtherFrame(unsigned char *buff, int &size, unsigned int ctime, unsigned char type);
	bool					AdjustBuffers(int dtime);
	/// request
	bool					PushQuery(stream::Command::Type cmnd_type, void *query);
	void					RequestKeyFrame(unsigned int ctime);
	void					RequestTimeDelayPacket(unsigned int ctime);
	void					RequestPacket(unsigned short id, unsigned char type);
	/// analyse
	int						AnalyseResendQueue(unsigned short firstId, int num, unsigned char type, unsigned int ctime);
	void					CalculateJitter(unsigned int ctime, unsigned char type, unsigned int timestamp, int dId);
	void					AnalyseStream(unsigned char type, unsigned int ctime, unsigned int timestamp, int dId, int size);
	bool					AnalyseBitrateLoss(unsigned int ctime, int num_loss, unsigned char type);
	bool					AnalyseDePacketizerQueue(unsigned char *pPacket, int size, VS_NhpFirstHeader *pHeader, unsigned char type, unsigned int timestamp, unsigned int cur_time);
public:
	VS_ReceiveFrameQueueNhp(bool is_request = true, int type_decoding = 0, bool is_cmd_send = true);
	~VS_ReceiveFrameQueueNhp();
	bool					Add(unsigned char* buff, int size);
	bool					Get(unsigned char *buff, int &size, int &type);
	bool					SetReceivedCommand(stream::Command& cmd);
	void					GetStatistics(int &jitter, int &loss);
	void					ResetBuffer();
};

#endif /*__VS_UDPSTREAM_H__*/