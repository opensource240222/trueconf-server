#include "FrameFilterLib/Audio/FilterAudioFormatReader.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "MediaParserLib/VS_MPAParser.h"
#include "std/cpplib/VS_MediaFormat_io.h"

namespace ffl {
	std::shared_ptr<FilterAudioFormatReader> FilterAudioFormatReader::Create(const std::shared_ptr<AbstractSource>& src)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterAudioFormatReader>());
	}

	FilterAudioFormatReader::FilterAudioFormatReader()
	{
		SetName("audio format reader");
		m_mf_in.SetZero();
	}

	auto FilterAudioFormatReader::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (md.track != stream::Track::audio)
			return e_lastBuffer; // forward non-audio data

		ReadFormatFromFrame(buffer);
		return e_lastBuffer;
	}

	bool FilterAudioFormatReader::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterAudioFormatReader*>(sink);
		if (!p)
			return false;
		return true;
	}

	bool FilterAudioFormatReader::ProcessFormat(const std::shared_ptr<AbstractSource>& /*src*/, const FilterFormat& format)
	{
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format
		if (m_mf_in == format.mf)
			return true; // format didn't change, nothing to do
		m_mf_in = format.mf;

		SetFormat(FilterFormat::MakeMF(m_mf_in));
		return true;
	}

	void FilterAudioFormatReader::ReadFormatFromFrame(vs::SharedBuffer& buffer)
	{
		VS_MediaFormat mf;
		mf.SetZero();
		MPALayer mpa_l;
		unsigned sr(0);
		if (GetFormat().mf.dwAudioCodecTag == VS_ACODEC_MP3
		 && ParseMPAFrameHeader(buffer.data<const void>(), buffer.size(), nullptr, &mpa_l, nullptr, &sr)
		 && mpa_l == MPA_L3 && sr != 0)
		{
			mf.SetAudio(sr, VS_ACODEC_MP3);
		}
		if (mf.IsAudioValid())
		{
			if (auto log = m_log.lock())
			{
				std::ostringstream msg;
				msg << "read format: " << stream_aformat(mf);
				log->TraceMessage(this, msg.str());
			}
			SetFormat(FilterFormat::MakeMF(mf, m_mf_in));
		}
	}
}