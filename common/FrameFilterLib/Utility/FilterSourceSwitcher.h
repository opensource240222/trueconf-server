#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractMultiSourceSink.h"

#include <memory>
#include <mutex>

namespace ffl {
	class FilterSourceSwitcher : public AbstractFilter<AbstractMultiSourceSink>
	{
	public:
		static std::shared_ptr<FilterSourceSwitcher> Create();

		FilterSourceSwitcher();

		void SetActive(const std::weak_ptr<AbstractSource>& src);

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		std::mutex m_mutex;
		std::weak_ptr<AbstractSource> m_active_src; // waiting for std::atomic_weak_ptr (n4162)
	};
}