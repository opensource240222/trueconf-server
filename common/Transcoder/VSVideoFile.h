#pragma once

extern "C"
{
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif

#include <libavformat/avformat.h>
}

#include <string>

class VSVideoFile
{
public:
	enum EVideoCodecID
	{
		VCODEC_ID_NONE = AV_CODEC_ID_NONE,

		VCODEC_ID_H263 = AV_CODEC_ID_H263,
		VCODEC_ID_H263P = AV_CODEC_ID_H263P,
		VCODEC_ID_H264 = AV_CODEC_ID_H264,
		VCODEC_ID_VP8 = AV_CODEC_ID_VP8,
		VCODEC_ID_VP9 = AV_CODEC_ID_VP9,
		VCODEC_ID_MPEG4 = AV_CODEC_ID_MPEG4,
		VCODEC_ID_RAWVIDEO = AV_CODEC_ID_RAWVIDEO
	};

	enum EAudioCodecID
	{
		ACODEC_ID_NONE = AV_CODEC_ID_NONE,

		ACODEC_ID_AAC = AV_CODEC_ID_AAC,
		ACODEC_ID_MP3 = AV_CODEC_ID_MP3,
		ACODEC_ID_OPUS = AV_CODEC_ID_OPUS,
		ACODEC_ID_PCM = AV_CODEC_ID_PCM_S16LE
	};

	enum EPixelFormat
	{
		PF_YUV420 = AVPixelFormat::AV_PIX_FMT_YUV420P,
		PF_RGB24 = AVPixelFormat::AV_PIX_FMT_RGB24,
		PF_RGBA = AVPixelFormat::AV_PIX_FMT_RGBA
	};

	struct SVideoInfo
	{
		int Width;
		int Height;
		EPixelFormat PixFormat;

		int FPS;

		EVideoCodecID CodecID;
	};

	struct SAudioInfo
	{
		int SampleRate;
		int NumChannels;

		int BitsPerSample;

		EAudioCodecID CodecID;
	};

	bool GetVideoFormat(SVideoInfo& videoInfo) const;
	bool GetAudioFormat(SAudioInfo& audioInfo) const;

	unsigned int GetCurrentVideoTime() const;
	unsigned int GetCurrentAudioTime() const;

	uint64_t GetSize() const;

protected:
	VSVideoFile();
	~VSVideoFile();

	AVFormatContext* m_FormatContext;

	AVStream* m_AudioStream;
	AVStream* m_VideoStream;

	unsigned int m_CurrVideoTime;
	unsigned int m_CurrAudioTime;
	unsigned int m_FPS;
};
