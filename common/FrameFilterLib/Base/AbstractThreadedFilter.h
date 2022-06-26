#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "std/cpplib/VS_ThreadPool.h"

#include <chrono>

namespace ffl {
	template <class Sink>
	class AbstractThreadedFilter : public AbstractFilter<Sink>
	{
	public:
		explicit AbstractThreadedFilter(std::chrono::steady_clock::duration frame_timeout);
		void PutFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer buffer, FrameMetadata md) override;
		void NotifyNewFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		VS_ThreadPool::Strand m_strand;
		std::chrono::steady_clock::duration m_frame_timeout;
	};
}

