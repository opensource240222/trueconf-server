
#include "std/VS_FifoBuffer.h"
#include "VS_AudioSource.h"
#include "Transcoder/AudioCodec.h"
#include "VSAudioVad.h"
#include "VS_CountRecomendedLen.h"
#include "Transcoder/VS_VoiceAnalyze.h"

#include <cmath>
#include <chrono>

///////////////////////////////

#define MAX_AUDIO_OUT				150000
#define ARENDER_BUFFER_MAXTIME		2000
#define ARENDER_INIT_LATENCYBUFFS	3

///////////////////////////////

VS_AudioSource::VS_AudioSource()
{
	m_vad = 0;
	m_RecLen = 0;
	m_fifo = 0;
	m_AudioTemp = 0;
	m_IsValid = false;
	SetBlock(false);
}

VS_AudioSource::~VS_AudioSource()
{
	Release();
}

bool VS_AudioSource::Init(VS_MediaFormat *mf)
{
	if (!mf || !mf->IsAudioValid()) return false;
	m_mf = *mf;

	Release();

	std::chrono::milliseconds currMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
	m_NoiseTime = currMs.count();
	m_SkipTime = m_NoiseTime;
	m_LastFillTime = 0;
	m_LastFillVoice = 0;

	m_codec = VS_RetriveAudioCodec(mf->dwAudioCodecTag, false);
	if (!m_codec)
		return false;

	m_BuffLen = mf->dwAudioBufferLen;
	m_AudioTemp = new char[mf->dwAudioSampleRate * 2];

	m_vad = new VSAudioVAD;
	m_vad->Init(mf->dwAudioSampleRate, 16);
	m_vad->SetMode(ENABLE_NOISEGEN);

	WAVEFORMATEX wf;
	wf.nChannels = 1;
	wf.nSamplesPerSec = mf->dwAudioSampleRate;
	wf.wBitsPerSample = 16;
	if (m_codec->Init(&wf) != 0)
		return false;

	m_RecLen = new VS_CountRecomendedLen;

	m_fifo = new VS_FifoBuffer(ARENDER_BUFFER_MAXTIME * mf->dwAudioSampleRate * 2 / 1000);
	m_AvgLevel = new VS_VoiceAnalyze;

	return m_IsValid = true;
}

void VS_AudioSource::Release()
{
	m_IsValid = false;
	SetBlock(false);
	m_PlayedBuffs = 0;
	m_NoiseTime = 0;
	m_SkipTime = 0;
	m_LastFillTime = 0;
	m_LastFillVoice = 0;
	m_LastAddSample = 0;
	m_LastGetSample = 0;
	m_LastAddFType = FT_NULL;
	m_LastGetFType = FT_NULL;
	m_FramesToSkip = 0;
	m_BuffLen = 0;
	if (m_vad) delete m_vad; m_vad = 0;
	if (m_RecLen) delete m_RecLen; m_RecLen = 0;
	if (m_fifo) delete m_fifo; m_fifo = 0;
	delete m_codec; m_codec = nullptr;
	delete[] m_AudioTemp; m_AudioTemp = 0;
	delete m_AvgLevel; m_AvgLevel = nullptr;
}

bool VS_AudioSource::ReInit(VS_MediaFormat *mf)
{
	if (mf && mf->IsAudioValid()) {
		if (!mf->AudioEq(m_mf)) {
			auto block = m_IsBlock;
			Release();
			Init(mf);
			SetBlock(block);
			return true;
		}
	}
	return false;
}

int VS_AudioSource::AddData(char* buff, int size)
{
	if (!m_IsValid || m_IsBlock) return 0;

	size = m_codec->Convert((unsigned char *)buff, (unsigned char *)m_AudioTemp, size);
	buff = m_AudioTemp;

	CalcLevel((short*)m_AudioTemp, size / 2);

	std::chrono::milliseconds currMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
	uint64_t CurrTime = currMs.count();
	uint64_t dt = CurrTime - m_LastFillTime;
	if (dt > ARENDER_BUFFER_MAXTIME * 2 / 3) {
		m_RecLen->Reset();
	}
	m_LastFillTime = CurrTime;

	int num_smpls = m_fifo->GetDataLen() / 2;
	int PendTime = num_smpls * 1000 / m_mf.dwAudioSampleRate;
	int OneBuffDurr = m_BuffLen / 2 * 1000 / m_mf.dwAudioSampleRate;
	int dB = ARENDER_BUFFER_MAXTIME * 2 / 3;
	int DurrBound = OneBuffDurr;

	m_RecLen->Snap(CurrTime);
	m_PlayedBuffs++;

	if (m_PlayedBuffs < ARENDER_INIT_LATENCYBUFFS) {	/// then add buffers to fifo
		return 1;
	}

	int RecBuffLen = m_RecLen->GetRTime();
	if (RecBuffLen > dB)
		RecBuffLen = dB;
	if (RecBuffLen < DurrBound)
		RecBuffLen = DurrBound;

	bool IsVad = m_vad->IsVad((unsigned char*)buff, size);

	bool SkipFrame = false;
	int SkipInterval = 0;

	if (PendTime >(RecBuffLen + OneBuffDurr)) {
		SkipInterval = RecBuffLen / (PendTime - RecBuffLen - OneBuffDurr + 10);
		SkipInterval += 4; // skiping speed correction
	}

	if (SkipInterval > 0) {
		if (m_FramesToSkip == 0) {
			m_FramesToSkip = SkipInterval;
			m_SkipTime = 0;
		}
		if (--m_FramesToSkip > 0) {
			if (m_SkipTime == 0) // wasnt
				SkipFrame = !IsVad;
		}
		else {
			if (m_SkipTime == 0)
				SkipFrame = true; // hard skip
		}
	}
	if (SkipFrame) {
		m_SkipTime = CurrTime;
		m_LastAddFType = FT_SKIP;
	}
	else {
		CorrectSpike((short*)buff, size, FT_PLAY, true);
		m_fifo->AddData(buff, size);
		m_PlayedBuffs++;
	}

	return 1;
}

int VS_AudioSource::GetData(char* buff, int size)
{
	if (!m_IsValid) return 0;

	int insize = m_fifo->GetDataLen();
	if (insize >= size) { // enouth data
		m_fifo->GetData(buff, size);
		CorrectSpike((short*)buff, size, FT_PLAY);
		std::chrono::milliseconds currMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
		m_LastFillVoice = currMs.count();
	}
	else {
		if (insize > 0) {
			m_fifo->GetData(buff, insize);
			CorrectSpike((short*)buff, insize, FT_PLAY);
		}
		std::chrono::milliseconds currMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
		//TODO SM: fade volume of noise
		if (currMs.count() - m_LastFillVoice < 2000) // 2 sec for noice generation
			m_vad->GetNoise((short*)(buff + insize), size - insize);
		else
			memset(buff + insize, 0, size - insize);
		CorrectSpike((short*)(buff + insize), size - insize, FT_NOISE);
		//DTRACE(VSTM_AUDI0, "h=%x | NOISE = %d", this, size - insize);
	}
	return size;
}

int VS_AudioSource::GetCurrBuffDurr()
{
	if (!m_IsValid)
		return 0;
	return m_fifo->GetDataLen() * 1000 / (2 * m_mf.dwAudioSampleRate);
}

void VS_AudioSource::CorrectSpike(short* data, int lenght, frame_type type, bool add)
{
	int & lastSample = add ? m_LastAddSample : m_LastGetSample;
	frame_type & lastFType = add ? m_LastAddFType : m_LastGetFType;
	if (add) {
		if (m_fifo->GetDataLen() == 0){
			lastSample = m_LastGetSample;
			lastFType = m_LastGetFType;
		}
	}
	if (type != lastFType) {
		int d = (lastSample - data[0]) / 3;
		data[0] += 2 * d;
		data[1] += d;
	}
	if (type != FT_NULL) {
		lastFType = type;
		lastSample = data[lenght / 2 - 1];
	}
}

void VS_AudioSource::CalcLevel(short* in, int samples)
{
	if (!m_IsValid) {
		return;
	}
	uint64_t sum = 0;

	for (int i = 0; i < samples; i++)
		sum += in[i] * in[i];

	uint32_t lvl = (int)std::sqrt((double)sum / samples);

	std::chrono::milliseconds currMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
	m_AvgLevel->Set(currMs.count(), lvl);
}

void VS_AudioSource::SetBlock(bool block)
{
	m_IsBlock = block;
}

uint32_t VS_AudioSource::GetLevel()
{
	if (!m_IsValid) {
		return 0;
	}
	std::chrono::milliseconds currMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
	return m_AvgLevel->Get(currMs.count());
}

