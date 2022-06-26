#pragma once

#include "FrameFilterLib/Base/AbstractThreadedFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std/cpplib/VS_MediaFormat.h"

#include <memory>

class AudioCodec;

namespace ffl {
	class FilterAudioDecoder : public AbstractThreadedFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterAudioDecoder> Create(const std::shared_ptr<AbstractSource>& src);

		FilterAudioDecoder();

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		void UpdateCodec();

		VS_MediaFormat m_mf_in;
		std::unique_ptr<AudioCodec> m_codec;
	};
}