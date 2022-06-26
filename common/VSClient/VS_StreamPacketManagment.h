/**
 **************************************************************************
 * \file VS_StreamPacketManagment.h
 * (c) 2002-2006 Visicron Systems, Inc. All rights reserved.
 *									http://www.visicron.net/
 * \brief Contain interfaces to use with outgoing and incoming packets from
 *		  meadia stream
 * \b Project Client
 * \author SMirnovK
 * \author AnufrievS
 * \date 03.11.2006
 *
 * $Revision: 13 $
 *
 * $History: VS_StreamPacketManagment.h $
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 16.07.12   Time: 19:59
 * Updated in $/VSNA/VSClient
 * - update SVC client to spatial
 * - update vpx libs to v1.1.0
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 15.05.12   Time: 16:19
 * Updated in $/VSNA/VSClient
 * - enhancement UDP Multicast:
 * a) decrease key-frame interval
 * b) change decoder for udp: can decode after loss - only for vpx & h.264
 * c) vpx encoder without svc for udp
 * - can use any codec in groupconf
 * - SVC client capability is ON only for vpx
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 16.02.12   Time: 17:00
 * Updated in $/VSNA/VSClient
 * - add SVC capability
 * - change MediaFormat structure
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 22.12.11   Time: 16:00
 * Updated in $/VSNA/VSClient
 * - svc routing
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
 * User: Sanufriev    Date: 14.12.09   Time: 12:54
 * Updated in $/VSNA/VSClient
 * - were added new send packet algorithm
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 11.12.09   Time: 19:23
 * Updated in $/VSNA/VSClient
 * - isIntercom defined for NHP classes (control queue, receiver); remove
 * stat/request send fo Intercom
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 20.07.09   Time: 13:54
 * Updated in $/VSNA/VSClient
 * - change priority video and data
 * - change nhp bitrate scheme  for data
 * - status bar bitrate include data
 * - fix memory allocation for nhp
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
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 20.12.06   Time: 11:54
 * Updated in $/VS/VSClient
 * - added Notifay for packets added in ControlBand Module
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 12.12.06   Time: 15:56
 * Updated in $/VS/VSClient
 * - new bitrate control module
 * - comand interfase changed
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 8.11.06    Time: 16:53
 * Created in $/VS/VSClient
 * - NHP headers added
 *
 ****************************************************************************/
#ifndef _VS_STREAM_PACKET_MANAGMENT_H_
#define _VS_STREAM_PACKET_MANAGMENT_H_

#include "VS_MiscCommandProc.h"
#include "VSClientBase.h"
#include "boost/shared_ptr.hpp"
#include "streams/VS_SendFrameQueueBase.h"

/// Conference statistic sender
struct VS_ConferenceStatSnd {
	int	bytes_cur[5];
	int	bytes_all[5];
};

/// Conference statistic receiver Nhp
struct VS_ConferenceStatRcv {
	int				started;
	int				loss_packets;
	unsigned int	prev_timestamp;
	unsigned int	prev_rcvtime;
	unsigned int	inc_jitter;
	double			prev_jitter;
	double			sum_jitters;
};

/**
 **************************************************************************
 * \brief Coolect NHP packets in frames of data, monitor loss in network
 ****************************************************************************/
class VS_NhpBuffBase: public VS_MiscCommandProcess
{
public:
	/// Retrive TCP or NHP interfase
	static VS_NhpBuffBase* Factory(bool useNhp, bool isIntercom, int iTypeDecoder);
	/// do nothing
	virtual ~VS_NhpBuffBase() {}
	/// Parsing incoming stream packets
	virtual bool Add(unsigned char* buff, int size) {return false;}
	/// Copy formed from pakects data to buffer
	virtual bool Get(unsigned char* buff, int &size, int &type) {return false;}
	/// Get receiver statistic
	virtual void GetStatistics(int &jitter, int &loss) {};
	/// Reset buffers
	virtual void ResetBuffer() {};
};


/**
 **************************************************************************
 * \brief Make desision about current video Bandwidth
 ****************************************************************************/
class VS_ControlBandBase: public VS_MiscCommandProcess
{
protected:
	VS_SendFrameQueueBase*	m_sendFrameQueue;
	void*					m_sendEvent;
	VS_ConferenceStatSnd	m_conf_st;
	bool					m_direct_connection;
	bool					m_hw_encoder;
public:
	enum BandType
	{
		BT_CMD,
		BT_AUDIO,
		BT_VIDEO,
		BT_DATA,
		BT_ALL
	};
	/// Retrive TCP or NHP interfase
	static VS_ControlBandBase* Factory(bool useNhp, bool isIntercom);
	/// zero vars
	VS_ControlBandBase() : m_sendFrameQueue(0), m_sendEvent(0), m_direct_connection(true), m_hw_encoder(false) {}
	/// do nothing
	virtual ~VS_ControlBandBase() {}
	/// Store internals
	void SetQueuePointers(VS_SendFrameQueueBase *queue, void* event);
	/// Set connection type
	void SetTypeConnection(bool bDirect) { m_direct_connection = bDirect; }
	/// Notifay queue handler
	void NotifyQueueHandler();
	/// Calculate need video bandwidth
	virtual int GetVideoBandwidth(int MaxBand) = 0;
	/// Deside put data into the queue or not
	virtual bool IsDataAllowed(int MaxBand = 2000) {return true;}
	/// add statistic for send packet
	virtual void Add(unsigned long time, unsigned short size, unsigned short track, unsigned short stime, unsigned short stimeonly, unsigned char slayer){}
	/// return current band
	virtual int GetCurrBand(BandType type) {return 0;}
	/// Get statisrics
	virtual void GetStatistics(VS_ConferenceStatSnd *stat, bool is_reset);
	/// Set type encoder
	virtual void SetHardwareEncoder(bool useHW) { m_hw_encoder = useHW; }
	/// Get load factor
	virtual int GetLoadQueue() { return 0; }
};

class VS_StreamClientSender;
class VS_StreamCrypter;

/**
 **************************************************************************
 * \brief
 ****************************************************************************/
class VS_ControlPacketQueue: public CVSThread
{
private:
	VS_SendFrameQueueBase*	m_sendFrameQueue;
	boost::shared_ptr<VS_StreamClientSender>	m_streamSender;
	VS_StreamCrypter*		m_streamCrypter;
	int						m_period_time, m_start_t;
	double					m_calc_t, m_dt_wait;
	double					m_bound_bytes_ms;
	bool					m_is_init;
	unsigned char*			m_tmp_frame;
	DWORD					Loop(LPVOID lpParameter);
	int	 ProcessSendQueueThread(int cur_t);
public:
	VS_ControlPacketQueue();
	~VS_ControlPacketQueue();
	bool Init(VS_SendFrameQueueBase* pFrameQueue, const boost::shared_ptr<VS_StreamClientSender>& pStreamSnd, VS_StreamCrypter* pStreamCrypt, int B, int T);
	void Release();
	int	 ProcessSendQueue(int cur_t, unsigned char* &pBuffer, unsigned char &track, unsigned char &slayer, int &size, bool is_nhp);
};

#endif /*_VS_STREAM_PACKET_MANAGMENT_H_*/