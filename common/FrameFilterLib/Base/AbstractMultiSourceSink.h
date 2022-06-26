#pragma once

#include "FrameFilterLib/Base/AbstractSink.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"

#include <set>

namespace ffl {
	class AbstractMultiSourceSink : public AbstractSink
	{
	public:
		AbstractMultiSourceSink();

		void NotifyNewSource(const std::shared_ptr<AbstractSource>& src) override;
		void NotifyRemoveSource(const std::shared_ptr<AbstractSource>& src) override;
		void Detach() override;
		bool SendCommandToSources(const FilterCommand& cmd, bool log = true) override;

	private:
		using source_set_t = std::set<std::shared_ptr<AbstractSource>>;
		vs::atomic_shared_ptr<const source_set_t> m_sources;
	};
}
