#include "VS_AudioProcessing.h"
#include "../VS_EchoDebugger.h"
#include "VS_AudioEchoCancel.h"
#include "modules/audio_processing/audio_buffer.h"
#include "../../std/VS_ProfileTools.h"
#include "../../Transcoder/VS_FfmpegResampler.h"
#include "std/VS_FifoBuffer.h"
#include <timeapi.h>

#if !defined (DEBUG_ECHO_TXT)
//#define DEBUG_ECHO_TXT
#endif

VS_AudioProcessing::VS_AudioProcessing() :
	m_hp_filter(&m_CritCapture),
	m_gainctrl_filter(&m_CritCapture, &m_CritCapture),
	m_ns_filter(&m_CritCapture),
	m_vad_filter(&m_CritCapture),
	m_level_estimator(&m_CritCapture)
{
	m_processingSampleRate = 0;
	m_senderSampleRate = 0;

	m_caf = nullptr;
	m_ca = nullptr;
	m_capturerFifo = nullptr;
	m_capturerResampler = nullptr;
	m_rendererResampler = nullptr;
	m_capturerFifoChunk = nullptr;
	m_rendererFifoChunk = nullptr;

	m_audioHALLevel = 128;
	m_audioPreferedLevel = -1;
	m_audioRmsLevel = -1;
	m_audioRmsLevelStat = 127.0f;

	m_bInit = false;
}

VS_AudioProcessing::~VS_AudioProcessing()
{
	Release();
}

bool VS_AudioProcessing::Init(int processingSampleRate, uint32_t filters)
{
	//rtc::CritScope lockCapturer(&m_CritCapture);
	//rtc::CritScope lockRenderer(&m_CritRender);

	Release();
	if (processingSampleRate <= 0) {
		return false;
	}
	m_numChannels = 1;
	m_senderSampleRate = processingSampleRate;
	int dv = webrtc::AudioProcessing::kSampleRate8kHz;
	if (m_senderSampleRate >= webrtc::AudioProcessing::kSampleRate16kHz) {
		dv = webrtc::AudioProcessing::kSampleRate16kHz;
		if (m_senderSampleRate >= webrtc::AudioProcessing::kSampleRate32kHz) {
			dv = webrtc::AudioProcessing::kSampleRate32kHz;
		}
	}
	m_processingSampleRate = processingSampleRate / dv * dv;
	m_chunkSize = webrtc::AudioProcessing::kChunkSizeMs * m_processingSampleRate / 1000;
	m_caf = new webrtc::AudioFrame();
	m_ca = new webrtc::AudioBuffer(m_chunkSize, 1, m_chunkSize, 1, m_chunkSize);
	m_capturerResampler = new VS_FfmpegResampler();
	m_rendererResampler = new VS_FfmpegResampler();
	m_capturerFifo = new VS_FifoBuffer(m_senderSampleRate * sizeof(int16_t) * 2);
	m_capturerFifoChunk = new int16_t [m_senderSampleRate * sizeof(int16_t) * 2];
	m_rendererFifoChunk = new int16_t [m_senderSampleRate * sizeof(int16_t) * 2];

	m_hp_filter.Initialize(m_numChannels, m_processingSampleRate);
	m_gainctrl_filter.Initialize(m_numChannels, m_processingSampleRate);
	m_ns_filter.Initialize(m_numChannels, m_processingSampleRate);
	m_vad_filter.Initialize(m_processingSampleRate);
	m_echo_canceller.Initialize(m_processingSampleRate);
	m_level_estimator.Initialize();

	/*int db(0);
	if (__debugTuning.loadGainControlCompressionDb(&db)) {
		m_gainctrl_filter->set_compression_gain_db(db);
	}*/

	if (filters & HighPass_Filter) m_hp_filter.Enable(true);
	if (filters & GainControl_Filter) m_gainctrl_filter.Enable(true);
	if (filters & NoiseSuppression_Filter) m_ns_filter.Enable(true);
	if (filters & VoiceDetection_Filter) m_vad_filter.Enable(true);
	if (filters & EchoCanceller_Filter) m_echo_canceller.Enable(true);
	if (filters & LevelEstimator_Filter) m_level_estimator.Enable(true);

	m_bInit = true;

	EchoDetectStart = 0;

	EnableLogging(true);

	return true;
}

void VS_AudioProcessing::Release()
{
	//const VSAutoLock _lockCapturer(m_capturerLock);
	//const VSAutoLock _lockRenderer(m_rendererLock);

	/*;
	for (auto item : m_component_list) {
		item->Destroy();
	}*/

	m_bInit = false;

	m_echo_canceller.Destroy();

	m_gainctrl_filter.Enable(false);
	m_hp_filter.Enable(false);
	m_ns_filter.Enable(false);
	m_vad_filter.Enable(false);
	m_echo_canceller.Enable(false);
	m_level_estimator.Enable(false);

	m_audioPreferedLevel = -1;
	m_audioRmsLevel = -1;
	m_audioRmsLevelStat = 127.0f;
	m_processingSampleRate = 0;
	m_senderSampleRate = 0;
	delete m_caf; m_caf = nullptr;
	delete m_ca; m_ca = nullptr;
	delete m_capturerResampler; m_capturerResampler = nullptr;
	delete m_rendererResampler; m_rendererResampler = nullptr;
	delete m_capturerFifo; m_capturerFifo = nullptr;
	delete[] m_capturerFifoChunk; m_capturerFifoChunk = nullptr;
	delete[] m_rendererFifoChunk; m_rendererFifoChunk = nullptr;

}

void VS_AudioProcessing::EnableFilter(filterType filter, bool enable)
{
	//const VSAutoLock _lockCapturer(m_capturerLock);
	//const VSAutoLock _lockRenderer(m_rendererLock);

	if (!m_bInit) return;

	switch (filter)
	{
		case HighPass_Filter:
		{
			m_hp_filter.Enable(enable);
			break;
		}
		case GainControl_Filter:
		{
			m_gainctrl_filter.Enable(enable);
			break;
		}
		case NoiseSuppression_Filter:
		{
			m_ns_filter.Enable(enable);
			break;
		}
		case VoiceDetection_Filter:
		{
			m_vad_filter.Enable(enable);
			break;
		}
		case EchoCanceller_Filter:
		{
			m_echo_canceller.Enable(enable);
			break;
		}
		case LevelEstimator_Filter:
		{
			m_level_estimator.Enable(enable);
			break;
		}
		default:
		{
			// do nothing
		}
		break;
	}
}

void VS_AudioProcessing::EnableLogging(bool enable)
{
	//VS_EchoDebugger::GetInstance().Init(std::to_string(timeGetTime()));
}

void VS_AudioProcessing::ProcessRenderChunk(webrtc::AudioBuffer *ra)
{
	m_gainctrl_filter.ProcessRenderAudio(ra);
}

void VS_AudioProcessing::StartDelayDetectTest(size_t delayBeforeStart)
{
	rtc::CritScope cs(&m_CritDelayDetector);

	m_echo_canceller.StopDelayDetectTest(); // stop, start after DelayBeforeStart

	DelayBeforeStart = delayBeforeStart;
	EchoDetectStart = timeGetTime();
}

void VS_AudioProcessing::StopDelayDetectTest()
{
	rtc::CritScope cs(&m_CritDelayDetector);

	EchoDetectStart = 0;

	m_echo_canceller.StopDelayDetectTest();
}

bool VS_AudioProcessing::ProcessRenderAudio(const short* buffer, size_t samples, float sampleRate, int bufferedSamples)
{
	uint_fast64_t ctime = timeGetTime();

	{
		rtc::CritScope cs(&m_CritDelayDetector);

		if (EchoDetectStart && timeGetTime() - EchoDetectStart > DelayBeforeStart)
			m_echo_canceller.StartDelayDetectTest();
	}

	//const VSAutoLock _lockRenderer(m_rendererLock);

	if (!m_bInit) {
		return false;
	}
	//DEBUG_ECHO_LOG("from renderer : %5d s, %8.2f", samples, sampleRate);

	float ms = (1000.0 * (bufferedSamples + (int)samples)) / sampleRate;
	const int16_t *buffFifo = buffer;
	if (m_senderSampleRate != sampleRate) {
		int resamplerSize = m_rendererResampler->Process((int16_t*)buffer, m_rendererFifoChunk, samples * sizeof(int16_t), (uint32_t)sampleRate, m_senderSampleRate);
		int resamplerSamples = resamplerSize / sizeof(int16_t);

		//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_FAREND, m_rendererFifoChunk, resamplerSamples * sizeof(m_rendererFifoChunk[0]));

		if (resamplerSamples < 0) {
			//DEBUG_ECHO_LOG("renderer resampler error !!!");
			resamplerSamples = m_senderSampleRate / (uint32_t)sampleRate * samples;
			memset(m_rendererFifoChunk, 0, resamplerSamples * sizeof(int16_t));
		}
		samples = resamplerSamples;
		buffFifo = (const int16_t*)m_rendererFifoChunk;
		//DEBUG_ECHO_LOG("from renderer resampler : %5d s, %8.2f", samples, static_cast<float>(m_senderSampleRate));
	}

	{
		rtc::CritScope cs(&m_CritDelayDetector);

		m_echo_canceller.ProcessRenderAudio(buffFifo, sampleRate, ms, (int)samples, ctime);
	}

	return true;
}

int VS_AudioProcessing::ProcessCaptureAudio(short* buffer, short *& processBuffer, size_t samples, float sampleRate, int bufferedSamples, bool muted)
{
	//AUTO_PROF
	//const VSAutoLock _lockCapturer(m_capturerLock);

	if (!m_bInit) {
		processBuffer = buffer;
		return samples;
	}
	//DEBUG_ECHO_LOG("from capturer : %5d s, %8.2f", samples, sampleRate);
	float captureMs = (1000.0 * bufferedSamples) / sampleRate;
	unsigned char *buffFifo = (unsigned char*)buffer;

	if (m_senderSampleRate != sampleRate) {
		int resamplerSamples = m_capturerResampler->Process(buffFifo, m_capturerFifoChunk, samples * sizeof(int16_t), (uint32_t)sampleRate, m_senderSampleRate) / sizeof(int16_t);
		if (resamplerSamples < 0) {
			//DEBUG_ECHO_LOG("capturer resampler error !!!");
			resamplerSamples = m_senderSampleRate / (uint32_t)sampleRate * samples;
			memset(m_capturerFifoChunk, 0, resamplerSamples * sizeof(int16_t));
		}
		samples = resamplerSamples;
		buffFifo = (unsigned char*)m_capturerFifoChunk;
		//DEBUG_ECHO_LOG("from capturer resampler : %5d s, %8.2f", samples, static_cast<float>(m_senderSampleRate));
	}

	float ms = captureMs + (1000.0 * m_capturerFifo->GetDataLen() / 2) / m_senderSampleRate;

	{
		rtc::CritScope cs(&m_CritDelayDetector);

		m_echo_canceller.AnalyzeCaptureAudio(sampleRate, ms, samples);
	}

	m_capturerFifo->AddData(buffFifo, samples * sizeof(int16_t));

	short *dstBuffer = processBuffer;
	int procSamples = 0;
	int buffSize = m_chunkSize * sizeof(int16_t);
	//DEBUG_ECHO_LOG("captfifo: ms = %6.3f, devms = %6.3f, fifo = %5d)", ms, captureMs, m_capturerFifo->GetDataLen() / 2);

	while (m_capturerFifo->GetData(m_capturerFifoChunk, buffSize))
	{
		//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_NEAREND, m_capturerFifoChunk, m_chunkSize * sizeof(m_capturerFifoChunk[0]));

		m_caf->UpdateFrame(0, m_capturerFifoChunk, m_chunkSize, m_processingSampleRate, webrtc::AudioFrame::kNormalSpeech, webrtc::AudioFrame::kVadUnknown);
		m_ca->DeinterleaveFrom(m_caf);

		bool data_processed = IsDataProcessed();
		if (AnalysisNeeded(data_processed)) {
			m_ca->SplitIntoFrequencyBands();
		}

#if !defined (WEBRTC_NS_FIXED)
		m_gainctrl_filter.set_stream_analog_level(m_audioHALLevel);
#endif

		m_hp_filter.ProcessCaptureAudio(m_ca);
		m_gainctrl_filter.AnalyzeCaptureAudio(m_ca);
		m_ns_filter.AnalyzeCaptureAudio(m_ca);

#if defined (WEBRTC_NS_FIXED)
		if (m_echo_canceller->is_enabled() && m_ns_filter->is_enabled()) {
			m_ca->CopyLowPassToReference();
		}
#else
		{
			rtc::CritScope cs(&m_CritDelayDetector);

			m_echo_canceller.ProcessCaptureAudio(m_ca);
		}
#endif

		m_ns_filter.ProcessCaptureAudio(m_ca);

#if defined (WEBRTC_NS_FIXED)
		m_echo_canceller->ProcessCaptureAudio(m_ca);
#endif

		m_vad_filter.ProcessCaptureAudio(m_ca);
		m_gainctrl_filter.ProcessCaptureAudio(m_ca, m_echo_canceller.StreamHasEcho());
		if (SynthesisNeeded(data_processed)) {
			m_ca->MergeFrequencyBands();
		}

		if (!muted)
			m_level_estimator.ProcessStream(m_ca);

		m_ca->InterleaveTo(m_caf, data_processed);
		for (int i = 0; i < m_chunkSize; i++) {
			dstBuffer[i] = m_caf->data()[i];
		}
		//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_ECHO, dstBuffer, m_chunkSize * sizeof(dstBuffer[0]));
		dstBuffer += m_chunkSize;
		procSamples += m_chunkSize;
	}

#if !defined (WEBRTC_NS_FIXED)
	m_audioPreferedLevel = m_gainctrl_filter.stream_analog_level();

#if defined(DEBUG_AGC_TXT)
	//LOGD(true, "agc to hal: lvl = %4d", m_audioPreferedLevel);
#endif

#endif

	// Computes the RMS in dBFS. The returned value is positive but should
	// be interpreted as negative as per the RFC. It is constrained to [0, 127].
	m_audioRmsLevel = m_level_estimator.RMS();

#if defined(DEBUG_AGC_TXT)
	//LOGD(true, "rms level: rms = %4d", m_audioRmsLevel);
#endif

	//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_ECHO, m_capturerBuffer, procSamples * sizeof(m_capturerBuffer[0]));

	return procSamples;
}

void VS_AudioProcessing::SetAnalogLevel(int level)
{

#if defined(DEBUG_AGC_TXT)
	LOGD(true, "agc from hal: lvl = %4d", level);
#endif

	m_audioHALLevel = level;
}

void VS_AudioProcessing::SetEchoDelay(int32_t delay)
{
	m_echo_canceller.SetManualOffset(delay);
}

VSEchoDelayDetector::EGetDelayResult VS_AudioProcessing::GetEchoDelay(int32_t& delay)
{
	rtc::CritScope cs(&m_CritDelayDetector);

	return m_echo_canceller.GetEchoDelay(delay);
}

float VS_AudioProcessing::GetAnalogLevel()
{
	return (float)m_audioPreferedLevel / 1000.0;
}

float VS_AudioProcessing::GetRmsLevel()
{
	m_audioRmsLevelStat = (2.0f * m_audioRmsLevel + m_audioRmsLevelStat) / 3.0f;
	float rms(m_audioRmsLevelStat);

	if (rms > 63.0f) {
		rms = 63.0f;
	}
	return (1.0f - rms / 63.0f);
}

bool VS_AudioProcessing::IsInit()
{
	return m_bInit;
}

bool VS_AudioProcessing::IsDataProcessed()
{
	int enabled_count = 0;

	if (m_hp_filter.is_enabled()) enabled_count += 1;
	if (m_gainctrl_filter.is_enabled()) enabled_count += 1;
	if (m_ns_filter.is_enabled()) enabled_count += 1;
	if (m_vad_filter.is_enabled()) enabled_count += 1;
	if (m_echo_canceller.is_enabled()) enabled_count += 1;
	if (m_level_estimator.is_enabled()) enabled_count += 1;

	// Data is unchanged if no components are enabled, or if only level_estimator
	// or voice_detection is enabled.
	if (enabled_count == 0) {
		return false;
	}
	else if (enabled_count == 1) {
		if (m_level_estimator.is_enabled() || m_vad_filter.is_enabled()) {
			return false;
		}
	}
	else if (enabled_count == 2) {
		if (m_level_estimator.is_enabled() && m_vad_filter.is_enabled()) {
			return false;
		}
	}
	return true;
}

bool VS_AudioProcessing::AnalysisNeeded(bool bDataProcessed)
{
	if (!bDataProcessed && !m_vad_filter.is_enabled()/* &&
													  !transient_suppressor_enabled_*/) {
		// Only level_estimator_ is enabled.
		return false;
	}
	else if (m_processingSampleRate >= webrtc::AudioProcessing::kSampleRate32kHz) {
		// Something besides level_estimator_ is enabled, and we have super-wb.
		return true;
	}
	return false;
}

bool VS_AudioProcessing::SynthesisNeeded(bool bDataProcessed)
{
	return (bDataProcessed && (m_processingSampleRate >= webrtc::AudioProcessing::kSampleRate32kHz));
}
