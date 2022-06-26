
#ifndef __FF_VIDEO_CODEC_H__
#define __FF_VIDEO_CODEC_H__

#include "av_allocator.h"

#include <memory>
#include <vector>

extern "C"
{
#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif
#include "ffmpeg/libavcodec/avcodec.h"
}

enum FF_VCodecID
{
	FFVC_NONE = AVCodecID::AV_CODEC_ID_NONE,
	FFVC_H261 = AVCodecID::AV_CODEC_ID_H261,
	FFVC_H263 = AVCodecID::AV_CODEC_ID_H263,
	FFVC_H263P = AVCodecID::AV_CODEC_ID_H263P,
	FFVC_H264 = AVCodecID::AV_CODEC_ID_H264,
	FFVC_VP8 = AVCodecID::AV_CODEC_ID_VP8,
	FFVC_VP9 = AVCodecID::AV_CODEC_ID_VP9,
	FFVC_MPEG4 = AVCodecID::AV_CODEC_ID_MPEG4,
	FFVC_H265 = AVCodecID::AV_CODEC_ID_HEVC,
	FFVC_MJPEG = AVCodecID::AV_CODEC_ID_MJPEG
};

enum FF_VCodecAnnexes
{
	FFVC_ANONE		= 0x00,	///< all Annexes is 'off'
	FFVC_OBMC		= 0x01,	///< enable OBMC
	FFVC_4MV		= 0x02,	///< enable 4MV
	FFVC_AIC		= 0x04,	///< enable Advanced Intra Coding
	FFVC_DEBLOCKING	= 0x08,	///< enable Deblocking loop-filter
	FFVC_MODIFQUANT	= 0x10,	///< enable Modified Quantization
	// enable Annexes
	FFVC_F			= 0x03,	///< enable Annex F
	FFVC_I			= 0x04,	///< enable Annex I
	FFVC_J			= 0x0a,	///< enable Annex J
	FFVC_T			= 0x10	///< enable Annex T
};

class FF_VideoCodec
{
	AVCodec*		m_codec;
	AVCodecContext*	m_codecCtx;
	AVFrame*		m_picture;
	AVFrame*		m_lastDecoded;
	bool			m_isCoder;
	bool			m_isValid;
	int				m_CodecId;
	int				m_w, m_h;

	std::vector<uint8_t, vs::av_allocator<uint8_t>> m_AlignedBuff;

public:
	FF_VideoCodec(FF_VCodecID CodecId, bool IsCoder);
	~FF_VideoCodec();
	int Init(int w, int h, int annex,int framerate);
	void Release();
	int Convert(unsigned char *in, unsigned char* out, int* param);
	int SetBitrate(int bitrate);

private:
	int Encode(unsigned char* in, unsigned char* out, int* param);
	int Decode(unsigned char* in, unsigned char* out, int* param);
};

#endif /*__FF_VIDEO_CODEC_H__*/
