#pragma once

#include "FrameFilterLib/Base/AbstractThreadedFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std/cpplib/VS_MediaFormat.h"

#include <memory>

class VideoCodec;

namespace ffl {
	class FilterVideoDecoder : public AbstractThreadedFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterVideoDecoder> Create(const std::shared_ptr<AbstractSource>& src);

		FilterVideoDecoder();
		~FilterVideoDecoder();

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		void UpdateCodec();

		VS_MediaFormat m_mf_in;
		std::unique_ptr<VideoCodec> m_codec;
	};
}