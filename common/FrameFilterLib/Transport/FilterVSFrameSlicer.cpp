#include "FrameFilterLib/Transport/FilterVSFrameSlicer.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "streams/VS_SendFrameQueueBase.h"
#include "streams/ParseSvcStream.h"

#include "std-generic/compat/memory.h"
#include <chrono>

namespace ffl {
	std::shared_ptr<FilterVSFrameSlicer> FilterVSFrameSlicer::Create(const std::shared_ptr<AbstractSource>& src, bool out_svc_stream)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterVSFrameSlicer>(out_svc_stream));
	}

	FilterVSFrameSlicer::FilterVSFrameSlicer(bool out_svc_stream)
	{
		SetName("VS frame slicer");
		m_FrameQueue.reset(VS_SendFrameQueueBase::Factory(false, out_svc_stream));
		m_vframe_num = 0;
		m_timestamp = 0;
		m_out_svc_stream = out_svc_stream;
	}

	FilterVSFrameSlicer::~FilterVSFrameSlicer()
	{

	}

	auto FilterVSFrameSlicer::ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (buffer.empty())
			return e_noResult;

		switch (md.track)
		{
		case stream::Track::audio:
		{
			FrameQueueInfo info;
			info.timestamp = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now().time_since_epoch()).count();
			m_FrameQueue->AddFrame(1, buffer.size(), const_cast<unsigned char*>(buffer.data<const unsigned char>()), FRAME_PRIORITY_AUDIO, info, nullptr);
		}
			break;
		case stream::Track::video:
		{
			m_timestamp += md.interval;
			stream::SVCHeader h = { 0 };
			int32_t lsize(0), size(buffer.size());
			int32_t shift(0);
			auto src = const_cast<uint8_t*>(buffer.data<const uint8_t>());
			uint8_t *frame(nullptr);
			FrameQueueInfo info = { m_vframe_num, static_cast<uint8_t>(md.keyframe ? 0 : 1), m_timestamp, static_cast<uint32_t>(md.interval) };
			while (frame = ParseSvcStream(src, size, m_svc_mode != 0, &lsize, &h, &shift)) {
				m_FrameQueue->AddFrame(2, lsize, frame, FRAME_PRIORITY_VIDEO, info, (m_out_svc_stream) ? &h : nullptr);
				src += shift;
				size -= shift;
				if (m_only_base_layer) {
					break;
				}
			}
			m_vframe_num++;
		}
			break;
		case stream::Track::command:
			m_FrameQueue->AddFrame(254, buffer.size(), const_cast<unsigned char*>(buffer.data<const unsigned char>()), FRAME_PRIORITY_COMMAND, {}, nullptr);
			break;
		}

		return GetNextBuffer(buffer, md);
	}

	auto FilterVSFrameSlicer::GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		unsigned char* data = nullptr;
		int size(0);
		unsigned char track(0);
		unsigned char slayer(0);

		int res = m_FrameQueue->GetFrame(data, size, track, slayer);
		if (res > 0)
		{
			buffer = vs::SharedBuffer(size);
			std::memcpy(buffer.data(), data, buffer.size());
			md.track = static_cast<stream::Track>(track);
			m_FrameQueue->MarkFirstAsSend();
			return e_moreBuffers;
		}
		return e_noResult;
	}

	bool FilterVSFrameSlicer::ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat & format)
	{
		if (format.type != FilterFormat::e_mf)
			return false;
		if (m_svc_mode != format.mf.dwSVCMode) {
			if (auto log = m_log.lock()) {
				std::string msg;
				msg.reserve(128);
				msg.append("warning: change svc ").append(std::to_string(m_svc_mode)).append(" -> ").append(std::to_string(format.mf.dwSVCMode));
				log->TraceMessage(this, msg);
			}
			m_svc_mode = format.mf.dwSVCMode;
		}
		if (!m_out_svc_stream && m_svc_mode != 0) {
			if (auto log = m_log.lock()) {
				std::string msg;
				msg.reserve(128);
				msg.append("incorrect svc mode : out = false, input = ").append(std::to_string(m_svc_mode)).append("; send only base layer");
				log->TraceMessage(this, msg);
			}
			m_only_base_layer = true;
		}
		else {
			m_only_base_layer = false;
		}
		return true;
	}

	bool FilterVSFrameSlicer::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterVSFrameSlicer*>(sink);
		if (!p)
			return false;
		return true;
	}
}