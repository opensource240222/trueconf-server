#include "FrameFilterLib/Base/AbstractThreadedFilter.h"
#include "FrameFilterLib/Base/AbstractMultiSourceSink.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std/cpplib/VS_Singleton.h"

namespace ffl {
	template <class Sink>
	AbstractThreadedFilter<Sink>::AbstractThreadedFilter(std::chrono::steady_clock::duration frame_timeout)
		: m_strand(VS_Singleton<VS_ThreadPool>::Instance())
		, m_frame_timeout(frame_timeout)
	{
	}

	template <class Sink>
	void AbstractThreadedFilter<Sink>::PutFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer buffer, FrameMetadata md)
	{
		m_strand.Post([this, src, buffer, md, frame_expire_time = std::chrono::steady_clock::now() + m_frame_timeout, self = this->shared_from_this()]() mutable {
			if (std::chrono::steady_clock::now() > frame_expire_time)
				return;
			AbstractFilter<Sink>::PutFrame(src, std::move(buffer), md);
		});
	}

	template <class Sink>
	void AbstractThreadedFilter<Sink>::NotifyNewFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		m_strand.Post([this, src, format, self = this->shared_from_this()]() {
			AbstractFilter<Sink>::NotifyNewFormat(src, format);
		});
	}

	template class AbstractThreadedFilter<AbstractMultiSourceSink>;
	template class AbstractThreadedFilter<AbstractSingleSourceSink>;
}

