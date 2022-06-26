#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tools/SingleGatewayLib/rtcp_utils.h"

TEST(RTCPUtilsTest, TestVSR) {

	const unsigned char in_buf[] = {
		0x8f, 0xce, 0x00, 0x18, 0x08, 0x01, 0x99, 0x80, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x01, 0x00, 0x58,
		0xff, 0xff, 0xff, 0xfe, 0x00, 0x02, 0x00, 0x02, 0x00, 0x01, 0x01, 0x44, 0x00, 0x00, 0x00, 0x00,
		0x7a, 0x01, 0x00, 0x01, 0x01, 0xb0, 0x01, 0xb0, 0x00, 0x05, 0x57, 0x30, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x27, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00,
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x95, 0x00 };

	rtcp_utils::FeedbackMsgHeader fm_hdr;

	EXPECT_TRUE(ParseFeedbackMsgHeader(in_buf, sizeof(in_buf), fm_hdr));
	EXPECT_EQ(fm_hdr.FMT, 15);
	EXPECT_EQ(fm_hdr.PT, rtcp_utils::PSFB);
	EXPECT_EQ(fm_hdr.length, 24); // 100 bytes
	EXPECT_EQ(fm_hdr.ssrc_packet_sender, 134322560);
	EXPECT_EQ(fm_hdr.ssrc_media_source, rtcp_utils::SourceAny);

	rtcp_utils::VSRHeader vsr_hdr;

	EXPECT_TRUE(ParseVSRHeader(in_buf + sizeof(rtcp_utils::FeedbackMsgHeader), sizeof(in_buf) - sizeof(rtcp_utils::FeedbackMsgHeader), vsr_hdr));
	EXPECT_EQ(vsr_hdr.afb_type, 0x01);
	EXPECT_EQ(vsr_hdr.length, 88);
	EXPECT_EQ(vsr_hdr.msi, rtcp_utils::SourceAny);
	EXPECT_EQ(vsr_hdr.request_id, 2);
	EXPECT_EQ(vsr_hdr.version, 0);

	rtcp_utils::VSREntry vsr_ent;

	EXPECT_TRUE(ParseVSREntry(in_buf + sizeof(rtcp_utils::FeedbackMsgHeader) + sizeof(rtcp_utils::VSRHeader), sizeof(in_buf) - sizeof(rtcp_utils::FeedbackMsgHeader) - sizeof(rtcp_utils::VSRHeader), vsr_ent));
	EXPECT_EQ(vsr_ent.pt, 122);
	EXPECT_EQ(vsr_ent.uc_config_mode, rtcp_utils::UCConfigMode1);
	EXPECT_EQ(vsr_ent.flags, 0);
	EXPECT_EQ(vsr_ent.aspect, rtcp_utils::_4x3);
	EXPECT_EQ(vsr_ent.max_w, 432);
	EXPECT_EQ(vsr_ent.max_h, 432);
	EXPECT_EQ(vsr_ent.min_bitrate, 350000);
	EXPECT_EQ(vsr_ent.bitrate_per_level, 10000);
	EXPECT_EQ(vsr_ent.bitrate_histogram[0], 1);
	for (int i = 1; i < 10; i++) {
		EXPECT_EQ(vsr_ent.bitrate_histogram[i], 0);
	}
	EXPECT_EQ(vsr_ent.framerate_bitmask, rtcp_utils::_15);
	EXPECT_EQ(vsr_ent.num_must_instances, 1);
	EXPECT_EQ(vsr_ent.num_may_instances, 0);
	EXPECT_EQ(vsr_ent.quality_report_histogram[0], 1);
	for (int i = 1; i < 8; i++) {
		EXPECT_EQ(vsr_ent.quality_report_histogram[i], 0);
	}
	EXPECT_EQ(vsr_ent.max_num_pixels, 103680);

	unsigned char out_buf[256];

	size_t off = 0;
	off += WriteFeedbackMsgHeader(fm_hdr, out_buf + off, sizeof(out_buf) - off);
	off += WriteVSRHeader(vsr_hdr, out_buf + off, sizeof(out_buf) - off);
	off += WriteVSREntry(vsr_ent, out_buf + off, sizeof(out_buf) - off);

	EXPECT_EQ(off, sizeof(in_buf));
	EXPECT_EQ(memcmp(out_buf, in_buf, off), 0);

}


TEST(RTCPUtilsTest, TestSenderReport) {
	{
		const unsigned char in_buf[] = {
			0x80, 0xc8, 0x00, 0x06, 0x01, 0x3d, 0xc1, 0xf3,
			0x00, 0x92, 0x56, 0xa0, 0xf9, 0x42, 0x40, 0x47,
			0xdb, 0x82, 0xc5, 0x3f, 0x8f, 0xc3, 0x25, 0x0f, 0x7c, 0x70, 0x32, 0xef };

		rtcp_utils::SenderReportHeader sr_hdr;
		EXPECT_TRUE(ParseSenderReportHeader(in_buf, sizeof(in_buf), sr_hdr));
		EXPECT_EQ(sr_hdr.ssrc, 20824563);

		unsigned char out_buf[128];
		size_t off = 0;
		off += WriteSenderReportHeader(sr_hdr, out_buf, sizeof(out_buf));

		EXPECT_EQ(off, sizeof(in_buf));
		EXPECT_EQ(memcmp(in_buf, out_buf, off), 0);
	}

	{
		const unsigned char in_buf[] = {
			0x80, 0xc8, 0x00, 0x0b, 0xf3, 0xcc, 0xa1, 0x9a, 0xdc, 0x03, 0x86, 0x07, 0x9a, 0x70, 0x08, 0x00,
			0x0b, 0xf9, 0x7a, 0x87, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x29, 0x29, 0x00, 0x0c, 0x00, 0x14,
			0xf3, 0xcc, 0xa1, 0x9a, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
		};

		rtcp_utils::SenderReportHeader sr_hdr;
		EXPECT_TRUE(ParseSenderReportHeader(in_buf, sizeof(in_buf), sr_hdr));
		EXPECT_EQ(sr_hdr.length, 11);
		EXPECT_EQ(sr_hdr.ssrc, 4090274202);
		EXPECT_EQ(sr_hdr.timestamp_msw, 3691218439);
		EXPECT_EQ(sr_hdr.timestamp_lsw, 2591033344);
		EXPECT_EQ(sr_hdr.packet_count, 22);
		EXPECT_EQ(sr_hdr.octet_count, 10537);

		rtcp_utils::PeerInfoExchange info;
		EXPECT_TRUE(ParsePeerInfoExchange(in_buf + sizeof(rtcp_utils::SenderReportHeader), sizeof(in_buf) - sizeof(rtcp_utils::SenderReportHeader), info));
		EXPECT_EQ(info.type, 12);
		EXPECT_EQ(info.length, 20);
		EXPECT_EQ(info.ssrc, 4090274202);
		EXPECT_EQ(info.inbound_link_bandwidth, 2147483647);
		EXPECT_EQ(info.outbound_link_bandwidth, 2147483647);
		EXPECT_EQ(info.NC, 0);

		unsigned char out_buf[256];
		size_t off = 0;

		off += WriteSenderReportHeader(sr_hdr, out_buf + off, sizeof(out_buf) - off);
		off += WritePeerInfoExchange(info, out_buf + off, sizeof(out_buf) - off);

		EXPECT_EQ(off, sizeof(in_buf));
		EXPECT_EQ(memcmp(in_buf, out_buf, off), 0);
	}
}
