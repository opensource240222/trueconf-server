#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std-generic/cpplib/macro_utils.h"

#include <cstdint>
#include <list>
#include <queue>

namespace ffl {
	class FilterRTPSorter : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterRTPSorter> Create(const std::shared_ptr<AbstractSource>& src);

		FilterRTPSorter();
		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		e_processingResult GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md) override;

	private:
		struct packet_info
		{
			VS_FORWARDING_CTOR2(packet_info, seqno_ex, data) {}
			uint32_t seqno_ex; // sequence number extended to 32 bit by incrementing upper word on wrap-around
			vs::SharedBuffer data;
		};
		friend bool operator<(const packet_info& l, const packet_info& r);
		struct stream_info
		{
			uint32_t ssrc;
			std::priority_queue<packet_info> rtp_queue;
			uint32_t newest_timestamp; // timestamp of the last packet added to the queue
			uint32_t last_seqno_ex; // exteneded sequence number of the last packet returned from filter
		};
		std::list<stream_info> m_streams; // MRU stream list, we will keep only a small number of items in it
	};
}