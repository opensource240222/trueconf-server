#pragma once

#include "VideoCodec.h"
#include <Windows.h>
#include <Vfw.h>

class VS_VfwVideoCodec : public VideoCodec
{
protected:
	ICINFO				m_icinfo;
	uint32_t				m_FrameNum;	// counter
	HIC					m_hic;
	BITMAPINFO			*m_bmRGB, *m_bmfcc;
	BITMAPINFOHEADER	*m_bmhRGB, *m_bmhfcc;

public:
	VS_VfwVideoCodec(uint32_t fcc, bool coder);
	~VS_VfwVideoCodec();

	virtual int Init(int w, int h, uint32_t ColorMode = BI_RGB, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10);
	virtual void Release();
	virtual int Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param);
	virtual bool SetCoderOption(uint32_t param);
	virtual bool SetBitrate(uint32_t param);
};
