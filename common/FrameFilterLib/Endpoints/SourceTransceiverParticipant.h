#pragma once

#include "FrameFilterLib/Base/AbstractSource.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "std/cpplib/VS_ClientCaps.h"

class VS_TransceiverParticipant;

namespace ffl {
	class SourceTransceiverParticipant : public AbstractSource
	{
	public:
		static std::shared_ptr<SourceTransceiverParticipant> Create(std::shared_ptr<VS_TransceiverParticipant> src, const VS_ClientCaps& caps);

		SourceTransceiverParticipant(std::shared_ptr<VS_TransceiverParticipant> src, const VS_ClientCaps& caps);
		~SourceTransceiverParticipant();

		bool ProcessCommand(const FilterCommand& cmd) override;

		void DisableAudio();
		void DisableVideo();

	private:
		void SendChangeSndMFormat();

		std::shared_ptr<VS_TransceiverParticipant> m_src;
		VS_MediaFormat m_mf_in_requested;
		VS_ClientCaps m_caps;
		std::chrono::steady_clock::time_point m_audio_last_disable_time;
		std::chrono::steady_clock::time_point m_video_last_disable_time;
		bool m_audio_disabled;
		bool m_video_disabled;
	};
}