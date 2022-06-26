#pragma once

#include <memory>
#include <sstream>

class VS_StatisticCollector
{
public:
	VS_StatisticCollector(void)
	{
		memset(this,0,sizeof(*this));
	}

	void UpdateBitRate(const unsigned long int bytes, const unsigned long int lastTime)
	{
		m_sendByterate = unsigned long(double(bytes) / double(lastTime - m_lastTime) + .5);
		m_sendBitrate = m_sendByterate * 8;
		m_bytesPs = unsigned long(double(bytes * 1000) / double(lastTime - m_lastTime) + .5);
		m_bps = m_bytesPs * 8;
		m_kbps = unsigned long(double(m_bps) / 1000.0 + 0.5);
		m_acc_kbps += m_kbps;
		m_bytes = bytes;
		if(m_lastTime)
			m_averageTime += lastTime - m_lastTime;
		m_lastTime = lastTime;
		++m_averageSending;

		if(m_averageTime > 10000)
		{
			m_averageTime = m_averageTime - 10000;
			m_averageKbps = unsigned long(double(m_acc_kbps) / m_averageSending + .5);
			m_averageSending = 0;
			m_acc_kbps = 0;
		}
	}

	std::string DumpStat() const
	{
		std::stringstream ss;
		ss << "BPS[" << m_kbps << ']';
		return ss.str();
	}

	unsigned long int GetAverageKBPS() const
	{
		return m_averageKbps;
	}

private:
	unsigned long int m_bytes;
	unsigned long int m_sendByterate;
	unsigned long int m_sendBitrate;
	unsigned long int m_lastTime;
	unsigned long int m_timeIntrval;
	unsigned long int m_bytesPs;
	unsigned long int m_bps;
	unsigned long int m_kbps;
	unsigned long int m_acc_kbps;
	unsigned long int m_averageKbps;
	unsigned long int m_averageTime;
	unsigned long int m_averageSending;
};
