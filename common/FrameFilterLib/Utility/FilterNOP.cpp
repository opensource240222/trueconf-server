#include "FrameFilterLib/Utility/FilterNOP.h"

namespace ffl {
	std::shared_ptr<FilterNOP> FilterNOP::Create()
	{
		return std::make_shared<FilterNOP>();
	}

	FilterNOP::FilterNOP()
	{
		SetName("nop");
	}

	auto FilterNOP::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& /*buffer*/, FrameMetadata& /*md*/) -> e_processingResult
	{
		return e_lastBuffer;
	}

	bool FilterNOP::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterNOP*>(sink);
		if (!p)
			return false;
		if (p != this)
			return false;
		return true;
	}
}
