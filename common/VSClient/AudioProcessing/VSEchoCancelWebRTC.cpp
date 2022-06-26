/*
 * File:   VSEchoCancelWebRtc.cpp
 * Author: dront78
 *
 * Created on 8 Октябрь 2012 г., 15:28
 */

#include "VSEchoCancelWebRTC.h"

#include <modules/audio_processing/audio_buffer.h>
#include <modules/audio_processing/aec/echo_cancellation.h>
#include <modules/audio_processing/aec/aec_resampler.h>
#include <modules/audio_processing/ns/ns_core.h>
#include <common_audio/ring_buffer.h>

#include <cassert>
#include <cstdlib>
#include <cstring>

namespace vs_webrtc {

	const int kSamplesPer16kHzChannel = 160;
	const int kSamplesPer32kHzChannel = 320;
	const int kSamplesPer48kHzChannel = 480;

	int GetNumBandsFromSamplesPerChannel(int num_frames) {
		int num_bands = 1;
		if (num_frames == kSamplesPer32kHzChannel ||
			num_frames == kSamplesPer48kHzChannel) {
            num_bands = num_frames / static_cast<int>(kSamplesPer16kHzChannel);
        }
		return num_bands;
	}

}

VSEchoCancelWebRTC::VSEchoCancelWebRTC()
{
	m_frame_size = 0;
	m_st = nullptr;
}

VSEchoCancelWebRTC::~VSEchoCancelWebRTC()
{
    Release();
}

void VSEchoCancelWebRTC::Init(uint32_t frequency)
{
	Release();

	if (frequency > 48000) frequency = 48000;

	int res = 0;
	m_frame_size = (frequency / 8000) * FRAME_LEN; /// 10 ms

	webrtc::Aec *st = new webrtc::Aec();
	m_st = st;
	if (m_st == NULL) return;
	/// init aec
	st->aec = webrtc::WebRtcAec_CreateAec(st->instance_count);
	if (st->aec) {
		st->resampler = webrtc::WebRtcAec_CreateResampler();
		if (st->resampler) {
			st->far_pre_buf = WebRtc_CreateBuffer(PART_LEN2 + webrtc::kResamplerBufferSize, sizeof(float));
			if (res != -1) {
				st->initFlag = 0;
				//st->lastError = 0;
				st->sampFreq = frequency;
				st->scSampFreq = frequency;
				res = WebRtcAec_InitAec(st->aec, st->sampFreq);
				if (res != -1) {
					res = webrtc::WebRtcAec_InitResampler(st->resampler, st->scSampFreq);
					if (res != -1) {
						WebRtc_InitBuffer(st->far_pre_buf);
						WebRtc_MoveReadPtr(st->far_pre_buf, -PART_LEN);
						if (res != -1) {
							st->initFlag = 42;
							st->splitSampFreq = frequency;
							st->skewFrCtr = 0;
							st->delayCtr = 0;
							st->sum = 0;
							st->counter = 0;
							st->checkBuffSize = 1;
							st->firstVal = 0;
							st->bufSizeStart = 0;
							st->checkBufSizeCtr = 0;
							st->filtDelay = 0;
							st->timeForDelayChange = 0;
							st->knownDelay = 0;
							st->lastDelayDiff = 0;
							st->skew = 0;
							st->resample = 0;
							st->highSkewCtr = 0;
							st->sampFactor = (st->scSampFreq * 1.0f) / st->splitSampFreq;
							st->skewMode = 0;
							st->msInSndCardBuf = 0;
						}
						WebRtcAec_enable_extended_filter(st->aec, 1);
					}
				}
			}
		}
	}
	if (res == -1) Release();
}

void VSEchoCancelWebRTC::Release()
{
	if (m_st != 0) {
		webrtc::Aec *st = (webrtc::Aec*)m_st;
		WebRtc_FreeBuffer(st->far_pre_buf);
		webrtc::WebRtcAec_FreeAec(st->aec);
		webrtc::WebRtcAec_FreeResampler(st->resampler);
		free(m_st);
		m_st = 0;
	}
	m_frame_size = 0;
}

void VSEchoCancelWebRTC::Cancellate(webrtc::AudioBuffer *ra, webrtc::AudioBuffer *ca, bool *stream_has_echo)
{
	if (!m_st || !ra || !ca) return;

	webrtc::Aec *st = (webrtc::Aec*)m_st;
	int numOfSamplesRA = static_cast<int16_t>(ra->num_frames_per_band());
	st->farend_started = 1;
	WebRtcAec_SetSystemDelay(st->aec, WebRtcAec_system_delay(st->aec) + numOfSamplesRA); // TO DO or "m_frame_size" ??
	WebRtc_WriteBuffer(st->far_pre_buf, ra->split_bands_const_f(0)[webrtc::kBand0To8kHz], numOfSamplesRA);
	while (WebRtc_available_read(st->far_pre_buf) >= PART_LEN2) {
		{
			float* ptmp = nullptr;
			float tmp[PART_LEN2];
			WebRtc_ReadBuffer(st->far_pre_buf, (void**)&ptmp, tmp, PART_LEN2);
			webrtc::WebRtcAec_BufferFarendBlock(st->aec, &ptmp[PART_LEN]);
		}
		WebRtc_MoveReadPtr(st->far_pre_buf, -PART_LEN);
	}
	WebRtcAec_ProcessFrames(st->aec,
							ca->split_bands_const_f(0),
							ca->num_bands(),
							static_cast<int16_t>(ca->num_frames_per_band()),
							0,
							ca->split_bands_f(0));
	*stream_has_echo = (WebRtcAec_echo_state(st->aec) == 1);

}
