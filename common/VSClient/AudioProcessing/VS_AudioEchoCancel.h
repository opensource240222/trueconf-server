/**
 **************************************************************************
 * \file VSAudioEchoCancel.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Wraper to real Echo Cancelation. Use EchoCancelation class from Audio
 *
 * \b Project Client
 * \author SMirnovK
 * \date 26.03.2003
 *
 * $Revision: 16 $
 *
 * $History: VSAudioEchoCancel.h $
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 21.04.10   Time: 15:54
 * Updated in $/VSNA/VSClient
 * - MAX_NUM_ECHO_CHANNEL = 64
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 16.04.10   Time: 14:53
 * Updated in $/VSNA/VSClient
 * - were enhancement aec (increase resampling precision, statistic queue
 * lenght, upper bandwith for skip)
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 6.04.10    Time: 15:21
 * Updated in $/VSNA/VSClient
 * - update aec statistics
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 24.09.09   Time: 14:07
 * Updated in $/VSNA/VSClient
 * - fix aec crash. increase maximum number of aec chanel
 * (MAX_NUM_ECHO_CHANNEL = 12).
 * - new agc
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 20.07.09   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - debuging bug
 * - fix aec for Vista
 * - change directx version detect
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 18.05.09   Time: 15:07
 * Updated in $/VSNA/VSClient
 * - were fixed Echo Module: always first jump after init arender, not
 * change offset when arender is stoped
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 15.05.09   Time: 19:43
 * Updated in $/VSNA/VSClient
 * - were adapted AEC for Direct Sound
 * - were fixed calculation buffer duration in DS
 * - were fixed calculation write position in VS_AudioBarrel
 * - were fixed some AEC bug: reset statistic for capture, render time
 * calculation
 * - were improved AEC: histogram statistic, adaptive range jump, average
 * replaced median for calculation offset error
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 1.04.09    Time: 19:23
 * Updated in $/VSNA/VSClient
 * - bugfix #5815
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 18.03.09   Time: 16:22
 * Updated in $/VSNA/VSClient
 * - were separated preprocessing amd aec ib Global Echo Module
 * (preprocessing is always work)
 * - were added AGC in speex preprocessing
 * - update speex lib (include float-point agc version)
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 5.03.09    Time: 14:59
 * Updated in $/VSNA/VSClient
 * - were added support multi-cnnel aec in VS_GlobalEcho &
 * VS_SpeexEchoCancel
 * - were removed skip-frames for first calculation audio render frequency
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 9.02.09    Time: 9:54
 * Updated in $/VSNA/VSClient
 * - were improved speex aec
 * - were added audio devices frequency calculate
 * - were added speex resample in echo module
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 23.12.08   Time: 13:07
 * Updated in $/VSNA/VSClient
 * - capability of use aec dll library is added
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 5.12.08    Time: 15:13
 * Updated in $/VSNA/VSClient
 * - aec: were changed manual offset for near end audio signal
 * - aec: were changed algorithm offset correction (for far end audio
 * signal)
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 7.04.08    Time: 16:58
 * Updated in $/VSNA/VSClient
 * - change fixed direct port to random one
 * - zoomchat defines refreshed
 * - file size decreased
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 23.12.07   Time: 18:33
 * Updated in $/VS2005/VSClient
 * - more smart ofset calc
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 20.12.07   Time: 16:14
 * Updated in $/VS2005/VSClient
 * - added software AEC
 * - Speex AEC improved
 * - Audio MediaFormat changed to commit Speex preprocess
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 25  *****************
 * User: Smirnov      Date: 15.11.05   Time: 18:26
 * Updated in $/VS/VSClient
 * - new projects configurations
 * - ZoomChat client dll size decreased
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 4.02.05    Time: 17:38
 * Updated in $/VS/VSClient
 * comented echocancel
 * latency analis window = 30 msec
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 3.02.05    Time: 20:22
 * Updated in $/VS/VSClient
 * EcHo cancel repaired
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/
#pragma once

/****************************************************************************
 * Includes
 ****************************************************************************/
#include <stdint.h>
#include <queue>
#include <cstdio>
#include <atomic>

#include "../../std/cpplib/VS_MediaFormat.h"
#include "../../Transcoder/VS_AudioReSampler.h"
#include "../../std/cpplib/VS_Lock.h"
#include "../VSAudioEchoCancel.h"
#include "modules/audio_processing/audio_buffer.h"
#include "VSEchoDelayDetector.h"
//#include "../../Services/Managers/VS_DebugTuningStorage.h"

#define MAX_NUM_ECHO_CHANNEL (64)

#if !defined (DEBUG_ECHO_WAVFILES_WRITE)
// #define DEBUG_ECHO_WAVFILES_WRITE
#endif

#if !defined (DEBUG_ECHO_WAVFILES_THREAD)
// #define DEBUG_ECHO_WAVFILES_THREAD
#endif

#if !defined (DEBUG_ECHO_TXT)
// #define DEBUG_ECHO_TXT
#endif

#if defined (DEBUG_ECHO_WAVFILES_THREAD)
class VSThreadedLogger;
#endif


/****************************************************************************
 * Classes
 ****************************************************************************/
class VSEchoCancelBase;
class VS_AudioPlaybackBase;
class VS_AudioBarrel;
class VS_EchoDebugger;

/**
 **************************************************************************
 * \brief Virtual empty class  interface
 ****************************************************************************/

int compare(const void *el1, const void *el2);
void reset_hist(int *hist, uint32_t size);
void update_hist(int *hist, int num_smpl, double freq);
void calc_bound_interval(int *hist, int size, int *bound, int min_bound);

/**
 **************************************************************************
 * Echo Cancelation class
 ****************************************************************************/

#define FILTER_LENGHT	(11)

struct VS_EchoHistogram
{
	int					window_derr[FILTER_LENGHT];
	int					lsize_wnd;
	bool				is_first;
	int					min_bound;
	uint32_t			size_hist;
	int					*hist_err;
	uint_fast64_t		last_hist_time;
	int					int_bound;

public:

	VS_EchoHistogram() {
		hist_err = nullptr;

#if defined(__linux) || defined(__FreeBSD__)
		min_bound = 16;
#else
		min_bound = 10;
#endif

		Clear();
	}

	~VS_EchoHistogram() {
		Clear();
	}

	void Init(int sizeHist, uint_fast64_t ct) {
		Clear();
		size_hist = sizeHist;
		hist_err = new int[size_hist];
		Reset(ct);
	}

	void Clear() {
		delete[] hist_err; hist_err = nullptr;
		lsize_wnd = 0;
		is_first = true;
		size_hist = 0;
		last_hist_time = 0;
		int_bound = 28;
		memset(window_derr, 0, sizeof(int) * FILTER_LENGHT);
	}

	void Reset(uint_fast64_t ct) {
		lsize_wnd = 0;
		last_hist_time = ct;
		memset(hist_err, 0, size_hist * sizeof(int));
	}

};

class VS_AudioEchoCancel : public VS_Lock
{
	VSEchoCancelBase		*m_ec;
	VS_EchoHistogram		m_echoHist;

	short					*m_bufferFarend;
	webrtc::AudioBuffer		*m_ra;
	float					*m_rab[2]; // 2 channel max
	VS_AudioBarrel			*m_farendBarrel;

	int						m_manualOfset;
	uint_fast64_t			m_start_time;
	uint32_t				m_processingSampleRate;
	bool					m_zerroFarend;
	unsigned int			m_samples;
	bool					m_stream_has_echo;
	std::atomic<int>		m_farendOffsetSamples;
	float					m_renderBufferedMs;
	uint_fast64_t			m_renderTimestamp;
	bool					m_Enabled;

	VSEchoDelayDetector		m_DelayDetector;

public:

	VS_AudioEchoCancel();
	~VS_AudioEchoCancel();
	void AnalyzeCaptureAudio(double capturerFrequency, float bufferedMs, int samples);
	void ProcessRenderAudio(const int16_t *farEnd, double rendererFrequency, float bufferedMs, int samples, uint_fast64_t ct);
	void ProcessCaptureAudio(webrtc::AudioBuffer *ca);
	// ProcessingComponent implementation.
	int Initialize(int sample_rate_hz);
	int Destroy();
	// VS_AudioEchoCancelBase
	bool is_enabled() const;
	void Enable(bool enable);
	bool StreamHasEcho();

	void SetManualOffset(int manualOffset);
	void ResetManualOffset();

	void StartDelayDetectTest();
	void StopDelayDetectTest();
	VSEchoDelayDetector::EGetDelayResult GetEchoDelay(int32_t& delay);

protected:

	bool Init(uint32_t frequency);
	void Release();

private:
	void CloseRendererStream();
	void OpenRendererStream();
};

