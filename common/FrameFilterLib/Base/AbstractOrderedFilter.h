#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"

#include <mutex>

namespace ffl {
	template <class Sink>
	class AbstractOrderedFilter : public AbstractFilter<Sink>
	{
	public:
		void PutFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer buffer, FrameMetadata md) override;
		void NotifyNewFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	protected:
		std::mutex m_mutex;
	};
}

