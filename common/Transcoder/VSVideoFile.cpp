#include "VSVideoFile.h"

VSVideoFile::VSVideoFile()
{
	m_FormatContext = nullptr;
	m_AudioStream = nullptr;
	m_VideoStream = nullptr;
	m_CurrVideoTime = 0;
	m_CurrAudioTime = 0;
	m_FPS = 0;

	av_register_all();
}

VSVideoFile::~VSVideoFile()
{
}

bool VSVideoFile::GetVideoFormat(SVideoInfo& videoInfo) const
{
	if (!m_VideoStream)
		return false;

	videoInfo.Width = m_VideoStream->codecpar->width;
	videoInfo.Height = m_VideoStream->codecpar->height;

	videoInfo.FPS = m_VideoStream->r_frame_rate.num / m_VideoStream->r_frame_rate.den;
	videoInfo.CodecID = EVideoCodecID(m_VideoStream->codecpar->codec_id);

	if (videoInfo.CodecID == VCODEC_ID_RAWVIDEO)
		videoInfo.PixFormat = PF_YUV420;
	else
		videoInfo.PixFormat = (EPixelFormat)m_VideoStream->codecpar->format;

	return true;
}

bool VSVideoFile::GetAudioFormat(SAudioInfo& audioInfo) const
{
	if (!m_AudioStream)
		return false;

	audioInfo.BitsPerSample = av_get_bits_per_sample(m_AudioStream->codecpar->codec_id);
	audioInfo.NumChannels = m_AudioStream->codecpar->channels;
	audioInfo.SampleRate = m_AudioStream->codecpar->sample_rate;
	audioInfo.CodecID = EAudioCodecID(m_AudioStream->codecpar->codec_id);

	return true;
}

unsigned int VSVideoFile::GetCurrentVideoTime() const
{
	return m_CurrVideoTime;
}

unsigned int VSVideoFile::GetCurrentAudioTime() const
{
	return m_CurrAudioTime;
}

uint64_t VSVideoFile::GetSize() const
{
	const auto sz = m_FormatContext != nullptr ? avio_size(m_FormatContext->pb) : 0;
	return sz >= 0 ? sz : 0;
}
