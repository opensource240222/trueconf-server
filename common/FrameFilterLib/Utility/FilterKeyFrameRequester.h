#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"

#include <chrono>

namespace ffl {
	class FilterKeyFrameRequester : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterKeyFrameRequester> Create(const std::shared_ptr<AbstractSource>& src, std::chrono::steady_clock::duration max_interval);

		explicit FilterKeyFrameRequester(std::chrono::steady_clock::duration max_interval);

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;

	private:
		std::chrono::steady_clock::duration m_max_interval;

		std::chrono::steady_clock::time_point m_last_keyframe_time;
	};
}
