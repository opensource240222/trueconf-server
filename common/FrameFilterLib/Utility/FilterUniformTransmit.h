#pragma once

#include <array>
#include <deque>
#include <mutex>

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"

namespace ffl {
	class FilterUniformTransmit : public AbstractFilter<AbstractSingleSourceSink> {
	public:
		static std::shared_ptr<FilterUniformTransmit> Create(const std::shared_ptr<AbstractSource>& src, unsigned max_bitrate_bytes_per_sec);

		explicit FilterUniformTransmit(unsigned base_bitrate_bytes_per_sec);
		~FilterUniformTransmit();

		void SetBitrate(unsigned bitrate);
		void SetKorr(double v);
		void SetKorr2(double v);
		void SetN(double v);

		size_t GetBufferedDataSize() const;
		std::string GetStatistics() const;

		bool IsCompatibleWith(const AbstractSink* sink) override;
	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessCommand(const FilterCommand& cmd) override;
	private:
		mutable std::mutex m_mutex;
		// Configuration
		unsigned m_base_bitrate;
		double m_korr, m_korr2, m_N;

		// Current state
		std::deque<std::pair<vs::SharedBuffer, FrameMetadata>> m_buffers;
		size_t m_total_data_size;
		size_t m_last_send_size;
		std::chrono::steady_clock::time_point m_last_send;
		bool m_send_active;

		// Statistics
		int m_stat_counter;
		double m_mean_bitrate, m_mean_buf_size, m_mean_duration;
		unsigned m_max_bitrate, m_max_buf_size, m_min_duration, m_max_duration;
		std::array<unsigned, 9> m_send_group_distribution;

		std::shared_ptr<FilterUniformTransmit> shared_from_this()
		{
			return std::shared_ptr<FilterUniformTransmit>(AbstractFilter<AbstractSingleSourceSink>::shared_from_this(), this);
		}

		std::shared_ptr<FilterUniformTransmit const> shared_from_this() const
		{
			return std::shared_ptr<FilterUniformTransmit const>(AbstractFilter<AbstractSingleSourceSink>::shared_from_this(), this);
		}

		void SendBuffer();

		double GetCurrentBitrate() const;
	};
}