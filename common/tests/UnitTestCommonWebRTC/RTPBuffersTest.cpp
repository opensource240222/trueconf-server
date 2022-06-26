#include "Transcoder/VS_RTP_Buffers.h"
#include "h264_stap-a.h"

#include <gtest/gtest.h>

// ktrushnikov: sniff from sokpb.ru with "Codian MCU 4520" version "4.5(1.77)" over H.323
// He sends H.264 stream with packetization-mode=1 (Non-interleaved mode), so first rtp packet has four NALs: STAP-A SPS PPS IDR(first packet of key_frame).
// This unittest checks correct decode of STAP-A (before we did it incorrectly at VS_RTP_H264InputBuffer::AddRTPData())
TEST(RTPBuffersTest, STAP_A)
{
	unsigned char* out = (unsigned char*)malloc(0x100000);	// 1 Mb
	unsigned long sz = 0;
	unsigned long VideoInterval = 0;
	char IsKey(false);

	VS_RTP_H264InputBuffer h264;
	for (const auto& p : h264_stap_a)
	{
		static int i = 0;
		printf("rtp packet number %d\n", ++i);
		RTPPacket rtp_packet(p.data(), p.size());
		ASSERT_EQ(h264.Add(&rtp_packet), 0);

		while (h264.Get(out, sz, VideoInterval, IsKey) >= 0)
		{
			int width = 0, height = 0;
			h264.GetResolution(out, sz, width, height);
			printf("Got %dx%d resolution from h264 stream\n", width, height);
			ASSERT_EQ(width, 1024);
			ASSERT_EQ(height, 768);
		}
	}
	free(out);
}
