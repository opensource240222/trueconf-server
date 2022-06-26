/****************************************************************************
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
*
* Project: TransCoder
*
* $History: RtpPayload.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Transcoder
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Transcoder
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 16.09.05   Time: 12:04
 * Updated in $/VS/Transcoder
 * - added g7221_24 support in gateway
 * - added h.263+ RTP support
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 18.08.05   Time: 20:34
 * Updated in $/VS/Transcoder
 * - added support for g729a
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 10.06.05   Time: 19:27
 * Updated in $/VS/Transcoder
 *  - new base rtp buffer
 *  - g728 codec are embeded
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 27.09.04   Time: 16:11
 * Updated in $/VS/transcoder
 * old variant of packet forming
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 24.09.04   Time: 20:22
 * Updated in $/VS/Transcoder
 * mode formin packets
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 17.05.04   Time: 17:40
 * Updated in $/VS/Transcoder
 * added RTP foming audio buffers
 * added Visicron buffers
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 14.05.04   Time: 19:07
 * Updated in $/VS/Transcoder
 * added RTP H263 buffers
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 5.05.04    Time: 16:53
 * Updated in $/VS/Transcoder
 * test extended by 4 video codec
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 26.04.04   Time: 13:54
 * Updated in $/VS/Transcoder
 * decoding size
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 23.04.04   Time: 20:15
 * Updated in $/VS/transcoder
 * Video Codec
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 20.04.04   Time: 16:22
 * Updated in $/VS/Transcoder
 * payload debugged
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 20.04.04   Time: 13:30
 * Created in $/VS/Transcoder
*
****************************************************************************/

/****************************************************************************
* \file RTPPayload.h
* \brief media headers
****************************************************************************/
#ifndef RYP_PAYLOAD_H
#define RYP_PAYLOAD_H

/****************************************************************************
* Includes
****************************************************************************/
#include "RTPPacket.h"

#include <algorithm>

/****************************************************************************
* H263 definition
****************************************************************************/
struct h263rtpheader_A {
	unsigned int	tr:8;
	unsigned int	trb:3;
	unsigned int	dbq:2;
	unsigned int	reserved:4;
	unsigned int	advanced_prediction:1;
	unsigned int	syntax_based_arithmetic:1;
	unsigned int	unrestricted_motion_vector:1;
	unsigned int	picture_coding_type:1;
	unsigned int	srcformat:3;
	unsigned int	ebit:3;
	unsigned int	sbit:3;
	unsigned int	pbframes:1;	/* 0 for mode A */
	unsigned int	ftype:1;	/* 0 for mode A */
};

struct h263rtpheader_B {
	unsigned int	reserved:2;
	unsigned int	mba:9;
	unsigned int	gobn:5;
	unsigned int	quant:5;
	unsigned int	srcformat:3;
	unsigned int	ebit:3;
	unsigned int	sbit:3;
	unsigned int	pbframes:1;	/* 0 for mode B */
	unsigned int	ftype:1;	/* 1 for mode B */
	/* second 32 bit word */
	unsigned int	vmv2:7;
	unsigned int	hmv2:7;
	unsigned int	vmv1:7;
	unsigned int	hmv1:7;
	unsigned int	advanced_prediction:1;
	unsigned int	syntax_based_arithmetic:1;
	unsigned int	unrestricted_motion_vector:1;
	unsigned int	picture_coding_type:1;
};

struct h263rtpheader_C {
	unsigned int	reserved1:2;
	unsigned int	mba:9;
	unsigned int	gobn:5;
	unsigned int	quant:5;
	unsigned int	srcformat:3;
	unsigned int	ebit:3;
	unsigned int	sbit:3;
	unsigned int	pbframes:1;	/* 1 for mode C */
	unsigned int	ftype:1;	/* 1 for mode C */
	/* second 32 bit word */
	unsigned int	vmv2:7;
	unsigned int	hmv2:7;
	unsigned int	vmv1:7;
	unsigned int	hmv1:7;
	unsigned int	advanced_prediction:1;
	unsigned int	syntax_based_arithmetic:1;
	unsigned int	unrestricted_motion_vector:1;
	unsigned int	picture_coding_type:1;
	/* third 32 bit word */
	unsigned int	tr:8;
	unsigned int	trb:3;
	unsigned int	dbq:2;
	unsigned int	reserved2:19;
};

/****************************************************************************
* H261 definition
****************************************************************************/
struct h261rtpheader {
	unsigned int	vmvd:5;
	unsigned int	hmvd:5;
	unsigned int	quant:5;
	unsigned int	mbap:5;
	unsigned int	gobn:4;				// set to 0 if the packet begins with a GOB header
	unsigned int	motion_vector:1;	// set to 0 if MV -=not used=- (during RTP session)
	unsigned int	intra_frame:1;		// set to 1 if contain -=only=- INTRA bloks (during RTP session)
	unsigned int	ebit:3;
	unsigned int	sbit:3;
};

/****************************************************************************
* H263+ Rtp Payload Header
****************************************************************************/
struct h263p_rtpheader {
	unsigned int	reserved1:16;
	unsigned int	pebit:3;
	unsigned int	plen:6;
	unsigned int	v:1;
	unsigned int	p:1;
	unsigned int	rr:5;
};


/****************************************************************************
* dummy definition
****************************************************************************/
struct rtpheader_unk {
	unsigned int	dummy[3];
};

/****************************************************************************
* g723 definition
****************************************************************************/
struct g723header {
	unsigned int	resv1:24;
	unsigned int	hdr:2;
	unsigned int	resv2:6;
};

/****************************************************************************
* Common Payload header
****************************************************************************/
union RTPPayload {
	h263rtpheader_A	h263a;
	h263rtpheader_B h263b;
	h263rtpheader_C h263c;
	h263p_rtpheader h263Plus;
	h261rtpheader	h261;
	g723header		g723;
	rtpheader_unk	unknown;
	void Clean() {
		unknown.dummy[0] = 0;
		unknown.dummy[1] = 0;
		unknown.dummy[2] = 0;
	}
};

/****************************************************************************
* media parcer
****************************************************************************/
enum RTPMedia_Mode{
	RTPMODE_NONE,
	RTPMODE_G711_A,
	RTPMODE_G711_MU,
	RTPMODE_G723_6400,
	RTPMODE_G723_5333,
	RTPMODE_G723_SID,
	RTPMODE_G728,
	RTPMODE_G729,
	RTPMODE_G729_A,
	RTPMODE_SID,
	RTPMODE_H261,
	RTPMODE_H263_A,
	RTPMODE_H263_B,
	RTPMODE_H263_C
};
class RTPMediaHeader
{
public:
	RTPMediaHeader(){
		Empty();
	}
	RTPMediaHeader(RTPPacket* rtp) {
		Set(rtp);
	}
	virtual ~RTPMediaHeader(){
		Empty();
	}
	void Set(RTPPacket* rtp) {
		int	headersize = 0;

		memcpy(header.unknown.dummy, rtp->Data(),
			std::min(sizeof(header.unknown.dummy), rtp->DataSize()));
		for (auto &d : header.unknown.dummy) {
			d = vs_ntohl(d);
		}

		switch(rtp->PayloadType())
		{
		case RTP_PT_G711_ULAW:
			mode = RTPMODE_G711_MU;
			break;
		case RTP_PT_G711_ALAW:
			mode = RTPMODE_G711_A;
			break;
		case RTP_PT_G723:
			if (header.g723.hdr==0)
				mode = RTPMODE_G723_6400;
			else if (header.g723.hdr==1)
				mode = RTPMODE_G723_5333;
			else
				mode = RTPMODE_G723_SID;
			break;
		case RTP_PT_SID:
			mode = RTPMODE_SID;
			break;
		case RTP_PT_G728:
			mode = RTPMODE_G728;
			break;
		case RTP_PT_G729A:
			mode = RTPMODE_G729;
			break;
		case RTP_PT_H261:
			mode = RTPMODE_H261;
			headersize = 4;
			break;
		case RTP_PT_H263:
			if (header.h263a.ftype==0) {
				headersize = 4;
				mode = RTPMODE_H263_A;
			}
			else if (header.h263a.pbframes==0) {
				headersize = 8;
				mode = RTPMODE_H263_B;
			}
			else {
				headersize = 12;
				mode = RTPMODE_H263_C;
			}
			break;
		default:
			mode = RTPMODE_NONE;
			break;
		}
		data = rtp->Data() + headersize;
		datasize = rtp->DataSize() - headersize;
	}
	void Empty() {
		header.unknown.dummy[0] = 0;
		header.unknown.dummy[1] = 0;
		header.unknown.dummy[2] = 0;
		data = NULL;
		datasize = 0;
		mode = RTPMODE_NONE;
	}
//private:
	RTPPayload		header;
	const uint8_t*	data;
	int				datasize;
	RTPMedia_Mode	mode;
};

#endif
