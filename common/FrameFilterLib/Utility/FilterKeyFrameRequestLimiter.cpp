#include "FrameFilterLib/Utility/FilterKeyFrameRequestLimiter.h"
#include "FrameFilterLib/Base/FilterCommand.h"

namespace ffl {
	std::shared_ptr<FilterKeyFrameRequestLimiter> FilterKeyFrameRequestLimiter::Create(const std::shared_ptr<AbstractSource>& src, std::chrono::steady_clock::duration min_interval, unsigned max_burst)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterKeyFrameRequestLimiter>(min_interval, max_burst));
	}

	FilterKeyFrameRequestLimiter::FilterKeyFrameRequestLimiter(std::chrono::steady_clock::duration min_interval, unsigned max_burst)
		: m_min_interval(min_interval)
		, m_max_burst(max_burst)
		, m_current_burst(0)
	{
		SetName("key req limiter");
	}

	auto FilterKeyFrameRequestLimiter::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& /*buffer*/, FrameMetadata& /*md*/) -> e_processingResult
	{
		return e_lastBuffer;
	}

	bool FilterKeyFrameRequestLimiter::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterKeyFrameRequestLimiter*>(sink);
		if (!p)
			return false;
		if (m_min_interval != p->m_min_interval)
			return false;
		if (m_max_burst != p->m_max_burst)
			return false;
		return true;
	}

	bool FilterKeyFrameRequestLimiter::ProcessCommand(const FilterCommand& cmd)
	{
		if (AbstractSource::ProcessCommand(cmd))
			return true;

		if (cmd.type == FilterCommand::e_keyFrameRequest)
		{
			auto now = std::chrono::steady_clock::now();
			if (now - m_last_request_time < m_min_interval)
			{
				++m_current_burst;
				if (m_current_burst > m_max_burst)
					return false;
			}
			else
				m_current_burst = 1;
			m_last_request_time = now;
		}
		return SendCommandToSources(cmd, false);
	}
}