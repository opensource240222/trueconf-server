#include "VS_CountRecomendedLen.h"

VS_CountRecomendedLen::VS_CountRecomendedLen(uint64_t monitoring_time)
{
	m_mtime = monitoring_time;
	Reset();
}

void VS_CountRecomendedLen::Snap(uint64_t rcv_time)
{
	m_qu.push_back(rcv_time);
	while (m_qu.back() - m_qu.front() > m_mtime)
		m_qu.pop_front();

	if (m_qu.size() <= 1)
		m_recomeded = -1;
	else if (m_qu.size() == 2)
		m_recomeded = m_qu[1] - m_qu[0];
	else
	{
		// Find 2 different pairs of elements with the maximal difference between the elements.
		uint64_t max_diff = 0;
		uint64_t max2_diff = 0;
		for (std::size_t i = 1; i < m_qu.size(); ++i)
		{
			const uint64_t diff = m_qu[i] - m_qu[i-1];
			if (diff >= max_diff)
			{
				max2_diff = max_diff;
				max_diff = diff;
			}
			else if (diff >= max2_diff)
				max2_diff = diff;
		}
		m_recomeded = (max_diff + max2_diff * 3) / 4;
	}
}

void VS_CountRecomendedLen::Reset()
{
	m_qu.clear();
	m_recomeded = -1;
}

uint64_t VS_CountRecomendedLen::GetRTime()
{
	return m_recomeded;
}
