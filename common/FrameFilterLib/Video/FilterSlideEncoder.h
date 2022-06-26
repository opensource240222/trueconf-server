#pragma once

#include "FrameFilterLib/Base/AbstractThreadedFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std/cpplib/VS_MediaFormat.h"

#include <atomic>
#include <chrono>
#include <memory>

struct SlideInfo;

namespace ffl {
	class FilterSlideEncoder : public AbstractThreadedFilter<AbstractSingleSourceSink> {
	public:
		static std::shared_ptr<FilterSlideEncoder> Create(const std::shared_ptr<AbstractSource>& src,
															const std::function<void(const std::vector<unsigned char> &data, const SlideInfo & info)> &on_slide_update,
															std::chrono::steady_clock::duration slide_interval,
															unsigned max_side, unsigned quality);

		FilterSlideEncoder(const std::function<void(const std::vector<unsigned char> &data, const SlideInfo &info)> &on_slide_update, std::chrono::steady_clock::duration slide_interval,
						   unsigned max_side, unsigned quality);

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		VS_MediaFormat m_mf_in;
		std::chrono::steady_clock::duration m_slide_interval;
		std::chrono::steady_clock::time_point m_next_slide;
		std::atomic<int> m_slide_counter;
		std::atomic<bool> m_slide_updating;
		unsigned m_max_side, m_quality;

		std::function<void(const std::vector<unsigned char> &data, const SlideInfo &info)> m_on_slide_update;

		void PrepareSlide(vs::SharedBuffer& buffer, FrameMetadata& md);

		std::shared_ptr<FilterSlideEncoder> shared_from_this() {
			return std::shared_ptr<FilterSlideEncoder>(AbstractFilter<AbstractSingleSourceSink>::shared_from_this(), this);
		}

		std::shared_ptr<FilterSlideEncoder const> shared_from_this() const {
			return std::shared_ptr<FilterSlideEncoder const>(AbstractFilter<AbstractSingleSourceSink>::shared_from_this(), this);
		}
	};
}