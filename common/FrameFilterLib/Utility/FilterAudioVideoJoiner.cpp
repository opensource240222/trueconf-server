#include "FrameFilterLib/Utility/FilterAudioVideoJoiner.h"
#include "FrameFilterLib/Base/FilterCommand.h"

namespace ffl {
	std::shared_ptr<FilterAudioVideoJoiner> FilterAudioVideoJoiner::Create()
	{
		return std::make_shared<FilterAudioVideoJoiner>();
	}

	FilterAudioVideoJoiner::FilterAudioVideoJoiner()
	{
		SetName("joiner");
	}

	void FilterAudioVideoJoiner::SetAudio(const std::shared_ptr<AbstractSource>& src)
	{
		if (m_audio == src)
			return;
		if (m_audio)
			m_audio->UnregisterSink(shared_from_this());
		m_audio = src;
		if (m_audio)
			m_audio->RegisterSinkOrGetCompatible(shared_from_this());
	}

	void FilterAudioVideoJoiner::SetVideo(const std::shared_ptr<AbstractSource>& src)
	{
		if (m_video == src)
			return;
		if (m_video)
			m_video->UnregisterSink(shared_from_this());
		m_video = src;
		if (m_video)
			m_video->RegisterSinkOrGetCompatible(shared_from_this());
	}

	auto FilterAudioVideoJoiner::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& /*buffer*/, FrameMetadata& /*md*/) -> e_processingResult
	{
		return e_lastBuffer;
	}

	bool FilterAudioVideoJoiner::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterAudioVideoJoiner*>(sink);
		if (!p)
			return false;
		if (m_audio && p->m_audio && m_audio != p->m_audio)
			return false;
		if (m_video && p->m_video && m_video != p->m_video)
			return false;
		return true;
	}

	bool FilterAudioVideoJoiner::ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format

		VS_MediaFormat mf;
		if (GetFormat().type == FilterFormat::e_mf)
			mf = GetFormat().mf;
		else
			mf.SetZero();
		if (src == m_audio && format.mf.dwAudioCodecTag != 0)
			mf.SetAudio(format.mf.dwAudioSampleRate, format.mf.dwAudioCodecTag);
		if (src == m_video && format.mf.dwVideoCodecFCC != 0)
			mf.SetVideo(format.mf.dwVideoWidht, format.mf.dwVideoHeight, format.mf.dwVideoCodecFCC, format.mf.dwFps, format.mf.dwStereo, format.mf.dwSVCMode);
		SetFormat(FilterFormat::MakeMF(mf));
		return true;
	}

	bool FilterAudioVideoJoiner::ProcessCommand(const FilterCommand& cmd)
	{
		if (AbstractSource::ProcessCommand(cmd))
			return true;

		if (cmd.type == FilterCommand::e_keyFrameRequest || cmd.type == FilterCommand::e_setBitrateRequest)
		{
			if (m_video)
				return m_video->ProcessCommand(cmd);
			return false;
		}
		return SendCommandToSources(cmd, false);
	}
}