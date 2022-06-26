#include "FrameFilterLib/Utility/FilterActivityMonitor.h"
#include "std/cpplib/VS_Singleton.h"
#include "std/cpplib/VS_ThreadPool.h"

namespace ffl {
	std::shared_ptr<FilterActivityMonitor> FilterActivityMonitor::Create(
		const std::shared_ptr<AbstractSource>& src,
		std::chrono::steady_clock::duration min_on_period,
		std::chrono::steady_clock::duration max_off_period,
		std::chrono::steady_clock::duration report_interval)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterActivityMonitor>(min_on_period, max_off_period, report_interval));
	}

	FilterActivityMonitor::FilterActivityMonitor(std::chrono::steady_clock::duration min_on_period, std::chrono::steady_clock::duration max_off_period, std::chrono::steady_clock::duration report_interval)
		: m_min_on_period(min_on_period)
		, m_max_off_period(max_off_period)
		, m_report_interval(report_interval)
		, m_active(false)
		, m_frame_id(0)
		, m_report_id(0)
	{
		SetName("activity monitor");
	}

	auto FilterActivityMonitor::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& /*md*/) -> e_processingResult
	{
		auto now = std::chrono::steady_clock::now();
		++m_frame_id;

		// If this is the first frame then only save the time
		if (m_last_frame_time.time_since_epoch().count() == 0)
		{
			m_last_frame_time = now;
			return e_lastBuffer;
		}

		if (m_active)
		{
			// Schedule inactivity check
			VS_Singleton<VS_ThreadPool>::Instance().Post([this, frame_id = m_frame_id.load(), self_weak = weak_from_this()]() {
				auto self = self_weak.lock();
				if (!self)
					return;
				if (frame_id != m_frame_id)
					return;

				++m_report_id;
				m_signal_StateReport(m_active = false);
			}, now + m_max_off_period);
		}
		else
		{
			// Check if source became active
			if (now - m_last_frame_time <= m_min_on_period)
			{
				SchedulePeriodicReport();
				m_signal_StateReport(m_active = true);
			}
		}

		m_last_frame_time = now;
		return e_lastBuffer;
	}

	void FilterActivityMonitor::SchedulePeriodicReport()
	{
		if (m_report_interval.count() == 0)
			return;

		VS_Singleton<VS_ThreadPool>::Instance().Post([this, report_id = ++m_report_id, self_weak = weak_from_this()]() {
			auto self = self_weak.lock();
			if (!self)
				return;
			if (report_id != m_report_id)
				return;

			SchedulePeriodicReport();
			m_signal_StateReport(m_active);
		}, m_report_interval);
	}

	bool FilterActivityMonitor::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterActivityMonitor*>(sink);
		if (!p)
			return false;
		if (m_min_on_period != p->m_min_on_period)
			return false;
		if (m_max_off_period != p->m_max_off_period)
			return false;
		return true;
	}
}
