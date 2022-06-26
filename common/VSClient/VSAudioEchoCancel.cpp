/**
 **************************************************************************
 * \file VSAudioEchoCancel.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief VS_GlobalEcho Implementation
 *
 * \b Project Client
 * \author SMirnovK
 * \date 26.03.2003
 *
 * $Revision: 25 $
 *
 * $History: VSAudioEchoCancel.cpp $
 *
 * *****************  Version 25  *****************
 * User: Sanufriev    Date: 29.06.12   Time: 14:04
 * Updated in $/VSNA/VSClient
 * - webrtc aec + ns
 *
 * *****************  Version 24  *****************
 * User: Sanufriev    Date: 27.06.12   Time: 16:49
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 17.09.10   Time: 21:27
 * Updated in $/VSNA/VSClient
 * - do not write sound files
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 18.08.10   Time: 14:09
 * Updated in $/VSNA/VSClient
 * - vad corrected
 * - alfa Native freq
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 21.04.10   Time: 18:52
 * Updated in $/VSNA/VSClient
 * - round error found
 *
 * *****************  Version 20  *****************
 * User: Sanufriev    Date: 16.04.10   Time: 14:53
 * Updated in $/VSNA/VSClient
 * - were enhancement aec (increase resampling precision, statistic queue
 * lenght, upper bandwith for skip)
 *
 * *****************  Version 19  *****************
 * User: Sanufriev    Date: 6.04.10    Time: 15:21
 * Updated in $/VSNA/VSClient
 * - update aec statistics
 *
 * *****************  Version 18  *****************
 * User: Sanufriev    Date: 24.11.09   Time: 17:47
 * Updated in $/VSNA/VSClient
 * - fix memory leak for aec
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 27.10.09   Time: 16:56
 * Updated in $/VSNA/VSClient
 * - aec, bugfix #6565
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 20.07.09   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - debuging bug
 * - fix aec for Vista
 * - change directx version detect
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 17.06.09   Time: 18:57
 * Updated in $/VSNA/VSClient
 * - echocancel small impr
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 16.06.09   Time: 17:18
 * Updated in $/VSNA/VSClient
 * - fix aec (change position restart render, reinit aec stat after jump)
 * - update h.264 libs (link icc libs)
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 18.05.09   Time: 15:56
 * Updated in $/VSNA/VSClient
 * - were removed noise insert in CheckNoiseInsert(), only checked
 * ARENDER_NOISE_MAXTIME for all devices
 * - were added first jump after zero buffer for WaveOut device in Echo
 * Module
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 18.05.09   Time: 15:07
 * Updated in $/VSNA/VSClient
 * - were fixed Echo Module: always first jump after init arender, not
 * change offset when arender is stoped
 *
 * *****************  Version 11  *****************
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
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 1.04.09    Time: 19:23
 * Updated in $/VSNA/VSClient
 * - bugfix #5815
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 18.03.09   Time: 16:22
 * Updated in $/VSNA/VSClient
 * - were separated preprocessing amd aec ib Global Echo Module
 * (preprocessing is always work)
 * - were added AGC in speex preprocessing
 * - update speex lib (include float-point agc version)
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 5.03.09    Time: 14:59
 * Updated in $/VSNA/VSClient
 * - were added support multi-cnnel aec in VS_GlobalEcho &
 * VS_SpeexEchoCancel
 * - were removed skip-frames for first calculation audio render frequency
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 9.02.09    Time: 12:41
 * Updated in $/VSNA/VSClient
 * - were fixed speex resample for fixed point and removed
 * speex_resample.lib
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
 * User: Sanufriev    Date: 22.05.08   Time: 17:31
 * Updated in $/VSNA/VSClient
 * - were replace Speex libs (v1.2 beta 3)
 * - were remove audio signal clipping (for Speex)
 * - were change _TestTranscoder project
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 27.12.07   Time: 16:23
 * Updated in $/VS2005/VSClient
 * - prefiltering in software echocansellation in case of render absence
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
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 4.02.05    Time: 17:38
 * Updated in $/VS/VSClient
 * comented echocancel
 * latency analis window = 30 msec
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 3.02.05    Time: 20:22
 * Updated in $/VS/VSClient
 * EcHo cancel repaired
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "windows.h"
#include "../Audio/WinApi/dsutil.h"
#include "VSAudioEchoCancel.h"
#include "VSAudioDs.h"
#include "VS_Dmodule.h"
#include "../VSClient/VS_ApplicationInfo.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "VS_EchoDebugger.h"

/****************************************************************************
 * Defines
 ****************************************************************************/
#ifdef _DEBUG
/// uncomment this define to write cancelation voice in wav files
//#define ECHO_WAVFILES_WRITE
#endif
/****************************************************************************
 * Classes
 ****************************************************************************/
VS_AudioBarrel::VS_AudioBarrel(unsigned long samples)
{
	m_buff = (short*)malloc(samples*sizeof(short));
	m_samples = samples;
	m_WritePos = 0;
	m_ReadPos = 0;
}

VS_AudioBarrel::~VS_AudioBarrel()
{
	if (m_buff) free(m_buff);
}


void VS_AudioBarrel::Add(short* data, unsigned long samples)
{
	unsigned long csmpl = samples;
	if (samples > m_samples) {
		// add not more than m_samples
		data+=samples-m_samples;
		samples = m_samples;
	}
	unsigned long CurrPos = m_WritePos%m_samples;
	if (CurrPos + samples > m_samples) {
		unsigned long SamplesToEnd = m_samples - CurrPos;
		memcpy(m_buff + CurrPos, data, SamplesToEnd*sizeof(short));
		samples-=SamplesToEnd;
		memcpy(m_buff, data+SamplesToEnd, samples*sizeof(short));
	}
	else {
		memcpy(m_buff + CurrPos, data, samples*sizeof(short));
	}
	m_WritePos+=csmpl;
}

bool VS_AudioBarrel::Read(short* data, unsigned long samples, unsigned long OfsetFromEnd)
{
	if (m_WritePos < OfsetFromEnd || samples > OfsetFromEnd || samples > m_samples)
		return false;
	unsigned long CurrPos = (m_WritePos-OfsetFromEnd)%m_samples;
	if (CurrPos + samples > m_samples) {
		unsigned long SamplesToEnd = m_samples - CurrPos;
		memcpy(data, m_buff + CurrPos, SamplesToEnd*sizeof(short));
		samples-=SamplesToEnd;
		memcpy(data+SamplesToEnd, m_buff, samples*sizeof(short));
	}
	else {
		memcpy(data, m_buff + CurrPos, samples*sizeof(short));
	}
	return true;
}

/******************************************************************/
#define TIME_OFSET			-4

VS_GlobalEcho::VS_GlobalEcho() : m_manualOfset(TIME_OFSET)
{
	m_ec_state.clear();
	m_num_channel = 0;
	m_CuptFreq = m_RealCuptFreq = 0;
	m_RealCuptFreqFloat = 0.0;
	m_temp1 = m_temp2 = m_far_tmp = 0;
	m_last_size_far = 0;

	int ec = 1;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&ec, 4, VS_REG_INTEGER_VT, "TypeEC");
	if (ec == 0) m_type_aec = AEC_SPEEX;
	else if (ec == 1) m_type_aec = AEC_WEBRTC;
	else m_type_aec = AEC_WEBRTCFAST;

	key.GetValue(&m_manualOfset, 4, VS_REG_INTEGER_VT, "OffsetEC");

	m_ec = VS_RetriveEchoCancel((int)m_type_aec);
}

VS_GlobalEcho::~VS_GlobalEcho()
{
	Release();
	if (m_ec) delete m_ec; m_ec = 0;
}

void VS_GlobalEcho::ChangeTypeAEC(int num_channels, int frequency)
{
	if (num_channels > 1 && m_ec->GetType() != AEC_SPEEX) {
		delete m_ec;
		m_ec = VS_RetriveEchoCancel((int)AEC_SPEEX);
		m_ec->Init(frequency);
	} else if (num_channels < 2 && m_ec->GetType() != m_type_aec) {
		delete m_ec;
		m_ec = VS_RetriveEchoCancel((int)m_type_aec);
		m_ec->Init(frequency);
	}
}

void VS_GlobalEcho::Init(int CuptFreq, bool isDX10)
{
	if (m_ec == 0) return;
	VS_AutoLock lock(this);
	Release();

	m_RealCuptFreq = m_CuptFreq = CuptFreq;
	m_RealCuptFreqFloat = m_RealCuptFreq;
	int sizetemp = CuptFreq*8*sizeof(short); // 8 sec
	m_temp1 = (short*)malloc(sizetemp);
	m_temp2 = (short*)malloc(sizetemp);
	m_far_tmp = (short*)malloc(1920 * sizeof(short) * MAX_NUM_ECHO_CHANNEL);
	memset(m_far_tmp, 0, 1920 * sizeof(short) * MAX_NUM_ECHO_CHANNEL);
	m_last_size_far = 1920;
	if (isDX10)
		m_min_bound = 16;
	else
		m_min_bound = 8;
	DebugNew();

	m_ec->Init(m_CuptFreq);
	m_IsValid = true;	// capture is valid

	DTRACE(VSTM_ECHO, "INIT Capture f = %d", m_CuptFreq);

	if (m_num_channel > 0) {
		ChangeTypeAEC(m_num_channel, m_CuptFreq);
		m_ec->Init(m_CuptFreq, 1, m_num_channel);
	}

	m_DelayDetector.Init(2000, 2000);
}

void VS_GlobalEcho::Release()
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_ECHO, "RELEASE Capture IsValid = %d", m_IsValid);

	if (m_IsValid) {
		m_IsValid = false;
		m_ec->Release();
		free(m_temp1);
		free(m_temp2);
		free(m_far_tmp);
		m_CuptFreq = m_RealCuptFreq = 0;
		m_RealCuptFreqFloat = 0.0;
		m_temp1 = m_temp2 = m_far_tmp = 0;
		m_last_size_far = 0;
		DebugDelete();
	}
}

#define CONFIDENCE_BAND (90)
#define BIN_RANGE		(4)
#define	INTERVAL_WIDTH	(50)

void VS_GlobalEcho::OpenEchoChannel(void* handle_channel, int RendFreq)
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_ECHO, "INIT Render %x, f = %d, valid=%d", handle_channel, RendFreq, m_IsValid);

	ec_iter iter = m_ec_state.find(handle_channel);
	if (iter != m_ec_state.end()) return;

	VS_EchoChannelState st;
	memset(&st, 0, sizeof(VS_EchoChannelState));
	st.RendFreq = RendFreq;
	st.id_channel = m_num_channel;
	st.out_abuffer = new VS_AudioBarrel();
	st.rs = new VS_AudioReSamplerSpeex();
	st.size_hist = 2 * INTERVAL_WIDTH / BIN_RANGE + 1;
	st.hist_err = (int*)malloc(st.size_hist*sizeof(int));
	st.last_hist_time = timeGetTime();
	st.int_bound = 28;
	st.is_first = true;
	st.m_samples = 0;
	memset(st.hist_err, 0, st.size_hist*sizeof(int));
	m_ec_state.emplace(handle_channel, st);
	DebugNew(handle_channel);
	m_num_channel++;

	if (m_IsValid) {
		ChangeTypeAEC(m_num_channel, m_CuptFreq);
		m_ec->Init(m_CuptFreq, 1, m_num_channel);
	}
}

void VS_GlobalEcho::CloseEchoChannel(void* handle_channel)
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_ECHO, "RELEASE Render %x, valid=%d", handle_channel, m_IsValid);

	ec_iter iter = m_ec_state.find(handle_channel);
	if (iter == m_ec_state.end()) return;

	if (iter->second.out_abuffer) delete iter->second.out_abuffer; iter->second.out_abuffer = 0;
	if (iter->second.rs) delete iter->second.rs; iter->second.rs = 0;
	if (iter->second.hist_err) free(iter->second.hist_err); iter->second.hist_err = 0;
	DebugDelete(handle_channel);
	m_ec_state.erase(iter);
	m_num_channel--;
	ec_iter i = m_ec_state.begin();
	for (int k = 0; k < m_num_channel; k++) {
		i->second.id_channel = k;
		i++;
	}
	if (m_IsValid) {
		ChangeTypeAEC(m_num_channel, m_CuptFreq);
		m_ec->Init(m_CuptFreq, 1, m_num_channel);
	}
}

void VS_GlobalEcho::AddOutBufer(short* out, int samples, double RendFreq, void* handle_channel)
{
	VS_AutoLock lock(this);
	if (!m_IsValid) return;

	ec_iter iter = m_ec_state.find(handle_channel);
	if (iter == m_ec_state.end()) return;
	VS_EchoChannelState *st = &(iter->second);

	double fdf = m_RealCuptFreqFloat - RendFreq;
	int df = (int)(fdf + (fdf > 0. ? 0.5 : -0.5));

	st->RendFreq = m_RealCuptFreq - df;

	for (int i = 0; i < samples; i++)
		m_temp2[i] = out[i];

	DebugWrite(0, 0, m_temp2, 0, samples, iter->first);
	//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_FAREND, m_temp2, samples * sizeof(m_temp2[0]));

	if (m_RealCuptFreq > 0 && abs(df) > 0) {
		samples = st->rs->Process(m_temp2, m_temp1, samples*2, m_RealCuptFreq - df, m_RealCuptFreq)/2;
		st->out_abuffer->Add(m_temp1, samples);
	} else {
		st->out_abuffer->Add(m_temp2, samples);
	}

	st->Offset+=samples;
}

int compare(const void *el1, const void *el2)
{
	int a = *(int*)el1,
		b = *(int*)el2;
	return ((a > b) ? 1 : ((a < b) ? -1 : 0));
}

void reset_hist(int *hist, int size)
{
	memset(hist, 0, size * sizeof(int));
}

void update_hist(int *hist, int num_smpl, double freq)
{
	int val = (int)(num_smpl / (freq / 1000.0));
	val = (val < -INTERVAL_WIDTH) ? -INTERVAL_WIDTH :((val > INTERVAL_WIDTH) ? INTERVAL_WIDTH : val);
	val /= BIN_RANGE;
	hist[val+INTERVAL_WIDTH/BIN_RANGE]++;
}

void calc_bound_interval(int *hist, int size, int *bound, int min_bound)
{
	int val = 0, i, sum = 0, sum_l, sum_r;
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

void VS_GlobalEcho::CanselInBuffer(short* in, int samples, int CaptTime, double CaptFreq)
{
	VS_AutoLock lock(this);
	if (!m_IsValid) return;

	if (CaptFreq) {
		m_RealCuptFreqFloat = CaptFreq;
		m_RealCuptFreq = (int)(CaptFreq + 0.5);
	}

	if (samples > m_last_size_far) {
		m_far_tmp = (short*)realloc(m_far_tmp, samples * sizeof(short) * MAX_NUM_ECHO_CHANNEL);
		memset(m_far_tmp, 0, samples * sizeof(short) * MAX_NUM_ECHO_CHANNEL);
		m_last_size_far = samples;
	}

	bool is_reset_ec = false;

	for (ec_iter i = m_ec_state.begin(), e = m_ec_state.end(); i != e; ) {
		VS_AudioDeviceCommon *arender = (VS_AudioDeviceCommon*)(i->first);
		VS_EchoChannelState *st = &(i->second);

		short *p_far = m_far_tmp + st->id_channel * samples;

		if (arender->State() != ADSTATE_START) {
			memset(p_far, 0, samples * sizeof(short));
			DTRACE(VSTM_ECHO, "arender %x is stoped: s = %d", i->first, i->second.m_samples);
			st->is_first = true;
		} else {
			int RendTime = arender->GetBufferedDurr(true);
			if (RendTime <= 0) {
				reset_hist(st->hist_err, st->size_hist);
				st->last_hist_time = timeGetTime();
				st->lsize_wnd = 0;
				memset(p_far, 0, samples * sizeof(short));
				st->Offset -= samples;
				if (arender->Type() == ADTYPE_WAVEOUT) st->is_first = true;
				DTRACE(VSTM_ECHO, "zero buffer in %x: s = %d", i->first, i->second.m_samples);
			} else {
				st->Offset -= samples;
				int dif = (RendTime * m_RealCuptFreq / st->RendFreq + CaptTime + (m_manualOfset * m_RealCuptFreq) / 1000) + samples;
				int diferror = dif - st->Offset;
				int med_cr = diferror;
				update_hist(st->hist_err, diferror, m_RealCuptFreq);
				if (st->lsize_wnd < FILTER_LENGHT) {
					st->window_derr[st->lsize_wnd] = diferror;
				} else {
	#ifdef AEC_ALG_AVGSORT
					int k;
					st->window_derr[FILTER_LENGHT-1] = diferror;
					med_cr = diferror;
					for (k = 1; k < FILTER_LENGHT; k++) {
						med_cr += st->window_derr[k-1];
						st->window_derr[k-1] = st->window_derr[k];
					}
					med_cr /= FILTER_LENGHT;
	#else
					st->window_derr[FILTER_LENGHT-1] = diferror;
					int tmp_wnd[FILTER_LENGHT], k;
					tmp_wnd[0] = st->window_derr[0];
					for (k = 1; k < FILTER_LENGHT; k++) {
						tmp_wnd[k] = st->window_derr[k];
						st->window_derr[k-1] = st->window_derr[k];
					}
					qsort(tmp_wnd, FILTER_LENGHT, sizeof(int), compare);
					med_cr = tmp_wnd[FILTER_LENGHT/2];
	#endif
				}
				st->lsize_wnd++;
				int ctime = timeGetTime();
				if (ctime - st->last_hist_time > 5000) {
					calc_bound_interval(st->hist_err, st->size_hist, &st->int_bound, m_min_bound);
					st->last_hist_time = ctime;
					DTRACE(VSTM_ECHO, "hist %x : bnd = %d", i->first, st->int_bound);
				}
				if ((st->lsize_wnd > FILTER_LENGHT || st->is_first) && (abs(med_cr) > st->int_bound * m_CuptFreq / 1000) &&
					(st->is_first || m_DelayDetector.IsPatternAdding()))
				{
					DTRACE(VSTM_ECHO, "jump %x : s = %d | from %5d to %5d | cpt = %d | rnd = %d | bnd = %d | avg = %d",
									  i->first, i->second.m_samples, st->Offset, st->Offset + med_cr, m_RealCuptFreq, st->RendFreq, st->int_bound, med_cr);
					st->Offset += med_cr;
					st->lsize_wnd = 0;
					reset_hist(st->hist_err, st->size_hist);
					st->last_hist_time = ctime;
					is_reset_ec = true;
				}
				DTRACE(VSTM_ECHO, "stat %x : s=%10d, r=%3d, c=%3d, er=%3d | avg=%4d",
								  i->first, i->second.m_samples, RendTime*1000/st->RendFreq, CaptTime*1000/m_CuptFreq, diferror, med_cr);
				if (!(st->out_abuffer->Read(p_far, samples, st->Offset))) {
					memset(p_far, 0, samples * sizeof(short));
				}
				st->prev_derr = diferror;
				st->is_first = false;
			}
		}
		DebugWrite(0, p_far, 0, 0, samples, i->first);
		st->m_samples += samples;
		i++;
	}

	if (is_reset_ec && m_num_channel == 1)
		m_ec->Init(m_CuptFreq, 1, m_num_channel);
	memcpy(m_temp2,in,sizeof(short)*samples);

	m_DelayDetector.ProcessAudio(in, m_far_tmp, samples);

	m_ec->Cancellate(m_far_tmp, in, m_temp2, samples);
	if (m_num_channel > 0)
	{
		DebugWrite(in, 0, 0, m_temp2, samples);
		//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_FAREND,  m_far_tmp, samples * sizeof(m_temp2[0]));
		//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_NEAREND, in,        samples * sizeof(m_temp2[0]));
		//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_ECHO,    m_temp2,   samples * sizeof(m_temp2[0]));
	}

	memcpy(in, m_temp2, samples*sizeof(short));
}

void VS_GlobalEcho::AddPattern(short* out, int samples)
{
	m_DelayDetector.AddPatternToRenderedAudio(out, samples);
}

void VS_GlobalEcho::SetEchoDelay(int32_t delay)
{
	m_manualOfset = delay;
}

void VS_GlobalEcho::ResetEchoDelay()
{
	m_manualOfset = TIME_OFSET;

	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&m_manualOfset, 4, VS_REG_INTEGER_VT, "OffsetEC");
}

VSEchoDelayDetector::EGetDelayResult VS_GlobalEcho::GetEchoDelay(int32_t& delay)
{
	return m_DelayDetector.GetDelay(delay);
}

void VS_GlobalEcho::StartDelayDetectTest()
{
	m_DelayDetector.Start();
}

void VS_GlobalEcho::StopDelayDetectTest()
{
	m_DelayDetector.Stop();
}

#ifdef ECHO_WAVFILES_WRITE
void VS_GlobalEcho::DebugNew(void* handle_channel)
{
	WAVEFORMATEX wf;
	wf.nAvgBytesPerSec = m_CuptFreq*2;
	wf.nBlockAlign = 2;
	wf.nChannels = 1;
	wf.nSamplesPerSec = m_CuptFreq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;

	if (handle_channel == 0) {
		m_inf = new CWaveFile;
		m_compf = new CWaveFile;
		m_inf->Open("m_inf.wav", &wf, WAVEFILE_WRITE);
		m_compf->Open("m_compf.wav", &wf, WAVEFILE_WRITE);
	} else {
		ec_iter iter = m_ec_state.find(handle_channel);
		if (iter == m_ec_state.end()) return;
		char fn[128];
		VS_EchoChannelState *st = &iter->second;
		st->outf = new CWaveFile;
		sprintf(fn, "m_outf_%x.wav", handle_channel);
		st->outf->Open(fn, &wf, WAVEFILE_WRITE);
		sprintf(fn, "m_outf_norsmpl_%x.wav", handle_channel);
		st->outf_no_rsmpl = new CWaveFile;
		st->outf_no_rsmpl->Open(fn, &wf, WAVEFILE_WRITE);
	}
}
void VS_GlobalEcho::DebugWrite(short *in, short* out, short* out_nr, short *comp, int samples, void* handle_channel)
{
	UINT szwrote = 0;
	UINT szwrite = samples*2;
	if (handle_channel == 0) {
		m_inf->Write(szwrite, (BYTE*)in, &szwrote);
		m_compf->Write(szwrite, (BYTE*)comp, &szwrote);
	} else {
		ec_iter iter = m_ec_state.find(handle_channel);
		if (iter == m_ec_state.end()) return;
		VS_EchoChannelState *st = &iter->second;
		if (out)
			st->outf->Write(szwrite, (BYTE*)out, &szwrote);
		else
			st->outf_no_rsmpl->Write(szwrite, (BYTE*)out_nr, &szwrote);
	}
}
void VS_GlobalEcho::DebugDelete(void* handle_channel)
{
	if (handle_channel == 0) {
		if (m_inf) m_inf->Close(); delete m_inf;
		if (m_compf) m_compf->Close(); delete m_compf;
		m_inf = m_compf = 0;
	} else {
		ec_iter iter = m_ec_state.find(handle_channel);
		if (iter == m_ec_state.end()) return;
		VS_EchoChannelState *st = &iter->second;
		if (st->outf) st->outf->Close(); delete st->outf; st->outf = 0;
		if (st->outf_no_rsmpl) st->outf_no_rsmpl->Close(); delete st->outf_no_rsmpl; st->outf_no_rsmpl = 0;
	}
}
#else
void VS_GlobalEcho::DebugNew(void* handle_channel) {}
void VS_GlobalEcho::DebugWrite(short *in, short* out, short* out_nr, short *comp, int samples, void* handle_channel) {}
void VS_GlobalEcho::DebugDelete(void* handle_channel) {}
#endif