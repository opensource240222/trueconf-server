#include <gtest/gtest.h>

#include "../Transcoder/RTPPacket.h"
#include "../Transcoder/VS_RTP_Buffers.h"

/*
Artem Boldarev:

This unit test checks parsing of the empty (payload free, header only) RTP packets.
*/

#include "rtp_nopayload_data.h"

struct RTP_RawPacketData
{
	uint8_t *data;
	size_t len;
};

static RTP_RawPacketData packets[] = {
	{(uint8_t *)packet_3424, sizeof(packet_3424) },
    {(uint8_t *)packet_3452, sizeof(packet_3452) },
	{(uint8_t *)packet_3475, sizeof(packet_3475) }, // this is empty RTP packet
	{(uint8_t *)packet_3503, sizeof(packet_3503) },
	{(uint8_t *)packet_3507, sizeof(packet_3507) }
};

TEST(RTPBuffersTest, NoPayload)
{
	VS_RTP_InputBufferVideo inbuf;

	for (const auto& packet_data : packets)
	{
		RTPPacket p(packet_data.data, packet_data.len);
		ASSERT_NE(inbuf.Add(&p), -1) << "Can't handle RTP packet.";
	}
}
