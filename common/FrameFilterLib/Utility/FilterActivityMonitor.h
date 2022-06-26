#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"

#include <boost/signals2/signal.hpp>
#include <boost/signals2/connection.hpp>

#include <atomic>
#include <chrono>

namespace ffl {
	class FilterActivityMonitor : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterActivityMonitor> Create(
			const std::shared_ptr<AbstractSource>& src,
			std::chrono::steady_clock::duration min_on_period,
			std::chrono::steady_clock::duration max_off_period,
			std::chrono::steady_clock::duration report_interval = std::chrono::steady_clock::duration::zero());

		// When period between frames becomes less than min_on_period source is considered active.
		// When period between frames becomes more than max_off_period source is considered inactive.
		// If report_interval is non zero, signal is activated with period in addition to when state changes.
		FilterActivityMonitor(std::chrono::steady_clock::duration min_on_period, std::chrono::steady_clock::duration max_off_period, std::chrono::steady_clock::duration report_interval);

		typedef boost::signals2::signal<void(bool state)> StateReportSignalType;
		boost::signals2::connection ConnectToStateReport(const StateReportSignalType::slot_type& slot)
		{
			return m_signal_StateReport.connect(slot);
		}

		bool State() const
		{
			return m_active;
		}

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;

	private:
		void SchedulePeriodicReport();

	private:
		std::chrono::steady_clock::duration m_min_on_period;
		std::chrono::steady_clock::duration m_max_off_period;
		std::chrono::steady_clock::duration m_report_interval;

		std::atomic<bool> m_active;
		std::atomic<unsigned> m_frame_id;
		std::atomic<unsigned> m_report_id;
		std::chrono::steady_clock::time_point m_last_frame_time;

		StateReportSignalType m_signal_StateReport;
	};
}
