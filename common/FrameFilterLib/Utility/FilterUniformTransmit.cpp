#include "FrameFilterLib/Utility/FilterUniformTransmit.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/VS_Singleton.h"
#include "std/cpplib/VS_ThreadPool.h"

#include <boost/container/small_vector.hpp>

#include <cassert>
#include <cmath>
#include <iomanip>

#define UT_VERBOSE_LOGS 0

namespace ffl {
	std::shared_ptr<FilterUniformTransmit> FilterUniformTransmit::Create(const std::shared_ptr<AbstractSource>& src, unsigned max_bitrate_bytes_per_sec)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterUniformTransmit>(max_bitrate_bytes_per_sec));
	}

	FilterUniformTransmit::FilterUniformTransmit(unsigned base_bitrate_bytes_per_sec)
		: m_base_bitrate(base_bitrate_bytes_per_sec)
		, m_korr(0.05)
		, m_korr2(1)
		, m_N(1.5)
		, m_total_data_size(0)
		, m_last_send_size(0)
		, m_send_active(false)
		, m_stat_counter(0)
		, m_mean_bitrate(0)
		, m_mean_buf_size(0)
		, m_mean_duration(0)
		, m_max_bitrate(0)
		, m_max_buf_size(0)
		, m_min_duration(std::numeric_limits<unsigned>::max())
		, m_max_duration(0)
		, m_send_group_distribution({})
	{
		SetName("uniform transmit");
	}

	FilterUniformTransmit::~FilterUniformTransmit()
	{
	}

	void FilterUniformTransmit::SetBitrate(unsigned bitrate)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_base_bitrate = bitrate;
	}

	void FilterUniformTransmit::SetKorr(double v)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_korr = v;
	}

	void FilterUniformTransmit::SetKorr2(double v)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_korr2 = v;
	}

	void FilterUniformTransmit::SetN(double v)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_N = v;
	}

	size_t FilterUniformTransmit::GetBufferedDataSize() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_total_data_size;
	}

	std::string FilterUniformTransmit::GetStatistics() const
	{
		std::ostringstream ss;
		std::lock_guard<std::mutex> lock(m_mutex);
		ss << "Bitrate: mean = " << m_mean_bitrate << " bytes/s; max = " << m_max_bitrate << " bytes/s\n";
		ss << "Send size: mean = " << m_mean_buf_size << " bytes; max = " << m_max_buf_size << " bytes\n";
		ss << "Send duration: mean = " << m_mean_duration << " us; min = " << m_min_duration << " us; max = " << m_max_duration << " us\n";
		ss << "Send group distribution: ";
		for (std::size_t i = 0; i < m_send_group_distribution.size() - 1; ++i)
			ss << '[' << i << "]=" << m_send_group_distribution[i] << ' ';
		ss << '[' << (m_send_group_distribution.size() - 1) << "+]=" << m_send_group_distribution.back();
		return ss.str();
	}

	bool FilterUniformTransmit::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterUniformTransmit*>(sink);
		if (!p)
			return false;
		if (m_base_bitrate != p->m_base_bitrate)
			return false;
		if (m_korr != p->m_korr)
			return false;
		if (m_korr2 != p->m_korr2)
			return false;
		if (m_N != p->m_N)
			return false;
		return true;
	}

	auto FilterUniformTransmit::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_total_data_size += buffer.size();
		m_buffers.emplace_back(std::move(buffer), md);

		if (!m_send_active) {
			m_last_send = std::chrono::steady_clock::now();
			m_last_send_size = 0;
			m_send_active = true;
			VS_Singleton<VS_ThreadPool>::Instance().Post(std::bind(&FilterUniformTransmit::SendBuffer, shared_from_this()));
		}

		return e_noResult;
	}

	bool FilterUniformTransmit::ProcessCommand(const FilterCommand& cmd)
	{
		if (AbstractSource::ProcessCommand(cmd))
			return true;

		if (cmd.type == FilterCommand::e_setBitrateRequest)
		{
			SetBitrate(cmd.bitrate);
			return true;
		}

		return SendCommandToSources(cmd, false);
	}

	void FilterUniformTransmit::SendBuffer()
	{
		const double send_window = 0.002; // seconds
		const std::chrono::milliseconds max_wait_time(10);

		std::chrono::steady_clock::time_point next_wakeup;
		VS_SCOPE_EXIT {
			if (next_wakeup != std::chrono::steady_clock::time_point{})
				VS_Singleton<VS_ThreadPool>::Instance().Post(std::bind(&FilterUniformTransmit::SendBuffer, shared_from_this()), next_wakeup);
		};

		boost::container::small_vector<std::pair<vs::SharedBuffer, FrameMetadata>, 4> to_send;

		{
			std::lock_guard<decltype(m_mutex)> lock(m_mutex);
			if (m_buffers.empty())
			{
				m_send_group_distribution[0]++;
				assert(next_wakeup == std::chrono::steady_clock::time_point{});
				m_send_active = false;
				return;
			}

			const auto now = std::chrono::steady_clock::now();
			const auto last_bitrate = GetCurrentBitrate();
			const uint64_t last_duration_us = 1000000u * static_cast<double>(m_last_send_size) / last_bitrate;
			const auto next_send = m_last_send + std::chrono::microseconds(last_duration_us);

			if (now < next_send)
			{
				// We woke up during the time interval allocated for sending the last data batch.
				// We shouldn't send anything right now and need to try again after that interval has passed.
				// We don't want to wait for too long to respond more quickly to the queue growing.
				next_wakeup = std::min(next_send, now + max_wait_time);
				m_send_active = true;
				return;
			}

			// Calculate the amount of data that we can send in one batch,
			// then pop all buffers from the queue that fit into that size.
			const size_t send_budget = last_bitrate * send_window;
			m_last_send_size = 0;
			while (!m_buffers.empty())
			{
				const auto sz = m_buffers.front().first.size();
				if (m_last_send_size + sz > send_budget && !to_send.empty())
					break;
				m_last_send_size += sz;
				to_send.push_back(std::move(m_buffers.front()));
				m_buffers.pop_front();
			}
			assert(!to_send.empty());
#if UT_VERBOSE_LOGS
			if (auto log = m_log.lock())
			{
				std::ostringstream ss;
				ss << "n_buffers: " << std::setw(2) << to_send.size() << '/' << std::setw(2) << m_buffers.size() << ", sz: " << std::setw(4) << m_last_send_size << '/' << std::setw(5) << m_total_data_size << ", br: " << std::setw(6) << last_bitrate;
				log->TraceMessage(this, ss.str());
			}
#endif
			m_total_data_size -= m_last_send_size;

			// Start counting the time interval allocated for sending the current data batch from the end of the previous interval.
			// But avoid setting the start time too far into the past in case the previous interval ended a long time ago.
			m_last_send = std::max(next_send, now - max_wait_time);

			const auto bitrate = GetCurrentBitrate();
			const uint64_t duration_us = 1000000u * static_cast<double>(m_last_send_size) / bitrate;

			// We don't want to wait for too long to respond more quickly to the queue growing.
			next_wakeup = std::min(m_last_send + std::chrono::microseconds(duration_us), now + max_wait_time);
			m_send_active = true;

			// Update statistics
			m_stat_counter++;
			m_send_group_distribution[std::min<unsigned>(to_send.size(), m_send_group_distribution.size() - 1)]++;
			m_mean_bitrate += (bitrate - m_mean_bitrate) / m_stat_counter;
			m_max_bitrate = std::max<decltype(m_max_bitrate)>(m_max_bitrate, bitrate);
			m_mean_buf_size += (static_cast<double>(m_total_data_size) - m_mean_buf_size) / m_stat_counter;
			m_max_buf_size = std::max<decltype(m_max_buf_size)>(m_max_buf_size, m_total_data_size);
			m_mean_duration += (static_cast<double>(duration_us) - m_mean_duration) / m_stat_counter;
			m_min_duration = std::min<decltype(m_min_duration)>(m_min_duration, duration_us);
			m_max_duration = std::max<decltype(m_max_duration)>(m_max_duration, duration_us);
		}

		for (auto& x : to_send)
			SendFrameToSubscribers(std::move(x.first), x.second);
	}

	double FilterUniformTransmit::GetCurrentBitrate() const
	{
		// Calculate bitrate adjusted to the current length of the queue in bytes.
		return m_base_bitrate * (1.0 + m_korr + std::pow(static_cast<double>(m_total_data_size) / (m_korr2 * m_base_bitrate), m_N));
	}
}
