#pragma once

#include <cstdint>
#include <deque>

class VS_CountRecomendedLen
{
	uint64_t					m_mtime;
	std::deque<uint64_t>		m_qu;
	uint64_t					m_recomeded;

public:

	VS_CountRecomendedLen(uint64_t monitoring_time = 30000);
	void Snap(uint64_t rcv_time);
	void Reset();
	uint64_t GetRTime();
};
