#include "FrameFilterLib/Transport/FilterVSFrameUnwrapper.h"
#include "streams/Protocol.h"

#include <cassert>

namespace ffl {
	std::shared_ptr<FilterVSFrameUnwrapper> FilterVSFrameUnwrapper::Create(const std::shared_ptr<AbstractSource>& src, stream::Track track)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterVSFrameUnwrapper>(track));
	}

	FilterVSFrameUnwrapper::FilterVSFrameUnwrapper(stream::Track track)
		: m_track(track)
	{
		SetName("VS frame unwrapper");
	}

	auto FilterVSFrameUnwrapper::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (buffer.empty())
			return e_noResult;

		const auto& frame = *buffer.data<const stream::FrameHeader>();
		if (m_track != frame.track)
			return e_noResult;

		m_inBuffer.Add(buffer.data<const unsigned char>() + sizeof(stream::FrameHeader), buffer.size() - sizeof(stream::FrameHeader), m_track);
		return GetNextBuffer(buffer, md);
	}

	auto FilterVSFrameUnwrapper::GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (m_inBuffer.NextGetSize() == 0)
			return e_noResult;

		buffer = vs::SharedBuffer(m_inBuffer.NextGetSize());

		stream::Track track;
		unsigned long video_interval;
		bool key;
		unsigned long size;
		int res = m_inBuffer.Get(buffer.data<unsigned char>(), size, track, video_interval, &key);
		assert(res >= 0 && buffer.size() == size);
		md.track = track;
		switch (md.track)
		{
		case stream::Track::video:
			md.interval = video_interval;
			md.keyframe = key;
			break;
		}

		if (m_inBuffer.NextGetSize() > 0)
			return e_moreBuffers;
		else
			return e_lastBuffer;
	}

	bool FilterVSFrameUnwrapper::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterVSFrameUnwrapper*>(sink);
		if (!p)
			return false;
		if (m_track != p->m_track)
			return false;
		return true;
	}

	bool FilterVSFrameUnwrapper::ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		// expect mft with both audio and video.

		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format

		if      (m_track == stream::Track::audio)
			SetFormat(FilterFormat::MakeAudio(format.mf.dwAudioCodecTag, format.mf.dwAudioSampleRate));
		else if (m_track == stream::Track::video)
			SetFormat(FilterFormat::MakeVideo(format.mf.dwVideoCodecFCC, format.mf.dwVideoWidht, format.mf.dwVideoHeight));
		return true;
	}
}