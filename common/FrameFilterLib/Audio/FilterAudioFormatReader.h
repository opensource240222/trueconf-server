#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std/cpplib/VS_MediaFormat.h"

namespace ffl {
	class FilterAudioFormatReader : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterAudioFormatReader> Create(const std::shared_ptr<AbstractSource>& src);

		FilterAudioFormatReader();

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		void ReadFormatFromFrame(vs::SharedBuffer& buffer);

		VS_MediaFormat m_mf_in;
	};
}