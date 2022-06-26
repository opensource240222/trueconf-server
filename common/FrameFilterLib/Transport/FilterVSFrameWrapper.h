#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"

#include <chrono>

namespace ffl {
	class FilterVSFrameWrapper : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterVSFrameWrapper> Create(const std::shared_ptr<AbstractSource>& src);

		FilterVSFrameWrapper();
		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;

	private:
		std::chrono::steady_clock::time_point m_last_frame_time;
	};
}