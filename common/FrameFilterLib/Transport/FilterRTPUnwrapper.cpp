#include "FrameFilterLib/Transport/FilterRTPUnwrapper.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "Transcoder/VS_RTP_Buffers.h"

#include <cassert>

namespace ffl {
	std::shared_ptr<FilterRTPUnwrapper> FilterRTPUnwrapper::Create(const std::shared_ptr<AbstractSource>& src, const format_map_t& formats, const mpeg4es_config_map_t& mpeg4es_conf, const swap_bytes_map_t& swap_bytes, bool use_h264_uc)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterRTPUnwrapper>(formats, mpeg4es_conf, swap_bytes, use_h264_uc));
	}

	FilterRTPUnwrapper::FilterRTPUnwrapper(const format_map_t& formats, const mpeg4es_config_map_t& mpeg4es_conf, const swap_bytes_map_t& swap_bytes, bool use_h264_uc)
		: m_possibleFormats(formats)
		, m_mpeg4es_conf(mpeg4es_conf)
		, m_swap_bytes(swap_bytes)
		, m_use_h264_uc(use_h264_uc)
		, m_currPT(255)
	{
		SetName("RTP unwrapper");
	}

	auto FilterRTPUnwrapper::ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		RTPPacket packet(buffer.data<const void>(), buffer.size());
		if (!packet.IsValid() || packet.DataSize() == 0)
			return e_noResult;

		UpdateFormat(packet.PayloadType());
		if (m_currPT != packet.PayloadType())
			return e_noResult;
		if (!m_inBuff)
			return e_noResult;

		m_inBuff->Add(&packet);
		return GetNextBuffer(buffer, md);
	}

	auto FilterRTPUnwrapper::GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (!m_inBuff || m_inBuff->NextGetSize() == 0)
			return e_noResult;

		buffer = vs::SharedBuffer(m_inBuff->NextGetSize());

		unsigned long size;
		unsigned long video_interval;
		char key;
		int res = m_inBuff->Get(buffer.data<unsigned char>(), size, video_interval, key);
		assert(res >= 0 && buffer.size() == size);
		switch (md.track)
		{
		case stream::Track::video:
			md.interval = video_interval;
			md.keyframe = key != 0;
			break;
		}

		if (m_inBuff->NextGetSize() > 0)
			return e_moreBuffers;
		else
			return e_lastBuffer;
	}

	void FilterRTPUnwrapper::UpdateFormat(uint8_t pt)
	{
		if (m_currPT == pt)
			return;

		if (auto log = m_log.lock())
		{
			std::string msg;
			msg.reserve(64);
			msg.append("received payload type ").append(std::to_string(pt));
			log->TraceMessage(this, msg);
		}

		auto fmt_it = m_possibleFormats.find(pt);
		if (fmt_it == m_possibleFormats.end())
			return;

		m_inBuff.reset();
		m_currPT = pt;
		const VS_MediaFormat& mf = fmt_it->second;
		if (mf.dwVideoCodecFCC != 0)
		{
			switch (mf.dwVideoCodecFCC)
			{
			case VS_VCODEC_H261:  m_inBuff = std::make_unique<VS_RTP_H261InputBuffer>(); break;
			case VS_VCODEC_H263:
			case VS_VCODEC_H263P: m_inBuff = std::make_unique<VS_RTP_H263PlusInputBuffer>(); break;
			case VS_VCODEC_H264:  {
					if (!m_use_h264_uc) {
						m_inBuff = std::make_unique<VS_RTP_H264InputBuffer>();
					} else {
						m_inBuff = std::make_unique<VS_RTP_InputBufferXH264UC>();
					}
				} break;
			}
		}
		else
		{
			switch (mf.dwAudioCodecTag)
			{
			case VS_ACODEC_G711a:      m_inBuff = std::make_unique<VS_RTP_InputBufferG711a>(); break;
			case VS_ACODEC_G711mu:     m_inBuff = std::make_unique<VS_RTP_InputBufferG711mu>(); break;
			case VS_ACODEC_G723:       m_inBuff = std::make_unique<VS_RTP_InputBufferG723>(); break;
			case VS_ACODEC_G728:       m_inBuff = std::make_unique<VS_RTP_InputBufferG728>(); break;
			case VS_ACODEC_G729A:      m_inBuff = std::make_unique<VS_RTP_InputBufferG729>(); break;
			case VS_ACODEC_G722:       m_inBuff = std::make_unique<VS_RTP_InputBufferG722>(); break;
			case VS_ACODEC_G7221_24:   m_inBuff = std::make_unique<VS_RTP_InputBufferAudio>((Rtp_PayloadType)pt, true); break;
			case VS_ACODEC_G7221_32:   m_inBuff = std::make_unique<VS_RTP_InputBufferAudio>((Rtp_PayloadType)pt, true); break;
			case VS_ACODEC_G7221C_24:  m_inBuff = std::make_unique<VS_RTP_InputBufferAudio>((Rtp_PayloadType)pt, !m_swap_bytes[pt]); break;
			case VS_ACODEC_G7221C_32:  m_inBuff = std::make_unique<VS_RTP_InputBufferAudio>((Rtp_PayloadType)pt, !m_swap_bytes[pt]); break;
			case VS_ACODEC_G7221C_48:  m_inBuff = std::make_unique<VS_RTP_InputBufferAudio>((Rtp_PayloadType)pt, !m_swap_bytes[pt]); break;
			case VS_ACODEC_SPEEX:      m_inBuff = std::make_unique<VS_RTP_InputBufferAudio>((Rtp_PayloadType)pt); break;
			case VS_ACODEC_OPUS_B0914: m_inBuff = std::make_unique<VS_RTP_InputBufferAudio>((Rtp_PayloadType)pt); break;
			case VS_ACODEC_MP3:        m_inBuff = std::make_unique<VS_RTP_InputBufferMPA>(); break;
			case VS_ACODEC_AAC:        m_inBuff = std::make_unique<VS_RTP_InputBufferAAC>(m_mpeg4es_conf[pt]); break;
			}
		}

		if (!m_inBuff)
		{
			m_currPT = 255;
			return;
		}

		SetFormat(FilterFormat::MakeMF(mf));
	}

	bool FilterRTPUnwrapper::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterRTPUnwrapper*>(sink);
		if (!p)
			return false;
		if (m_possibleFormats != p->m_possibleFormats)
			return false;
		if (m_mpeg4es_conf != p->m_mpeg4es_conf)
			return false;
		if (m_swap_bytes != p->m_swap_bytes)
			return false;
		return true;
	}
}