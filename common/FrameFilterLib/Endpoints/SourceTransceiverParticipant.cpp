#include "FrameFilterLib/Endpoints/SourceTransceiverParticipant.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "TransceiverCircuit/VS_TransceiverParticipant.h"
#include "streams/Protocol.h"
#include "streams/Command.h"

#include <cassert>

namespace ffl {
	std::shared_ptr<SourceTransceiverParticipant> SourceTransceiverParticipant::Create(std::shared_ptr<VS_TransceiverParticipant> src, const VS_ClientCaps& caps)
	{
		return std::make_shared<SourceTransceiverParticipant>(src, caps);
	}

	SourceTransceiverParticipant::SourceTransceiverParticipant(std::shared_ptr<VS_TransceiverParticipant> src, const VS_ClientCaps& caps)
		: m_src(std::move(src))
		, m_caps(caps)
		, m_audio_disabled(false)
		, m_video_disabled(false)
	{
		SetName("participant source");
		m_mf_in_requested.SetZero();

		VS_MediaFormat mf;
		m_caps.GetMediaFormat(mf);
		SetFormat(FilterFormat::MakeMF(mf));

		m_src->ConnectToNewFrame([this](const vs::SharedBuffer& buffer) {
			if (GetFormat().type != FilterFormat::e_mf)
				return;
			auto track = buffer.data<const stream::FrameHeader>()->track;
			if (track == stream::Track::audio && m_audio_disabled)
				DisableAudio();
			if (track == stream::Track::video && m_video_disabled)
				DisableVideo();
			SendFrameToSubscribers(vs::SharedBuffer(buffer), FrameMetadata{ track, 0, false });
		});
		m_src->ConnectToChangeRcvFormatRequest([this](const VS_MediaFormat& mf, bool need_reply) {
			SetFormat(FilterFormat::MakeMF(mf));
			if (need_reply)
			{
				stream::Command cmd;
				cmd.ChangeRcvMFormat(mf);
				cmd.MakeReply();
				m_src->SendFrame(reinterpret_cast<unsigned char*>(&cmd), cmd.Size(), stream::Track::command);
			}
		});
	}

	SourceTransceiverParticipant::~SourceTransceiverParticipant()
	{
	}

	bool SourceTransceiverParticipant::ProcessCommand(const FilterCommand& cmd)
	{
		if (AbstractSource::ProcessCommand(cmd))
			return true;

		switch (cmd.type)
		{
		case FilterCommand::e_keyFrameRequest:
		{
			stream::Command vscmd;
			vscmd.RequestKeyFrame();
			m_src->SendFrame(reinterpret_cast<unsigned char*>(&vscmd), vscmd.Size(), stream::Track::command);
			return true;
		}
		case FilterCommand::e_changeFormatRequest:
		{
			assert(GetFormat().type == FilterFormat::e_mf);
			bool request_changed = false;
			if (cmd.mf.IsAudioValid() && m_caps.FindAudioCodec(cmd.mf.dwAudioCodecTag))
			{
				m_mf_in_requested.SetAudio(cmd.mf.dwAudioSampleRate, cmd.mf.dwAudioCodecTag);
				m_audio_disabled = false;
				request_changed = true;
			}
			if (cmd.mf.IsVideoValid_WithoutMultiplicity8() && m_caps.FindVideoCodec(cmd.mf.dwVideoCodecFCC))
			{
				m_mf_in_requested.SetVideo(cmd.mf.dwVideoWidht, cmd.mf.dwVideoHeight, cmd.mf.dwVideoCodecFCC, cmd.mf.dwFps);
				m_video_disabled = false;
				request_changed = true;
			}
			if (!request_changed)
				return true;

			SendChangeSndMFormat();
			return true;
		}
		case FilterCommand::e_setBitrateRequest:
		{
			stream::Command vscmd;
			vscmd.RestrictBitrate(cmd.bitrate);
			m_src->SendFrame(reinterpret_cast<unsigned char*>(&vscmd), vscmd.Size(), stream::Track::command);
			return true;
		}
		default:
			return false;
		}
	}

	void SourceTransceiverParticipant::DisableAudio()
	{
		m_audio_disabled = true;
		m_mf_in_requested.SetAudio(0, 0, 0);
		auto now = std::chrono::steady_clock::now();
		if (now - m_audio_last_disable_time > std::chrono::seconds(5))
		{
			SendChangeSndMFormat();
			m_audio_last_disable_time = now;
		}
	}

	void SourceTransceiverParticipant::DisableVideo()
	{
		m_video_disabled = true;
		m_mf_in_requested.SetVideo(0, 0, 0);
		auto now = std::chrono::steady_clock::now();
		if (now - m_video_last_disable_time > std::chrono::seconds(5))
		{
			SendChangeSndMFormat();
			m_video_last_disable_time = now;
		}
	}

	void SourceTransceiverParticipant::SendChangeSndMFormat()
	{
		VS_MediaFormat mf(m_mf_in_requested);
		if (!m_audio_disabled && !mf.IsAudioValid())
			mf.SetAudio(GetFormat().mf.dwAudioSampleRate, GetFormat().mf.dwAudioCodecTag);
		if (!m_video_disabled && !mf.IsVideoValid_WithoutMultiplicity8())
			mf.SetVideo(GetFormat().mf.dwVideoWidht, GetFormat().mf.dwVideoHeight, GetFormat().mf.dwVideoCodecFCC);
		stream::Command vscmd;
		vscmd.ChangeSndMFormat(mf);
		m_src->SendFrame(reinterpret_cast<unsigned char*>(&vscmd), vscmd.Size(), stream::Track::command);
	}
}
