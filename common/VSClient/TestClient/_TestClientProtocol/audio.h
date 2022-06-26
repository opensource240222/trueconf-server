
#ifndef AUDIO_TEST_H
#define AUDIO_TEST_H

#include "std-generic/cpplib/VS_Container.h"
#include "Transcoder/VSAudioVad.h"
#include "Transcoder/AudioCodec.h"

#include <windows.h>
#include <mmreg.h>
#include <stdio.h>
#include <math.h>

#define NUM_OF_BUFF 50
#define BUFFER_LEN  (640*2)


class VS_AudioJitter
{
	int *	m_values;
	int		m_TotalCount;
	int		m_CurrCount;
public:
	static const int JITTER_MAX = 567;
	VS_AudioJitter() {
		m_values = 0;
		m_TotalCount = m_CurrCount = 0;
	}
	~VS_AudioJitter() {
		if (m_values) free(m_values);
	}
	void Snap(int val) {
		m_values[m_CurrCount%m_TotalCount] = val;
		m_CurrCount++;
	}
	int GetBound(double percent) {
		//if (m_CurrCount<1) return 0;
		//int count = m_CurrCount<m_TotalCount ? m_CurrCount : m_TotalCount;
		//int maxval = 0; int i;
		//for (i = 0; i<count; i++)
		//	if (maxval < m_values[i])
		//		maxval = m_values[i];
		//int * hist = (int*) malloc((maxval+1)*4);
		//memset(hist, 0, (maxval+1)*4);
		//for (i = 0; i<count; i++)
		//	hist[m_values[i]]++;
		//int threahold = (int)(percent*count/100);
		//count = 0; i = maxval;
		//do count+=hist[i];	while (count<threahold && i-->=0);
		//free(hist);
		//return i + 30; // + double timer presision
		int count = m_CurrCount<m_TotalCount ? m_CurrCount : m_TotalCount;
		double M0 = 0., D = 0.;
		double a0 = 0., a1 = 0., d = 0.;
		int i = 0;
		if (!Predict(a0, a1)) return JITTER_MAX;
		for (i = m_CurrCount - count; i<m_CurrCount; i++) {
			d = a0 + a1*i - (double)m_values[i%m_TotalCount];
			M0+=d;
			D+=d*d;
		}
		M0 = M0/(double)count;
		D = D/(double)count;
		return (int)(sqrt(D - M0*M0)+0.5);
	}

	bool Predict(double &a0, double &a1) {
		// P(x) = a0 + a1*x;
		a0 = 0.;
		a1 = 0.;
		if (m_CurrCount < m_TotalCount/60) 	return false;
		double A0 , A1, A2, B, D;

		A0 = A1 = A2 =  B = D = 0.;
		int count = m_CurrCount<m_TotalCount ? m_CurrCount : m_TotalCount;
		for (int i = m_CurrCount - count; i<m_CurrCount; i++) {
			A0+=1.;
			A1 += i;
			A2 += i*i;
			B += (double)m_values[i%m_TotalCount];
			D += (double)m_values[i%m_TotalCount]*i;
		}
		if (A1!=0. && (A0*A2-A1*A1)!=0.) {
			a0 = (B*A2-D*A1)/(A0*A2-A1*A1);
			a1 = (B-A0*a0)/A1;
			return true;
		}
		else return false;
	}
	void Clear() {
		m_CurrCount = 0;
	}
	void Init(int SampleCount) {
		Clear();
		m_TotalCount = SampleCount;
		m_values = (int*)malloc(SampleCount*4);
	}
};



struct VS_WaveBuff
{
	bool			m_IsVad;
	bool			m_IsCompr;
	int				m_Size;
	WAVEHDR			m_wh;
	AudioCodec*		m_codec;
	VSAudioVAD*		m_vad;
	VS_WaveBuff() {
		memset(this, 0, sizeof(VS_WaveBuff));
	}
	void Init(AudioCodec* codec, VSAudioVAD* vad, void* buff, int size) {
		m_wh.dwUser = (DWORD)this;
		m_wh.lpData = (LPSTR)buff;
		m_wh.dwBufferLength = size;
		m_codec = codec;
		m_vad = vad;
	}
	void Set(void* buff, int size, bool IsCompr, bool IsNoise = false) {
		m_Size = size;
		if (!IsNoise)	m_IsVad = m_vad->IsVad(buff, size);
		else			m_IsVad = false;
		m_IsCompr = IsCompr;
		memcpy(m_wh.lpData, buff, size);
	}
	void Compr(void* buff, int &size){
		if (m_codec->IsCoder()) {
			if (!m_IsCompr) {
				size = m_codec->Convert((BYTE*)m_wh.lpData, (BYTE*)buff, m_wh.dwBytesRecorded);
				memcpy(m_wh.lpData, buff, size);
				m_IsCompr=true;
			}
			else {
				size = m_Size;
				memcpy(buff, m_wh.lpData, size);
			}
		}
		else {
			if (m_IsCompr) {
				size = m_codec->Convert((BYTE*)m_wh.lpData, (BYTE*)buff, m_Size);
				memcpy(m_wh.lpData, buff, size);
				m_IsCompr=false;
			}
			else {
				size = m_Size;
				memcpy(buff, m_wh.lpData, size);
			}
		}
		m_Size = size;
	}
};

class AudioDevice
{
protected:
	int					m_StartBuff;
	VS_WaveBuff			m_buffs[NUM_OF_BUFF];
	WAVEFORMATEX		swf;
	void *				m_Memory;
	FILE * 				m_Report;
	char *				m_AudioTemp;
	VSAudioVAD			m_vad;
	AudioCodec*			m_codec;
	VS_AudioJitter		m_jitter;
public:
	AudioDevice(bool type);
	~AudioDevice();
};

class ARender: public AudioDevice
{
	HWAVEOUT	m_hw;
	int			m_NoiseTime;
	int			m_SkipTime;
	int			m_LastFillTime;
	__int64		m_Samples;
public:
	ARender() : AudioDevice(true) {
		m_hw = 0;
		m_codec = VS_RetriveAudioCodec(VS_ACODEC_GSM610, false);
		m_NoiseTime = 0;
		m_SkipTime = 0;
		m_LastFillTime = 0;
		m_Samples = 0;
	}
	~ARender(){Release();}
	bool Init(int dev, HANDLE hCapt);
	int Play(char* buff, int size);
	void GenNoise();
	void Release();
};


class ACapture: public AudioDevice
{
	HWAVEIN		m_hw;
public:
	ACapture():AudioDevice(false) {m_hw = 0; m_codec = VS_RetriveAudioCodec(VS_ACODEC_GSM610, false);}
	~ACapture(){Release();}
	bool Init(int dev, HANDLE hVoice);
	int Capture(char* buff, int &size);
	void Release();
};


#endif
