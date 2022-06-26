#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"

namespace ffl {
	class FilterH264SLInjector : public AbstractFilter<AbstractSingleSourceSink> {
	public:
		static std::shared_ptr<FilterH264SLInjector> Create(const std::shared_ptr<AbstractSource>& src);

		FilterH264SLInjector();

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessCommand(const FilterCommand& cmd) override;
	private:
		void InsertStreamLayout(const FilterFormat& format, vs::SharedBuffer& buffer, bool is_key);

		unsigned m_last_bitrate;
	};
}