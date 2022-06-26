/****************************************************************************
 * (c) 2002-2007 Visicron Inc.  http://www.visicron.net/
 *
 * Project: TransCoder
 *
 * $History: VS_SpeexAudioCodec.h $
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 24.03.10   Time: 20:33
 * Updated in $/VSNA/Transcoder
 * - change regulate audio quality
 *
 * *****************  Version 1  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Created in $/VSNA/Transcoder
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 24.12.07   Time: 21:05
 * Updated in $/VS2005/Transcoder
 * - Speex rewrited
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 16.11.07   Time: 15:40
 * Updated in $/VS2005/Transcoder
 * - added speex audio codec (16 kHz, 13 kBit)
 * - added preprpocessing to speex
 *
 * *****************  Version 1  *****************
 * User: Sanufriev    Date: 31.08.07   Time: 16:54
 * Created in $/VS2005/Transcoder
 * - add Speex library (audio codec, echo cancellation)
 *
 ****************************************************************************/

/****************************************************************************
 * \file VS_SpeexAudioCodec.h
 * \brief Common for Speex codec base class
 ****************************************************************************/
#ifndef VS_SPEEX_AUDIO_CODEC_H
#define VS_SPEEX_AUDIO_CODEC_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "AudioCodec.h"

/****************************************************************************
 * Implementation VS_SpeexAudioCodec
 ****************************************************************************/

struct SpeexBits;

class VS_SpeexAudioCodec : public AudioCodec
{
protected:
	SpeexBits*	m_bits;
	void*		m_state;
	int			m_frame_size;
public:
	VS_SpeexAudioCodec(uint32_t tag, bool coder);
	virtual ~VS_SpeexAudioCodec();
	int	 Init(WAVEFORMATEX* in);
	void Release();
	int  ConvertFunction();
	void SetQuality(int quality);
};

#endif
