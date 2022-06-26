/**
 **************************************************************************
 * \file VS_StreamPacketManagment.cpp
 * (c) 2002-2006 Visicron Systems, Inc. All rights reserved.
 *									http://www.visicron.net/
 * \brief pack and unpack packets for meadia stream
 * \b Project Client
 * \author SMirnovK
 * \date 03.11.2006
 *
 * $Revision: 5 $
 *
 * $History: VS_StreamPacketManagment.cpp $
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 15.05.12   Time: 16:19
 * Updated in $/VSNA/VSClient
 * - enhancement UDP Multicast:
 * a) decrease key-frame interval
 * b) change decoder for udp: can decode after loss - only for vpx & h.264
 * c) vpx encoder without svc for udp
 * - can use any codec in groupconf
 * - SVC client capability is ON only for vpx
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
 * User: Sanufriev    Date: 11.12.09   Time: 19:23
 * Updated in $/VSNA/VSClient
 * - isIntercom defined for NHP classes (control queue, receiver); remove
 * stat/request send fo Intercom
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 20.12.06   Time: 11:54
 * Updated in $/VS/VSClient
 * - added Notifay for packets added in ControlBand Module
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 12.12.06   Time: 15:56
 * Updated in $/VS/VSClient
 * - new bitrate control module
 * - comand interfase changed
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 10.11.06   Time: 16:49
 * Updated in $/VS/VSClient
 * - added NHP moules implementation
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 9.11.06    Time: 15:48
 * Updated in $/VS/VSClient
 * - timestump for video frame
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 8.11.06    Time: 16:53
 * Created in $/VS/VSClient
 * - NHP headers added
 *
 ****************************************************************************/
#include "VS_StreamPacketManagment.h"
#include "VS_UDPStream.h"
#include "VS_TCPStream.h"
#include "streams/VS_SendFrameQueueTCP.h"
#include "streams/VS_SendFrameQueueNhp.h"

/**
 **************************************************************************
 ****************************************************************************/
VS_NhpBuffBase* VS_NhpBuffBase::Factory(bool useNhp, bool isIntercom, int iTypeDecoder)
{
	VS_NhpBuffBase* p = 0;
	if (useNhp)	p = new VS_ReceiveFrameQueueNhp(!isIntercom, iTypeDecoder, !isIntercom);
	else		p = new VS_NhpBuffBase;
	return p;
}

/**
 **************************************************************************
 ****************************************************************************/
VS_ControlBandBase * VS_ControlBandBase::Factory(bool useNhp, bool isIntercom)
{
	VS_ControlBandBase* p = 0;
	if (useNhp)	p = new VS_ControlQueueNhp(!isIntercom);
	else		p = new VS_ControlBandTCP;
	return p;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ControlBandBase::SetQueuePointers(VS_SendFrameQueueBase *queue, void* event)
{
	if (queue) m_sendFrameQueue = queue;
	if (event) m_sendEvent = event;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ControlBandBase::NotifyQueueHandler()
{
	SetEvent(m_sendEvent);
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ControlBandBase::GetStatistics(VS_ConferenceStatSnd *stat, bool is_reset)
{
	int i = 0;
	if (is_reset) {
		for (i = 0; i < 5; i++) m_conf_st.bytes_all[i] += m_conf_st.bytes_cur[i];
		memcpy(stat, &m_conf_st, sizeof(VS_ConferenceStatSnd));
		for (i = 0; i < 5; i++) m_conf_st.bytes_cur[i] = 0;
	} else {
		for (i = 0; i < 5; i++) {
			stat->bytes_all[i] = m_conf_st.bytes_all[i] + m_conf_st.bytes_cur[i];
			stat->bytes_cur[i] = m_conf_st.bytes_cur[i];
		}
		memcpy(stat, &m_conf_st, sizeof(VS_ConferenceStatSnd));
	}
}
