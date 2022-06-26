/****************************************************************************
 *
 * Project: TransCoder
 *
 * $History: VS_iSACAudioCodec.h $
 *
 * *****************  Version 1  *****************
 * User: Sanufriev    Date: 2.07.12    Time: 18:11
 * Created in $/VSNA/Transcoder
 * - add iSAC codec
 *
 ****************************************************************************/

/****************************************************************************
 * \file VS_iSACAudioCodec.h
 * \brief Common for iSAC codec base class
 ****************************************************************************/
#ifndef VS_ISAC_AUDIO_CODEC_H
#define VS_ISAC_AUDIO_CODEC_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "AudioCodec.h"

/****************************************************************************
 * Implementation VS_iSACAudioCodec
 ****************************************************************************/

struct WebRtcISACStruct;

class VS_iSACAudioCodec : public AudioCodec
{
protected:
	WebRtcISACStruct*	m_state;
	int					m_frame_size;
public:
	VS_iSACAudioCodec(uint32_t tag, bool coder);
	virtual ~VS_iSACAudioCodec();
	int	 Init(WAVEFORMATEX* in);
	void Release();
	int  ConvertFunction();
	void SetQuality(int quality);
};


#endif /* VS_ISAC_AUDIO_CODEC_H */