#include "FrameFilterLib/Endpoints/SinkRTPChannel.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "tools/SingleGatewayLib/VS_RTPMediaChannels.h"

#include <cmath>

namespace ffl {
	std::shared_ptr<SinkRTPChannel> SinkRTPChannel::Create()
	{
		return std::make_shared<SinkRTPChannel>();
	}

	SinkRTPChannel::SinkRTPChannel()
	{
		SetName("RTP sink");
	}

	SinkRTPChannel::~SinkRTPChannel()
	{
		SetRTPChannel(nullptr);
	}

	std::shared_ptr<VS_RTP_Channel> SinkRTPChannel::GetRTPChannel() const
	{
		return m_rtp.load(std::memory_order_relaxed);
	}

	void SinkRTPChannel::SetRTPChannel(const std::shared_ptr<VS_RTP_Channel>& rtp)
	{
		auto old_rtp = m_rtp.exchange(rtp, std::memory_order_relaxed);

		if (old_rtp == rtp)
			return;

		if (old_rtp)
		{
			old_rtp->SetFIRCallBack(nullptr);
		}
		if (rtp)
		{
			rtp->SetFIRCallBack([this, weak_self = weak_from_this()]() {
				auto self = weak_self.lock();
				if (!self)
					return;

				SendCommandToSources(FilterCommand::MakeKeyFrameRequest());
			});
			rtp->SetVSRCallBack([this, weak_self = weak_from_this()](int max_w, int max_h, int max_fps, unsigned max_bitrate) {
				auto self = weak_self.lock();
				if (!self)
					return;

				static const struct { unsigned w, h; } modes_16x9[] = { { 1920, 1080 }, { 1280, 720 }, { 864, 480 }, { 640, 360 }, { 320, 176 } };
				static const struct { unsigned w, h; } modes_4x3[]  = { { 1920, 1440 }, { 1280, 960 }, { 864, 648 }, { 640, 480 }, { 320, 240 } };

				int w = 320, h = 176;
				float aspect = float(max_h) / max_w;

				if (std::abs(aspect - 9.0f / 16) < 0.1f) {
					w = 320; h = 176;

					for (const auto &m : modes_16x9) {
						if (max_w >= m.w) {
							w = m.w;
							h = m.h;
							break;
						}
					}
				} else if (std::abs(aspect - 3.0f / 4) < 0.1f) {
					w = 320; h = 240;

					for (const auto &m : modes_4x3) {
						if (max_w >= m.w) {
							w = m.w;
							h = m.h;
							break;
						}
					}
				}

				VS_MediaFormat mf;
				mf.dwVideoCodecFCC = 0; // keep codec the same
				mf.dwVideoWidht = w;
				mf.dwVideoHeight = h;
				SendCommandToSources(FilterCommand::MakeChangeFormatRequest(mf));
				SendCommandToSources(FilterCommand::MakeSetBitrateRequest(max_bitrate/1000));
			});
		}
	}

	bool SinkRTPChannel::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const SinkRTPChannel*>(sink);
		if (!p)
			return false;
		if (m_rtp.load(std::memory_order_relaxed) != p->m_rtp.load(std::memory_order_relaxed))
			return false;
		return true;
	}

	void SinkRTPChannel::PutFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer buffer, FrameMetadata md)
	{
		if (auto log = m_log.lock())
			log->TraceBuffer(this, buffer, md);

		auto rtp = m_rtp.load(std::memory_order_relaxed);
		if (!rtp)
			return;
		rtp->SendRTP(std::move(buffer));
	}

	void SinkRTPChannel::NotifyNewFormat(const std::shared_ptr<AbstractSource>& /*src*/, const FilterFormat& format)
	{
		if (auto log = m_log.lock())
			log->TraceFormat(this, format);
	}

	void SinkRTPChannel::Stop()
	{
		Detach();
	}
}