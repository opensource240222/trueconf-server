#include "FrameFilterLib/Transport/FilterRTPValidator.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "Transcoder/RTPPacket.h"

namespace ffl {
	std::shared_ptr<FilterRTPValidator> FilterRTPValidator::Create(const std::shared_ptr<AbstractSource>& src, const pt_set_t& valid_pt, bool accept_zero_ssrc)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterRTPValidator>(valid_pt, accept_zero_ssrc));
	}

	FilterRTPValidator::FilterRTPValidator(const pt_set_t& valid_pt, bool accept_zero_ssrc)
		: m_valid_pt(valid_pt)
		, m_accept_zero_ssrc(accept_zero_ssrc)
		, m_non_zero_ssrc_seen(false)
	{
		SetName("RTP validator");
	}

	auto FilterRTPValidator::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		RTPPacket packet(buffer.data<const void>(), buffer.size());

		if (!packet.IsValid())
		{
			if (auto log = m_log.lock())
				log->TraceMessage(this, "invalid packet");
			return e_noResult;
		}

		if (packet.Version() != 2)
		{
			if (auto log = m_log.lock())
			{
				std::string msg;
				msg.reserve(64);
				msg.append("invalid version: ").append(std::to_string(packet.Version()));
				log->TraceMessage(this, msg);
			}
			return e_noResult;
		}

		if (packet.DataSize() == 0)
		{
			if (auto log = m_log.lock())
				log->TraceMessage(this, "empty payload");
			return e_noResult;
		}

		if (packet.SSRC() == 0 && !m_accept_zero_ssrc && m_non_zero_ssrc_seen)
		{
			if (auto log = m_log.lock())
				log->TraceMessage(this, "zero SSRC");
			return e_noResult;
		}

		if (m_valid_pt.count(packet.PayloadType()) == 0)
		{
			if (auto log = m_log.lock())
			{
				std::string msg;
				msg.reserve(64);
				msg.append("unknown payload type: ").append(std::to_string(packet.PayloadType()));
				log->TraceMessage(this, msg);
			}
			return e_noResult;
		}

		if (packet.SSRC() != 0)
			m_non_zero_ssrc_seen = true;

		return e_lastBuffer;
	}

	bool FilterRTPValidator::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterRTPValidator*>(sink);
		if (!p)
			return false;
		if (m_valid_pt != p->m_valid_pt)
			return false;
		if (m_accept_zero_ssrc != p->m_accept_zero_ssrc)
			return false;
		return true;
	}
}