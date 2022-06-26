#pragma once

#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"

#include <functional>

class VS_RTP_Channel;

namespace ffl {
	class SinkRTPChannel : public AbstractSingleSourceSink
	{
	public:
		typedef std::function<void()> fir_handler_t;

		static std::shared_ptr<SinkRTPChannel> Create();

		SinkRTPChannel();
		~SinkRTPChannel();

		std::shared_ptr<VS_RTP_Channel> GetRTPChannel() const;
		void SetRTPChannel(const std::shared_ptr<VS_RTP_Channel>& rtp);

		bool IsCompatibleWith(const AbstractSink* sink) override;
		void PutFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer buffer, FrameMetadata md) override;
		void NotifyNewFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

		void Stop();

	private:
		vs::atomic_shared_ptr<VS_RTP_Channel> m_rtp;
	};
}