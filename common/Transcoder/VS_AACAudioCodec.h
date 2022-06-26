#pragma once

#include "AudioCodec.h"

class FFAudioCodec;

class VS_AACAudioCodec : public AudioCodec
{
protected:
	FFAudioCodec* m_Codec;

public:
	VS_AACAudioCodec(uint32_t tag, bool coder);
	virtual ~VS_AACAudioCodec();
	int	 Init(WAVEFORMATEX* in);
	void Release();
	int Convert(uint8_t *in, uint8_t *out, uint32_t insize);
	int  ConvertFunction();
	void SetQuality(int quality);
};