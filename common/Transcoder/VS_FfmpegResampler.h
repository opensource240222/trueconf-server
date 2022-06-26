#pragma once

extern "C"
{
#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libswresample/swresample.h"
}

#include "VS_AudioResamplerBase.h"

class VS_FfmpegResampler : public VS_AudioResamplerBase
{
public:
	VS_FfmpegResampler();
	virtual ~VS_FfmpegResampler();

private:
	bool Init(uint32_t oldFreq, uint32_t newFreq) override;
	bool SetRates(uint32_t oldFreq, uint32_t newFreq) override;
	int /* bytes */ InternalProcess(const void *inbuffer, void *outbuffer, uint32_t insize /* bytes */) override;

	SwrContext* m_SwrContext = nullptr;
};
