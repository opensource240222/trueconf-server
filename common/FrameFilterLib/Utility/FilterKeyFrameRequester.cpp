#include "FrameFilterLib/Utility/FilterKeyFrameRequester.h"
#include "FrameFilterLib/Base/FilterCommand.h"

namespace ffl {
	std::shared_ptr<FilterKeyFrameRequester> FilterKeyFrameRequester::Create(const std::shared_ptr<AbstractSource>& src, std::chrono::steady_clock::duration max_interval)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterKeyFrameRequester>(max_interval));
	}

	FilterKeyFrameRequester::FilterKeyFrameRequester(std::chrono::steady_clock::duration max_interval)
		: m_max_interval(max_interval)
	{
		SetName("key requester");
	}

	auto FilterKeyFrameRequester::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& /*buffer*/, FrameMetadata& md) -> e_processingResult
	{
		if (md.track == stream::Track::video)
		{
			auto now = std::chrono::steady_clock::now();
			if (md.keyframe)
				m_last_keyframe_time = now;
			else if (now - m_last_keyframe_time > m_max_interval)
				SendCommandToSources(FilterCommand::MakeKeyFrameRequest());
		}

		return e_lastBuffer;
	}

	bool FilterKeyFrameRequester::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterKeyFrameRequester*>(sink);
		if (!p)
			return false;
		if (m_max_interval != p->m_max_interval)
			return false;
		return true;
	}
}
