#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <utility>
#include "FFCodec.h"
#include "AVLockMgrInit.h"

#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif

extern "C"
{
#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
}

#define _MM_ALIGN16

FF_VideoCodec::FF_VideoCodec(FF_VCodecID CodecId, bool IsCoder)
{
	AVLockMgrInit();

	avcodec_register_all();
	m_isCoder = IsCoder;
	m_CodecId = CodecId;
	if (m_isCoder)
		m_codec = avcodec_find_encoder((AVCodecID)m_CodecId);
	else
		m_codec = avcodec_find_decoder((AVCodecID)m_CodecId);
	m_codecCtx = avcodec_alloc_context3(m_codec);

	m_picture = av_frame_alloc();
	m_lastDecoded = av_frame_alloc();

	m_w = 0;
	m_h = 0;
	m_isValid = false;
}

FF_VideoCodec::~FF_VideoCodec()
{
	Release();
	if (m_codecCtx) {
		av_free(m_codecCtx);
		m_codecCtx = 0;
	}
	if (m_picture) {
		av_frame_free(&m_picture);
		m_picture = 0;
	}
	if (m_lastDecoded) {
		av_frame_free(&m_lastDecoded);
		m_lastDecoded = 0;
	}

	m_AlignedBuff.clear();
}

int FF_VideoCodec::Init(int w, int h, int annex,int framerate)
{
	m_codecCtx->width = w;
	m_codecCtx->height = h;
	m_w = w;
	m_h = h;
	if (m_isCoder) {
		//		c->bit_rate = bitrate * 1024;
		//		c->bit_rate_tolerance = c->bit_rate * 10;

		/*c->bit_rate = 1000 * 1000;
		c->bit_rate_tolerance = c->bit_rate * framerate;*/

		m_codecCtx->time_base.num = 1;
		m_codecCtx->time_base.den = framerate;
		m_codecCtx->gop_size = 300;// framerate * 15; /* emit one intra frame every x frames */
		m_codecCtx->max_b_frames = 0;
		m_codecCtx->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;
		m_codecCtx->qmin = 3;     // min quant

		if (m_CodecId == AVCodecID::AV_CODEC_ID_H263P) {
			if (annex & FFVC_4MV)
				m_codecCtx->flags |= AV_CODEC_FLAG_4MV;
			if (annex & FFVC_AIC)
				m_codecCtx->flags |= AV_CODEC_FLAG_AC_PRED;  // abramenko, new aic
			if (annex & FFVC_DEBLOCKING)
				m_codecCtx->flags |= AV_CODEC_FLAG_LOOP_FILTER;
		}
		m_picture->format = m_codecCtx->pix_fmt;
		m_picture->width = m_codecCtx->width;
		m_picture->height = m_codecCtx->height;

		if (av_image_alloc(m_picture->data, m_picture->linesize, m_codecCtx->width, m_codecCtx->height, m_codecCtx->pix_fmt, 32) < 0) {
			return -1;
		}
	} else {
		//c->pix_fmt = PIX_FMT_YUV420P;
		//if(codec->capabilities&CODEC_CAP_TRUNCATED)
		//	c->flags|= CODEC_FLAG_TRUNCATED; /* we dont send complete frames */
	}
	if (avcodec_open2(m_codecCtx, m_codec, 0) < 0)
		return -1;

	m_isValid = true;
	return 0;
}

void FF_VideoCodec::Release()
{
	if (m_isValid) {
		avcodec_close(m_codecCtx);

		if (m_isCoder)
			av_freep(&m_picture->data[0]);

		m_isValid = false;
	}

}

float GetValueOfScale(float x)
{
	float value = 0;
	if ((x >= 0) && (x <= 5.86))
		value = -0.0001 * pow(x, 5) + 0.0027 * pow(x, 4) - 0.0215 * pow(x, 3) + 0.0282 * pow(x, 2) + 0.405 * x + 1.0053;
	else
		value = 2.4;
	return value;
}

int FF_VideoCodec::SetBitrate(int bitrate) // SAnufriev
{
	if (!m_isValid)
		return -1;

	if (m_isCoder) {
		m_codecCtx->bit_rate = bitrate * 1024;
		float density=0;
		if((m_codecCtx->width*m_codecCtx->height)!=0)
		density=(float)m_codecCtx->bit_rate/(float)(m_codecCtx->width*m_codecCtx->height);     //
		m_codecCtx->i_quant_factor= GetValueOfScale(density);               //reduce diff between I and P frame size
		m_codecCtx->bit_rate_tolerance = m_codecCtx->bit_rate * m_codecCtx->time_base.den;
		if (avcodec_set_options(m_codecCtx, m_codec))
			return -1;
	}
	return 0;
}

int FF_VideoCodec::Convert(unsigned char *in, unsigned char* out, int* param)
{
	if (!m_isValid)
		return -1;

	int size = m_codecCtx->width * m_codecCtx->height;
	if (m_isCoder) {
		return Encode(in, out, param);
	} else {
		return Decode(in, out, param);
	}
}

int FF_VideoCodec::Encode(unsigned char* in, unsigned char* out, int * param)
{
	int size = m_codecCtx->width * m_codecCtx->height;

	for (int i = 0; i < m_codecCtx->height; i++)
	{
		uint8_t* dst = m_picture->data[0] + m_picture->linesize[0] * i;
		const uint8_t* src = in + m_codecCtx->width * i;

		memcpy(dst, src, m_codecCtx->width);
	}

	for (int i = 0; i < m_codecCtx->height / 2; i++)
	{
		uint8_t* dst = m_picture->data[1] + m_picture->linesize[1] * i;
		const uint8_t* src = in + size + m_codecCtx->width / 2 * i;

		memcpy(dst, src, m_codecCtx->width / 2);

		dst = m_picture->data[2] + m_picture->linesize[2] * i;
		src = in + size + size / 4 + m_codecCtx->width / 2 * i;

		memcpy(dst, src, m_codecCtx->width / 2);
	}

	m_codecCtx->opaque = (void*)*param;

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = out;
	pkt.size = 512000;

	int gotPicture = 0;
	int ret = avcodec_encode_video2(m_codecCtx, &pkt, m_picture, &gotPicture);

	*param = m_codecCtx->opaque != nullptr;

	return pkt.size;
}

int FF_VideoCodec::Decode(unsigned char* in, unsigned char* out, int* param)
{
	AVPacket avpkt;
	int got_picture, i = *param;

	avpkt.size = i + AV_INPUT_BUFFER_PADDING_SIZE;
	av_init_packet(&avpkt);
	avpkt.size = i;

	m_AlignedBuff.resize(i + AV_INPUT_BUFFER_PADDING_SIZE);
	memcpy(m_AlignedBuff.data(), in, i);
	avpkt.data = m_AlignedBuff.data();

	for (int idx = i; idx < m_AlignedBuff.size(); idx++) /// hack for h261 and bitstream for other codecs
		m_AlignedBuff[idx] = 0;

	int len = avcodec_decode_video2(m_codecCtx, m_picture, &got_picture, &avpkt);       //new version
	av_free_packet(&avpkt);
	if (len < 0)
		return -2;

	if (got_picture)
		std::swap(m_picture, m_lastDecoded);

	if (m_lastDecoded->data[0])
	{
		for (i = 0; i < m_codecCtx->height; i++, out += m_codecCtx->width)
			memcpy(out, m_lastDecoded->data[0] + i * m_lastDecoded->linesize[0], m_codecCtx->width);
		for (i = 0; i < m_codecCtx->height / 2; i++, out += m_codecCtx->width / 2)
			memcpy(out, m_lastDecoded->data[1] + i * m_lastDecoded->linesize[1], m_codecCtx->width / 2);
		for (i = 0; i < m_codecCtx->height / 2; i++, out += m_codecCtx->width / 2)
			memcpy(out, m_lastDecoded->data[2] + i * m_lastDecoded->linesize[2], m_codecCtx->width / 2);
	}

	if (got_picture)
		return m_codecCtx->width * m_codecCtx->height * 3 / 2;
	else
		return -1;
}
