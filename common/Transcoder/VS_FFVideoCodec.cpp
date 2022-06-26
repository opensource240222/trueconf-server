#include "VS_FFVideoCodec.h"
#include "../MediaParserLib/VS_H264Parser.h"

uint32_t VS_FFVideoCodec::Formfcc(FF_VCodecID CocecId) {
	if (CocecId == FFVC_H261)
		return '162H';
	else if (CocecId == FFVC_H263)
		return '362H';
	else if (CocecId == FFVC_H263P)
		return 'P62H';
	else if (CocecId == FFVC_H264)
		return '46hv';
	else if (CocecId == FFVC_H265)
		return '56hv';
	else if (CocecId == FFVC_MPEG4)
		return 'fmp4';
	else
		return 0;
}

VS_FFVideoCodec::VS_FFVideoCodec(FF_VCodecID CocecId, bool IsCoder, int options)
	: VideoCodec(Formfcc(CocecId), IsCoder), m_ffcodec(CocecId, IsCoder), m_options(options) {};

int VS_FFVideoCodec::Init(int w, int h, uint32_t ColorMode, unsigned char sndLvl, int numThreads, unsigned int framerate)
{
	Release();
	if (ColorMode != FOURCC_I420)
		return -1;

	m_valid = m_ffcodec.Init(w, h, m_options, framerate) == 0;
	SetBitrate(m_bitrate);

	return m_valid ? 0 : -1;
}

void VS_FFVideoCodec::Release()
{
	m_valid = false;
	m_bitrate = 128;
	m_bitrate_prev = m_bitrate;
	m_ffcodec.Release();
}

int VS_FFVideoCodec::Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param) {
	if (IsCoder()) {
		UpdateBitrate();
		int prm = param->cmp.KeyFrame;
		int ret = m_ffcodec.Convert(in, out, &prm);
		param->cmp.IsKeyFrame = prm;
		return ret;
	}
	else
		return m_ffcodec.Convert(in, out, (int*)&param->dec.FrameSize);
}

bool VS_FFVideoCodec::UpdateBitrate()
{
	if (m_bitrate_prev != m_bitrate) {
		m_bitrate_prev = m_bitrate;
		m_ffcodec.SetBitrate(m_bitrate_prev);
	}
	return true;
}

int VS_VideoDecoderH264::Convert(uint8_t * in, uint8_t * out, VS_VideoCodecParam * param)
{
	int frameSize = param->dec.FrameSize;

	if (IsMultipleSpsPps(in, frameSize))
		frameSize = RemoveMultipleSpsPps(in, frameSize);

	return m_ffcodec.Convert(in, out, &frameSize);
}

bool VS_VideoDecoderH264::IsMultipleSpsPps(const uint8_t * frameData, size_t frameSize)
{
	bool hasSps = false;
	bool hasPps = false;
	const uint8_t* inScanPos = frameData;
	const uint8_t* const inEnd = frameData + frameSize;

	while (inScanPos < inEnd)
	{
		const unsigned char* nal;
		const unsigned char* nalEnd;
		unsigned int startCodeSize;

		if (NALFromBitstream_H264(inScanPos, inEnd - inScanPos, nal, nalEnd, startCodeSize) != 0)
			break;

		inScanPos = nalEnd;

		const unsigned char nalType = nal[startCodeSize] & 0x1f;

		if (nalType == 7)
		{
			if (hasSps)
				return true;
			else
				hasSps = true;
		}
		else if (nalType == 8)
		{
			if (hasPps)
				return true;
			else
				hasPps = true;
		}
	}

	return false;
}

int VS_VideoDecoderH264::RemoveMultipleSpsPps(uint8_t* frameData, size_t frameSize)
{
	bool hasSps = false;
	bool hasPps = false;
	int resultFrameSize = 0;
	const uint8_t* inScanPos = frameData;
	uint8_t* inAddPos = frameData;
	uint8_t* const inEnd = frameData + frameSize;

	while (inScanPos < inEnd)
	{
		const unsigned char* nal;
		const unsigned char* nalEnd;
		unsigned int startCodeSize;

		if (NALFromBitstream_H264(inScanPos, inEnd - inScanPos, nal, nalEnd, startCodeSize) != 0)
			break;

		inScanPos = nalEnd;

		const unsigned char nalType = nal[startCodeSize] & 0x1f;

		if (nalType == 7)
		{
			if (hasSps)
				continue;
			else
				hasSps = true;
		}
		else if (nalType == 8)
		{
			if (hasPps)
				continue;
			else
				hasPps = true;
		}

		memmove(inAddPos, nal, nalEnd - nal);
		inAddPos += nalEnd - nal;
		resultFrameSize += nalEnd - nal;
	}

	return resultFrameSize;
}
