#include "VideoCodec.h"
#include "Transcoder/VS_RetriveVideoCodec.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../std/cpplib/VS_Cpu.h"

#ifdef __cplusplus
}
#endif

VideoCodec::VideoCodec(uint32_t fcc, bool coder)
{
	m_valid = false;
	m_bitrate = m_bitrate_prev = 0;

	m_coder = coder;
	m_fcc = fcc;

	VS_GetNumCPUCores(&m_num_phcores, &m_num_lcores);
}

VideoCodec::~VideoCodec()
{
	VS_UnregisteredVideoCodec(this);
}

int VideoCodec::InitExtended(const base_Param &settings)
{
	return Init(settings.width, settings.height, settings.color_space);
}

bool VideoCodec::SetBitrate(uint32_t param)
{
	if (IsCoder()) {
		m_bitrate = param;
		return true;
	}
	else
		return false;
}

int VideoCodec::GetBitrate()
{
	if (m_valid)
		return m_bitrate;
	else
		return -1;
}

uint32_t VideoCodec::GetNumThreads() const
{
	return m_num_threads;
}

bool VideoCodec::SetCoderOption(uint32_t param)
{
	if (IsCoder()) {
		return true;
	}
	else
		return false;
}
