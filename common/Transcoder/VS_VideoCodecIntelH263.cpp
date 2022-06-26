#ifdef _WIN32
/****************************************************************************
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
*
* Project: TransCoder
*
* $History: VS_VideoCodecIntelH263.cpp $
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 15.02.12   Time: 18:46
 * Updated in $/VSNA/Transcoder
 * - fix lock for SetBitrate & Compress in VideoCodec
 * - update to vpx v1.0.0
 * - add SVC case for vpx
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Updated in $/VSNA/Transcoder
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 10.10.07   Time: 17:54
 * Updated in $/VS2005/Transcoder
 * - fixed XCCodec, VideoCodec (BITMAPINFO)
 * - add XCCodec wrap
 * - change VS_RetriveVideoCodec function
 * - add support h.264 codecs
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Transcoder
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 15.12.05   Time: 14:16
 * Updated in $/VS/Transcoder
 * - h264 deblocking on
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 29.11.05   Time: 19:36
 * Updated in $/VS/Transcoder
 * - set codec state for H264
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 25.11.05   Time: 18:26
 * Updated in $/VS/Transcoder
 * - I and P frame interval
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 24.11.05   Time: 18:00
 * Updated in $/VS/Transcoder
 * - h264 codec ver 1.0.1.2
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 16.11.05   Time: 19:04
 * Updated in $/VS/Transcoder
 * - setstate return value corrected
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 16.11.05   Time: 17:49
 * Updated in $/VS/Transcoder
 * - set bitrate func for h264 codec
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 27.09.05   Time: 15:16
 * Updated in $/VS/Transcoder
 * - now codecs for h323 side can be changed dinamicaly
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 13.08.05   Time: 15:32
 * Updated in $/VS/Transcoder
 * - buf fix whith H263 codec
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 20.09.04   Time: 19:02
 * Updated in $/VS/Transcoder
 * added user application type support
 * added SetOption in vcoder
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 25.08.04   Time: 19:40
 * Updated in $/VS/Transcoder
 * Added Stat in transcoder
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 23.08.04   Time: 18:22
 * Updated in $/VS/Transcoder
 * option flag for Polycam
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 21.07.04   Time: 17:16
 * Updated in $/VS/Transcoder
 * now Intell set to split frames
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 28.06.04   Time: 17:46
 * Updated in $/VS/Transcoder
 * addded time support for video
 * added set-get media format support
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 12.05.04   Time: 19:07
 * Created in $/VS/Transcoder
 * added form of RTP header
*
****************************************************************************/

/****************************************************************************
* \file VS_VideoCodecIntelH263.h
* \brief some Intel H263 specific func
****************************************************************************/

#include "VideoCodec.h"
#include "VS_VideoCodecIntelH263.h"

struct VS_IntelH263State
{
	uint32_t	fcc;
	uint32_t	sharpness;
	uint32_t	bitrate;
	uint32_t	option_flags;
	uint32_t	width;
	uint32_t	height;
	uint32_t	postfilter_strength;
};

bool VS_VideoCoderIntelH263::UpdateBitrate()
{
	if (m_bitrate_prev == m_bitrate) return true;
	m_bitrate_prev = m_bitrate;
	char buff[256];
	if (ICGetState(m_hic, buff, 256)>=0) {
		VS_IntelH263State* state = (VS_IntelH263State*)buff;
		state->bitrate = m_bitrate_prev*1000;
		if (ICSetState(m_hic, buff, 256) == ICERR_OK)
			return true;
	}
	return false;
}

inline VS_VideoCoderIntelH263::VS_VideoCoderIntelH263() : VS_VfwVideoCodec('362I', true)
{
}

inline VS_VideoCoderIntelH263::~VS_VideoCoderIntelH263()
{
}

bool VS_VideoCoderIntelH263::SetCoderOption(uint32_t param)
{
	if (!IsCoder()) return false;
	char buff[256];
	if (ICGetState(m_hic, buff, 256)>=0) {
		VS_IntelH263State* state = (VS_IntelH263State*)buff;
		state->option_flags = param;
		if (ICSetState(m_hic, buff, 256) == ICERR_OK)
			return true;
	}
	return false;
}
#endif
