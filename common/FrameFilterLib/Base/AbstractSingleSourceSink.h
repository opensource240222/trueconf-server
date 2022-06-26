#pragma once

#include "FrameFilterLib/Base/AbstractSink.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"

namespace ffl {
	class AbstractSingleSourceSink : public AbstractSink
	{
	public:
		AbstractSingleSourceSink();

		void NotifyNewSource(const std::shared_ptr<AbstractSource>& src) override;
		void NotifyRemoveSource(const std::shared_ptr<AbstractSource>& src) override;
		void Detach() override;
		bool SendCommandToSources(const FilterCommand& cmd, bool log = true) override;

	private:
		vs::atomic_shared_ptr<AbstractSource> m_source;
	};
}
