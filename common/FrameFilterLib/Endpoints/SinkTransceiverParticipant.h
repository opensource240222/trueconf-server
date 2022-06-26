#pragma once

#include "FrameFilterLib/Base/AbstractMultiSourceSink.h"

#include <memory>

class VS_TransceiverParticipant;

namespace ffl {
	class SinkTransceiverParticipant : public AbstractMultiSourceSink
	{
	public:
		static std::shared_ptr<SinkTransceiverParticipant> Create(std::shared_ptr<VS_TransceiverParticipant> src);

		explicit SinkTransceiverParticipant(std::shared_ptr<VS_TransceiverParticipant> src);
		~SinkTransceiverParticipant();

		bool IsCompatibleWith(const AbstractSink* sink) override;
		void PutFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer buffer, FrameMetadata md) override;
		void NotifyNewFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

		void Stop();

	private:
		std::shared_ptr<VS_TransceiverParticipant> m_src;
	};
}