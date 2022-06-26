#include "VS_HardwareH264VideoCodec.h"
#include "VSClient/VSCapture.h"
#include "MediaParserLib/VS_H264Parser.h"

VS_HardwareH264VideoCodec::VS_HardwareH264VideoCodec(int CodecId, bool IsCoder) : VideoCodec(CodecId, IsCoder)
{
	m_valid = false;
	m_pHWControl = 0;
}

VS_HardwareH264VideoCodec::~VS_HardwareH264VideoCodec()
{
	Release();
	m_pHWControl = 0;
}

int VS_HardwareH264VideoCodec::Init(int w, int h, uint32_t ColorMode, unsigned char sndLvl, int numThreads, unsigned int framerate)
{
	m_pHWControl = VS_CaptureDevice::GetHardwareObserver();

	Release();

	if (!IsCoder() || m_pHWControl == 0) return -1;
	if (ColorMode != FOURCC_I420) return -1;

	m_pHWControl->SetHWEncoderRequest(HARDWARE_SETBITRATE, m_bitrate * 1000);
	m_pHWControl->SetHWEncoderRequest(HARDWARE_NEEDKEY);

	SetBitrate(m_bitrate);

	m_valid = true;

	return m_valid ? 0 : -1;
}

void VS_HardwareH264VideoCodec::Release()
{
	m_valid = false;
	m_bitrate = 128;
	m_bitrate_prev = m_bitrate;
	m_pHWControl->SetHWEncoderRequest(HARDWARE_RESET);
}

bool VS_HardwareH264VideoCodec::UpdateBitrate()
{
	if (m_bitrate_prev == m_bitrate) return true;
	m_bitrate_prev = m_bitrate;
	m_pHWControl->SetHWEncoderRequest(HARDWARE_SETBITRATE, m_bitrate_prev * 1000);
	return false;
}

int VS_HardwareH264VideoCodec::Convert(BYTE *in, BYTE* out, VS_VideoCodecParam* param)
{
	UpdateBitrate();
	int prm = param->cmp.KeyFrame != 1;
	if (prm == 0) m_pHWControl->SetHWEncoderRequest(HARDWARE_NEEDKEY);
	int ret = *(unsigned int*)in;
	memcpy(out, in + sizeof(unsigned int), ret);
	TypeSliceFromBitstream_H264(in + sizeof(unsigned int), ret, prm);
	param->cmp.IsKeyFrame = (prm == 0);
	return ret;
}