#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"

#include <chrono>

namespace ffl {
	class FilterKeyFrameRequestLimiter : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterKeyFrameRequestLimiter> Create(const std::shared_ptr<AbstractSource>& src, std::chrono::steady_clock::duration min_interval, unsigned max_burst = 1);

		FilterKeyFrameRequestLimiter(std::chrono::steady_clock::duration min_interval, unsigned max_burst);

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessCommand(const FilterCommand& cmd) override;

	private:
		std::chrono::steady_clock::duration m_min_interval;
		unsigned m_max_burst;

		std::chrono::steady_clock::time_point m_last_request_time;
		unsigned m_current_burst;
	};
}