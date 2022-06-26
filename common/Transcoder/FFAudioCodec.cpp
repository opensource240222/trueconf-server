#include <cstdlib>
#include "FFAudioCodec.h"
#include "AVLockMgrInit.h"

extern "C"
{
#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif
#include "libavutil/channel_layout.h"
#include "libavutil/samplefmt.h"
}

FFAudioCodec::FFAudioCodec()
{
	m_CodecContext = nullptr;
	m_Frame = nullptr;

	m_TmpIn = nullptr;
	m_TmpInSize = 0;
	m_TmpInCapacity = 0;
}

FFAudioCodec::~FFAudioCodec()
{
	Release();
}

bool FFAudioCodec::Init(int frequency, bool isCoder, int bitrate)
{
	Release();

	AVCodec* codec = nullptr;

	avcodec_register_all();

	if (isCoder)
		codec = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_AAC);
	else
		codec = avcodec_find_decoder(AVCodecID::AV_CODEC_ID_AAC);

	if (!codec)
		return false;

	m_CodecContext = avcodec_alloc_context3(codec);

	if (!m_CodecContext)
		return false;

	m_CodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

	/* put sample parameters */
	m_CodecContext->bit_rate = bitrate;

	/* check that the encoder supports s16 pcm input */
	m_CodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;

	/* select other audio parameters supported by the encoder */
	m_CodecContext->sample_rate = frequency;
	m_CodecContext->channel_layout = AV_CH_LAYOUT_MONO;
	m_CodecContext->channels = 1;

	if (avcodec_open2(m_CodecContext, codec, NULL) < 0)
	{
		avcodec_close(m_CodecContext);

		m_CodecContext = nullptr;

		return false;
	}

	m_Frame = av_frame_alloc();

	if (!m_Frame)
		return false;

	m_Frame->nb_samples = m_CodecContext->frame_size;
	m_Frame->format = m_CodecContext->sample_fmt;
	m_Frame->channel_layout = m_CodecContext->channel_layout;

	return true;
}

void FFAudioCodec::Release()
{
	delete[] m_TmpIn;

	if (m_CodecContext)
	{
		avcodec_close(m_CodecContext);
		av_free(m_CodecContext);
	}

	if (m_Frame)
		av_frame_free(&m_Frame);

	m_CodecContext = nullptr;
	m_Frame = nullptr;

	m_TmpIn = nullptr;
	m_TmpInSize = 0;
	m_TmpInCapacity = 0;
}

int FFAudioCodec::Convert(char* in, char* out, int insize)
{
	int got_output = 0;
	AVPacket pkt;

	if (av_codec_is_encoder(m_CodecContext->codec))
	{
		if (m_TmpInCapacity - m_TmpInSize < (size_t)insize) // if aviable free space small than input buffer
		{
			char* newTmpIn = new char[m_TmpInSize + insize];

			if (!newTmpIn)
				return -1;

			memcpy(newTmpIn, m_TmpIn, m_TmpInSize);

			delete[] m_TmpIn;

			m_TmpIn = newTmpIn;
			m_TmpInCapacity = m_TmpInSize + insize;
		}

		memcpy(m_TmpIn + m_TmpInSize, in, insize);
		m_TmpInSize += insize;

		size_t totalCompressed = 0;
		size_t tmpInUsed = 0;
		size_t neededFrameSize;

		neededFrameSize = av_samples_get_buffer_size(NULL, m_CodecContext->channels, m_CodecContext->frame_size,
			m_CodecContext->sample_fmt, 0);

		av_init_packet(&pkt);

		pkt.data = NULL; // packet data will be allocated by the encoder
		pkt.size = 0;

		while (m_TmpInSize - tmpInUsed >= neededFrameSize)
		{
			avcodec_fill_audio_frame(m_Frame, m_CodecContext->channels,
				m_CodecContext->sample_fmt, (const uint8_t*)m_TmpIn + tmpInUsed, neededFrameSize, 0);

			if (avcodec_encode_audio2(m_CodecContext, &pkt, m_Frame, &got_output) < 0)
				return -1;

			if (got_output)
			{
				memcpy(out + totalCompressed, pkt.data, pkt.size);

				totalCompressed += pkt.size;

				av_free_packet(&pkt);
			}

			tmpInUsed += neededFrameSize;
		}

		if (tmpInUsed)
		{
			memmove(m_TmpIn, m_TmpIn + tmpInUsed, m_TmpInSize - tmpInUsed);

			m_TmpInSize -= tmpInUsed;
		}

		return totalCompressed;
	}
	else
	{
		size_t totalDecompressed = 0;

		av_init_packet(&pkt);

		pkt.data = (uint8_t*)in;
		pkt.size = insize;

		while (pkt.size > 0)
		{
			int consumed = avcodec_decode_audio4(m_CodecContext, m_Frame, &got_output, &pkt);

			if (consumed < 0)
				return -1;

			if (got_output)
			{
				int decompressed = av_samples_get_buffer_size(NULL, m_CodecContext->channels,
					m_Frame->nb_samples, m_CodecContext->sample_fmt, 1);

				decompressed /= m_Frame->channels;

				memcpy(out + totalDecompressed, m_Frame->data[0], decompressed);

				totalDecompressed += decompressed;
				pkt.size -= consumed;
				pkt.data += consumed;
			}
			else
				break;
		}

		return totalDecompressed;
	}

	return 0;
}
