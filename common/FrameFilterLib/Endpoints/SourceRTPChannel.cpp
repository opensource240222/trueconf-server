#include "FrameFilterLib/Endpoints/SourceRTPChannel.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "tools/SingleGatewayLib/VS_RTPMediaChannels.h"

namespace ffl {
	std::shared_ptr<SourceRTPChannel> SourceRTPChannel::Create()
	{
		return std::make_shared<SourceRTPChannel>();
	}

	SourceRTPChannel::SourceRTPChannel()
		: m_track()
	{
		SetName("RTP source");
		SetFormat(FilterFormat::MakeRTP());
	}

	SourceRTPChannel::~SourceRTPChannel()
	{
		SetRTPChannel(nullptr);
	}

	std::shared_ptr<VS_RTP_Channel> SourceRTPChannel::GetRTPChannel() const
	{
		return m_rtp.load(std::memory_order_relaxed);
	}

	void SourceRTPChannel::SetRTPChannel(const std::shared_ptr<VS_RTP_Channel>& rtp, stream::Track track)
	{
		auto old_rtp = m_rtp.exchange(rtp, std::memory_order_relaxed);
		m_track = track;

		if (old_rtp == rtp)
			return;

		if (old_rtp)
		{
			old_rtp->SetReceiveCallBack(nullptr);
		}
		if (rtp)
		{
			rtp->SetReceiveCallBack([this, weak_self = weak_from_this()](vs::SharedBuffer&& packet) {
				auto self = weak_self.lock();
				if (!self)
					return;

				if (packet.empty())
					return;

				SendFrameToSubscribers(std::move(packet), FrameMetadata{ m_track, 0, false });
			});
		}
	}

	void SourceRTPChannel::SetRTPChannel(std::nullptr_t)
	{
		SetRTPChannel(nullptr, m_track);
	}

	void SourceRTPChannel::SetAlternativeFIRHandler(fir_handler_t&& fn)
	{
		m_fir_handler = std::move(fn);
	}

	bool SourceRTPChannel::ProcessCommand(const FilterCommand& cmd)
	{
		if (AbstractSource::ProcessCommand(cmd))
			return true;

		switch (cmd.type)
		{
		case FilterCommand::e_keyFrameRequest:
			if (m_fir_handler)
				m_fir_handler();
			else
			{
				auto rtp = m_rtp.load(std::memory_order_relaxed);
				if (!rtp)
					return false;
				rtp->SendFIR();
			}
			return true;
		default:
			return false;
		}
	}
}