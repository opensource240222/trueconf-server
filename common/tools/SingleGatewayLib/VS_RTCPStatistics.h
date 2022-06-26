#pragma once

#include "rtcp_utils.h"
#include "../../tools/Server/CommonTypes.h"

#include "modules/rtp_rtcp/source/rtcp_packet/report_block.h"

#include <cstdint>
#include <chrono>

#include "../../std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER


struct VS_RTCPIncomingStatistics
{
	VS_RTCPIncomingStatistics()
		: m_last_seqno(0)
		, m_report_seqno(0)
		, m_seqno_cycles(0)
		, m_packet_count(0)
		, m_total_lost_packets(0)
		, m_last_rtp_receive_time(0)
		, m_last_rtp_timestamp(0)
		, m_last_sr_ts(0)
		, m_jitter_q4(0)
	{}

	void AddPacket(uint16_t seqno, uint32_t rtp_timestamp, std::chrono::steady_clock::time_point current_time_point, uint32_t clock_rate)
	{
		++m_packet_count;
		if (m_report_seqno == 0)
			m_report_seqno = seqno;
		m_last_seqno = seqno;

		if (rtp_timestamp != m_last_rtp_timestamp
			&& current_time_point > m_last_rtp_receive_time_point)
		{
			UpdateJitter(rtp_timestamp, current_time_point, clock_rate);
			m_last_rtp_receive_time_point = current_time_point;
			m_last_rtp_timestamp = rtp_timestamp;
		m_clock_rate = clock_rate;
		}

	}

	unsigned PacketCount() const
	{
		return m_packet_count;
	}

	unsigned ExpectedPacketCount() const
	{
		return m_last_seqno > m_report_seqno
			? m_last_seqno - m_report_seqno
			: 0x10000 + m_last_seqno - m_report_seqno;
	}

	unsigned LostPackets() const
	{
		const auto expected = ExpectedPacketCount();
		return expected > m_packet_count ? expected - m_packet_count : 0;
	}

	unsigned DuplicatePackets() const
	{
		const auto expected = ExpectedPacketCount();
		return expected < m_packet_count ? m_packet_count - expected : 0;
	}

	unsigned TotalLostPackets() const
	{
		return m_total_lost_packets;
	}

	webrtc::rtcp::ReportBlock GetReportBlock(uint32_t ssrc)
	{
		using duration_dlsr = std::chrono::duration<uint32_t, std::ratio<1, 0x10000>>;
		const auto expected = ExpectedPacketCount();
		const auto lost_fraction = m_packet_count < expected
			? (expected - m_packet_count) * 255 / expected // division by 0 isn't possible here because unsigned (m_packet_count) can't be less that zero (expected).
			: 0;
		m_total_lost_packets += LostPackets();
		const auto extended_seqno = (static_cast<uint32_t>(m_seqno_cycles) << 16) | m_last_seqno;

		webrtc::rtcp::ReportBlock report;
		report.SetMediaSsrc(ssrc);
		report.SetFractionLost(lost_fraction);
		report.SetCumulativeLost(m_total_lost_packets);
		report.SetExtHighestSeqNum(extended_seqno);
		report.SetJitter(m_jitter_q4 / 16);
		report.SetLastSr(m_last_sr_ts);

		const auto tp_now = std::chrono::steady_clock::now();

		report.SetDelayLastSr((m_last_rtcp_sr_receive_time_point.time_since_epoch().count() == 0) ? 0 :
			 (std::chrono::duration_cast<duration_dlsr>(tp_now - m_last_rtcp_sr_receive_time_point)).count());

		m_report_seqno = m_last_seqno;
		m_packet_count = 0;
		return report;
	}

	uint32_t EstimatedRTPTimestamp(std::chrono::steady_clock::time_point current_time_point, unsigned rtp_rate) const
	{
		const uint32_t diff = std::chrono::duration_cast<std::chrono::microseconds>(current_time_point - m_last_rtp_receive_time_point).count() * rtp_rate / 1000000;
		return m_last_rtp_receive_time + diff;
	}

	void SetLastSrTS(uint32_t last_sr_ts)
	{
		m_last_sr_ts = last_sr_ts;
	}

	void SetLastSrReceive(std::chrono::steady_clock::time_point&& _last_rtcp_sr_receive_time_point)
	{
		m_last_rtcp_sr_receive_time_point = std::move(_last_rtcp_sr_receive_time_point);
	}

	void UpdateJitter(uint32_t rtp_timestamp, std::chrono::steady_clock::time_point current_time_point, uint32_t clock_rate)
	{
		uint32_t receive_time_rtp =
			EstimatedRTPTimestamp(current_time_point, clock_rate);

		int32_t time_diff_samples = 0;
		if ((receive_time_rtp != 0) && (m_last_rtp_receive_time != 0) && (receive_time_rtp >= m_last_rtp_receive_time)
			&& (rtp_timestamp != 0) && (m_last_rtp_timestamp != 0))
		{
			time_diff_samples = (receive_time_rtp - m_last_rtp_receive_time) - (rtp_timestamp - m_last_rtp_timestamp);
		}

		time_diff_samples = std::abs(time_diff_samples);

		// lib_jingle sometimes deliver crazy jumps in TS for the same stream.
		// If this happens, don't update jitter value. Use 5 secs video frequency
		// as the threshold.
		if (time_diff_samples < 450000)
		{
			// Note we calculate in Q4 to avoid using float.
			int32_t jitter_diff_q4 = (time_diff_samples << 4) - m_jitter_q4;
			m_jitter_q4 += ((jitter_diff_q4 + 8) >> 4);
		}
		else
		{
			dstream4 << "VS_RTCPIncomingStatistics::UpdateJitter(): time_diff_samples > " << 450000 <<
				", receive_time_rtp = " << receive_time_rtp <<
				", m_last_rtp_receive_time = " << m_last_rtp_receive_time <<
				", rtp_timestamp = " << rtp_timestamp <<
				", m_last_rtp_timestamp = " << m_last_rtp_timestamp;
		}
		m_last_rtp_receive_time = receive_time_rtp;
	}

	bool IsEmpty()
	{
		return m_packet_count == 0;
	}
	uint32_t GetClockRate() const
	{
		return m_clock_rate;
	}

private:
	uint16_t m_last_seqno; // Last received sequence number
	uint16_t m_report_seqno; // Sequence number included in the last report
	uint16_t m_seqno_cycles; // Number of timer sequence numbers did wraparound
	unsigned m_packet_count;
	unsigned m_total_lost_packets;
	uint32_t m_last_rtp_receive_time;
	std::chrono::steady_clock::time_point m_last_rtp_receive_time_point;
	uint32_t m_last_rtp_timestamp;
	uint32_t m_last_sr_ts;
	uint32_t m_jitter_q4;
	std::chrono::steady_clock::time_point m_last_rtcp_sr_receive_time_point;
	uint32_t m_clock_rate;//need for statistic logs

};

struct VS_RTCPOutgoingStatistics
{
	VS_RTCPOutgoingStatistics()
		: m_packet_count(0)
		, m_octet_count(0)
		, m_last_rtp_timestamp(0)
		, m_last_ntp_time(0)
	{}

	void AddPacket(size_t size, uint32_t rtp_timestamp, uint32_t clock_rate)
	{
		++m_packet_count;
		m_octet_count = static_cast<uint32_t>(m_octet_count + size); // RFC 3550 doesn't say anything about overflow behaviour
		m_last_rtp_timestamp = rtp_timestamp;
		m_last_ntp_time = rtcp_utils::ntp_time();
		m_clock_rate = clock_rate;
	}

	uint32_t GetPacketCount() const
	{
		return m_packet_count;
	}

	uint32_t GetOctetCount() const
	{
		return m_octet_count;
	}

	uint32_t EstimatedRTPTimestamp(uint64_t ntp_now, unsigned rtp_rate) const
	{
		if (m_last_ntp_time == 0)
			return 0;
		return m_last_rtp_timestamp
			+ static_cast<uint32_t>(((ntp_now - m_last_ntp_time) * rtp_rate) >> 32);
	}

	uint32_t GetClockRate() const
	{
		return m_clock_rate;
	}

private:
	uint32_t m_packet_count;
	uint32_t m_octet_count;
	uint32_t m_last_rtp_timestamp;
	uint64_t m_last_ntp_time;
	uint32_t m_clock_rate;//need for statistic logs
};

#undef DEBUG_CURRENT_MODULE
