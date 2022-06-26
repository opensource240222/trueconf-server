#include "FrameFilterLib/Utility/FilterSourceSwitcher.h"
#include "FrameFilterLib/Base/FilterCommand.h"

namespace ffl {
	std::shared_ptr<FilterSourceSwitcher> FilterSourceSwitcher::Create()
	{
		return std::make_shared<FilterSourceSwitcher>();
	}

	FilterSourceSwitcher::FilterSourceSwitcher()
	{
		SetName("source switcher");
	}

	void FilterSourceSwitcher::SetActive(const std::weak_ptr<AbstractSource>& src)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_active_src = src;
	}

	auto FilterSourceSwitcher::ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& /*buffer*/, FrameMetadata& /*md*/) -> e_processingResult
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (src != m_active_src.lock())
			return e_noResult;

		SetFormat(src->GetFormat());
		return e_lastBuffer;
	}

	bool FilterSourceSwitcher::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterSourceSwitcher*>(sink);
		if (!p)
			return false;
		return true;
	}

	bool FilterSourceSwitcher::ProcessFormat(const std::shared_ptr<AbstractSource>& /*src*/, const FilterFormat& /*format*/)
	{
		return true;
	}
}