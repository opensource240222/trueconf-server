#pragma once

#include "VideoCodec.h"
#include "FFCodec.h"

class VS_FFVideoCodec : public VideoCodec
{
protected:
	static uint32_t Formfcc(FF_VCodecID CocecId);
	FF_VideoCodec	m_ffcodec;
	int				m_options;
	bool			UpdateBitrate();
public:
	VS_FFVideoCodec(FF_VCodecID CocecId, bool IsCoder, int options = 0);
	int Init(int w, int h, uint32_t ColorMode = FOURCC_I420, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10);
	void Release();
	int Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param);
};

class VS_VideoCoderH261 : public VS_FFVideoCodec {
public: VS_VideoCoderH261() : VS_FFVideoCodec(FFVC_H261, true) {}
};
class VS_VideoDecoderH261 : public VS_FFVideoCodec {
public: VS_VideoDecoderH261() : VS_FFVideoCodec(FFVC_H261, false) {}
};

class VS_VideoCoderH263 : public VS_FFVideoCodec {
public: VS_VideoCoderH263() : VS_FFVideoCodec(FFVC_H263, true) {}
};
class VS_VideoDecoderH263 : public VS_FFVideoCodec {
public: VS_VideoDecoderH263() : VS_FFVideoCodec(FFVC_H263, false) {}
};

class VS_VideoCoderH263P : public VS_FFVideoCodec
{
public:
	VS_VideoCoderH263P(int annexes = 0) : VS_FFVideoCodec(FFVC_H263P, true, annexes) {}
};
class VS_VideoDecoderH263P : public VS_FFVideoCodec {
public: VS_VideoDecoderH263P() : VS_FFVideoCodec(FFVC_H263, false) {}
};

class VS_VideoDecoderH264 : public VS_FFVideoCodec
{
public:
	VS_VideoDecoderH264() : VS_FFVideoCodec(FFVC_H264, false) {}

	int	Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param);

	bool IsMultipleSpsPps(const uint8_t* frameData, size_t frameSize);
	int RemoveMultipleSpsPps(uint8_t* frameData, size_t frameSize);
};

class VS_VideoDecoderH265 : public VS_FFVideoCodec {
public: VS_VideoDecoderH265() : VS_FFVideoCodec(FFVC_H265, false) {}
};

class VS_VideoCoderMPEG4 : public VS_FFVideoCodec {
public: VS_VideoCoderMPEG4() : VS_FFVideoCodec(FFVC_MPEG4, true) {}
};
class VS_VideoDecoderMPEG4 : public VS_FFVideoCodec {
public: VS_VideoDecoderMPEG4() : VS_FFVideoCodec(FFVC_MPEG4, false) {}
};

class VS_VideoDecoderVP8 : public VS_FFVideoCodec {
public: VS_VideoDecoderVP8() : VS_FFVideoCodec(FFVC_VP8, false) {}
};