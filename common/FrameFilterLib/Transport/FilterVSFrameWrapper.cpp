#include "FrameFilterLib/Transport/FilterVSFrameWrapper.h"
#include "streams/Protocol.h"

namespace ffl {
	std::shared_ptr<FilterVSFrameWrapper> FilterVSFrameWrapper::Create(const std::shared_ptr<AbstractSource>& src)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterVSFrameWrapper>());
	}

	FilterVSFrameWrapper::FilterVSFrameWrapper()
	{
		SetName("VS frame wrapper");
	}

	auto FilterVSFrameWrapper::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (buffer.empty())
			return e_noResult;

		vs::SharedBuffer out_buffer(buffer.size() + sizeof(stream::FrameHeader));
		auto& frame = *out_buffer.data<stream::FrameHeader>();
		frame.length = buffer.size();
		frame.track = md.track;
		frame.cksum = stream::GetFrameBodyChecksum(buffer.data<const unsigned char>(), buffer.size());
		std::memcpy(out_buffer.data<char>() + sizeof(stream::FrameHeader), buffer.data<const void>(), buffer.size());
		buffer = std::move(out_buffer);

		auto now = std::chrono::steady_clock::now();
		if (m_last_frame_time.time_since_epoch() == std::chrono::steady_clock::duration::zero())
			m_last_frame_time = now;
		frame.tick_count = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_frame_time).count();
		m_last_frame_time = now;

		return e_lastBuffer;
	}

	bool FilterVSFrameWrapper::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterVSFrameWrapper*>(sink);
		if (!p)
			return false;
		return true;
	}
}