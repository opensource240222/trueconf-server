#pragma once

#include "AudioCodec.h"

struct lame_global_struct;
typedef struct lame_global_struct lame_global_flags;
typedef lame_global_flags *lame_t;

struct hip_global_struct;
typedef struct hip_global_struct hip_global_flags;
typedef hip_global_flags *hip_t;

class VS_MP3AudioCodec : public AudioCodec
{
protected:
	lame_t m_Lame;
	hip_t m_Hip;
	uint8_t* m_rightBuff;
public:
	VS_MP3AudioCodec(uint32_t tag, bool coder);
	virtual ~VS_MP3AudioCodec();
	int	 Init(WAVEFORMATEX* in);
	void Release();
	int Convert(uint8_t *in, uint8_t *out, uint32_t insize);
	int  ConvertFunction();
	void SetQuality(int quality);
};