#include "FrameFilterLib/Transport/FilterRTPSorter.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "Transcoder/RTPPacket.h"

#include <iomanip>

namespace ffl {
	static const int c_seqno_threshold = 1000; // Maximum absolut difference between sequence numbers for them to be considered related (from same "epoch")
	static const unsigned c_max_streams = 8; // Maximum number of streams which we will track
	static const int c_max_queue_size = 16; // Maximum size of reordering queue when it contains packets with different timestamps (from multiple frames).
	static const int c_max_queue_size_one_frame = 128 * 3; // Maximum size of reordering queue when it contains packets with the same timestamp (from one frame).

	namespace {

	inline uint16_t GetRTPSeqNo(const vs::SharedBuffer& buffer)
	{
		return RTPPacket::SeqNo(buffer.data<const void>(), buffer.size());
	}

	inline uint32_t GetRTPTimestamp(const vs::SharedBuffer& buffer)
	{
		return RTPPacket::Timestamp(buffer.data<const void>(), buffer.size());
	}

	inline uint32_t GetRTPSsrc(const vs::SharedBuffer& buffer)
	{
		return RTPPacket::SSRC(buffer.data<const void>(), buffer.size());
	}

	}

	bool operator<(const FilterRTPSorter::packet_info& l, const FilterRTPSorter::packet_info& r)
	{
		// We are sorting by priority, so packet with lower priority has higher extended sequence number
		return l.seqno_ex > r.seqno_ex;
	}

	std::shared_ptr<FilterRTPSorter> FilterRTPSorter::Create(const std::shared_ptr<AbstractSource>& src)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterRTPSorter>());
	}

	FilterRTPSorter::FilterRTPSorter()
	{
		SetName("RTP sorter");
	}

	auto FilterRTPSorter::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (buffer.empty())
			return e_noResult;
		// don't store audio
		if (md.track == stream::Track::audio)
			return e_lastBuffer;

		const auto ssrc = GetRTPSsrc(buffer);
		auto stream_it = std::find_if(m_streams.begin(), m_streams.end(), [&](const stream_info& x) {
			return x.ssrc == ssrc;
		});
		if (stream_it == m_streams.end())
		{
			stream_it = m_streams.emplace(m_streams.begin());
			stream_it->ssrc = ssrc;
			stream_it->newest_timestamp = 0;
			stream_it->last_seqno_ex = 0xffffffff;
			if (auto log = m_log.lock())
			{
				std::ostringstream msg;
				msg << "received SSRC " << std::hex << std::setfill('0') << std::setw(8) << ssrc;
				log->TraceMessage(this, msg.str());
			}

			// During normal operation there should be only a small number of
			// different streams. To prevent uncontolled list growth which may
			// be caused by a malicious source sending RTP packets with random
			// SSRC values we drop data associated with oldest known SSRC.
			if (m_streams.size() > c_max_streams)
			{
				if (auto log = m_log.lock())
				{
					std::ostringstream msg;
					msg << "dropping SSRC " << std::hex << std::setfill('0') << std::setw(8) << m_streams.back().ssrc;
					log->TraceMessage(this, msg.str());
				}
				m_streams.pop_back();
			}
		}
		else if (stream_it != m_streams.begin())
		{
			// Keep most recently used SSRC at the front of the list, so GetNextBuffer()
			// will know which stream received new packet.
			m_streams.splice(m_streams.begin(), m_streams, stream_it);
		}

		auto last_seqno = static_cast<uint16_t>(stream_it->last_seqno_ex);
		const auto seqno = GetRTPSeqNo(buffer);

		uint32_t seqno_ex = (stream_it->last_seqno_ex & 0xffff0000) | seqno;
		if (seqno - last_seqno > c_seqno_threshold)
			seqno_ex -= 0x10000;
		if (seqno - last_seqno < -c_seqno_threshold)
			seqno_ex += 0x10000;

		if (seqno_ex == stream_it->last_seqno_ex + 1)
		{
			// Next packet, use it immediately
			stream_it->last_seqno_ex = seqno_ex;
			return stream_it->rtp_queue.empty() ? e_lastBuffer : e_moreBuffers;
		}
		else
		{
			stream_it->newest_timestamp = GetRTPTimestamp(buffer);
			stream_it->rtp_queue.emplace(seqno_ex, std::move(buffer));
			return GetNextBuffer(buffer, md);
		}
	}

	auto FilterRTPSorter::GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& /*md*/) -> e_processingResult
	{
		// If some stream doesn't have ready packets in its queue then that
		// will remain true until more packets are added to its queue and
		// therefore it will become the first stream in MRU list.
		// Because of that it is enough to check only the first stream for ready packets.
		if (m_streams.empty())
			return e_noResult;
		auto& si = m_streams.front();

		// Drop packets that _likely_ are out-of-order packets that arrived too late (after our queue overflowed)
		while (!si.rtp_queue.empty())
		{
			if (si.rtp_queue.top().seqno_ex <= si.last_seqno_ex && si.last_seqno_ex < 0xffff0000)
			{
				if (auto log = m_log.lock())
				{
					std::ostringstream msg;
					msg << "dropping " << (si.rtp_queue.top().seqno_ex & 0xffff) << " (SSRC " << std::hex << std::setfill('0') << std::setw(8) << si.ssrc << ')';
					log->TraceMessage(this, msg.str());
				}
				si.rtp_queue.pop();
			}
			else
				break;
		}

		if (si.rtp_queue.empty())
			return e_noResult;

		bool return_top_packet = si.rtp_queue.top().seqno_ex == si.last_seqno_ex + 1;
		if (!return_top_packet)
		{
			const bool queue_is_full = GetRTPTimestamp(si.rtp_queue.top().data) == si.newest_timestamp
				? si.rtp_queue.size() > c_max_queue_size_one_frame
				: si.rtp_queue.size() > c_max_queue_size
				;
			if (queue_is_full)
			{
				if (auto log = m_log.lock())
				{
					std::ostringstream msg;
					msg << "queue is full (" << si.rtp_queue.size() << "), skipping " << (si.last_seqno_ex & 0xffff) << " -> " << (si.rtp_queue.top().seqno_ex & 0xffff) << " (SSRC " << std::hex << std::setfill('0') << std::setw(8) << si.ssrc << ')';
					log->TraceMessage(this, msg.str());
				}
				return_top_packet = true;
			}
		}

		if (return_top_packet)
		{
			si.last_seqno_ex = si.rtp_queue.top().seqno_ex;
			buffer = std::move(si.rtp_queue.top().data);

			si.rtp_queue.pop();
			return si.rtp_queue.empty() ? e_lastBuffer : e_moreBuffers;
		}

		return e_noResult;
	}

	bool FilterRTPSorter::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterRTPSorter*>(sink);
		if (!p)
			return false;
		return true;
	}
}