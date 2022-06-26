#pragma once

#include "FrameFilterLib/Base/AbstractThreadedFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std/cpplib/VS_MediaFormat.h"

#include <memory>

class AudioCodec;

namespace ffl {
	class FilterAudioEncoder : public AbstractThreadedFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterAudioEncoder> Create(const std::shared_ptr<AbstractSource>& src, unsigned codec_tag);

		explicit FilterAudioEncoder(unsigned codec_tag);
		unsigned GetCodec() const { return m_codec_tag; }

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		e_processingResult GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		void UpdateCodec();

		const unsigned m_codec_tag;
		VS_MediaFormat m_mf_in;
		std::unique_ptr<AudioCodec> m_codec;
		vs::SharedBuffer m_in_buffer;
	};
}