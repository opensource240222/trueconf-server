#ifdef _WIN32
#include "VS_VfwVideoCodec.h"

VS_VfwVideoCodec::VS_VfwVideoCodec(uint32_t fcc, bool coder)
	: VideoCodec(fcc, coder)
{
	memset(&m_icinfo, 0, sizeof(ICINFO));
	m_hic = 0;
}

VS_VfwVideoCodec::~VS_VfwVideoCodec()
{
	Release();
}

int VS_VfwVideoCodec::Init(int w, int h, uint32_t ColorMode, unsigned char sndLvl, int numThreads, unsigned int framerate)
{
	Release();

	m_bmRGB = (BITMAPINFO*)malloc(sizeof(BITMAPINFO));
	memset(m_bmRGB, 0, sizeof(BITMAPINFO));
	m_bmfcc = (BITMAPINFO*)malloc(sizeof(BITMAPINFO));
	memset(m_bmfcc, 0, sizeof(BITMAPINFO));

	m_bmhRGB = &(m_bmRGB->bmiHeader);
	m_bmhfcc = &(m_bmfcc->bmiHeader);

	m_bmhRGB->biSize = sizeof(BITMAPINFOHEADER);
	m_bmhRGB->biPlanes = 1;
	m_bmhRGB->biCompression = ColorMode;
	if (ColorMode == BI_RGB) {
		m_bmhRGB->biBitCount = 24;
		m_bmhRGB->biSizeImage = w * h * 3;
	}
	else if (ColorMode == FOURCC_I420) {
		m_bmhRGB->biBitCount = 12;
		m_bmhRGB->biSizeImage = w * h * 3 / 2;
	}
	else return -1;

	m_bmhfcc->biSize = sizeof(BITMAPINFOHEADER);
	m_bmhfcc->biWidth = w;
	m_bmhfcc->biHeight = h;
	m_bmhfcc->biPlanes = 1;
	m_bmhfcc->biCompression = GetFcc();

	if (IsCoder()) {
		m_hic = ICLocate(ICTYPE_VIDEO, GetFcc(), m_bmhRGB, m_bmhfcc, ICMODE_COMPRESS);
		ICGetInfo(m_hic, &m_icinfo, sizeof(ICINFO));
		if (m_hic) {
			m_valid = ICCompressQuery(m_hic, m_bmRGB, m_bmfcc) == ICERR_OK;
			ICCOMPRESSFRAMES icofr;
			memset(&icofr, 0, sizeof(ICCOMPRESSFRAMES));
			icofr.dwRate = framerate;
			icofr.dwScale = 1;
			icofr.lpbiInput = m_bmhRGB;
			icofr.lpbiOutput = m_bmhfcc;
			m_valid &= ICSendMessage(m_hic, ICM_COMPRESS_FRAMES_INFO, (uint32_t)(LPVOID)&icofr, sizeof(ICCOMPRESSFRAMES)) == ICERR_OK;
			m_valid &= ICCompressBegin(m_hic, m_bmRGB, m_bmfcc) == ICERR_OK;
		}
	}
	else {
		m_hic = ICLocate('CDIV', GetFcc(), m_bmhfcc, m_bmhRGB, ICMODE_DECOMPRESS);
		ICGetInfo(m_hic, &m_icinfo, sizeof(ICINFO));
		if (m_hic) {
			m_valid = ICDecompressQuery(m_hic, m_bmfcc, m_bmRGB) == ICERR_OK;
			m_valid &= ICDecompressBegin(m_hic, m_bmfcc, m_bmRGB) == ICERR_OK;
		}
	}
	return m_valid ? 0 : -1;
}

void VS_VfwVideoCodec::Release()
{
	m_valid = false;
	if (m_hic) {
		if (IsCoder())
			ICCompressEnd(m_hic);
		else
			ICDecompressEnd(m_hic);
		ICClose(m_hic);
	}
	m_FrameNum = 0;
	m_bitrate = 128;
	m_bitrate_prev = m_bitrate;
	if (m_bmRGB) free(m_bmRGB); m_bmRGB = 0;
	if (m_bmfcc) free(m_bmfcc); m_bmfcc = 0;
}

int VS_VfwVideoCodec::Convert(uint8_t * in, uint8_t * out, VS_VideoCodecParam * param)
{
	uint32_t ret = 0;
	if (!m_valid) return -1;
	if (!IsCoder()) {
		m_bmhfcc->biSizeImage = param->dec.FrameSize;
		ret = ICDecompress(m_hic, param->dec.Flags, m_bmhfcc, in, m_bmhRGB, out);
		if (ret == ICERR_OK) {
			return m_bmhRGB->biSizeImage;
			m_FrameNum++;
		}
		else return -2;
	}
	else {
		DWORD reserved;
		DWORD isKeyFrame = 0;
		ret = ICCompress(m_hic, param->cmp.KeyFrame, m_bmhfcc, out, m_bmhRGB, in,
			&reserved, &isKeyFrame, m_FrameNum, param->cmp.FrameSize, 0, 0, 0);
		param->cmp.IsKeyFrame = isKeyFrame;
		if (ret == ICERR_OK) {
			return m_bmhfcc->biSizeImage;
			m_FrameNum++;
		}
		else return -2;
	}
}

bool VS_VfwVideoCodec::SetCoderOption(uint32_t param)
{
	return false;
}

bool VS_VfwVideoCodec::SetBitrate(uint32_t param)
{
	return false;
}
#endif
