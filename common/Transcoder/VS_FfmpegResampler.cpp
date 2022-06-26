#include "VS_FfmpegResampler.h"

VS_FfmpegResampler::VS_FfmpegResampler()
{
}

VS_FfmpegResampler::~VS_FfmpegResampler()
{
	swr_free(&m_SwrContext);
}

bool VS_FfmpegResampler::Init(uint32_t oldFreq, uint32_t newFreq)
{
	swr_free(&m_SwrContext);

	m_SwrContext = swr_alloc();

	av_opt_set_channel_layout(m_SwrContext, "in_channel_layout", AV_CH_LAYOUT_MONO, 0);
	av_opt_set_channel_layout(m_SwrContext, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
	av_opt_set_sample_fmt(m_SwrContext, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	av_opt_set_sample_fmt(m_SwrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int(m_SwrContext, "in_sample_rate", oldFreq, 0);
	av_opt_set_int(m_SwrContext, "out_sample_rate", newFreq, 0);

	if (swr_init(m_SwrContext) < 0)
		return false;

	return true;
}

bool VS_FfmpegResampler::SetRates(uint32_t oldFreq, uint32_t newFreq)
{
	return Init(oldFreq, newFreq);
}

int /* bytes */ VS_FfmpegResampler::InternalProcess(const void *inbuffer, void *outbuffer, uint32_t insize /* bytes */)
{
	int inSamples = insize / av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
	int maxOutSamples = (int)((double)m_outFrequency / m_inFrequency * inSamples * 1.1);

	int outSamples = swr_convert(m_SwrContext,
		(uint8_t**)&outbuffer, maxOutSamples,
		(const uint8_t**)&inbuffer, inSamples);

	return outSamples * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
}
