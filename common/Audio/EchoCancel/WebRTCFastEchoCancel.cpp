
#include "SpeexEchoCancel.h"

#include "modules/audio_processing/ns/ns_core.h"

VS_WebRTCFastEchoCancel::VS_WebRTCFastEchoCancel()
{
	m_frame_size = 0;
	m_st = 0;
	m_type = AEC_WEBRTCFAST;
}

VS_WebRTCFastEchoCancel::~VS_WebRTCFastEchoCancel()
{
	Release();
}

void VS_WebRTCFastEchoCancel::Init(int frequency)
{
}

void VS_WebRTCFastEchoCancel::Init(int frequency, int num_mic, int num_spk)
{
	ReleaseAEC();
	if (num_spk > 0) {
		if (frequency != 8000 && frequency != 16000) return;
		int res = 0;
		m_frame_size = (frequency / 8000) * FRAME_LEN; /// 10 ms
		/// init aec
		WebRtcSpl_Init();
		m_st = WebRtcAecm_CreateCore();
		if (m_st) {
			res = WebRtcAecm_InitCore(m_st, frequency);
		}
		if (res == -1) ReleaseAEC();
	}
}

void VS_WebRTCFastEchoCancel::ReleaseAEC()
{
	if (m_st != 0) {
		WebRtcAecm_FreeCore(m_st); m_st = 0;
	}
}

void VS_WebRTCFastEchoCancel::Release()
{
	m_frame_size = 0;
	ReleaseAEC();
}

void VS_WebRTCFastEchoCancel::Cancellate(short* far_end, short* near_end, short* echo, int samples)
{
	if(!m_st)
		return;

	int i, j = 0;

		for (i = 0; i < samples; i += m_frame_size) {
			for (j = 0; j < m_frame_size; j += FRAME_LEN) {
				WebRtcAecm_ProcessFrame(m_st, far_end + j + i, near_end + j + i, 0, echo + j + i);
			}
		}
}
