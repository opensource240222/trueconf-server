#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "streams/fwd.h"
#include "Transcoder/VS_VS_Buffers.h"

namespace ffl {
	class FilterVSFrameUnwrapper : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterVSFrameUnwrapper> Create(const std::shared_ptr<AbstractSource>& src, stream::Track track);

		explicit FilterVSFrameUnwrapper(stream::Track track);
		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		e_processingResult GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		stream::Track m_track;
		VS_VS_InputBuffer m_inBuffer;
	};
}