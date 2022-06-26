#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"

class VS_SendFrameQueueBase;

namespace ffl {
	class FilterVSFrameSlicer : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterVSFrameSlicer> Create(const std::shared_ptr<AbstractSource>& src, bool out_svc_stream);

		FilterVSFrameSlicer(bool out_svc_stream);
		~FilterVSFrameSlicer();

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		e_processingResult GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		std::unique_ptr<VS_SendFrameQueueBase> m_FrameQueue;
		unsigned char m_vframe_num;
		unsigned int m_timestamp;
		bool m_out_svc_stream;
		bool m_only_base_layer = false;
		uint32_t m_svc_mode = 0;

	};
}