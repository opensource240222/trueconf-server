/**
 **************************************************************************
 * \file VS_RTP_Buffers.cpp
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief Input and Output RTP buffers implementatin
 *
 * \b Project: TransCoder
 * \author SMirnovK
 * \date 14.05.2004
 *
 * $Revision: 2 $
 *
 * $History: VS_RTP_Buffers.cpp $
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Updated in $/VSNA/Transcoder
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 29.09.07   Time: 13:03
 * Updated in $/VS2005/Transcoder
 * - added h.263/h.264 bitsream parsing (get frame resolution from
 * bitstream)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Transcoder
 *
 * *****************  Version 35  *****************
 * User: Smirnov      Date: 28.11.06   Time: 16:44
 * Updated in $/VS/Transcoder
 * - more complex random generation functions
 *
 * *****************  Version 34  *****************
 * User: Smirnov      Date: 6.12.05    Time: 17:49
 * Updated in $/VS/Transcoder
 * - h264 IsKey Func corrected, parsing more accurate
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 2.12.05    Time: 19:59
 * Updated in $/VS/Transcoder
 * new Buffers and keyFrames
 *
 * *****************  Version 32  *****************
 * User: Smirnov      Date: 20.10.05   Time: 17:36
 * Updated in $/VS/Transcoder
 * - h261 output buffer corrected
 *
 * *****************  Version 31  *****************
 * User: Smirnov      Date: 19.10.05   Time: 19:36
 * Updated in $/VS/Transcoder
 * - h263+ input buffers now backward compatible with h263
 *
 * *****************  Version 30  *****************
 * User: Smirnov      Date: 19.10.05   Time: 17:48
 * Updated in $/VS/Transcoder
 * - h263++ otput buffer
 *
 * *****************  Version 29  *****************
 * User: Smirnov      Date: 12.10.05   Time: 18:08
 * Updated in $/VS/Transcoder
 * - Polycom h264 adaptation
 *
 * *****************  Version 28  *****************
 * User: Smirnov      Date: 10.10.05   Time: 20:06
 * Updated in $/VS/Transcoder
 *
 * *****************  Version 27  *****************
 * User: Smirnov      Date: 10.10.05   Time: 13:20
 * Updated in $/VS/Transcoder
 * -h264 buffres bug fix
 *
 * *****************  Version 26  *****************
 * User: Smirnov      Date: 21.09.05   Time: 14:13
 * Updated in $/VS/Transcoder
 * - added RTP buffers for H264
 * - added base class for all video input buffers
 *
 * *****************  Version 25  *****************
 * User: Smirnov      Date: 19.09.05   Time: 18:06
 * Updated in $/VS/Transcoder
 * - added RTP H261 buffers
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 16.09.05   Time: 12:04
 * Updated in $/VS/Transcoder
 * - added g7221_24 support in gateway
 * - added h.263+ RTP support
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 19.08.05   Time: 15:37
 * Updated in $/VS/Transcoder
 * - metall voise in Polycom fixed for g728
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 1.08.05    Time: 19:23
 * Updated in $/VS/Transcoder
 * - moved GenRAndom()
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 10.06.05   Time: 19:27
 * Updated in $/VS/Transcoder
 *  - new base rtp buffer
 *  - g728 codec are embeded
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 28.02.05   Time: 14:48
 * Updated in $/VS/Transcoder
 * repaired call from Visi to h323
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 25.02.05   Time: 18:54
 * Updated in $/VS/Transcoder
 * added mu low audio codec support
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 25.02.05   Time: 16:08
 * Updated in $/VS/Transcoder
 * g711 -> g711a
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 24.02.05   Time: 19:29
 * Updated in $/VS/Transcoder
 * added g711
 * added format initaalization
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 15.10.04   Time: 19:20
 * Updated in $/VS/Transcoder
 * fixed packets lost in transcoder
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 28.09.04   Time: 22:01
 * Updated in $/VS/Transcoder
 * output buffers in transcoders now are barrel
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 27.09.04   Time: 16:11
 * Updated in $/VS/Transcoder
 * old variant of packet forming
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 24.09.04   Time: 20:22
 * Updated in $/VS/Transcoder
 * mode formin packets
*
* *****************  Version 12  *****************
* User: Smirnov      Date: 10.09.04   Time: 19:12
* Updated in $/VS/Transcoder
* lost frames detection
*
* *****************  Version 11  *****************
* User: Smirnov      Date: 6.09.04    Time: 19:32
* Updated in $/VS/Transcoder
* changed rtp audio paket len = 60 ms
*
* *****************  Version 10  *****************
* User: Smirnov      Date: 23.07.04   Time: 14:59
* Updated in $/VS/Transcoder
* returned video interval correction
*
* *****************  Version 9  *****************
* User: Smirnov      Date: 23.07.04   Time: 14:31
* Updated in $/VS/Transcoder
* video frames interval
*
* *****************  Version 8  *****************
* User: Smirnov      Date: 23.07.04   Time: 12:24
* Updated in $/VS/Transcoder
* set frame len = 24
*
* *****************  Version 7  *****************
* User: Smirnov      Date: 22.07.04   Time: 20:13
* Updated in $/VS/Transcoder
* corrected timestamp
*
* *****************  Version 6  *****************
* User: Smirnov      Date: 21.07.04   Time: 18:48
* Updated in $/VS/Transcoder
* corrected audio time
*
* *****************  Version 5  *****************
* User: Smirnov      Date: 21.07.04   Time: 18:24
* Updated in $/VS/Transcoder
* new payload in rtp schema
*
* *****************  Version 4  *****************
* User: Avlaskin     Date: 19.07.04   Time: 21:16
* Updated in $/VS/Transcoder
*
* *****************  Version 3  *****************
* User: Smirnov      Date: 28.06.04   Time: 17:46
* Updated in $/VS/Transcoder
* addded time support for video
* added set-get media format support
*
* *****************  Version 2  *****************
* User: Smirnov      Date: 17.05.04   Time: 17:40
* Updated in $/VS/Transcoder
* added RTP foming audio buffers
* added Visicron buffers
*
* *****************  Version 1  *****************
* User: Smirnov      Date: 14.05.04   Time: 19:07
* Created in $/VS/Transcoder
* added RTP H263 buffers
*
****************************************************************************/

/****************************************************************************
* Includes
****************************************************************************/
#include "VS_RTP_Buffers.h"
#include "RtpPayload.h"
#include "MediaParserLib/VS_H263Parser.h"
#include "MediaParserLib/VS_H264Parser.h"
#include "MediaParserLib/VS_AACParser.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/VS_BitField.h"
#include "std-generic/cpplib/hton.h"
#include <stdio.h>
#include <boost/archive/iterators/binary_from_base64.hpp>

#include "sei.h"

/****************************************************************************
* Globals
****************************************************************************/
static const size_t c_max_out_queue_size = 1024;

/****************************************************************************
* Structures
****************************************************************************/
/**
**************************************************************************
* Stream Header of H263 picture
****************************************************************************/
struct h263streamheader {
	unsigned int	FirstDWord;

	unsigned int	unrestricted_motion_vector:1;
	unsigned int	picture_coding_type:1;
	unsigned int	srcformat:3;
	unsigned int	freeze_picture_release:1;
	unsigned int	doccamera:1;
	unsigned int	splitscreen:1;
	unsigned int	LostBits:24;
};

/**
**************************************************************************
* Stream Header of H261 picture
****************************************************************************/
struct h261streamheader {
	unsigned int	pei:1;
	unsigned int	spare:1;
	unsigned int	still_image_mode:1;
	unsigned int	srcformat:1;
	unsigned int	freeze_picture_release:1;
	unsigned int	doccamera:1;
	unsigned int	splitscreen:1;
	unsigned int	LostBits:25;
};

/**
**************************************************************************
* Stream Header of H263+ picture, ufep = 0
****************************************************************************/
struct h263streamheader_p12bit {
	unsigned int	FirstDWord;
	//--------------------------
	unsigned int	first8:8;
	//--------------------------
	unsigned int	second4:4;
	unsigned int	pct:3;
	unsigned int	ufep:1;
	//--------------------------
	unsigned int	LostBits:16;
};

/**
**************************************************************************
* Stream Header of H263+ picture, ufep = 1
****************************************************************************/
struct h263streamheader_p30bit {
	unsigned int	FirstDWord;
	//--------------------------
	unsigned int	first8:8;
	//--------------------------
	unsigned int	second7:7;	// +11 bit
	unsigned int	ufep:1;		// +18 bit
	//--------------------------
	unsigned int	third8:8;	//+ 3 bit
	//--------------------------
	unsigned int	LostBits:2;
	unsigned int	pct:3;
	unsigned int	fourth3:3;	//+ 0 bit
};

/**
**************************************************************************
* Find left_aligned_pattern in bitstream buff, begin from start to end,
* matcing bits bits of left_aligned_pattern
****************************************************************************/
static int FindPattern(uint8_t* buff, int start, int end, unsigned long left_aligned_pattern, int bits)
{
	if (bits>24) return end;
	unsigned long mask = (~0u)<<(32-bits);
	left_aligned_pattern &= mask;
	while (start < (end-bits)) {
		uint8_t *p = buff + (start>>3);
		unsigned long val = (p[0]<<24)|(p[1]<<16)|(p[2]<<8)|(p[3]<<0);
		int startbit = start&0x7;
		for (; startbit<8; startbit++) {
			unsigned long cmpval = val<<startbit;
			cmpval &= mask;
			if (left_aligned_pattern == cmpval) return start<(end-bits)? start : end;
			start++;
		}
	}
	return end;
}

/****************************************************************************
* Classes
****************************************************************************/
/****************************************************************************
* VS_RTP_InputBuffer
****************************************************************************/
/**
**************************************************************************
* Constructor
****************************************************************************/
VS_RTP_InputBuffer::VS_RTP_InputBuffer()
{
	m_time = 0;
	m_seq = 0;
}
/**
**************************************************************************
* Destructor
****************************************************************************/
VS_RTP_InputBuffer::~VS_RTP_InputBuffer()
{
}


/****************************************************************************
* VS_RTP_OutputBuffer
****************************************************************************/
/**
**************************************************************************
* Constructor
****************************************************************************/
VS_RTP_OutputBuffer::VS_RTP_OutputBuffer(unsigned char ptype)
	: m_packets(32)
{
	m_seq = (unsigned short)VS_GenKeyByMD5();
	m_time = (unsigned short)VS_GenKeyByMD5();
	m_ssrc = VS_GenKeyByMD5();
	m_ptype = ptype;
}


/**
**************************************************************************
* Clean buffers
****************************************************************************/
void VS_RTP_OutputBuffer::Clean()
{
	m_packets.clear();
	m_buff.Reset();
}

/**
**************************************************************************
* Add New RTP packet into queue
****************************************************************************/
void VS_RTP_OutputBuffer::NewPacket(unsigned char *buff, int len, unsigned long timestamp, bool marker)
{
	if (m_packets.full() && m_packets.capacity() < c_max_out_queue_size)
		m_packets.set_capacity(std::min(m_packets.capacity() * 3 / 2, c_max_out_queue_size));

	m_packets.push_back();
	auto& packet = m_packets.back();
	packet.SetData(buff, len, true);
	packet.SeqNo(m_seq++);
	packet.Timestamp(timestamp);
	packet.PayloadType(m_ptype);
	packet.SSRC(m_ssrc);
	packet.Marker(marker);
}

/**
**************************************************************************
* Copy next packet data in supplied buffer, return true if any data was copied
****************************************************************************/
bool VS_RTP_OutputBuffer::Get(unsigned char* in, unsigned long &size)
{
	size = 0;
	if (!m_packets.empty())
	{
		size = m_packets.front().GetPacket(in);
		m_packets.pop_front();
		return true;
	}
	else
		return false;
}

/****************************************************************************
* Return size of data that next call to Get() will return
****************************************************************************/
unsigned long VS_RTP_OutputBuffer::NextGetSize() const
{
	if (!m_packets.empty())
		return m_packets.front().GetPacket(nullptr);
	else
		return 0;
}

/****************************************************************************
* VS_RTP_InputBufferVideo
****************************************************************************/
/**
**************************************************************************
* Constructor
****************************************************************************/
VS_RTP_InputBufferVideo::VS_RTP_InputBufferVideo()
{
	m_frame_complete[0] = 0;
	m_frame_complete[1] = 0;
	m_curr_buff = 0;
	m_timestamp = 0;
}

VS_RTP_InputBufferVideo::VS_RTP_InputBufferVideo(VS_RTP_InputBufferVideo &&rhs)
{
	m_time = rhs.m_time;
	m_seq = rhs.m_seq;
	m_buff[0] = std::move(rhs.m_buff[0]);
	m_buff[1] = std::move(rhs.m_buff[1]);
	m_curr_buff = rhs.m_curr_buff;
	m_frame_complete[0] = rhs.m_frame_complete[0];
	m_frame_complete[1] = rhs.m_frame_complete[1];
}

VS_RTP_InputBufferVideo &VS_RTP_InputBufferVideo::operator=(VS_RTP_InputBufferVideo &&rhs)
{
	if (this != &rhs) {
		m_time = rhs.m_time;
		m_seq = rhs.m_seq;
		m_buff[0] = std::move(rhs.m_buff[0]);
		m_buff[1] = std::move(rhs.m_buff[1]);
		m_curr_buff = rhs.m_curr_buff;
		m_frame_complete[0] = rhs.m_frame_complete[0];
		m_frame_complete[1] = rhs.m_frame_complete[1];
	}

	return *this;
}

/**
**************************************************************************
* Extract from RTP packet common parametrs for video data
****************************************************************************/
int VS_RTP_InputBufferVideo::Add(RTPPacket *rtp)
{
	int ret = 0;
	if (m_seq+1 != rtp->SeqNo() && m_seq != 0 && rtp->SeqNo() != 0 )
		ret =-1;
	m_seq = rtp->SeqNo();

	if (m_buff[m_curr_buff].Bits()==0 || m_time != rtp->Timestamp()) { 	// then new frame
		if (m_time==0)					// first referenced value
			m_time = rtp->Timestamp();

		m_frame_complete[m_curr_buff] = true;
		m_curr_buff ^= 1;
		m_buff[m_curr_buff].Reset();
		int diff = rtp->Timestamp() - m_time;
		if (diff < 0)
			diff = 0; // fast fix for not monolitic time stamps
		unsigned long videoIntraval = diff/90; // time in 90 kHz samples
		m_time = rtp->Timestamp();
		m_buff[m_curr_buff].Add((unsigned char*)&videoIntraval, 0, 32);
		uint32_t timestamp = m_time / 90;
		m_buff[m_curr_buff].Add((unsigned char*)&timestamp, 0, 32);
	}
	m_frame_complete[m_curr_buff] = rtp->Marker();

	if (rtp->DataSize() > 0)
		AddRTPData(rtp);
	return ret;
}

/**
**************************************************************************
* Extract video data from RTP and add it to interanal buffer
****************************************************************************/
void VS_RTP_InputBufferVideo::AddRTPData(RTPPacket *rtp)
{
	RTPMediaHeader mh(rtp);
	m_buff[m_curr_buff].Add(mh.data, 0, mh.datasize*8);
}


/**
**************************************************************************
* Fill buffer and size, return num of lost buffers
****************************************************************************/
int VS_RTP_InputBufferVideo::Get(unsigned char* in, unsigned long &size, unsigned long &VideoInterval, char &key)
{
	size = 0;
	int ret = -1;
	int prev_buff = m_curr_buff^1;
	if (m_frame_complete[prev_buff]) {
		size = (m_buff[prev_buff].Bits()+7)/8;
		if (size>8) {
			VideoInterval = *(int*)m_buff[m_curr_buff].Buff();
			size-=4;
			m_timestamp = *(uint32_t*)(m_buff[m_curr_buff].Buff()+4);
			size-=4;
			memcpy(in, m_buff[prev_buff].Buff()+8, size);
			ret++;
		}
		m_buff[prev_buff].Reset();
		m_frame_complete[prev_buff] = false;
	}
	if (ret==-1) { // prev was empy
		if (m_frame_complete[m_curr_buff]) {
			size = (m_buff[m_curr_buff].Bits()+7)/8;
			if (size>8) {
				VideoInterval = *(int*)m_buff[m_curr_buff].Buff();
				size-=4;
				m_timestamp = *(uint32_t*)(m_buff[m_curr_buff].Buff()+4);
				size-=4;
				memcpy(in, m_buff[m_curr_buff].Buff()+8, size);
				ret++;
			}
			m_buff[m_curr_buff].Reset();
			m_frame_complete[m_curr_buff] = false;
		}
	}
	if (ret>=0)
		key = IsKey(in, size) ? 1 : 0;
	return ret;
}

/****************************************************************************
* Return size of data that next call to Get() will return
****************************************************************************/
unsigned long VS_RTP_InputBufferVideo::NextGetSize() const
{
	unsigned long size(0);
	const int prev_buff = m_curr_buff^1;
	if (m_frame_complete[prev_buff]) {
		size = (m_buff[prev_buff].Bits()+7)/8;
		if (size > 8)
			return size-8;
	}
	if (m_frame_complete[m_curr_buff]) {
		size = (m_buff[m_curr_buff].Bits()+7)/8;
		if (size > 8)
			return size-8;
	}
	return 0;
}

/****************************************************************************
* Return timestamp of video packet after call Get()
****************************************************************************/
uint32_t VS_RTP_InputBufferVideo::GetVideoTimestamp() const
{
	return m_timestamp;
}

/**
**************************************************************************
* Extract H261 (part of) frame from RTP and add it to interanal buffer
****************************************************************************/
void VS_RTP_H261InputBuffer::AddRTPData(RTPPacket *rtp)
{
	if (rtp->PayloadType()==RTP_PT_H261) {
		RTPMediaHeader mh(rtp);
		m_buff[m_curr_buff].Add(mh.data, mh.header.h261.sbit, mh.datasize*8 - mh.header.h261.ebit - mh.header.h261.sbit);
	}
}


/**
**************************************************************************
* Check H261 frame is KeyFrame
****************************************************************************/
bool VS_RTP_H261InputBuffer::IsKey(unsigned char *data, size_t size)
{
	h261streamheader* hdr = (h261streamheader*)data;
	return hdr->freeze_picture_release==1;
}


/**
**************************************************************************
* Extract H263 (part of) frame from RTP and add it to interanal buffer
****************************************************************************/
void VS_RTP_H263InputBuffer::AddRTPData(RTPPacket *rtp)
{
	if (rtp->PayloadType()==RTP_PT_H263) {
		RTPMediaHeader mh(rtp);
		m_buff[m_curr_buff].Add(mh.data, mh.header.h263a.sbit, mh.datasize*8 - mh.header.h263a.ebit - mh.header.h263a.sbit);
	}
}


/**
**************************************************************************
* Check H263 frame is KeyFrame
****************************************************************************/
bool VS_RTP_H263InputBuffer::IsKey(unsigned char *data, size_t size)
{
	h263streamheader* hdr = (h263streamheader*)data;
	if (hdr->srcformat>0 && hdr->srcformat<6)
		return hdr->picture_coding_type==0;
	else if (hdr->srcformat==7) {
		h263streamheader_p12bit* hdr = (h263streamheader_p12bit*)data;
		if (hdr->ufep==0)
			return hdr->pct==0;
		else {
			h263streamheader_p30bit* hdr = (h263streamheader_p30bit*)data;
			return hdr->pct==0;
		}
	}
	else
		return 0;
}

/**
**************************************************************************
* Extract frame resolution from h.263 bitstream
****************************************************************************/
int VS_RTP_H263InputBuffer::GetResolution(unsigned char *in, int size, int &width, int &height)
{
	return ResolutionFromBitstream_H263(in, size, width, height);
}

/**
**************************************************************************
* Extract H263+ (part of) frame from RTP and add it to interanal buffer
****************************************************************************/
void VS_RTP_H263PlusInputBuffer::AddRTPData(RTPPacket *rtp)
{
	if (rtp->PayloadType()==RTP_PT_H263)
		return VS_RTP_H263InputBuffer::AddRTPData(rtp);
	RTPMediaHeader mh(rtp);
	short leading_Zero = 0;
	int StartBitPos = 16 + (mh.header.h263Plus.v + mh.header.h263Plus.plen)*8;
	if (mh.header.h263Plus.p==1)
		m_buff[m_curr_buff].Add((unsigned char*)&leading_Zero, 0, 16);
	m_buff[m_curr_buff].Add(mh.data, StartBitPos, mh.datasize*8 - StartBitPos);
}


/**
**************************************************************************
* Extract H264 (part of) frame from RTP and add it to interanal buffer
****************************************************************************/
void VS_RTP_H264InputBuffer::AddRTPData(RTPPacket *rtp)
{
	auto headerStart = rtp->Data();
	int length = rtp->DataSize();

	int fCurPacketNALUnitType = (headerStart[0] & 0x1F);
	if (fCurPacketNALUnitType >= 1 && fCurPacketNALUnitType <= 23 && length >= 1)
	{
		// Complete NAL unit
		m_DropThisUnit = false;
		m_posInBuff = m_nalBuff;
		InitStartSequence(*headerStart);

		if (SIZE_OF_BUFFER - (m_posInBuff - m_nalBuff) > length - 1)
		{
			memcpy(m_posInBuff,  headerStart + 1, length - 1);
			m_posInBuff += length - 1;
		}
		else
			m_DropThisUnit = true;

		if (!m_DropThisUnit)
			m_buff[m_curr_buff].Add(m_nalBuff, 0, (m_posInBuff - m_nalBuff) * 8);
		m_posInBuff = m_nalBuff;
	}
	else if (fCurPacketNALUnitType == NAL_FU_A && length >= 2)
	{
		const bool is_first = (headerStart[1] & 0x80) != 0;
		const bool is_last  = (headerStart[1] & 0x40) != 0;

		if (!is_first && !is_last)
		{
			// Skip FU indicator and FU header
			headerStart += 2;
			length -= 2;

					if (m_posInBuff == m_nalBuff)
						m_DropThisUnit = true;
					if (m_seqNum + 1 != rtp->SeqNo() && m_seqNum != 0 && rtp->SeqNo() != 0)
						m_DropThisUnit = true;

					if (!m_DropThisUnit)
					{
						if (SIZE_OF_BUFFER - (m_posInBuff - m_nalBuff) > length)
						{
							memcpy(m_posInBuff,  headerStart, length);
							m_posInBuff += length;
						}
						else
							m_DropThisUnit = true;
					}
		}
		else if (is_last)
		{
			// Skip FU indicator and FU header
			headerStart += 2;
			length -= 2;

					if (m_posInBuff == m_nalBuff)
						m_DropThisUnit = true;
					if (m_seqNum + 1 != rtp->SeqNo() && m_seqNum != 0 && rtp->SeqNo() != 0)
						m_DropThisUnit = true;

					if (!m_DropThisUnit)
					{
						if (SIZE_OF_BUFFER - (m_posInBuff - m_nalBuff) > length)
						{
							memcpy(m_posInBuff,  headerStart, length);
							m_posInBuff += length;
						}
						else
							m_DropThisUnit = true;
					}

					if (!m_DropThisUnit)
						m_buff[m_curr_buff].Add( m_nalBuff, 0, (m_posInBuff - m_nalBuff) * 8 );
					m_posInBuff = m_nalBuff;
		}
		else if (is_first)
		{
			const unsigned char originalHeader = (headerStart[0] & 0xE0) | (headerStart[1] & 0x1F);

			// Skip FU indicator and FU header
			headerStart += 2;
			length -= 2;

			m_DropThisUnit = false;
			m_posInBuff = m_nalBuff;
			InitStartSequence(originalHeader);

			if (SIZE_OF_BUFFER - (m_posInBuff - m_nalBuff) > length)
			{
				memcpy(m_posInBuff,  headerStart, length);
				m_posInBuff += length;
			}
			else
				m_DropThisUnit = true;
		}
	}
	else if (fCurPacketNALUnitType == NAL_STAP_A && length >= 4)
	{
		// Discard the type byte
		headerStart += 1;
		length -= 1;

				m_posInBuff = m_nalBuff;

				while (length > 2) {

					// network byte order
					int nal_size = headerStart[0] * 256 + headerStart[1];

                    // consume the length of the aggregate
                    headerStart     += 2;
                    length			-= 2;

                    if (nal_size <= length && nal_size > 0) {
							m_posInBuff = m_nalBuff;
							InitStartSequence(*headerStart);

							memcpy(m_posInBuff, headerStart + 1, nal_size - 1);
							m_posInBuff += nal_size-1;

							m_buff[m_curr_buff].Add( m_nalBuff, 0, (m_posInBuff - m_nalBuff) * 8 );
							m_posInBuff = m_nalBuff;
					} else {
							break;
                    }

                    // eat what we handled
                    headerStart     += nal_size;
                    length			-= nal_size;

                    if (length < 0) break;
                }
	}

	m_seqNum = rtp->SeqNo();
}

/**
**************************************************************************
* Check H264 frame is KeyFrame
****************************************************************************/
bool VS_RTP_H264InputBuffer::IsKey(unsigned char *data, size_t size)
{
	const unsigned char* const data_end = data + size;
	const unsigned char* p = data;
	while (p < data_end)
	{
		const unsigned char* nal;
		const unsigned char* nal_end;
		unsigned int start_code_size;
		if (NALFromBitstream_H264(p, data_end - p, nal, nal_end, start_code_size) != 0)
			return false;

		const unsigned char nal_type = nal[start_code_size] & 0x1f;
		if (nal_type == NAL_UT_SPS || nal_type == NAL_UT_IDR_SLICE)
			return true; // Definitely a key frame
		if (nal_type == NAL_UT_SLICE)
			return false; // Definitely not a key frame

		p = nal_end;
	}
	return false;
}

/**
**************************************************************************
* Extract frame resolution from h.264 bitstream
****************************************************************************/
int VS_RTP_H264InputBuffer::GetResolution(unsigned char *in, int size, int &width, int &height)
{
	return ResolutionFromBitstream_H264(in, size, width, height);
}

void VS_RTP_H264InputBuffer::InitStartSequence(unsigned char header)
{
	NAL_Unit_Type uUnitType = (NAL_Unit_Type)(header & 0x1f);
	if ((uUnitType >= NAL_UT_SEI) && (uUnitType <= NAL_UT_PD)) {
		*m_posInBuff++ = 0;
	}
	*m_posInBuff++ = 0;
	*m_posInBuff++ = 0;
	*m_posInBuff++ = 1;
	*m_posInBuff++ = header & 0x7f;
}


/****************************************************************************
* VS_RTP_H263OutputBuffer
****************************************************************************/
/**
**************************************************************************
* Form New Packet
****************************************************************************/
bool VS_RTP_H263OutputBuffer::FormNewRTPPacket(int sbits, int ebits, unsigned long timestamp, bool last, RTPMedia_Mode mode)
{
	VS_BitStreamBuff currbuff;
	if (mode==RTPMODE_H263_A) {
		m_payload.h263a.sbit = sbits&0x7;
		m_payload.h263a.ebit = (8 - (ebits&0x7))&0x7;
	}
	else if (mode==RTPMODE_H263_B) {
		m_payload.h263b.sbit = sbits&0x7;
		m_payload.h263b.ebit = (8 - (ebits&0x7))&0x7;
	}
	else if (mode==RTPMODE_H261) {
		m_payload.h261.sbit = sbits&0x7;
		m_payload.h261.ebit = (8 - (ebits&0x7))&0x7;
	}
	else return false;
	uint32_t payload[3];
	payload[0] = vs_htonl(m_payload.unknown.dummy[0]);
	payload[1] = vs_htonl(m_payload.unknown.dummy[1]);
	payload[2] = vs_htonl(m_payload.unknown.dummy[2]);
	currbuff.Add((unsigned char *)&payload[0], 0, 32);
	if (mode==RTPMODE_H263_B)
		currbuff.Add((unsigned char *)&payload[1], 0, 32);
	currbuff.Add((unsigned char *)m_buff.Buff() + (sbits>>3), 0, ebits - sbits + (sbits&0x7));
	NewPacket(currbuff.Buff(), (currbuff.Bits()+7)>>3, timestamp, last);
	return true;
}

#if 1
/**
**************************************************************************
* Add H263 frame and split it to RTP frames
****************************************************************************/
void VS_RTP_H263OutputBuffer::Add(const void* buff, unsigned long buffsize)
{
	m_buff.Reset();
	m_buff.Add(static_cast<const unsigned char*>(buff), 0, buffsize*8);
	auto timePoint = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
	uint32_t timestamp = (m_time + timePoint)*90;
	int PSCode, PSCodeLen, GOBLen;
	RTPMedia_Mode rtp_mode;
	if (RTP_PT_H261==m_ptype) {
		PSCode = 0x00010000;
		PSCodeLen = 20;
		GOBLen = 16;
		rtp_mode = RTPMODE_H261;
	}
	else {
		PSCode = 0x00008000;
		PSCodeLen = 22;
		GOBLen = 17;
		rtp_mode = RTPMODE_H263_A;
		h263streamheader* h263hdr = (h263streamheader*)m_buff.Buff();
		m_payload.h263a.srcformat = h263hdr->srcformat;
		m_payload.h263a.picture_coding_type = h263hdr->picture_coding_type;
	}
	int found_bit = FindPattern(m_buff.Buff(), 0, m_buff.Bits(), PSCode, PSCodeLen);
	if (found_bit != 0)
		return; /// PSC not found
	int prev_bits = 0;
	int start_bits = 0;
	while (true) {
		found_bit = FindPattern(m_buff.Buff(), prev_bits+1, m_buff.Bits(), PSCode, GOBLen);
		bool bFoundSmall = found_bit-start_bits <= VS_RTP_MAXLEN*8;
		bool bEndOfStream = found_bit==m_buff.Bits();
		if		(bFoundSmall && !bEndOfStream) {
			prev_bits = found_bit;
			continue;
		}
		else if (!bFoundSmall && !bEndOfStream) {
			int len = prev_bits - start_bits;
			if (len!=0) {							// there was even if one small block
				FormNewRTPPacket(start_bits, prev_bits, timestamp, false, rtp_mode);
				start_bits = prev_bits;
			}
			else {
				while (prev_bits!=found_bit) {
					len = std::min(VS_RTP_MAXLEN*8, found_bit-prev_bits);
					prev_bits +=len;
					FormNewRTPPacket(start_bits, prev_bits, timestamp, false, rtp_mode);
					start_bits = prev_bits;
				}
			}
		}
		else if (!bFoundSmall && bEndOfStream) {
			int len = prev_bits - start_bits;
			if (len!=0) {							// there was even if one small block
				FormNewRTPPacket(start_bits, prev_bits, timestamp, false, rtp_mode);
				start_bits = prev_bits;
			}
			while(prev_bits!=found_bit) {
				len = std::min(VS_RTP_MAXLEN*8, found_bit-prev_bits);
				prev_bits +=len;
				FormNewRTPPacket(start_bits, prev_bits, timestamp, prev_bits==found_bit, rtp_mode);
				start_bits = prev_bits;
			}
			break;

		}
		else if (bFoundSmall && bEndOfStream) {
			prev_bits = found_bit;
			int len = prev_bits - start_bits;
			FormNewRTPPacket(start_bits, prev_bits, timestamp, true, rtp_mode);
			start_bits = prev_bits;
			break;
		}
	}
}

#else
/**
**************************************************************************
* Add H263 frame and split it to RTP frames
****************************************************************************/
void VS_RTP_H263OutputBuffer::Add(void* buff, unsigned long buffsize)
{
	Clean();
	printf("\nttotal bits = %7d\n", buffsize*8);
	m_buff.Add((unsigned char *)buff, 0, buffsize*8);
	unsigned long timestamp = (m_time + timeGetTime())*90;
	if (!m_parser.play_movie(buff, buffsize))
		return;
	int srcformat = ((h263streamheader*)m_buff.Buff())->srcformat;
	int picture_coding_type = ((h263streamheader*)m_buff.Buff())->picture_coding_type;

	GobLayer* gobs = m_parser.FrameSplit.gob;
	MBLayer* mbs = m_parser.FrameSplit.mb;
	int mbpergob = m_parser.FrameSplit.NumOfMB/m_parser.FrameSplit.NumOfGOB;

	for (int i = 0; i<m_parser.FrameSplit.NumOfGOB-1; i++) {
		gobs[i].bitpos = gobs[i+1].bitpos;
	}
	gobs[m_parser.FrameSplit.NumOfGOB-1].bitpos = m_buff.Bits();

	for (int i = 0; i<m_parser.FrameSplit.NumOfMB-1; i++) {
		mbs[i].bitpos = mbs[i+1].bitpos;
		if (i%mbpergob==mbpergob-1) {// last before new gob
			if (gobs[i/mbpergob].bitpos) { // next gob have a header
				mbs[i].bitpos = gobs[i/mbpergob].bitpos;
			}
		}
	}
	mbs[m_parser.FrameSplit.NumOfMB-1].bitpos = m_buff.Bits();

	int last_gob = 0, first_gob = 0;
	int last_bit = 0, first_bit = 0, curr_bit = 0;
	int maxgob = m_parser.FrameSplit.NumOfGOB;
	for (int i = first_gob; i<maxgob; i++) {
		curr_bit = gobs[i].bitpos;
		if (curr_bit==0)
			continue;
		if (curr_bit - first_bit <= VS_RTP_MAXLEN*8) {
			last_gob = i;
			last_bit = curr_bit;
			if (last_gob<maxgob-1)
				continue;
		}
		if (last_bit == first_bit) {
			last_gob = i;
			last_bit = curr_bit;
		}
		if (last_bit - first_bit <= VS_RTP_MAXLEN*8) {
			m_payload.Clean();
			m_payload.h263a.srcformat = srcformat;
			m_payload.h263a.picture_coding_type = picture_coding_type;
			FormNewRTPPacket(first_bit, last_bit, timestamp, last_bit==m_buff.Bits());
		}
		else {
			// split gob
			int maxmb = mbpergob*(last_gob+1);
			int first_mb = mbpergob*(first_gob);
			int last_mb = first_mb;
			for (int j=first_mb ;j<maxmb; j++) {
				curr_bit = mbs[j].bitpos;
				if (curr_bit - first_bit <= VS_RTP_MAXLEN*8) {
					last_mb = j;
					last_bit = curr_bit;
					if (last_mb<maxmb-1)
						continue;
				}
				if (last_bit == first_bit) {
					last_mb = j;
					last_bit = curr_bit;
				}
				m_payload.Clean();
				m_payload.h263b.srcformat = srcformat;
				m_payload.h263b.picture_coding_type = picture_coding_type;
				m_payload.h263b.ftype = 1;
				m_payload.h263b.gobn = first_mb/mbpergob;
				m_payload.h263b.mba = first_mb%mbpergob;
				m_payload.h263b.hmv1 = mbs[first_mb].hmv1;
				m_payload.h263b.vmv1 = mbs[first_mb].vmv1;
				m_payload.h263b.hmv2 = mbs[first_mb].hmv2;
				m_payload.h263b.vmv2 = mbs[first_mb].vmv2;
				m_payload.h263b.quant = mbs[first_mb].quant;
				if (m_payload.h263b.mba==0) {
					if (m_payload.h263b.gobn==0 || gobs[m_payload.h263b.gobn-1].bitpos!=0)
						m_payload.h263b.quant = 0;
				}
				FormNewRTPPacket(first_bit, last_bit, timestamp, last_bit==m_buff.Bits(), RTPMODE_H263_B);
				first_mb = last_mb+1;
				first_bit = last_bit;
				j = first_mb-1;
				if (j==maxmb-1)
					break;
			}
		}
		first_gob = last_gob+1;
		first_bit = last_bit;
		i = first_gob-1;
		if (i==maxgob-1)
			break;
	}
}
#endif



/****************************************************************************
* VS_RTP_H263PlusOutputBuffer
****************************************************************************/
/**
**************************************************************************
* Add H263+ frame and split it to RTP frames
****************************************************************************/
void VS_RTP_H263PlusOutputBuffer::Add(const void* buff, unsigned long buffsize)
{
	m_buff.Reset();
	m_buff.Add(static_cast<const unsigned char*>(buff), 0, buffsize * 8);
	auto timePoint = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
	uint32_t timestamp = (m_time + timePoint)*90;
	int found_bit = FindPattern(m_buff.Buff(), 0, m_buff.Bits(), 0x00008000, 22);
	if (found_bit!=0) // not h263 frame!
		return;
	VS_BitStreamBuff currbuff;
	bool last = false;
	while (!last) {
		int len = std::min(m_buff.Bits() - found_bit, VS_RTP_MAXLEN*8);;
		RTPPayload pl;
		pl.Clean();
		pl.h263Plus.p = found_bit == 0;
		last = found_bit + len == m_buff.Bits();

		uint32_t payload;
		payload = vs_htonl(pl.unknown.dummy[0]);
		currbuff.Reset();
		currbuff.Add((unsigned char *)&payload, 0, 16);
		currbuff.Add(m_buff.Buff() + (found_bit>>3) + 2, 0, len);
		NewPacket(currbuff.Buff(), (currbuff.Bits()+7)>>3, timestamp, last);
		found_bit+=len;
	}
}


/****************************************************************************
* VS_RTP_H264OutputBuffer
****************************************************************************/
/**
**************************************************************************
* Add H264 frame and split it to RTP frames
****************************************************************************/
void VS_RTP_H264OutputBuffer::Add(const void* buff, unsigned long buffsize)
{
	auto timePoint = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
	uint32_t timestamp = (m_time + timePoint)*90;
	const unsigned char * in = static_cast<const unsigned char*>(buff);
	int start_pos = -1, end_pos = -1;
	unsigned long ind = 0;
	bool LastFrame = false;
	if (m_buffer.size() < buffsize) {
		m_buffer.resize(buffsize);
	}
	uint8_t *out = m_buffer.data();

	sei::PACSINalUnit p;
	if (sei::ParsePACSINalUnit(in, buffsize, p) && buffsize > 54) {
		size_t sz = sizeof(sei::PACSINalUnit);
		if (!p.T) sz -= 2;
		sz += vs_ntohs(*(uint16_t *)(in + sz));
		sz += 2;
		if (buffsize >= sz) {
			memcpy(out, in, sz);
			NewPacket(out, sz, timestamp, LastFrame);
			ind += sz;
		}
	}

	while(!LastFrame) {
        for(; ind + 3 < buffsize; ind++){
            if(in[ind] == 0 && in[ind+1] == 0 && in[ind+2] == 1) // startcode found
                break;
        }
		end_pos = ind;
		ind+=3;
		LastFrame = ind==buffsize;
		if (start_pos==-1) {
			start_pos=ind;
			continue; // first startcode
		}
		if (LastFrame)
			end_pos+=3;
		int nalSize = end_pos - start_pos;
		const unsigned char * src = in + start_pos;
		int i = 0;
		for (; i < nalSize-2;i++) {
			/// SMirnovK: Polycom expect format
			//if (src[i] == 0 && src[i+1]==0 && src[i+2]==3) {
			//	out[i++] = 0;
			//	out[i++] = 0;
			//	src++;
			//	nalSize--;
			//}
			out[i] = src[i];
		}
		out[i] = src[i];
		i++;
		if (src[i])
		{
			out[i] = src[i];
			i++;
		}

		/// SMirnovK: Polycom not understand NALs type of 9
		if ((*out&0x1f) != 9)
			NewPacket(out, i, timestamp, LastFrame);
		start_pos = ind;
	}
}


/**
**************************************************************************
* Extract data
****************************************************************************/
int VS_RTP_InputBufferAudio::Add(RTPPacket *rtp)
{
	if (rtp->PayloadType()==m_ptype) {
		RTPMediaHeader mh(rtp);
		m_seq = rtp->SeqNo();
		m_time = rtp->Timestamp();
		m_buff.Add(mh.data, 0, mh.datasize*8);
	}
	return 0;
}

/**
**************************************************************************
* Fill buffer and size, return num of lost buffers
****************************************************************************/
int VS_RTP_InputBufferAudio::Get(unsigned char* in, unsigned long &size, unsigned long &VideoInterval, char &key)
{
	size = 0;
	int ret = -1;
	if (m_buff.Bits()) {
		size = m_buff.Bits()/8;
		memcpy(in, m_buff.Buff(), size);
		m_buff.Reset();
		ret+= size!=0;

		if (m_reverse_bytes)
		{
			auto p = in;
			size_t i = size;

			while (i >= 4)
			{
				auto& x = *reinterpret_cast<uint32_t*>(p);
				x = ( (x >> 8) & 0x00FF00FF ) | ((x << 8) & 0xFF00FF00);
				p += 4;
				i -= 4;
			}
			if (i >= 2)
			{
				auto& x = *reinterpret_cast<uint16_t*>(p);
				x = (x >> 8) | (x << 8);
				p += 2;
				i -= 2;
			}
			assert(i < 2);
		}
	}
	return ret;
}

/****************************************************************************
* Return size of data that next call to Get() will return
****************************************************************************/
unsigned long VS_RTP_InputBufferAudio::NextGetSize() const
{
	unsigned long size(0);
	size = m_buff.Bits()/8;
	if (size > 0)
		return size;
	return 0;
}

VS_RTP_InputBufferMPA::VS_RTP_InputBufferMPA()
{
}

int VS_RTP_InputBufferMPA::Add(RTPPacket *rtp)
{
	int ret = 0;
	if (m_seq + 1 != rtp->SeqNo() && m_seq != 0 && rtp->SeqNo() != 0)
	{
		ret = -1;
	}
	m_seq = rtp->SeqNo();

	const bool new_timestamp = m_time != rtp->Timestamp();
	m_time = rtp->Timestamp();

	if (rtp->DataSize() < 4)
		return ret;

	const unsigned fragmentation_offset = GetBitFieldBE(rtp->Data(), 16, 16);

	if (fragmentation_offset == 0)
	{
		// Start of a new frame
		if (!m_fragment.empty())
			m_queue.emplace(std::move(m_fragment));
		// cppcheck-suppress accessMoved ; std::vector::assign can be called on a moved-from object
		m_fragment.assign(rtp->Data() + 4, rtp->Data() + rtp->DataSize());
	}
	else if (fragmentation_offset == m_fragment.size())
	{
		// Part of the last frame (correct offset)
		m_fragment.insert(m_fragment.end(), rtp->Data() + 4, rtp->Data() + rtp->DataSize());
	}
	else
	{
		// Part of some other frame, or part of the last frame but sent out of order
		if (new_timestamp)
		{
			// Timestamp changed, be optimistic and assume that last frame was fully reassembled
			if (!m_fragment.empty())
				m_queue.emplace(std::move(m_fragment));
			// cppcheck-suppress accessMoved ; std::vector::clear can be called on a moved-from object
			m_fragment.clear();
		}
		else
		{
			// Timestamp didn't change, so this part is (most likely) a continuation of the last frame, which means we have to drop it
			m_fragment.clear();
		}
	}

	return ret;
}

int VS_RTP_InputBufferMPA::Get(unsigned char* in, unsigned long &size, unsigned long &VideoInterval, char &key)
{
	if (m_queue.empty())
		return -1;

	size = m_queue.front().size();
	std::memcpy(in, m_queue.front().data(), size);
	m_queue.pop();
	return 0;
}

unsigned long VS_RTP_InputBufferMPA::NextGetSize() const
{
	if (m_queue.empty())
		return 0;

	return m_queue.front().size();
}

VS_RTP_OutputBufferMPA::VS_RTP_OutputBufferMPA()
	: VS_RTP_OutputBuffer(SDP_PT_MPA)
{
}

void VS_RTP_OutputBufferMPA::Add(const void* buff, unsigned long buffsize)
{
	m_time += 1800; // Assuming 20 ms of input audio: clock_rate * duration = 90000Hz * 0.020s
	for (unsigned long packed = 0; packed < buffsize; )
	{
		SetBitFieldBE(m_assemble_buff, 0, 16, 0u);
		SetBitFieldBE(m_assemble_buff, 16, 16, packed);
		const auto to_pack = std::min<unsigned long>(buffsize - packed, VS_RTP_MAXLEN - 4);
		std::memcpy(m_assemble_buff + 4, static_cast<const unsigned char*>(buff) + packed, to_pack);
		NewPacket(m_assemble_buff, 4 + to_pack, m_time, false);
		packed += to_pack;
	}
}

/**
**************************************************************************
* Add compressed audio frame and split it on to rtp packets
****************************************************************************/
const std::chrono::steady_clock::duration  VS_RTP_OutputBufferAudio::silence_duration_before_marker = std::chrono::seconds(1);
void VS_RTP_OutputBufferAudio::Add(const void* buff, unsigned long buffsize)
{
	int start_len = m_abuff.Bytes();
	m_abuff.Add(static_cast<const unsigned char*>(buff), buffsize);
	unsigned char *in = m_abuff.Buff() + start_len;

	if (m_reverse_bytes)
		for (unsigned int i = 0; i < buffsize; i += 4)
		{
			unsigned int &x = *(unsigned int *)(in + i);
			x = ( (x >> 8) & 0x00FF00FF ) | ((x << 8) & 0xFF00FF00);
		}

	while (m_abuff.Bytes() >= (int)m_splitSize) {
		NewPacket(m_abuff.Buff(), m_splitSize, m_time += m_sampleRate*m_timeStep, GetMarkerFlag());
		m_abuff.TruncLeft(m_splitSize);
	}
}
bool VS_RTP_OutputBufferAudio::GetMarkerFlag()
{
	auto now = std::chrono::steady_clock::now();
	bool marker(false);
	if (silence_duration_before_marker < now - m_last_add_pack_time)
		marker = true;
	m_last_add_pack_time = now;
	return marker;
}

void VS_RTP_OutputBufferAudioSpeex::Add(const void* buff, unsigned long buffsize)
{
	NewPacket((unsigned char*)buff, buffsize, m_time+=m_sampleRate*m_timeStep, GetMarkerFlag());
}

void VS_RTP_OutputBufferAudioOpus::Add(const void* buff, unsigned long buffsize)
{
	NewPacket((unsigned char*)buff, buffsize, m_time += m_sampleRate*m_timeStep, GetMarkerFlag());
}

VS_RTP_InputBufferMPEG4ES::VS_RTP_InputBufferMPEG4ES(const VS_MPEG4ESConfiguration& conf)
	: m_conf(conf)
	, m_fau_size(0)
{
}

VS_RTP_InputBufferMPEG4ES::~VS_RTP_InputBufferMPEG4ES()
{
}

int VS_RTP_InputBufferMPEG4ES::Add(RTPPacket *rtp)
{
	int ret = 0;
	if (m_seq+1 != rtp->SeqNo() && m_seq != 0 && rtp->SeqNo() != 0)
	{
		m_fau_size = 0;
		ret = -1;
	}
	m_seq = rtp->SeqNo();

	const unsigned hdr_size_first = m_conf.size_length + m_conf.index_length;
	const unsigned hdr_size_rest  = m_conf.size_length + m_conf.index_delta_length;

	if (hdr_size_first == 0 && hdr_size_rest == 0)
	{
		m_fau_size = 0;
		if (m_conf.constant_size == 0)
			AddAU(rtp->Data(), rtp->DataSize());
		else
		{
			auto p = rtp->Data();
			const auto p_end = rtp->Data() + rtp->DataSize();
			for (; p < p_end; p += m_conf.constant_size)
				AddAU(p, std::min<size_t>(m_conf.constant_size, p_end - p));
		}
		return ret;
	}

	if (rtp->DataSize() < 2)
		return ret;

	const unsigned hdrs_size = GetBitFieldBE(rtp->Data(), 0, 16);
	assert(hdrs_size == hdr_size_first || (hdrs_size - hdr_size_first) % hdr_size_rest == 0);
	if (rtp->DataSize() < 2 + (hdrs_size+7)/8)
		return ret;

	auto p = rtp->Data() + 2 + (hdrs_size+7)/8;
	const auto p_end = rtp->Data() + rtp->DataSize();
	bool first_au = true;
	//unsigned au_index;
	for (unsigned off = 16; off < 16 + hdrs_size;)
	{
		unsigned au_size = m_conf.size_length > 0 ? GetBitFieldBE(rtp->Data(), off, m_conf.size_length) : m_conf.constant_size;
		off += m_conf.size_length;

		if (p_end - p < au_size)
		{
			// AU fragment
			if (m_fau_size == 0)
			{
				m_fau_size = au_size;
				m_fau.reserve(m_fau_size);
				m_fau.clear();
			}
			m_fau.insert(m_fau.end(), p, p_end);
			if (m_fau.size() >= m_fau_size)
			{
				AddAU(m_fau.data(), m_fau_size);
				m_fau_size = 0;
			}
			return ret;
		}
		else
			m_fau_size = 0;

		if (first_au)
		{
			//au_index = GetBitFieldBE(rtp->Data(), off, m_conf.index_length);
			off += m_conf.index_length;
		}
		else
		{
			//au_index += GetBitFieldBE(rtp->Data(), off, m_conf.index_delta_length) + 1;
			off += m_conf.index_delta_length;
		}

		AddAU(p, au_size);
		p += au_size;

		first_au = false;
	}
	return ret;
}

VS_RTP_InputBufferAAC::VS_RTP_InputBufferAAC(const VS_MPEG4ESConfiguration& conf)
	: VS_RTP_InputBufferMPEG4ES(conf)
{
	ParseMPEG4AudioConfig(m_conf.config.data(), m_conf.config.size(), &m_aot, &m_sfi, &m_cc);
}

VS_RTP_InputBufferAAC::~VS_RTP_InputBufferAAC()
{
}

void VS_RTP_InputBufferAAC::AddAU(const void* data, size_t size)
{
	char adts_hdr[7] = { 0 };
	size_t adts_hdr_size = sizeof(adts_hdr);
	if (!MakeADTS(adts_hdr, adts_hdr_size, m_aot, m_sfi, m_cc, size))
		return;

	std::vector<char> frame;
	frame.reserve(sizeof(adts_hdr) + size);
	frame.insert(frame.end(), std::begin(adts_hdr), std::end(adts_hdr));
	frame.insert(frame.end(), static_cast<const char*>(data), static_cast<const char*>(data) + size);
	m_queue.emplace(std::move(frame));
}

int VS_RTP_InputBufferAAC::Get(unsigned char* in, unsigned long &size, unsigned long &VideoInterval, char &key)
{
	if (m_queue.empty())
		return -1;

	size = m_queue.front().size();
	std::memcpy(in, m_queue.front().data(), size);
	m_queue.pop();
	return 0;
}

unsigned long VS_RTP_InputBufferAAC::NextGetSize() const
{
	if (m_queue.empty())
		return 0;

	return m_queue.front().size();
}

int VS_RTP_InputBufferXH264UC::Add(RTPPacket *rtp) {
	return m_temporal_layers[rtp->SSRC()].Add(rtp);
}

int VS_RTP_InputBufferXH264UC::Get(unsigned char* in, unsigned long &size, unsigned long &VideoInterval, char &key) {
	for (auto &l : m_temporal_layers) {
		int ret = l.second.Get(in, size, VideoInterval, key);
		if (ret >= 0) {
			uint32_t tm = l.second.GetVideoTimestamp();
			if (m_timestamp == 0) {
				m_timestamp = tm;
			}
			VideoInterval = tm - m_timestamp;
			m_timestamp = tm;
			return ret;
		}
	}
	return -1;
}

unsigned long VS_RTP_InputBufferXH264UC::NextGetSize() const {
	for (auto &l : m_temporal_layers) {
		int ret = l.second.NextGetSize();
		if (ret) {
			return ret;
		}
	}
	return 0;
}

int VS_RTP_InputBufferXH264UC::GetResolution(unsigned char* in, int size, int &width, int &height) {
	for (auto &l : m_temporal_layers) {
		int ret = l.second.GetResolution(in, size, width, height);
		if (ret >= 0) {
			return ret;
		}
	}
	return -1;
}