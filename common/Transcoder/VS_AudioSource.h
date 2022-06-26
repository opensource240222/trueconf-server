
#ifndef VS_AUDIOSOURCE_H
#define	VS_AUDIOSOURCE_H

#include <stdio.h>
#include <list>
#include <map>
#include "../../std/cpplib/VS_MediaFormat.h"

class VSAudioVAD;
class VS_FifoBuffer;
class VS_VoiceAnalyze;
class VS_CountRecomendedLen;
class AudioCodec;

class VS_AudioSource
{

private:

	enum frame_type
	{
		FT_NULL,
		FT_PLAY,
		FT_SKIP,
		FT_NOISE
	};

	int								m_PlayedBuffs;		// number of played buffers
	uint64_t						m_NoiseTime;		// last time when noise passed to devise
	uint64_t						m_SkipTime;			// last time when data can but not passed to devise
	uint64_t						m_LastFillTime;		// last time when data passed to devise
	uint64_t						m_LastFillVoice;
	int								m_LastAddSample;	// last added sample
	int								m_LastGetSample;	// last read samole
	frame_type						m_LastAddFType;		// last added audio frame type
	frame_type						m_LastGetFType;		// last read audio frame type
	int								m_FramesToSkip;		// number of frames to skip in current time window
	int								m_BuffLen;			// Length of single audiodata chunk
	VSAudioVAD*						m_vad;				// pointer to VAD class
	VS_VoiceAnalyze*				m_AvgLevel = nullptr;
	VS_CountRecomendedLen*			m_RecLen;
	VS_FifoBuffer*					m_fifo;				// pointer to fifo buffer
	AudioCodec*						m_codec = nullptr;	// pointer to audio decoder
	char *							m_AudioTemp;		// temp buffer
	VS_MediaFormat					m_mf;
	bool							m_IsValid;			// true if Init() passed ok
	bool							m_IsBlock;

public:

	VS_AudioSource();
	/// call Release()
	virtual ~VS_AudioSource();
	/// Init members according to VS_MediaFormat
	bool Init(VS_MediaFormat *mf);
	bool ReInit(VS_MediaFormat *mf);
	int AddData(char* buff, int size);
	int GetData(char* buff, int size);
	/// Get Current Buffer Duration
	int GetCurrBuffDurr();
	/// Release resources
	void Release();
	bool IsValid() { return m_IsValid; }
	uint32_t GetFreq() { return m_IsValid ? m_mf.dwAudioSampleRate : 0; }
	void SetBlock(bool block);
	uint32_t GetLevel();

private:

	/// Correct first audio samples in case of nonsequential write
	void CorrectSpike(short* data, int lenght, frame_type type, bool add = false);
	void CalcLevel(short* in, int samples);
};

#endif /* VS_AUDIOSOURCE_H */
