/****************************************************************************
 *
 * Project: TransCoder
 *
 * $History: VS_OpusAudioCodec.h $
 *
 * *****************  Version 1  *****************
 * User: Sanufriev    Date: 2.08.12    Time: 10:24
 * Created in $/VSNA/Transcoder
 * - were added Opus audio codec
 *
 ****************************************************************************/

/****************************************************************************
 * \file VS_OpusAudioCodec.h
 * \brief Common for Opus codec base class
 ****************************************************************************/
#ifndef VS_OPUS_AUDIO_CODEC_H
#define VS_OPUS_AUDIO_CODEC_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "AudioCodec.h"

/****************************************************************************
 * Implementation VS_iSACAudioCodec
 ****************************************************************************/

class VS_OpusAudioCodec : public AudioCodec
{
protected:
	void				*m_state;
	void				*m_rp;
	unsigned char		*m_pRepacket;
	int					m_frame_size;
	int					m_samplerate;
public:
	VS_OpusAudioCodec(uint32_t tag, bool coder);
	virtual ~VS_OpusAudioCodec();
	int	 Init(WAVEFORMATEX* in);
	void Release();
	int  ConvertFunction();
	void SetQuality(int quality);
	void SetComplexity(int complexity);
};


#endif /* VS_OPUS_AUDIO_CODEC_H */