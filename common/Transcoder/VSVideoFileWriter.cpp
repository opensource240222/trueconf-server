#include "VSVideoFileWriter.h"

#include <cassert>

VSVideoFileWriter::VSVideoFileWriter()
{
}

VSVideoFileWriter::~VSVideoFileWriter()
{
	Release();
}

bool VSVideoFileWriter::Init(std::string fileName)
{
	if (avformat_alloc_output_context2(&m_FormatContext, NULL, NULL, fileName.c_str()) >= 0)
	{
		if (avio_open(&m_FormatContext->pb, fileName.c_str(), AVIO_FLAG_WRITE) >= 0)
		{
			return true;
		}
		else
		{
			avformat_free_context(m_FormatContext);
			m_FormatContext = nullptr;
		}
	}

	return false;
}

uint64_t VSVideoFileWriter::Release()
{
	uint64_t fsize = 0;
	if (m_FormatContext)
	{
		if (m_HeaderWrited)
			av_write_trailer(m_FormatContext);

		fsize = GetSize();
		avio_closep(&m_FormatContext->pb);
		avformat_free_context(m_FormatContext);
	}

	m_FormatContext = nullptr;
	m_AudioStream = nullptr;
	m_VideoStream = nullptr;
	m_CurrVideoTime = 0;
	m_CurrAudioTime = 0;
	m_HeaderWrited = false;
	return fsize;
}

static void AddH264Extradata(AVCodecParameters* par)
{
	const uint8_t h264_extradata_320_180[] = {
		0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0xc0, 0x1f, 0x8c, 0x68, 0x14, 0x19, 0x79, 0xe0, 0x1e, 0x11, 0x08, 0xd4,
		0x00, 0x00, 0x00, 0x01, 0x68, 0xce, 0x3c, 0x80
	};
	const uint8_t h264_extradata_640_360[] = {
		0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0xc0, 0x1f, 0x8c, 0x68, 0x0a, 0x02, 0xf7, 0x96, 0x01, 0xe1, 0x10, 0x8d, 0x40,
		0x00, 0x00, 0x00, 0x01, 0x68, 0xce, 0x3c, 0x80
	};
	const uint8_t h264_extradata_864_480[] = {
		0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0xc0, 0x1f, 0x8c, 0x68, 0x0d, 0x83, 0xd2, 0x01, 0xe1, 0x10, 0x8d, 0x40,
		0x00, 0x00, 0x00, 0x01, 0x68, 0xce, 0x3c, 0x80
	};
	const uint8_t h264_extradata_1280_720[] = {
		0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0xc0, 0x1f, 0x8c, 0x68, 0x05, 0x00, 0x5b, 0x20, 0x1e, 0x11, 0x08, 0xd4,
		0x00, 0x00, 0x00, 0x01, 0x68, 0xce, 0x3c, 0x80
	};
	const uint8_t h264_extradata_1920_1080[] = {
		0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0xc0, 0x28, 0x8c, 0x68, 0x07, 0x80, 0x22, 0x5e, 0x58, 0x07, 0x84, 0x42, 0x35,
		0x00, 0x00, 0x00, 0x01, 0x68, 0xce, 0x3c, 0x80
	};

	const uint8_t* extradata = nullptr;

	if (par->width == 320 && par->height == 180)
	{
		par->extradata_size = sizeof(h264_extradata_320_180);
		extradata = h264_extradata_320_180;
	}
	else if (par->width == 640 && par->height == 360)
	{
		par->extradata_size = sizeof(h264_extradata_640_360);
		extradata = h264_extradata_640_360;
	}
	else if (par->width == 864 && par->height == 480)
	{
		par->extradata_size = sizeof(h264_extradata_864_480);
		extradata = h264_extradata_864_480;
	}
	else if (par->width == 1280 && par->height == 720)
	{
		par->extradata_size = sizeof(h264_extradata_1280_720);
		extradata = h264_extradata_1280_720;
	}
	else if (par->width == 1920 && par->height == 1080)
	{
		par->extradata_size = sizeof(h264_extradata_1920_1080);
		extradata = h264_extradata_1920_1080;
	}

	par->extradata = (uint8_t*)av_mallocz(par->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
	memcpy(par->extradata, extradata, par->extradata_size);
}

bool VSVideoFileWriter::SetVideoFormat(SVideoInfo videoInfo)
{
	if (!m_FormatContext)
		return false;

	m_VideoStream = avformat_new_stream(m_FormatContext, nullptr);

	if (!m_VideoStream)
		return false;

	AVCodecParameters* codecContext = m_VideoStream->codecpar;
	m_FPS = videoInfo.FPS;

	m_VideoStream->time_base = av_make_q(1, m_FPS);

	codecContext->codec_type = AVMediaType::AVMEDIA_TYPE_VIDEO;
	codecContext->codec_id = (AVCodecID)videoInfo.CodecID;
	codecContext->width = videoInfo.Width;
	codecContext->height = videoInfo.Height;
	codecContext->format = (AVPixelFormat)videoInfo.PixFormat;

	if (videoInfo.CodecID == VCODEC_ID_H264)
		AddH264Extradata(codecContext); // for QuickTime
	else if (videoInfo.CodecID == VCODEC_ID_RAWVIDEO)
		codecContext->codec_tag = avcodec_pix_fmt_to_codec_tag(AVPixelFormat(codecContext->format)); // for .avi

	return true;
}

bool VSVideoFileWriter::SetAudioFormat(SAudioInfo audioInfo)
{
	if (!m_FormatContext)
		return false;

	m_AudioStream = avformat_new_stream(m_FormatContext, nullptr);

	if (!m_AudioStream)
		return false;

	AVCodecParameters* codecContext = m_AudioStream->codecpar;

	m_AudioStream->time_base = { 1, 1000 };

	codecContext->codec_type = AVMediaType::AVMEDIA_TYPE_AUDIO;
	codecContext->codec_id = AVCodecID(audioInfo.CodecID);
	codecContext->channels = audioInfo.NumChannels;
	codecContext->channel_layout = av_get_default_channel_layout(audioInfo.NumChannels);
	codecContext->sample_rate = audioInfo.SampleRate;
	codecContext->bit_rate = audioInfo.SampleRate * audioInfo.BitsPerSample; // for pcm
	codecContext->bits_per_raw_sample = audioInfo.BitsPerSample;
	codecContext->format = AV_SAMPLE_FMT_S16P;

	if (audioInfo.CodecID == ACODEC_ID_AAC)
	{
		const uint8_t aac_extradata[] = { 0x14, 0x08, 0x56, 0xe5, 0x00 };

		codecContext->extradata_size = sizeof(aac_extradata);
		codecContext->extradata = (uint8_t*)av_mallocz(codecContext->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
		memcpy(codecContext->extradata, aac_extradata, sizeof(aac_extradata));

		codecContext->format = AV_SAMPLE_FMT_FLTP;
	}

	avcodec_parameters_to_context(m_AudioStream->codec, m_AudioStream->codecpar);

	return true;
}

bool VSVideoFileWriter::WriteHeader()
{
	if (!m_FormatContext)
		return false;

	if (avformat_write_header(m_FormatContext, NULL) == 0)
		m_HeaderWrited = true;

	return m_HeaderWrited;
}

bool VSVideoFileWriter::WriteVideo(char* data, int size, bool isKey, int videoInterval)
{
	if (!m_VideoStream)
		return false;
	if (!data || size <= 0)
		return false;

	AVPacket pkt;
	uint8_t* pktdata;

	pktdata = (uint8_t*)av_memdup((void*)data, size);

	av_init_packet(&pkt);

	if (av_packet_from_data(&pkt, pktdata, size) < 0)
		return false;

	if (isKey)
		pkt.flags |= AV_PKT_FLAG_KEY;

	pkt.stream_index = m_VideoStream->index;

	pkt.pts = av_rescale_q(m_CurrVideoTime, av_make_q(1, 1000), m_VideoStream->time_base);
	pkt.dts = pkt.pts;

	if (pkt.dts == m_VideoStream->cur_dts)
	{
		pkt.pts++;
		pkt.dts++;

		m_CurrVideoTime = av_rescale_q(pkt.pts, m_VideoStream->time_base, av_make_q(1, 1000));
	}

	pkt.dts = pkt.pts;

	if (videoInterval < 0)
		m_CurrVideoTime += int(1000.0f * 1.0f / float(m_FPS));
	else
		m_CurrVideoTime += videoInterval;

	assert(m_FormatContext != nullptr);
	int res = av_write_frame(m_FormatContext, &pkt);

	av_packet_unref(&pkt);

	return res >= 0;
}

bool VSVideoFileWriter::WriteVideoTimeAbs(char* data, int size, bool isKey, int absoluteTime)
{
	if (!m_VideoStream)
		return false;
	if (!data || size <= 0)
		return false;

	if ((unsigned int)absoluteTime < m_CurrVideoTime)
		return false;

	m_CurrVideoTime = absoluteTime;

	return WriteVideo(data, size, isKey, 0);
}

bool VSVideoFileWriter::WriteAudioSamples(char* data, int size, int samples)
{
	if (!m_AudioStream)
		return false;
	if (!data || size <= 0)
		return false;

	if (EAudioCodecID(m_AudioStream->codecpar->codec_id) == ACODEC_ID_MP3)
	{
		static const int bitrateTable[] = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 };

		int bitrateIndex = int(((uint8_t*)data)[2] >> 4); // bits 16 - 17 of first header

		int bitrate = bitrateTable[bitrateIndex] * 1000 / 8; // bytes per second

		samples = (size * m_AudioStream->codecpar->sample_rate) / (bitrate);
	}

	if (samples < 0)
		samples = m_AudioStream->codec->frame_size;

	int audioInterval = samples * 1000 / m_AudioStream->codecpar->sample_rate;

	return WriteAudioTime(data, size, audioInterval);
}

bool VSVideoFileWriter::WriteAudioTime(char* data, int size, int audioInterval)
{
	if (!m_AudioStream)
		return false;
	if (!data || size <= 0)
		return false;

	AVPacket pkt;
	uint8_t* pktdata;

	pktdata = (uint8_t*)av_memdup((void*)data, size);

	av_init_packet(&pkt);

	if (av_packet_from_data(&pkt, pktdata, size) < 0)
		return false;

	pkt.stream_index = m_AudioStream->index;
	pkt.dts = pkt.pts = av_rescale_q(m_CurrAudioTime, av_make_q(1, 1000), m_AudioStream->time_base);

	assert(m_FormatContext != nullptr);
	int res = av_write_frame(m_FormatContext, &pkt);

	av_packet_unref(&pkt);

	m_CurrAudioTime += audioInterval;

	return res >= 0;
}

bool VSVideoFileWriter::WriteAudioTimeAbs(char* data, int size, int absoluteTime)
{
	if (!m_AudioStream)
		return false;
	if (!data || size <= 0)
		return false;

	if ((unsigned int)absoluteTime < m_CurrAudioTime)
		return false;

	m_CurrAudioTime = absoluteTime;

	return WriteAudioTime(data, size, 0);
}
