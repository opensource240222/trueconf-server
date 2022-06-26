/**
 **************************************************************************
 * \brief VS_GlobalEcho Implementation
 *
 * \b Project ClientLib
 * \author SMirnovK
 * \date 31.08.2016
 *
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <math.h>
#include "VS_AudioEchoCancel.h"

#if !defined (IOS_BUILD)
#include "VSEchoCancelWebRTC.h"
#include "modules/audio_processing/audio_buffer.h"
#endif // IOS_BUILD

#include "../VS_EchoDebugger.h"
#include "../VSAudioUtil.h"
#include "../../VSClient/VS_Dmodule.h"
//#include "../../Audio/Hardware/VSAudioPlaybackBase.h"
//#include "../../Foundation/Library/VSThreadedLogger.h"
#include <timeapi.h>


/****************************************************************************
 * Defines
 ****************************************************************************/

#define CONFIDENCE_BAND (95)
#define BIN_RANGE		(2)
#define	INTERVAL_WIDTH	(64)


/****************************************************************************
 * Classes
 ****************************************************************************/

/******************************************************************/



VS_AudioEchoCancel::VS_AudioEchoCancel()
	//m_EchoDebugger(new VS_EchoDebugger)
{
	m_farendBarrel = nullptr;
	for (int i = 0; i < 2; i++) {
		m_rab[i] = nullptr;
	}
	m_ra = nullptr;
	m_bufferFarend = nullptr;

	m_stream_has_echo = false;
	m_farendOffsetSamples = 0;
	m_renderBufferedMs = 0.0;
	m_processingSampleRate = 0;
	m_zerroFarend = false;
	m_samples = 0;
	m_renderTimestamp = 0;
	m_Enabled = false;

	ResetManualOffset();

	m_ec = new VSEchoCancelWebRTC();

}

VS_AudioEchoCancel::~VS_AudioEchoCancel()
{
	Release();
    delete m_ec; m_ec = nullptr;
}

int VS_AudioEchoCancel::Initialize(int sample_rate_hz)
{
	if (Init(sample_rate_hz))
		return webrtc::AudioProcessing::kNoError;

	return webrtc::AudioProcessing::kUnspecifiedError;
}

int VS_AudioEchoCancel::Destroy()
{
	//const VSAutoLock _lock(*this);

	Release();

	return 0;
}

void VS_AudioEchoCancel::Enable(bool enable)
{
	//const VSAutoLock _lock(*this);

	if (is_enabled() && !enable) {
		CloseRendererStream();
		m_DelayDetector.Stop();
	}
	else if (!is_enabled() && enable) {
		OpenRendererStream();
	}

	m_Enabled = enable;
}

void VS_AudioEchoCancel::OpenRendererStream()
{
	CloseRendererStream();
	//DEBUG_ECHO_LOG("Wakeup renderer echo stream");
	m_farendBarrel = new VS_AudioBarrel();
}

void VS_AudioEchoCancel::CloseRendererStream()
{
	//DEBUG_ECHO_LOG("Reset renderer echo stream");
	m_zerroFarend = false;
	m_stream_has_echo = true;
	m_farendOffsetSamples = 0;
	m_renderBufferedMs = 0.0;
	m_renderTimestamp = 0;
	delete m_farendBarrel; m_farendBarrel = nullptr;
}

void VS_AudioEchoCancel::SetManualOffset(int manualOffset)
{
	m_manualOfset = manualOffset;
}

void VS_AudioEchoCancel::ResetManualOffset()
{
#if defined(__linux) || defined(__FreeBSD__)
	SetManualOffset(-20);
#elif defined(__APPLE__) && !defined(IOS_BUILD)
	SetManualOffset(-25);
#elif defined(__APPLE__)
	SetManualOffset(-25);
#elif defined(__ANDROID__)
	SetManualOffset(0);
#else // Win
	SetManualOffset(5);
#endif
}

void VS_AudioEchoCancel::StartDelayDetectTest()
{
	m_DelayDetector.Start();
}

void VS_AudioEchoCancel::StopDelayDetectTest()
{
	m_DelayDetector.Stop();
}

VSEchoDelayDetector::EGetDelayResult VS_AudioEchoCancel::GetEchoDelay(int32_t& delay)
{
	return m_DelayDetector.GetDelay(delay);
}

bool VS_AudioEchoCancel::Init(uint32_t frequency)
{
    //const VSAutoLock _lock(*this);

	if (m_ec == 0 || frequency == 0) return false;
	Release();
	ResetManualOffset();

	m_processingSampleRate = frequency;
	m_start_time = timeGetTime();

	int samples = frequency / 1000 * webrtc::AudioProcessing::kChunkSizeMs;
	m_bufferFarend = new short[samples];
	memset(m_bufferFarend, 0, samples * sizeof(short));
	m_ra = new webrtc::AudioBuffer(samples, 1, samples, 1, samples);
	for (int i = 0; i < 2; i++) {
		m_rab[i] = new float[samples];
	}
	m_farendBarrel = new VS_AudioBarrel();

	m_echoHist.Init(2 * INTERVAL_WIDTH / BIN_RANGE + 1, m_start_time);
	DTRACE(VSTM_ECHO, "INIT Echo Canceller = %d", m_processingSampleRate);

	m_ec->Init(m_processingSampleRate);

	m_DelayDetector.Init(2000, 2000);

	return true;
}

void VS_AudioEchoCancel::Release()
{
	DTRACE(VSTM_ECHO, "RELEASE Echo Canceller");

    //const VSAutoLock _lock(*this);

	m_ec->Release();
	m_echoHist.Clear();

	m_start_time = 0;
	m_processingSampleRate = 0;
	m_zerroFarend = false;
	m_samples = 0;
	m_stream_has_echo = true;
	m_farendOffsetSamples = 0;
	m_renderBufferedMs = 0.0;
	m_renderTimestamp = 0;

	delete[] m_bufferFarend; m_bufferFarend = nullptr;
	delete m_ra; m_ra = nullptr;
	for (int i = 0; i < 2; i++) {
		delete[] m_rab[i];  m_rab[i] = nullptr;
	}
	delete m_farendBarrel; m_farendBarrel = nullptr;
}

namespace vs_aec_analyze {

	int compare(const void *el1, const void *el2) {
		int a = *(int*)el1,
			b = *(int*)el2;
		return ((a > b) ? 1 : ((a < b) ? -1 : 0));
	}

	void update_hist(int *hist, int num_smpl, double freq) {
		int val = (int)(num_smpl / (freq / 1000.0));
		val = (val < -INTERVAL_WIDTH) ? -INTERVAL_WIDTH : ((val > INTERVAL_WIDTH) ? INTERVAL_WIDTH : val);
		val /= BIN_RANGE;
		hist[val + INTERVAL_WIDTH / BIN_RANGE]++;
	}

	void calc_bound_interval(int *hist, int size, int *bound, int min_bound) {
		int i, sum = 0, sum_l, sum_r;
		int l_bound = -1;

		*bound = min_bound;
		for (i = 0; i < size; i++) {
			sum += hist[i];
		}
		if (sum < 5) {
			return;
		}
		sum_l = (sum * (100 - CONFIDENCE_BAND)) / (2 * 100);
		sum_r = (sum * (100 + CONFIDENCE_BAND)) / (2 * 100);
		sum = 0;
		for (i = 0; i < size; i++) {
			if (sum <= sum_l) l_bound = i;
			sum += hist[i];
			if (sum >= sum_r) {
				if (l_bound == -1) return;
				sum = ((i - l_bound) / 2) * BIN_RANGE;
				if (sum > min_bound) *bound = sum;
				return;
			}
		}
	}

}

void VS_AudioEchoCancel::ProcessRenderAudio(const int16_t *farEnd, double rendererFrequency, float bufferedMs, int samples, uint_fast64_t ct)
{
	//VS_EchoDebugger::GetInstance().Log() << __FUNCTION__ << std::endl;

	if (!is_enabled()) {
		return;
	}
	{
		m_DelayDetector.AddPatternToRenderedAudio(const_cast<int16_t*>(farEnd), samples);

		const VS_AutoLock _lock(this);
		m_farendOffsetSamples += samples;
		m_renderBufferedMs = bufferedMs;
		m_farendBarrel->Add((short*)farEnd, samples);
		m_renderTimestamp = ct;
		DTRACE(VSTM_ECHO, "farend offset + %4d s = %5d s (rend ms = %6.3f, ct = %10llu)", samples, m_farendOffsetSamples.load(), bufferedMs, ct);

		//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_FAREND, farEnd, samples * sizeof(farEnd[0]));

		/*VS_EchoDebugger::GetInstance().Log() << __FUNCTION__ <<
			" farend offset + " << samples <<
			"s = " << m_farendOffsetSamples.load() <<
			"s (rend ms = " << bufferedMs <<
			", ct = " << ct <<
			")" << std::endl;*/
	}
}

void VS_AudioEchoCancel::AnalyzeCaptureAudio(double capturerFrequency, float bufferedMs, int samples)
{
	//VS_EchoDebugger::GetInstance().Log() << __FUNCTION__ << std::endl;

	if (!is_enabled()) {
		return;
	}

	int farendOffset(0);
	int renderTimestamp(0);
	int renderBufferedMs(0);
	{
		const VS_AutoLock _lock(this);
		farendOffset = m_farendOffsetSamples.load();
		renderTimestamp = m_renderTimestamp;
		renderBufferedMs = m_renderBufferedMs;
	}
	bool is_reset_ec = false;
	uint_fast64_t ctime = timeGetTime();
	float dt = (renderTimestamp == 0) ? 0.0 : (float)(ctime - renderTimestamp);
	float renderMs = renderBufferedMs - dt;
	if (renderMs <= 0.0) {
		m_echoHist.Reset(ctime);
		m_zerroFarend = true;
		DTRACE(VSTM_ECHO, "zero buffer: rend = %6.3f, dt = %6.3f", renderMs, dt);
	}
	else {
		m_zerroFarend = false;
		int dif = (int)((renderMs + bufferedMs + m_manualOfset) * m_processingSampleRate / 1000.0) + samples;
		int diferror = dif - farendOffset;
		int med_cr = diferror;
		vs_aec_analyze::update_hist(m_echoHist.hist_err, diferror, (double)m_processingSampleRate);
		if (m_echoHist.lsize_wnd < FILTER_LENGHT) {
			m_echoHist.window_derr[m_echoHist.lsize_wnd] = diferror;
		} else {
#ifdef AEC_ALG_AVGSORT
			int k;
			m_echoHist.window_derr[FILTER_LENGHT - 1] = diferror;
			med_cr = diferror;
			for (k = 1; k < FILTER_LENGHT; k++) {
				med_cr += m_echoHist.window_derr[k - 1];
				m_echoHist.window_derr[k - 1] = m_echoHist.window_derr[k];
			}
			med_cr /= FILTER_LENGHT;
#else
			m_echoHist.window_derr[FILTER_LENGHT - 1] = diferror;
			int tmp_wnd[FILTER_LENGHT], k;
			tmp_wnd[0] = m_echoHist.window_derr[0];
			for (k = 1; k < FILTER_LENGHT; k++) {
				tmp_wnd[k] = m_echoHist.window_derr[k];
				m_echoHist.window_derr[k - 1] = m_echoHist.window_derr[k];
			}
			qsort(tmp_wnd, FILTER_LENGHT, sizeof(int), vs_aec_analyze::compare);
			med_cr = tmp_wnd[FILTER_LENGHT / 2];
#endif
		}
		m_echoHist.lsize_wnd++;
		if (ctime - m_echoHist.last_hist_time > 5000) {
			vs_aec_analyze::calc_bound_interval(m_echoHist.hist_err, (int)m_echoHist.size_hist, &m_echoHist.int_bound, m_echoHist.min_bound);
			m_echoHist.last_hist_time = ctime;
			DTRACE(VSTM_ECHO, "hist: bnd = %4d", m_echoHist.int_bound);
		}
		DTRACE(VSTM_ECHO, "time = %8d -- stat: total = %10u, s = %4d, r = %6.3f, c = %6.3f, er = %3d | avg = %4d, off = %5d",
						(int)(ctime - m_start_time), m_samples, samples, renderMs, bufferedMs, diferror, med_cr, farendOffset);

		/*VS_EchoDebugger::GetInstance().Log() << __FUNCTION__ <<
			" time = " << (int)(ctime - m_start_time) <<
			" -- stat: total = " << m_samples <<
			", s = " << samples <<
			", r = " << renderMs <<
			", c = " << bufferedMs <<
			", er = " << diferror <<
			" | avg = " << med_cr <<
			", off = " << farendOffset << std::endl;*/

		if ((m_echoHist.lsize_wnd > static_cast<int>(FILTER_LENGHT) || m_echoHist.is_first) &&
			(abs(med_cr) > static_cast<int>(m_echoHist.int_bound * m_processingSampleRate / 1000)) &&
			!m_DelayDetector.IsPatternAdding())
		{
			DTRACE(VSTM_ECHO, "time = %8d -- jump: s = %10u | from %5d to %5d | bnd = %3d | avg = %5d",
				(int)(ctime - m_start_time), m_samples, farendOffset, farendOffset + med_cr, m_echoHist.int_bound, med_cr);
			/*VS_EchoDebugger::GetInstance().Log() << __FUNCTION__ <<
				" time = " << (int)(ctime - m_start_time) <<
				" -- jump: s = " << m_samples <<
				" | from " << farendOffset <<
				" to " << farendOffset + med_cr <<
				" | bnd = " << m_echoHist.int_bound <<
				" | avg = " << med_cr << std::endl;*/

			m_farendOffsetSamples += med_cr;
			m_echoHist.Reset(ctime);
			is_reset_ec = abs(med_cr) > static_cast<int>(0.00141* m_echoHist.min_bound * m_processingSampleRate); // sqrt(2)/1000*...
		}

		m_echoHist.is_first = false;
	}

	if (is_reset_ec)
		m_ec->Init(m_processingSampleRate);
}

void VS_AudioEchoCancel::ProcessCaptureAudio(webrtc::AudioBuffer *ca)
{
	//VS_EchoDebugger::GetInstance().Log() << __FUNCTION__ << std::endl;
	//const VSAutoLock _lock(*this);

	m_stream_has_echo = false;
	if (!is_enabled()) {
		return;
	}
	int samples = ca->num_frames();
	{
		const VS_AutoLock _lock(this);
		if (m_zerroFarend) {
			//VS_EchoDebugger::GetInstance().Log() << __FUNCTION__ << " m_zerroFarend" << std::endl;
			memset(m_bufferFarend, 0, samples * sizeof(short));
		}
		else {
			if (!(m_farendBarrel->Read(m_bufferFarend, samples, (uint32_t)(m_farendOffsetSamples)))) {
				//VS_EchoDebugger::GetInstance().Log() << __FUNCTION__ << " samples:" << samples << " m_farendOffsetSamples:" << m_farendOffsetSamples << std::endl;
				memset(m_bufferFarend, 0, samples * sizeof(short));
			}
		}
		m_farendOffsetSamples -= samples;
	}

	//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_FAREND, m_bufferFarend, samples * sizeof(m_bufferFarend[0]));
//	DebugFarendWrite(m_bufferFarend, samples);

	m_samples += ca->num_frames();

	{
		for (int i = 0; i < samples; i++) {
			if (m_bufferFarend[i] >= 0) {
				m_rab[0][i] = (float)(m_bufferFarend[i] / 32767.0);
			} else {
				m_rab[0][i] = (float)(m_bufferFarend[i] / 32768.0);
			}
		}

		webrtc::StreamConfig sc(m_processingSampleRate, 1);

		//prev m_ra->CopyFrom(m_rab, samples, sc);
		m_ra->CopyFrom(m_rab, sc);
		if (m_processingSampleRate >= webrtc::AudioProcessing::kSampleRate32kHz) {
			m_ra->SplitIntoFrequencyBands(); // TO DO : need merge or not ???
		}
	}

	VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_FAREND, m_ra->channels()[0], m_ra->num_frames() * sizeof(int16_t));
	VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_NEAREND, ca->channels()[0], ca->num_frames() * sizeof(int16_t));

	m_DelayDetector.ProcessAudio(ca->channels()[0], m_ra->channels()[0], m_ra->num_frames());

	m_ec->Cancellate(m_ra, ca, &m_stream_has_echo);

	//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_ECHO, ca->channels()[0], ca->num_frames() * sizeof(int16_t));
}

bool VS_AudioEchoCancel::StreamHasEcho()
{
	return m_stream_has_echo;
}

bool VS_AudioEchoCancel::is_enabled() const
{
	return m_Enabled;
}
