/**
 **************************************************************************
 * \file VSAudioUtil.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Time deviation, Audio Buffer, FIFO buffer classes
 *
 * \b Project Client
 * \author SMirnovK
 * \date 19.11.2004
 *
 * $Revision: 26 $
 *
 * $History: VSAudioUtil.cpp $
 *
 * *****************  Version 26  *****************
 * User: Sanufriev    Date: 28.09.11   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - beta nhp revision
 * - fix fps on low bitrates
 *
 * *****************  Version 25  *****************
 * User: Sanufriev    Date: 30.03.11   Time: 18:19
 * Updated in $/VSNA/VSClient
 * - fix overflow
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 2.11.10    Time: 17:20
 * Updated in $/VSNA/VSClient
 * - restrict AGC
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 18.10.10   Time: 19:47
 * Updated in $/VSNA/VSClient
 * - agc is gooood
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 6.10.10    Time: 20:42
 * Updated in $/VSNA/VSClient
 * - agc: try to use disp
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 17.09.10   Time: 21:25
 * Updated in $/VSNA/VSClient
 * - agc corrected by Disp
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 18.08.10   Time: 19:42
 * Updated in $/VSNA/VSClient
 * - returned old agc
 * - old devices
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 18.08.10   Time: 14:09
 * Updated in $/VSNA/VSClient
 * - vad corrected
 * - alfa Native freq
 *
 * *****************  Version 18  *****************
 * User: Sanufriev    Date: 12.07.10   Time: 18:36
 * Updated in $/VSNA/VSClient
 * - fix AV in AviWriter (incorrect number of rays)
 * - DTRACE for AGC
 * - were added DTRACE log fro AviWriter
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 28.04.10   Time: 14:36
 * Updated in $/VSNA/VSClient
 * - preamp in agc
 * - use variance in agc
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 14.04.10   Time: 19:26
 * Updated in $/VSNA/VSClient
 * - koef in agc
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 5.04.10    Time: 18:56
 * Updated in $/VSNA/VSClient
 * - overload behavior changed
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 22.03.10   Time: 15:07
 * Updated in $/VSNA/VSClient
 * - agc with overloads counter
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 11.03.10   Time: 17:41
 * Updated in $/VSNA/VSClient
 * - fix overflow
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 19.02.10   Time: 20:39
 * Updated in $/VSNA/VSClient
 * - new AGC
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 2.11.09    Time: 10:51
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 27.10.09   Time: 16:56
 * Updated in $/VSNA/VSClient
 * - aec, bugfix #6565
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 24.09.09   Time: 14:07
 * Updated in $/VSNA/VSClient
 * - fix aec crash. increase maximum number of aec chanel
 * (MAX_NUM_ECHO_CHANNEL = 12).
 * - new agc
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 3.06.09    Time: 11:51
 * Updated in $/VSNA/VSClient
 * - some AGC impovements
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 2.06.09    Time: 15:35
 * Updated in $/VSNA/VSClient
 * - min autogain decreased
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 9.02.09    Time: 9:54
 * Updated in $/VSNA/VSClient
 * - were improved speex aec
 * - were added audio devices frequency calculate
 * - were added speex resample in echo module
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 26.11.08   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - temporary disable rising of microphone level
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 25.09.08   Time: 15:27
 * Updated in $/VSNA/VSClient
 * - were fixed GetBufferedDurr() (decrease probability of threads
 * conflict on multi-core cpu)
 * - were changed jitter algorithm
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 9.07.08    Time: 13:08
 * Updated in $/VSNA/VSClient
 * - were modified audio render algorithm
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 22.11.07   Time: 15:17
 * Updated in $/VS2005/VSClient
 *  - fix slow growing in AGC
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 15.11.07   Time: 21:10
 * Updated in $/VS2005/VSClient
 * - new AG Control
 * - fixed bug with audio capture devices having no control
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 5.04.06    Time: 17:24
 * Updated in $/VS/VSClient
 * - low-level audio devices
 * - Direct Sound devices added
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 9.03.06    Time: 17:38
 * Updated in $/VS/VSClient
 * - AGC implemented
 * - "long latency" audiorender mode
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 10.08.05   Time: 13:33
 * Updated in $/VS/VSClient
 * - new low latensy and noise generation schema
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 10.03.05   Time: 12:48
 * Updated in $/VS/VSClient
 * new sinc for audio-video
 *
 * *****************  Version 5  *****************
 * User: Admin        Date: 16.12.04   Time: 20:08
 * Updated in $/VS/VSClient
 * doxigen comments
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 9.12.04    Time: 19:48
 * Updated in $/VS/VSClient
 * last good sinc
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 8.12.04    Time: 20:26
 * Updated in $/VS/VSClient
 * new video-audio sinc
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 27.11.04   Time: 13:27
 * Updated in $/VS/VSClient
 * intefaca changed
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 24.11.04   Time: 20:13
 * Created in $/VS/VSClient
 * added new audio files and classes
*
****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSAudioUtil.h"
#include <malloc.h>
#include <string.h>
#include <math.h>
#include "VS_Dmodule.h"
#include "Transcoder/AudioCodec.h"
#include "Transcoder/VSAudioVad.h"

#include <algorithm>
#include <functional>
#include <vector>

/****************************************************************************
 * Defines
 ****************************************************************************/

/****************************************************************************
 * Classes
 ****************************************************************************/

/**
 **************************************************************************
 ****************************************************************************/

#define ARENDER_BUFFER_MAX_LENGTH (6000)

VS_ARenderAnalyse::VS_ARenderAnalyse()
{
	m_values = 0;
	m_hvalues = 0;
	m_pend_min = 0;
	m_skip = 0;
	m_empty_buff = 0;
	m_MaxCount = 0;
	m_MaxAvgCount = 0;
	m_Deviation_A = 0;
	m_Deviation_B = 0;
	m_chunk_size = 10;
	m_CurCount = 0;
	m_hMaxCount = ARENDER_BUFFER_MAX_LENGTH / m_chunk_size;
	m_IsValid = false;
	m_NewSample = false;
}

VS_ARenderAnalyse::~VS_ARenderAnalyse()
{
	if (m_values) free(m_values); m_values = 0;
	if (m_hvalues) free(m_hvalues); m_hvalues = 0;
	if (m_pend_min) free(m_pend_min); m_pend_min = 0;
	if (m_skip) free(m_skip); m_skip = 0;
	if (m_empty_buff) free(m_empty_buff); m_empty_buff = 0;
}

void VS_ARenderAnalyse::Init(int MaxCount, int MaxAvgCount)
{
	if (m_values) free(m_values); m_values = 0;
	if (m_hvalues) free(m_hvalues); m_hvalues = 0;
	if (m_pend_min) free(m_pend_min); m_pend_min = 0;
	if (m_skip) free(m_skip); m_skip = 0;
	m_MaxCount = MaxCount;
	m_MaxAvgCount = MaxAvgCount;
	m_values = (unsigned int*)malloc(MaxCount*sizeof(unsigned int));
	m_hvalues = (unsigned int*)malloc(m_hMaxCount*sizeof(unsigned int));
	m_pend_min = (unsigned int*)malloc(m_MaxAvgCount*sizeof(unsigned int));
	m_skip = (unsigned int*)malloc(m_MaxAvgCount*sizeof(unsigned int));
	m_empty_buff = (unsigned int*)malloc(m_MaxAvgCount*sizeof(unsigned int));
	memset(m_values, 0, m_MaxCount * sizeof(unsigned int));
	memset(m_hvalues, 0, m_hMaxCount * sizeof(unsigned int));
	memset(m_pend_min, 0, m_MaxAvgCount * sizeof(unsigned int));
	memset(m_skip, 0, m_MaxAvgCount * sizeof(unsigned int));
	memset(m_empty_buff, 0, m_MaxAvgCount * sizeof(unsigned int));
	m_IsValid = true;
}

void VS_ARenderAnalyse::Clear()
{
	if (m_IsValid) {
		memset(m_values, 0, m_MaxCount * sizeof(unsigned int));
		memset(m_hvalues, 0, m_hMaxCount * sizeof(unsigned int));
		memset(m_pend_min, 0, m_MaxAvgCount * sizeof(unsigned int));
		memset(m_skip, 0, m_MaxAvgCount * sizeof(unsigned int));
		memset(m_empty_buff, 0, m_MaxAvgCount * sizeof(unsigned int));
	}
	m_CurCount = 0;
}

void VS_ARenderAnalyse::Snap(int val, int pend_min)
{
	unsigned int ind = m_CurCount % m_MaxCount,
				 ind_h,
				 ind_avg = m_CurCount % m_MaxAvgCount;
	if (m_CurCount >= m_MaxCount) {
		m_hvalues[m_values[ind]]--;
	}
	ind_h = val / m_chunk_size;
	if (ind_h >= m_hMaxCount) {
		ind_h = m_hMaxCount - 1;
	}
	m_values[ind] = ind_h;
	m_hvalues[ind_h]++;
	m_pend_min[ind_avg] = pend_min;
	m_skip[ind_avg] = 0;
	m_empty_buff[ind_avg] = 0;
	m_CurCount++;
	m_NewSample = true;
}

void VS_ARenderAnalyse::SnapSkip()
{
	unsigned int ind = (m_CurCount == 0) ? m_MaxAvgCount - 1 : (m_CurCount - 1) % m_MaxAvgCount;
	m_skip[ind]++;
}

void VS_ARenderAnalyse::SnapEmptyBuffer()
{
	unsigned int ind = (m_CurCount == 0) ? m_MaxAvgCount - 1 : (m_CurCount - 1) % m_MaxAvgCount;
	m_empty_buff[ind]++;
}

bool VS_ARenderAnalyse::Calculate()
{
	if (!m_IsValid) return false;
	if (m_NewSample == false) return true;

	int i;
	int sum, sum_lim;
	double percent;
	bool is_thr = false;

	m_Deviation_A = 240;
	m_Deviation_B = 240;

	sum = (m_CurCount > m_MaxCount) ? m_MaxCount : m_CurCount;
	sum_lim = (m_MaxCount * 10) / 100;

	if (sum > sum_lim) {

		m_Deviation_A = m_Deviation_B = 0;
		percent = 0.01 * sum;
		sum = 0;
		for (i = (int)(m_hMaxCount - 1); i >= 0; i--) {
			sum += m_hvalues[i];
			if (sum >= percent) {
				m_Deviation_A = i * m_chunk_size;
				break;
			}
		}

		for (i = (int)(m_hMaxCount - 1); i >= 0; i--) {
			if (m_hvalues[i]) {
				if (is_thr) {
					m_Deviation_B = i * m_chunk_size;
					break;
				} else {
					is_thr = true;
				}
			}
		}

	}

	m_MaxBuffDur = std::max(m_Deviation_A, m_Deviation_B);
	m_MinBuffDur = std::min(m_Deviation_A, m_Deviation_B);
	m_NewSample = false;

	return true;
}

bool VS_ARenderAnalyse::GetBuffDur(int &BuffDurA, int &BuffDurB, int &MaxBuffDur, int &MinBuffDur,
								   int &GMinBuffDur, int &NumSkip, int &NumEmptyBuff)
{
	if (!Calculate())
		return false;
	BuffDurA = m_Deviation_A;
	BuffDurB = m_Deviation_B;
	MaxBuffDur = m_MaxBuffDur;
	MinBuffDur = m_MinBuffDur;
	int num = (m_CurCount > m_MaxAvgCount) ? m_MaxAvgCount : m_CurCount;
	GMinBuffDur = m_pend_min[0];
	NumSkip = m_skip[0];
	NumEmptyBuff = m_empty_buff[0];
	for (int i = 1; i < num; i++) {
		if ((unsigned int)GMinBuffDur > m_pend_min[i]) GMinBuffDur = m_pend_min[i];
		NumSkip += m_skip[i];
		NumEmptyBuff += m_empty_buff[i];
	}

	return true;
}

/**
 **************************************************************************
 ****************************************************************************/

VS_FreqDeviation::VS_FreqDeviation()
{
	m_values_x = 0;
	m_values_y = 0;
	m_MaxCount = 0;
	m_CurrCount = 0;
	m_IsValid = false;
	m_NewSample = true;
}

VS_FreqDeviation::~VS_FreqDeviation()
{
	if (m_values_x) free(m_values_x);
	if (m_values_y) free(m_values_y);
}

/**
 **************************************************************************
 * \param	MaxCount [in] max number of analised samples
 ****************************************************************************/
void VS_FreqDeviation::Init(int MaxCount)
{
	if (m_values_x) free(m_values_x);
	if (m_values_y) free(m_values_y);
	m_MaxCount = MaxCount;
	m_values_x = (double*)malloc(MaxCount*sizeof(double));
	m_values_y = (double*)malloc(MaxCount*sizeof(double));
	m_CurrCount = 0;
	m_IsValid = true;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_FreqDeviation::Clear()
{
	m_CurrCount = 0;
}

/**
 **************************************************************************
 * \return -1 if not enouth accumulated  data
 ****************************************************************************/
double VS_FreqDeviation::GetA1()
{
	if (!m_IsValid) return -1.0;
	double a0 = 0., a1 = 0.;
	if (!Predict(a0, a1))
		return -1.0;
	else
		return a1;
}

void VS_FreqDeviation::GetA(double &a0, double &a1)
{
	a0 = 0.0;
	a1 = 0.0;
	if (m_IsValid) Predict(a0, a1);
}

/**
 **************************************************************************
 * \param		a0 [in/out] a0 coef
 * \param		a1 [in/out] a1 coef
 * \return false if not enouth accumulated  data
 ****************************************************************************/
bool VS_FreqDeviation::Predict(double &a0, double &a1)
{
	a0 = 0.; a1 = 0.;
	if (m_CurrCount < m_MaxCount/50 || (m_CurrCount<5 && m_MaxCount>5))
		return false; // min 2 % of data or 5 count!
	double A0 , A1, A2, B, D;
	double valx, valy;

	A0 = A1 = A2 =  B = D = 0.;
	int count = m_CurrCount<m_MaxCount ? m_CurrCount : m_MaxCount;
	for (int i = m_CurrCount - count; i<m_CurrCount; i++) {
		valy = m_values_y[i%m_MaxCount];
		valx = m_values_x[i%m_MaxCount];
		A0 += 1.;
		A1 += valx;
		A2 += valx * valx;
		B += valy;
		D += valy * valx;
	}
	if (A1!=0. && (A0*A2-A1*A1)!=0.) {
		a0 = (B*A2-D*A1)/(A0*A2-A1*A1);
		a1 = (B-A0*a0)/A1;
		return true;
	}
	else return false;
}

/**
 **************************************************************************
 * \param		val [in] new current time value
 ****************************************************************************/
void VS_FreqDeviation::Snap(double valx, double valy)
{
	int ind = m_CurrCount%m_MaxCount;
	m_values_x[ind] = valx;
	m_values_y[ind] = valy;
	m_CurrCount++;
	m_NewSample = true;
}

void VS_FreqDeviation::DumpData(double dt)
{
	int count = m_CurrCount<m_MaxCount ? m_CurrCount : m_MaxCount;
	char fname[256];
	sprintf(fname, "%8.2f.txt", dt);
	FILE *f = fopen(fname, "w+");
	for (int i = m_CurrCount - count; i<m_CurrCount; i++) {
		fprintf(f, "dt = %8.2f, ds = %8.2f\n", m_values_x[i%m_MaxCount], m_values_y[i%m_MaxCount]);
	}
	fclose(f);
}

//////////////////////////////////////////////////////////////////////////////
// VS_AutoGain
//////////////////////////////////////////////////////////////////////////////
VS_AutoGain::VS_AutoGain()
{
	m_freq = 0;
	m_LastTime = m_NoLog = m_MixerCount = m_Counter = 0;
	m_oldGain = 0.0;
	m_maxDisp = 0.0;
	m_minFloor = 0.0;
	m_vad = new VSAudioVAD;
	m_wasOverload = 0;
	m_preamp = 0;
}

VS_AutoGain::~VS_AutoGain()
{
	if (m_vad) delete m_vad;
}


bool VS_AutoGain::SetMode(unsigned long freq)
{
	m_freq = freq;
	m_LastTime = 0;
	m_Counter = 0;
	m_oldGain = 0.;
	m_maxDisp = 20.;
	m_minFloor = 0.0;
	m_preamp = 0;
	memset(m_log, 0, sizeof (m_log));
	m_vad->Init(freq, 16);
	m_buff.SetSize(freq*4);
	m_lastx = 0;
	m_lasty = 0;
	return true;
}



void VS_AutoGain::BqProcess(short *x, int n)
{
	short* y = (short*)m_buff.Buffer();
	for (int i = 0; i < n; i++) {
		y[i] = x[i] - m_lastx + (m_lasty*15/16);
		m_lastx = x[i];
		m_lasty = y[i];
	}
}


long VS_AutoGain::AnaliseGain(short *in, int len)
{
	if (!m_freq)
		return 0;

	int ret = 0;

	// remove dc
	BqProcess(in, len);

	// calculate levels
	__int64 s = 0;
	int OverPers = 0;
	short* y = (short*)m_buff.Buffer();
	for(int i=0; i<len; i++) {
		int sq = y[i]*y[i];
		s+= sq;
		OverPers+= sq > 0x32D64617;
	}
	float Level = (float)sqrt((double)s/len);
	if (Level < 1.f)	// nothing to measure
		return 0;
	Level = (float)(log10((double)Level)*20.);	// max level = 87.3 db for sin(x) or 90.3 db for square wave
	float fOverPers = (float)OverPers/len;

	bool isVad = m_vad->IsVad(in, len);
	int VadLevel = m_vad->GetVadLevel();
	bool isOverLoad = Level > 81.3f || Level+m_oldGain > 84.3f || fOverPers > 1.;

	// fill buffer
	if (isVad || m_Counter < 20 || Level > 81.f || m_NoLog > 32) {
		m_log[m_Counter++&(AG_WIN_LEN-1)] = Level;
		m_LastTime++;
		m_NoLog = 0;
	}
	else {
		m_NoLog++;
	}

	// set zero first 10 frames
	if (m_Counter < 10)	{
		memset(in, 0, len*2);
	}

	// count metric
	float dmin=100, dmax=0, av = 0, D=0;
	int maxlen = std::min<int>(AG_WIN_LEN, m_Counter);
	memset(m_gist, 0, sizeof(m_gist));
	for (int j =0; j<maxlen; j++) {
		float v = m_log[j];
		m_gist[(int)v]++;
		if (dmin > v)
			dmin = v;
		if (dmax < v)
			dmax = v;
		av+=v;
		D+=v*v;
	}
	D/=maxlen;
	av/=maxlen;
	D = sqrt(D - av*av + 0.1f);

	// count average gain
	int imin = 0, imax = 100;
	int cmin = 3, cmax = 3;
	for (int j =0; j<100; j++) {
		if (m_gist[j] > 0) {
			if (cmin > 0) {
				imin = j;
				cmin-=m_gist[j];
			}
		}
		if (m_gist[99-j] > 0) {
			if (cmax > 0) {
				imax = 99-j;
				cmax-=m_gist[99-j];
			}
		}
	}

	if (m_minFloor==0.0)
		m_minFloor = (float)imin;
	float k = (m_Counter < 16 || imin < m_minFloor) ? 0.9f : 0.999f;
	m_minFloor = m_minFloor*k + imin * (1-k);

	float drange = float(imax - imin);
	if (drange < 24.f)
		drange = 24.f;
	k = (drange > m_maxDisp) ? 0.9f : 0.999f;
	m_maxDisp = m_maxDisp*k + drange * (1-k);

	float diff = (72.f - m_maxDisp - m_minFloor);
	float ddiff = diff;
	float newgain = m_oldGain;

	// correct internal gain
	if (m_LastTime > 16) {		// time thr
		diff -= newgain;
		if ( abs(diff) > (D/3.f +1.f))
			newgain += diff/2.f;
	}

	// set mixer volume
	m_MixerCount++;
	if (isOverLoad) {
		m_wasOverload = 1;
		if (newgain > -2.)
			newgain-=1.;
	}
	if (m_wasOverload && m_MixerCount > 4) {
		ret = -1;
		m_wasOverload = 0;
		m_MixerCount = 0;
	}

	if (m_MixerCount > 40 && m_LastTime > 10) {
		if (newgain < -6.f && diff < 0.f)
			ret = -1;
		else if (newgain > 6.f && diff > 0.f)
			ret = 1;
		m_MixerCount = 0;
	}

	// set bounds to gain
	if (newgain > 12.f)
		newgain = 12.f;
	if (newgain < -12.f)
		newgain = -12.f;
	if (m_oldGain!= newgain) {
		m_oldGain = newgain;
		m_LastTime = 0;
	}
	DTRACE(VSTM_AGCIN, "exp=%5.1f, av=%5.1f, imax=%2d, imin=%2d, D=%5.1f | lvl=%5.1f, va=%d, ov=%d | p=%3d | %5.2f -- %5.2f | %5.2f",
		ddiff, av, imax, imin, D, Level, isVad, isOverLoad, OverPers, newgain, m_maxDisp, m_minFloor);

	return ret;
}

void VS_AutoGain::AdjustVolume(short *in, int len)
{
	float factor = float(pow(10., (m_oldGain - m_preamp*6.f)/20.));
	int Volume = int(factor*4096 + 0.5);	// normalize to 12 bit
	if (Volume < 0x1) {
		memset(in, 0, len*2);
	}
	else if (Volume < 0x1000) {
		for(int i=0; i<len; i++)
			in[i] = (in[i]*Volume)>>12;
	}
	else if (Volume > 0x1000) {
		for(int i=0; i<len; i++) {
			int v = (in[i]*Volume)>>12;
			if		(v>0x7fff) v = 0x7fff;
			else if (v<-0x8000) v = -0x8000;
			in[i] = v;
		}
	}
}

void VS_AutoGain::Preamp(short *in, int len)
{
	if (!m_freq)
		return;

	if (abs(m_oldGain-m_preamp*6.f) > 6.f)
		m_preamp = int(m_oldGain)/6;

	if (m_preamp < 0) {
		for(int i=0; i<len; i++)
			in[i] = in[i] >> -m_preamp;
	}
	else if (m_preamp > 0) {
		for(int i=0; i<len; i++) {
			int v = in[i] << m_preamp;
			if		(v>0x7fff) v = 0x7fff;
			else if (v<-0x8000) v = -0x8000;
			in[i] = v;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// VS_CountRecomendedLen
//////////////////////////////////////////////////////////////////////////////


/// Calculate audio level without normalisation

int CalcLevelSimple(short* in, int samples)
{
	int64_t s = 0;
	for (int i = 0; i<samples; i++)
		s += in[i] * in[i];
	return (int)sqrt((double)s / samples);
}