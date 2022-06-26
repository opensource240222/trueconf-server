#include "FrameFilterLib/Video/FilterH264SpsPpsInjector.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "MediaParserLib/VS_H264Parser.h"

#include <boost/algorithm/hex.hpp>

namespace ffl {
	std::shared_ptr<FilterH264SpsPpsInjector> FilterH264SpsPpsInjector::Create(const std::shared_ptr<AbstractSource>& src)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterH264SpsPpsInjector>());
	}

	FilterH264SpsPpsInjector::FilterH264SpsPpsInjector()
	{
		SetName("SPS/PPS injector");
	}

	void FilterH264SpsPpsInjector::SetSPS(std::vector<unsigned char>&& sps)
	{
		m_saved_sps = std::move(sps);
	}

	void FilterH264SpsPpsInjector::SetPPS(std::vector<unsigned char>&& pps)
	{
		m_saved_pps = std::move(pps);
	}

	auto FilterH264SpsPpsInjector::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (md.track != stream::Track::video)
			return e_lastBuffer; // forward non-video data
		if (GetFormat().type != FilterFormat::e_mf || GetFormat().mf.dwVideoCodecFCC != VS_VCODEC_H264)
			return e_lastBuffer; // forward non-H264 video
		if (md.keyframe)
			FixH264KeyFrame(buffer);

		return e_lastBuffer;
	}

	bool FilterH264SpsPpsInjector::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterH264SpsPpsInjector*>(sink);
		if (!p)
			return false;
		return true;
	}

	void FilterH264SpsPpsInjector::FixH264KeyFrame(vs::SharedBuffer& buffer)
	{
		bool has_sps = false;
		bool has_pps = false;
		const unsigned char* p = buffer.data<const unsigned char>();
		const unsigned char* const p_end = p + buffer.size();
		while (p < p_end)
		{
			const unsigned char* nal;
			const unsigned char* nal_end;
			unsigned int start_code_size;
			if (NALFromBitstream_H264(p, p_end - p, nal, nal_end, start_code_size) != 0)
				break;

			const unsigned char nal_type = nal[start_code_size] & 0x1f;
			if (nal_type == 7)
			{
				m_saved_sps.assign(nal, nal_end);
				has_sps = true;
				if (auto log = m_log.lock())
				{
					std::string msg;
					msg.reserve(10 + m_saved_sps.size() * 2);
					msg += "read SPS: ";
					boost::algorithm::hex(m_saved_sps.begin(), m_saved_sps.end(), std::back_inserter(msg));
					log->TraceMessage(this, msg);
				}
			}
			else if (nal_type == 8)
			{
				m_saved_pps.assign(nal, nal_end);
				has_pps = true;
				if (auto log = m_log.lock())
				{
					std::string msg;
					msg.reserve(10 + m_saved_pps.size() * 2);
					msg += "read PPS: ";
					boost::algorithm::hex(m_saved_pps.begin(), m_saved_pps.end(), std::back_inserter(msg));
					log->TraceMessage(this, msg);
				}
			}
			else if (nal_type >= 1 && nal_type <= 5)
				break;
			p = nal_end;
		}

		size_t offset = 0;
		if (!has_sps)
			offset += m_saved_sps.size();
		if (!has_pps)
			offset += m_saved_pps.size();
		if (offset == 0)
			return;

		vs::SharedBuffer out_buffer(buffer.size() + offset);
		memcpy(out_buffer.data<unsigned char>() + offset, buffer.data<const void>(), buffer.size());
		unsigned char* out_p = out_buffer.data<unsigned char>();
		if (!has_sps)
		{
			memcpy(out_p, m_saved_sps.data(), m_saved_sps.size());
			out_p += m_saved_sps.size();
		}
		if (!has_pps)
		{
			memcpy(out_p, m_saved_pps.data(), m_saved_pps.size());
			out_p += m_saved_pps.size();
		}
		assert(out_p == out_buffer.data<unsigned char>() + offset);
		buffer = std::move(out_buffer);
	}
}
