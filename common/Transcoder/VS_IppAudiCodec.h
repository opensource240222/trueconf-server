/****************************************************************************
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 *
 * Project: TransCoder
 *
 * $History: VS_IppAudiCodec.h $
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 9.09.11    Time: 14:23
 * Updated in $/VSNA/Transcoder
 * - new ipp lib
 * - new audio codecs
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Transcoder
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Transcoder
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 20.04.06   Time: 13:46
 * Updated in $/VS/Transcoder
 * - new audio hardware test
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 18.04.06   Time: 18:43
 * Updated in $/VS/Transcoder
 * - added "empty" PCM codec
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 15.02.06   Time: 13:00
 * Updated in $/VS/Transcoder
 * - added virtual destructor
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 14.12.05   Time: 15:24
 * Updated in $/VS/Transcoder
 * - g722 audio codec added
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 15.11.05   Time: 12:32
 * Updated in $/VS/Transcoder
 * - multi video codecs support
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 17.08.05   Time: 20:20
 * Updated in $/VS/Transcoder
 * added g722.1 ipp codek
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 27.05.05   Time: 16:08
 * Created in $/VS/Transcoder
 * aded new IPP ver 4.1
 * added g711, g728, g729 from IPP
 *
 ****************************************************************************/

/****************************************************************************
 * \file VS_IppAudiCodec.h
 * \brief Common for IPP codecs base class
 ****************************************************************************/
#ifndef VS_IPP_AUDI_CODEC_H
#define VS_IPP_AUDI_CODEC_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "AudioCodec.h"

/****************************************************************************
 * Implementation VS_IppAudiCodec
 ****************************************************************************/
class VS_IppAudiCodec : public AudioCodec
{
protected:
	void*		m_hCodecHandle;
	void*		m_pCodecInfo;
	void*		m_pUSC_Fxns;
	void*		m_pBanks;
	int			m_iNumBanks;
	int			m_iFrameType;
public:
	VS_IppAudiCodec(uint32_t tag, bool coder);
	virtual ~VS_IppAudiCodec();
	int	 Init(WAVEFORMATEX* in);
	void Release();
	int  ConvertFunction();
	virtual int IppConvert(void* src, void* dst, void *pcmInfo, int len);
};

class VS_PcmAudioCodec : public VS_IppAudiCodec
{
public:
	VS_PcmAudioCodec(uint32_t tag, bool coder) : VS_IppAudiCodec(tag, coder) {};
	~VS_PcmAudioCodec() {};
	int IppConvert(void* src, void* dst, void *pcmInfo, int len) {memcpy(dst, src, m_GRAN); return 0;}
};

#endif
