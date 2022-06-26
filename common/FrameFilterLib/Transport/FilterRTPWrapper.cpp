#include "FrameFilterLib/Transport/FilterRTPWrapper.h"
#include "Transcoder/VS_RTP_Buffers.h"

#include <cassert>

namespace ffl {
	std::shared_ptr<FilterRTPWrapper> FilterRTPWrapper::Create(const std::shared_ptr<AbstractSource>& src, const codec_pt_map_t& formats, const swap_bytes_map_t& swap_bytes)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterRTPWrapper>(formats, swap_bytes));
	}

	FilterRTPWrapper::FilterRTPWrapper(const codec_pt_map_t& formats, const swap_bytes_map_t& swap_bytes)
		: m_possibleFormats(formats)
		, m_swap_bytes(swap_bytes)
		, m_haveNewFormat(false)
		, m_ssrc_override(0)
	{
		SetName("RTP wrapper");
		SetFormat(FilterFormat::MakeRTP());
	}

	auto FilterRTPWrapper::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (buffer.empty())
			return e_noResult;

		UpdateFormat();
		if (!m_outBuff)
			return e_noResult;

		m_outBuff->Add(buffer.data<const void>(), buffer.size());
		return GetNextBuffer(buffer, md);
	}

	auto FilterRTPWrapper::GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& /*md*/) -> e_processingResult
	{
		if (!m_outBuff || m_outBuff->PacketsNum() == 0)
			return e_noResult;

		buffer = vs::SharedBuffer(m_outBuff->NextGetSize());

		unsigned long size;
		bool res = m_outBuff->Get(buffer.data<unsigned char>(), size);
		assert(res && buffer.size() == size);

		if (m_outBuff->PacketsNum() > 0)
			return e_moreBuffers;
		else
			return e_lastBuffer;
	}

	void FilterRTPWrapper::UpdateFormat()
	{
		if (!m_haveNewFormat)
			return;
		m_haveNewFormat = false;

		// keep sequence number to do not break libsrtp
		unsigned short seq = 0;
		if (m_outBuff) {
			seq = m_outBuff->GetSeq();
		}

		m_outBuff.reset();
		const auto codec = m_mf.dwVideoCodecFCC != 0 ? m_mf.dwVideoCodecFCC : m_mf.dwAudioCodecTag;
		auto fmt_it = m_possibleFormats.find(codec);
		if (fmt_it == m_possibleFormats.end())
			return;

		const auto pt = fmt_it->second;
		switch (codec)
		{
		case VS_VCODEC_H261:  m_outBuff = std::make_unique<VS_RTP_H261OutputBuffer>(); break;
		case VS_VCODEC_H263:  m_outBuff = std::make_unique<VS_RTP_H263OutputBuffer>(); break;
		case VS_VCODEC_H263P: m_outBuff = std::make_unique<VS_RTP_H263PlusOutputBuffer>(pt); break;
		case VS_VCODEC_H264:  m_outBuff = std::make_unique<VS_RTP_H264OutputBuffer>(pt); break;

		case VS_ACODEC_G711a:      m_outBuff = std::make_unique<VS_RTP_OutputBufferG711a>(); break;
		case VS_ACODEC_G711mu:     m_outBuff = std::make_unique<VS_RTP_OutputBufferG711mu>(); break;
		case VS_ACODEC_G723:       m_outBuff = std::make_unique<VS_RTP_OutputBufferG723>(); break;
		case VS_ACODEC_G728:       m_outBuff = std::make_unique<VS_RTP_OutputBufferG728>(); break;
		case VS_ACODEC_G729A:      m_outBuff = std::make_unique<VS_RTP_OutputBufferG729>(); break;
		case VS_ACODEC_G722:       m_outBuff = std::make_unique<VS_RTP_OutputBufferG722>(); break;
		case VS_ACODEC_G7221_24:   m_outBuff = std::make_unique<VS_RTP_OutputBufferAudio>(pt, 60, 20, 16, true); break; // 40 ms but at 16 kHz
		case VS_ACODEC_G7221_32:   m_outBuff = std::make_unique<VS_RTP_OutputBufferAudio>(pt, 80, 20, 16, true); break; // 40 ms but at 16 kHz
		case VS_ACODEC_G7221C_24:  m_outBuff = std::make_unique<VS_RTP_OutputBufferAudio>(pt, 60, 20, 32, !m_swap_bytes[pt]); break; // 40 ms but at 16 kHz
		case VS_ACODEC_G7221C_32:  m_outBuff = std::make_unique<VS_RTP_OutputBufferAudio>(pt, 80, 20, 32, !m_swap_bytes[pt]); break; // 40 ms but at 16 kHz
		case VS_ACODEC_G7221C_48:  m_outBuff = std::make_unique<VS_RTP_OutputBufferAudio>(pt, 120, 20, 32, !m_swap_bytes[pt]); break; // 40 ms but at 16 kHz
		case VS_ACODEC_SPEEX:      m_outBuff = std::make_unique<VS_RTP_OutputBufferAudioSpeex>(pt, 0, 80, 16); break; // 80ms packet size (4 speex frames (20ms each) in 1 rtp packet) 16kHz
		case VS_ACODEC_OPUS_B0914: m_outBuff = std::make_unique<VS_RTP_OutputBufferAudioOpus>(pt, 0, 80, 16); break;
		case VS_ACODEC_MP3:        m_outBuff = std::make_unique<VS_RTP_OutputBufferMPA>(); break;
		}

		if (m_outBuff) {
			if (seq) {
				m_outBuff->SetSeq(seq);
			}
			if (m_ssrc_override) {
				m_outBuff->m_ssrc = m_ssrc_override;
			}
		}
	}

	bool FilterRTPWrapper::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterRTPWrapper*>(sink);
		if (!p)
			return false;
		if (m_possibleFormats != p->m_possibleFormats)
			return false;
		if (m_swap_bytes != p->m_swap_bytes)
			return false;
		if (m_outBuff && p->m_outBuff && m_outBuff->m_ssrc != p->m_outBuff->m_ssrc)
			return false;
		return true;
	}

	bool FilterRTPWrapper::ProcessFormat(const std::shared_ptr<AbstractSource>& /*src*/, const FilterFormat& format)
	{
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format

		if (!m_mf.AudioEq(format.mf) || !m_mf.VideoEq(format.mf))
		{
			m_mf = format.mf;
			m_haveNewFormat = true;
		}
		return true;
	}
}