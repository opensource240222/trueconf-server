#include "FrameFilterLib/Video/FilterVideoFormatReader.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "MediaParserLib/VS_H264Parser.h"
#include "MediaParserLib/VS_H263Parser.h"
#include "MediaParserLib/VS_VPXParser.h"
#include "std/cpplib/VS_MediaFormat_io.h"

namespace ffl {
	std::shared_ptr<FilterVideoFormatReader> FilterVideoFormatReader::Create(const std::shared_ptr<AbstractSource>& src)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterVideoFormatReader>());
	}

	FilterVideoFormatReader::FilterVideoFormatReader()
	{
		SetName("video format reader");
		m_mf_in.SetZero();
	}

	auto FilterVideoFormatReader::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (md.track != stream::Track::video)
			return e_lastBuffer; // forward non-video data

		if (md.keyframe)
			ReadFormatFromFrame(buffer);
		return e_lastBuffer;
	}

	bool FilterVideoFormatReader::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterVideoFormatReader*>(sink);
		if (!p)
			return false;
		return true;
	}

	bool FilterVideoFormatReader::ProcessFormat(const std::shared_ptr<AbstractSource>& /*src*/, const FilterFormat& format)
	{
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format
		if (m_mf_in == format.mf)
			return true; // format didn't change, nothing to do
		m_mf_in = format.mf;

		if (GetFormat().type == FilterFormat::e_mf)
			SetFormat(FilterFormat::MakeMF(m_mf_in, GetFormat().mf));
		else
			SetFormat(FilterFormat::MakeMF(m_mf_in));

		return true;
	}

	void FilterVideoFormatReader::ReadFormatFromFrame(vs::SharedBuffer& buffer)
	{
		VS_MediaFormat mf;
		mf.SetZero();
		int w(0), h(0), n(0);
		if (ResolutionFromBitstream_VPX(buffer.data<const void>(), buffer.size(), w, h, n) == 0)
			mf.SetVideo(w, h, VS_VCODEC_VPX);
		else
		{
			auto ret = ResolutionFromBitstream_H264(buffer.data<const void>(), buffer.size(), w, h);
			if (ret == 0)
				mf.SetVideo(w, h, VS_VCODEC_H264);
			else if (ret == -2)
				;
			else if (ResolutionFromBitstream_H263(buffer.data<const void>(), buffer.size(), w, h) == 0)
				mf.SetVideo(w, h, VS_VCODEC_H263);
		}
		if (mf.IsVideoValid_WithoutMultiplicity8())
		{
			if (auto log = m_log.lock())
			{
				std::ostringstream msg;
				msg << "read format: " << stream_vformat(mf);
				log->TraceMessage(this, msg.str());
			}
			SetFormat(FilterFormat::MakeMF(m_mf_in, mf));
		}
		else
			SetFormat(FilterFormat::MakeMF(m_mf_in));
	}
}