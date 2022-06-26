
#include "SpeexEchoCancel.h"
#include "../Features/AudioUtility.h"
#include "modules/audio_processing/aec/aec_resampler.h"

#include <cstdlib>

VS_WebRTCEchoCancel::VS_WebRTCEchoCancel()
{
	m_frame_size = 0;
	m_lowBandNear = 0;
	m_highBandNear = 0;
	m_lowBandFar = 0;
	m_highBandFar = 0;
	m_st = 0;
	m_type = AEC_WEBRTC;
	m_splitterCombinerFar = 0;
	m_splitterCombinerNear = 0;
}

VS_WebRTCEchoCancel::~VS_WebRTCEchoCancel()
{
	Release();
}

void VS_WebRTCEchoCancel::Init(int frequency)
{
}

void VS_WebRTCEchoCancel::Init(int frequency, int num_mic, int num_spk)
{
	ReleaseAEC();
	if (num_spk > 0) {
		if ((frequency >= 16000) && (frequency < 32000))
			frequency = 16000;
		else if(frequency >= 32000)
			frequency = 32000;
		else
			frequency = 8000;

		if(frequency == 32000)
			m_frame_size /= 2;
		m_freq = frequency;

		int res = 0;
		m_frame_size = (frequency / 8000) * FRAME_LEN; /// 10 ms

		m_splitterCombinerFar = new VS_BandSplitterCombiner;
		m_splitterCombinerNear = new VS_BandSplitterCombiner;
		m_lowBandNear = new short [10 * m_frame_size];
		m_highBandNear = new short [10 * m_frame_size];
		m_lowBandFar = new short [10 * m_frame_size];
		m_highBandFar = new short [10 * m_frame_size];

		m_st = (webrtc::Aec*)malloc(sizeof(webrtc::Aec));

		if (m_st == NULL) return;
		m_pFarEnd_float = (float*)malloc(10*m_frame_size*sizeof(float));
		/// init aec
		m_st->aec = webrtc::WebRtcAec_CreateAec(m_st->instance_count);
		if (m_st->aec) {
			m_st->resampler = webrtc::WebRtcAec_CreateResampler();
			if (m_st->resampler) {
				m_st->far_pre_buf = WebRtc_CreateBuffer(PART_LEN2 + webrtc::kResamplerBufferSize, sizeof(float));
				if (m_st->far_pre_buf) {
					m_st->initFlag = 0;
					m_st->sampFreq = frequency;
					m_st->scSampFreq = frequency;
					res = WebRtcAec_InitAec(m_st->aec, m_st->sampFreq);
					if (res != -1) {
						res = webrtc::WebRtcAec_InitResampler(m_st->resampler, m_st->scSampFreq);
						if (res != -1) {
							WebRtc_InitBuffer(m_st->far_pre_buf);
							WebRtc_MoveReadPtr(m_st->far_pre_buf, -PART_LEN);
							if (res != -1) {
								m_st->initFlag = 42;
								m_st->splitSampFreq = frequency;
								m_st->skewFrCtr = 0;
								m_st->delayCtr = 0;
								m_st->sum = 0;
								m_st->counter = 0;
								m_st->checkBuffSize = 1;
								m_st->firstVal = 0;
								m_st->bufSizeStart = 0;
								m_st->checkBufSizeCtr = 0;
								m_st->filtDelay = 0;
								m_st->timeForDelayChange = 0;
								m_st->knownDelay = 0;
								m_st->lastDelayDiff = 0;
								m_st->skew = 0;
								m_st->resample = 0;
								m_st->highSkewCtr = 0;
								m_st->sampFactor = (m_st->scSampFreq * 1.0f) / m_st->splitSampFreq;
								m_st->aec->nlp_mode = 1;
								m_st->skewMode = 0;
								m_st->msInSndCardBuf = 0;
								m_st->aec->system_delay = 0;
								m_st->aec->num_partitions = webrtc::kExtendedNumPartitions;
								m_st->aec->extended_filter_enabled = 1;
								//m_st->autoOnOff = 0;
								//m_st->activity = 0;
								//m_st->ECstartup = 1;
							}
						}
					}
				}
			}
		}
		if (res == -1) ReleaseAEC();
	}
}

void VS_WebRTCEchoCancel::ReleaseAEC()
{
	if (m_st != 0) {
		free(m_pFarEnd_float); m_pFarEnd_float = 0;
		WebRtc_FreeBuffer(m_st->far_pre_buf);
		WebRtcAec_FreeAec(m_st->aec);
		webrtc::WebRtcAec_FreeResampler(m_st->resampler);
		free(m_st);
		m_st = 0;
	}

	if(m_splitterCombinerFar) delete m_splitterCombinerFar; m_splitterCombinerFar = 0;
	if(m_splitterCombinerNear) delete m_splitterCombinerNear; m_splitterCombinerNear = 0;
	if(m_lowBandNear) delete [] m_lowBandNear; m_lowBandNear = 0;
	if(m_highBandNear) delete [] m_highBandNear; m_highBandNear = 0;
	if(m_lowBandFar) delete [] m_lowBandFar; m_lowBandFar = 0;
	if(m_highBandFar) delete [] m_highBandFar; m_highBandFar = 0;
	m_frame_size = 0;
}

void VS_WebRTCEchoCancel::Release()
{
	ReleaseAEC();
}

void VS_WebRTCEchoCancel::Cancellate(short* far_end, short* near_end, short* echo, int samples)
{
	if(!m_st)
		return;

	int i, j = 0;
	memcpy(m_lowBandNear, near_end, samples * sizeof(short));
	if(m_freq == 32000)
	{
		m_splitterCombinerNear->Split(near_end, samples, m_lowBandNear, m_highBandNear);
		m_splitterCombinerFar->Split(far_end, samples, m_lowBandFar, m_highBandFar);
		far_end = m_lowBandFar;
		samples /= 2;
	}
	for (i = 0; i < samples; i += m_frame_size)
	{
		const float* farend_float = m_pFarEnd_float;

		for (j = 0; j < m_frame_size; j++) m_pFarEnd_float[j] = (float)far_end[i+j];
		m_st->aec->system_delay += m_frame_size;
		WebRtc_WriteBuffer(m_st->far_pre_buf, farend_float, m_frame_size);
		while (WebRtc_available_read(m_st->far_pre_buf) >= PART_LEN2)
		{
			WebRtc_ReadBuffer(m_st->far_pre_buf, (void**) &farend_float, m_pFarEnd_float, PART_LEN2);
			WebRtcAec_BufferFarendBlock(m_st->aec, farend_float);
			WebRtc_MoveReadPtr(m_st->far_pre_buf, -PART_LEN);
		}
	}

	if(m_freq == 32000)
	{
		m_splitterCombinerNear->Combine(m_lowBandNear, m_highBandNear, samples, echo);
	}
	else
		memcpy(echo, m_lowBandNear, samples *sizeof(short));
}