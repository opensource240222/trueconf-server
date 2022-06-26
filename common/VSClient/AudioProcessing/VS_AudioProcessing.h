#ifndef VS_AUDIOPROC_H
#define VS_AUDIOPROC_H

#include <list>

#include "../../Transcoder/VS_AudioResamplerBase.h"
#include "../VSAudioUtil.h"
#include "VS_HighPassFilterImpl.h"
#include "VS_GainControlImpl.h"
#include "VS_NoiseSuppressionImpl.h"
#include "VS_VoiceDetectionImpl.h"
#include "VS_LevelEstimatorImpl.h"
#include "VS_AudioEchoCancel.h"
#include "std/VS_FifoBuffer.h"


enum filterType
{
	Idle_Filter				= 0x00000000,
	HighPass_Filter			= 0x00000001,
	GainControl_Filter		= 0x00000002,
	NoiseSuppression_Filter	= 0x00000004,
	VoiceDetection_Filter	= 0x00000008,
	EchoCanceller_Filter	= 0x00000010,
	LevelEstimator_Filter	= 0x00000020,
	All_Filter = (
		HighPass_Filter |
		GainControl_Filter |
		NoiseSuppression_Filter |
		VoiceDetection_Filter |
		EchoCanceller_Filter |
		LevelEstimator_Filter)
};

class VS_AudioProcessingBase
{
public :
	VS_AudioProcessingBase() {};
	virtual ~VS_AudioProcessingBase() {};
	virtual bool Init(int processingSampleRate, uint32_t filters) = 0;
	virtual void Release() = 0;
	virtual bool ProcessRenderAudio(const short* buffer, size_t samples, float sampleRate, int bufferedSamples) = 0;
	virtual  int ProcessCaptureAudio(short* buffer, short *& processBuffer, size_t samples, float sampleRate, int bufferedSamples, bool muted) = 0;
	virtual void EnableFilter(filterType filter, bool enable) = 0;
	virtual void EnableLogging(bool enable) = 0;
	virtual float GetAnalogLevel() = 0;
	virtual float GetRmsLevel() = 0;
	virtual void SetAnalogLevel(int level) = 0;
};

class VS_AudioProcessing : public VS_AudioProcessingBase
{
public :
	VS_AudioProcessing();
	virtual ~VS_AudioProcessing();
	bool Init(int processingSampleRate, uint32_t filters) override;
	void Release() override;
	bool ProcessRenderAudio(const short* buffer, size_t samples, float sampleRate, int bufferedSamples) override;
	int ProcessCaptureAudio(short* buffer, short *& processBuffer, size_t samples, float sampleRate, int bufferedSamples, bool muted) override;
	void EnableFilter(filterType filter, bool enable) override;
	void EnableLogging(bool enable) override;
	void SetAnalogLevel(int level) override;
	float GetAnalogLevel() override;
	float GetRmsLevel() override;
	bool IsInit();
	// VS_CallbackRenderChunk impl
	virtual void ProcessRenderChunk(webrtc::AudioBuffer *ra);

	void SetEchoDelay(int32_t delay);
	VSEchoDelayDetector::EGetDelayResult GetEchoDelay(int32_t& delay);
	void StartDelayDetectTest(size_t delayBeforeStart);
	void StopDelayDetectTest();

private :
	bool IsDataProcessed();
	bool AnalysisNeeded(bool bDataProcessed);
	bool SynthesisNeeded(bool bDataProcessed);

private :
	rtc::CriticalSection m_CritRender;
	rtc::CriticalSection m_CritCapture;
	rtc::CriticalSection m_CritDelayDetector;

	size_t DelayBeforeStart;
	size_t EchoDetectStart;

	bool m_bInit;
	int m_senderSampleRate;
	int m_processingSampleRate;
	int m_chunkSize; // 10ms samples
	size_t m_numChannels;

	webrtc::AudioBuffer *m_ca;
	webrtc::AudioFrame  *m_caf;
	VS_AudioResamplerBase *m_capturerResampler;
	VS_AudioResamplerBase *m_rendererResampler;
	VS_FifoBuffer *m_capturerFifo;
	int16_t *m_capturerFifoChunk;
	int16_t *m_rendererFifoChunk;
	int m_audioHALLevel;
	int m_audioPreferedLevel;
	int m_audioRmsLevel;
	float m_audioRmsLevelStat;

	webrtc::VS_HighPassFilterImpl			m_hp_filter;
	webrtc::VS_GainControlImpl				m_gainctrl_filter;
	webrtc::VS_NoiseSuppressionImpl			m_ns_filter;
	webrtc::VS_VoiceDetectionImpl			m_vad_filter;
	VS_AudioEchoCancel						m_echo_canceller;
	webrtc::VS_LevelEstimatorImpl			m_level_estimator;
};

class VS_AudioProcessingStub : public VS_AudioProcessingBase
{

public:

	VS_AudioProcessingStub() {};
	virtual ~VS_AudioProcessingStub() {};
	bool Init(int processingSampleRate, uint32_t filters) override {
		return false;
	}
	void Release() override {};
	bool ProcessRenderAudio(const short* buffer, size_t samples, float sampleRate, int bufferedSamples) override {
		return samples;
	}
	int ProcessCaptureAudio(short* buffer, short *& processBuffer, size_t samples, float sampleRate, int bufferedSamples, bool muted) override {
		processBuffer = buffer;
		return samples;
	}
	void EnableFilter(filterType filter, bool enable) override {};
	void EnableLogging(bool enable) override {};
	void SetAnalogLevel(int level) override {};
	float GetAnalogLevel() override {
		return 0.0;
	}
	float GetRmsLevel() override {
		return 0.0;
	}

};

#endif /* VS_AUDIOPROC_H */
