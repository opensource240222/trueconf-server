#pragma once

#include "VideoCodec.h"
#include "extlibs/openh264/include/wels/codec_api.h"

class VS_OpenH264VideoCodec : public VideoCodec
{
private:
	ISVCEncoder* m_Encoder;
	SEncParamExt m_EncodeParam;

protected:
	bool			UpdateBitrate();
	virtual void	FillDefaultParam(SEncParamExt& param);

public:
	VS_OpenH264VideoCodec(int CodecId, bool IsCoder);
	~VS_OpenH264VideoCodec();

	int		Init(int w, int h, uint32_t ColorMode = FOURCC_I420, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10) override;
	void	Release() override;

	int		Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param) override;

	bool	SetCoderOption(void *param) override;
};
