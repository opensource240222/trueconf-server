#include "FrameFilterLib/Endpoints/SinkTransceiverParticipant.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "TransceiverCircuit/VS_TransceiverParticipant.h"

#include <cassert>

namespace ffl {
	std::shared_ptr<SinkTransceiverParticipant> SinkTransceiverParticipant::Create(std::shared_ptr<VS_TransceiverParticipant> src)
	{
		return std::make_shared<SinkTransceiverParticipant>(src);
	}

	SinkTransceiverParticipant::SinkTransceiverParticipant(std::shared_ptr<VS_TransceiverParticipant> src)
		: m_src(std::move(src))
	{
		SetName("participant sink");
		m_src->ConnectToKeyFrameRequest([this]() {
			SendCommandToSources(FilterCommand::MakeKeyFrameRequest());
		});
		m_src->ConnectToSetBitrateRequest([this](unsigned int bitrate) {
			SendCommandToSources(FilterCommand::MakeSetBitrateRequest(bitrate));
		});
		m_src->ConnectToChangeSndFormatRequest([this](const VS_MediaFormat& mf) {
			SendCommandToSources(FilterCommand::MakeChangeFormatRequest(mf));
		});
		m_src->ConnectToChangeRcvFormatReply([this](const VS_MediaFormat& mf) {
			SendCommandToSources(FilterCommand::MakeChangeFormatRequest(mf));
		});
	}

	SinkTransceiverParticipant::~SinkTransceiverParticipant()
	{
	}

	bool SinkTransceiverParticipant::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const SinkTransceiverParticipant*>(sink);
		if (!p)
			return false;
		if (m_src != p->m_src)
			return false;
		return true;
	}

	void SinkTransceiverParticipant::PutFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer buffer, FrameMetadata md)
	{
		if (auto log = m_log.lock())
			log->TraceBuffer(this, buffer, md);

		m_src->SendFrame(std::move(buffer));
	}

	void SinkTransceiverParticipant::NotifyNewFormat(const std::shared_ptr<AbstractSource>& /*src*/, const FilterFormat& format)
	{
		if (auto log = m_log.lock())
			log->TraceFormat(this, format);
	}

	void SinkTransceiverParticipant::Stop()
	{
		Detach();
	}
}
