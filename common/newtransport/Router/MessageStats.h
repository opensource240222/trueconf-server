#pragma once

#include <cstdint>
#include <climits>

class MessageStats
{
 public:

	MessageStats():m_message_count(0), m_total_recv_size(0), m_min_message_size(0), m_max_message_size(0), m_average_message_size(0)
	{}


	void Update(uint32_t message_size)
	{
		if (!m_message_count)
			m_min_message_size = m_max_message_size = message_size;
		m_message_count++;
		m_total_recv_size+=message_size;
		if (m_min_message_size > message_size)
		{
			m_min_message_size = message_size;
		}
		if (m_max_message_size < message_size)
		{
			m_max_message_size = message_size;
		}
		if (m_message_count > 0)
		{
			m_average_message_size = m_total_recv_size / m_message_count;
		}
	}

	uint64_t   m_message_count, m_total_recv_size;
    uint64_t   m_min_message_size, m_max_message_size, m_average_message_size;
};
