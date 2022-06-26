#pragma once

#include "FrameFilterLib/Base/AbstractSource.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"

#include <cstddef>
#include <functional>

class VS_RTP_Channel;

namespace ffl {
	class SourceRTPChannel : public AbstractSource
	{
	public:
		typedef std::function<void()> fir_handler_t;

		static std::shared_ptr<SourceRTPChannel> Create();

		SourceRTPChannel();
		~SourceRTPChannel();

		std::shared_ptr<VS_RTP_Channel> GetRTPChannel() const;
		void SetRTPChannel(const std::shared_ptr<VS_RTP_Channel>& rtp, stream::Track track);
		void SetRTPChannel(std::nullptr_t);

		bool ProcessCommand(const FilterCommand& cmd) override;

		virtual void SetAlternativeFIRHandler(fir_handler_t&& fn);
		template <class Callable>
		void SetAlternativeFIRHandler(Callable fn)
		{
			SetAlternativeFIRHandler(fir_handler_t(fn));
		}

	private:
		vs::atomic_shared_ptr<VS_RTP_Channel> m_rtp;
		stream::Track m_track;
		fir_handler_t m_fir_handler;
	};
}