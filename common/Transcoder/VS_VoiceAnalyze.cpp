#include "VS_VoiceAnalyze.h"

void VS_VoiceAnalyze::Set(uint64_t currTime, unsigned long currVol)
{
	m_LastAudioTime = currTime;
	if (currVol > m_AverVol)
		m_AverVol = (7 * m_AverVol + currVol) / 8;
	else
		m_AverVol = (15 * m_AverVol + currVol) / 16;
}

unsigned long VS_VoiceAnalyze::Get(uint64_t currTime)
{
	if (currTime != 0 && currTime - m_LastAudioTime > 2000)
		m_AverVol = 0;
	return m_AverVol;
}