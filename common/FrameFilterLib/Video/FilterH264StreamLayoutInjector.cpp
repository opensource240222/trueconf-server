#include "FilterH264StreamLayoutInjector.h"

#include "FrameFilterLib/Base/FilterCommand.h"
#include "Transcoder/sei.h"

namespace ffl {
	std::shared_ptr<FilterH264SLInjector> FilterH264SLInjector::Create(const std::shared_ptr<AbstractSource>& src)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterH264SLInjector>());
	}

	FilterH264SLInjector::FilterH264SLInjector() : m_last_bitrate(56000)
	{
		SetName("SL injector");
	}

	auto FilterH264SLInjector::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (md.track != stream::Track::video)
			return e_lastBuffer; // forward non-video data

		const FilterFormat& format = GetFormat();
		if (format.type != FilterFormat::e_mf || format.mf.dwVideoCodecFCC != VS_VCODEC_H264)
			return e_lastBuffer; // forward non-H264 video

		InsertStreamLayout(format, buffer, md.keyframe);

		return e_lastBuffer;
	}

	bool FilterH264SLInjector::ProcessCommand(const FilterCommand& cmd)
	{
		if (AbstractSource::ProcessCommand(cmd))
			return true;

		if (cmd.type == FilterCommand::e_setBitrateRequest) {
			m_last_bitrate = cmd.bitrate;
		}

		return SendCommandToSources(cmd, false);
	}

	bool FilterH264SLInjector::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterH264SLInjector*>(sink);
		if (!p)
			return false;
		return true;
	}

	void FilterH264SLInjector::InsertStreamLayout(const FilterFormat& format, vs::SharedBuffer& buffer, bool /*is_key*/)
	{
		sei::PACSINalUnit p = {};

		p.F = 0;
		p.Type = 30;
		p.NRI = 3;
		p.I = 1; // is_key ? 1 : 0;
		p.R = 1;
		p.PRID = 0;
		p.N = 1;
		p.DID = 0;
		p.QID = 0;
		p.TID = 0;
		p.U = 1;
		p.D = 1;
		p.O = 0;
		p.RR = 3;

		p.T = 1;
		p.DONC = 0;

		sei::StreamLayoutMessage msg;
		msg.Type = 6;
		msg.PayloadType = 5;
		msg.PayloadSize = sizeof(sei::StreamLayoutMessage) - 3 + sizeof(sei::LayerDescription);
		msg.LPB0_0 = 1;
		msg.P = 1;
		msg.LDSize = sizeof(sei::LayerDescription);

		sei::LayerDescription l = {};
		l.CodedWidth = l.DisplayWidth = format.mf.dwVideoWidht;
		l.CodedHeight = l.DisplayHeight = format.mf.dwVideoHeight;
		l.Bitrate = m_last_bitrate * 1000;

		if (format.mf.dwFps == 30) l.FPSIdx = sei::fps_30;
		else if (format.mf.dwFps == 25) l.FPSIdx = sei::fps_25;
		else if (format.mf.dwFps == 15) l.FPSIdx = sei::fps_15;
		else l.FPSIdx = sei::fps_15;


		size_t buf_size = sizeof(sei::PACSINalUnit) + sizeof(sei::StreamLayoutMessage) +
			sizeof(sei::LayerDescription) + 2;
		if (!p.T) buf_size -= 2;

		vs::SharedBuffer out_buffer(buf_size + buffer.size());

		size_t off = 0;
		off += sei::WritePACSINalUnit(p, out_buffer.data<unsigned char>() + off, buf_size);
		*(uint16_t *)(out_buffer.data<unsigned char>() + off) = vs_htons(sizeof(sei::StreamLayoutMessage) + sizeof(sei::LayerDescription));
		off += 2;
		off += sei::WriteStreamLayoutMessage(msg, out_buffer.data<unsigned char>() + off, buf_size - off);
		off += sei::WriteLayerDescription(l, out_buffer.data<unsigned char>() + off, buf_size - off);

		memcpy(out_buffer.data<unsigned char>() + off, buffer.data<const void>(), buffer.size());
		off += buffer.size();

		buffer = std::move(out_buffer);
	}
}
