#include "FrameFilterLib/Base/AbstractOrderedFilter.h"
#include "FrameFilterLib/Base/AbstractMultiSourceSink.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"

namespace ffl {
	template <class Sink>
	void AbstractOrderedFilter<Sink>::PutFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer buffer, FrameMetadata md)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		AbstractFilter<Sink>::PutFrame(src, std::move(buffer), md);
	}

	template <class Sink>
	void AbstractOrderedFilter<Sink>::NotifyNewFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		AbstractFilter<Sink>::NotifyNewFormat(src, format);
	}

	template class AbstractOrderedFilter<AbstractMultiSourceSink>;
	template class AbstractOrderedFilter<AbstractSingleSourceSink>;
}

