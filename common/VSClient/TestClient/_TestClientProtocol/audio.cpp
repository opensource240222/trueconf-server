#include "audio.h"

#include <algorithm>
#include <stdio.h>

//////////////////////
AudioDevice::AudioDevice(bool type)
{
	m_StartBuff = 0;

	swf.wFormatTag = WAVE_FORMAT_PCM;
	swf.nChannels = 1;
	swf.nSamplesPerSec = 8000;
	swf.wBitsPerSample = 16;
	swf.nBlockAlign = swf.nChannels*swf.wBitsPerSample / 8;
	swf.nAvgBytesPerSec = swf.nSamplesPerSec*swf.nBlockAlign;

	m_vad.Init(swf.nSamplesPerSec, swf.wBitsPerSample);
	m_vad.SetMode(ENABLE_NOISEGEN);

	m_Memory = malloc((NUM_OF_BUFF+1)*BUFFER_LEN);
	if (swf.wBitsPerSample==8)
		memset(m_Memory, 128, (NUM_OF_BUFF+1)*BUFFER_LEN);
	else
		memset(m_Memory, 0, (NUM_OF_BUFF+1)*BUFFER_LEN);

	if (type)
		m_Report = fopen("out.txt", "w");
	else
		m_Report = 0;

	timeBeginPeriod(5);
	m_codec = 0;
	m_AudioTemp = (char*)malloc(1000000);
	m_jitter.Init(60*swf.nSamplesPerSec*swf.nBlockAlign/BUFFER_LEN);
}

AudioDevice::~AudioDevice()
{
	timeEndPeriod(5);
	m_vad.Release();
	free(m_Memory); m_Memory = 0;
	if (m_Report) fclose(m_Report); m_Report = 0;
	if (m_codec) delete m_codec; m_codec= 0;
	free(m_AudioTemp);
}

/////////////////////////////

bool ARender::Init(int dev, HANDLE hVoice)
{
	m_codec->Init(&swf);

	waveOutOpen(&m_hw, dev, &swf, (DWORD_PTR)hVoice, 0, CALLBACK_EVENT);
	for (int i = 0; i<NUM_OF_BUFF; i++) {
		m_buffs[i].Init(m_codec, &m_vad, (char*)m_Memory + BUFFER_LEN*(i+1), BUFFER_LEN);
		waveOutPrepareHeader(m_hw, &m_buffs[i].m_wh, sizeof(WAVEHDR));
		m_buffs[i].m_wh.dwFlags|=WHDR_DONE;
	}
	return true;
}

int ARender::Play(char* buff, int size)
{
	int  i = 0;

	int BuffTime = 5*m_jitter.GetBound(0.5) + BUFFER_LEN*1000/swf.nAvgBytesPerSec;
	MMTIME mmt;
	mmt.wType = TIME_SAMPLES;
	waveOutGetPosition(m_hw, &mmt, sizeof(MMTIME));
	int PendTime = (int)((m_Samples - mmt.u.sample)*1000/swf.nSamplesPerSec);

	size = m_codec->Convert((BYTE*)buff, (BYTE*)m_AudioTemp, size);
	int it = 0;
	buff = m_AudioTemp;

	int CurrTime = timeGetTime();
	if (CurrTime-m_LastFillTime > 5*VS_AudioJitter::JITTER_MAX)
		m_jitter.Clear();
	m_LastFillTime = CurrTime;

	while (size>0) {
		i = m_StartBuff%NUM_OF_BUFF;
		if (m_buffs[i].m_wh.dwFlags&WHDR_DONE) {
			int isize = std::min(size, BUFFER_LEN);
			m_buffs[i].Set(buff, isize, false);
			m_jitter.Snap(CurrTime);
			size-=isize;
			buff+=isize;
			if (m_Report)
				fprintf(m_Report, "%3d\n",  CurrTime);
			if (!m_buffs[i].m_IsVad &&
				PendTime > BuffTime &&
				CurrTime != m_SkipTime &&
				CurrTime-m_NoiseTime > 1000) {
				printf(" SKIP !!! \nPendTime = %3d, BuffTime = %3d\n", PendTime, BuffTime);
				m_SkipTime = CurrTime;
			}
			else {
				m_StartBuff++;
				m_buffs[i].m_wh.dwFlags&=~WHDR_DONE;
				waveOutWrite(m_hw, &m_buffs[i].m_wh, sizeof(WAVEHDR));
				m_Samples+=isize/swf.nBlockAlign;
			}
		}
		else {
			puts("NO BUFFERS NO BUFFERS NO BUFFERS NO BUFFERS NO BUFFERS");
			break;
		}
		if (m_StartBuff%10==0)
			printf("PendTime = %3d, BuffTime = %3d\n", PendTime, BuffTime);
	}
	return 1;
}


void ARender::GenNoise()
{
	int NumPend = 0, i;
	for (i = 0; i<NUM_OF_BUFF; i++)
		NumPend+= (m_buffs[i].m_wh.dwFlags&WHDR_INQUEUE)!=0;
	int CurrTime = timeGetTime();
	if (NumPend > 1 || CurrTime-m_LastFillTime > 4*VS_AudioJitter::JITTER_MAX) {
		return;
	}
	if (NumPend == 1 && 5*m_jitter.GetBound(0.5) < (int)(BUFFER_LEN*1000/swf.nAvgBytesPerSec)) {
		return;
	}
	puts(" NOISE  ~~~~~~~~~~~~~~~~~ !!!");
	m_vad.GetNoise((short*)m_AudioTemp, BUFFER_LEN);

	i = m_StartBuff%NUM_OF_BUFF;
	if (m_buffs[i].m_wh.dwFlags&WHDR_DONE) {
		m_buffs[i].Set(m_AudioTemp, BUFFER_LEN, false);
		m_StartBuff++;
		m_buffs[i].m_wh.dwFlags&=~WHDR_DONE;
		waveOutWrite(m_hw, &m_buffs[i].m_wh, sizeof(WAVEHDR));
		m_Samples+=BUFFER_LEN/swf.nBlockAlign;
		m_NoiseTime = timeGetTime();
	}
}

void ARender::Release()
{
	if (m_hw) {
		waveOutPause(m_hw);
		waveOutReset(m_hw);
		Sleep(0);
		for (int i = 0; i<NUM_OF_BUFF; i++)
			waveOutUnprepareHeader(m_hw, &m_buffs[i].m_wh, sizeof(WAVEHDR));
		waveOutClose(m_hw);
		m_StartBuff = 0;
		m_hw = 0;
		m_Samples = 0;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////

bool ACapture::Init(int dev, HANDLE hVoice)
{
	m_codec->Init(&swf);
	for (int i = 0; i<NUM_OF_BUFF; i++)
		m_buffs[i].Init(m_codec, &m_vad, (char*)m_Memory + BUFFER_LEN*(i+1), BUFFER_LEN);

	waveInOpen(&m_hw, dev, &swf, (DWORD_PTR)hVoice, 0, CALLBACK_EVENT);
	for (int i = 0; i<NUM_OF_BUFF; i++) {
		waveInPrepareHeader(m_hw, &m_buffs[i].m_wh, sizeof(WAVEHDR));
		waveInAddBuffer(m_hw, &m_buffs[i].m_wh, sizeof(WAVEHDR));
	}
	waveInStart(m_hw);
	return true;
}


int ACapture::Capture(char* buff, int &size)
{
	int isize = 0;
	size = 0;
	while (true) {
		int i = m_StartBuff%NUM_OF_BUFF;
		if (m_buffs[i].m_wh.dwFlags&WHDR_DONE) {
			m_buffs[i].Compr(buff, isize);
			m_buffs[i].m_wh.dwFlags&=~WHDR_DONE;
			m_buffs[i].m_IsCompr = false;
			waveInAddBuffer(m_hw, &m_buffs[i].m_wh, sizeof(WAVEHDR));
			m_StartBuff++;
			size+=isize;
			buff+=isize;
		}
		else
			break;
	}
	return size;
}

void ACapture::Release()
{
	if (m_hw) {
		waveInStop(m_hw);
		waveInReset(m_hw);
		for (int i = 0; i<NUM_OF_BUFF; i++)
			waveInUnprepareHeader(m_hw, &m_buffs[i].m_wh, sizeof(WAVEHDR));
		waveInClose(m_hw);
		m_StartBuff = 0;
		m_hw =0;
	}
}

