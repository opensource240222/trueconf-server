#pragma once

#include <cstdint>

class VS_VoiceAnalyze
{
public:
	void Set(uint64_t currTime, unsigned long currVol);
	unsigned long Get(uint64_t currTime = 0);

protected:
	uint64_t m_LastAudioTime = 0;
	unsigned long m_AverVol = 0;
};
