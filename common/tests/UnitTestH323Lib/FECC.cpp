#include "tools/H323Gateway/Lib/h224/VS_H281Frame.h"
#include <gtest/gtest.h>
#include <stdio.h>
#include <string.h>

namespace
{
	class H281FrameTest : public testing::Test
	{
	public:

	};


	unsigned char buffer1[] = {
		0x7e, 0x7e,             // flags
		0x00, 0x8e, 0xc0,       // q922 header
		0x00, 0x00, 0x00, 0x00, // addresses
		0x80, 0x03,             // clientID and control byte
		0x80, 0x0c, 0x00,       // actual h281 information
		0xf0, 0xc3,             // real fcs
		0x7e, 0x7e              // flags
	};

	unsigned char buffer2[] = {
		0x7e, 0x7e, 0x7e,       // flags
		0x00, 0x8e, 0xc0,       // q922 header
		0x00, 0x00, 0x00, 0x00, // addresses
		0x80, 0x03,             // clientID and control byte
		0x80, 0x0c, 0x00,       // actual h281 information
		0xf0, 0xc3,             // real fcs
		0x7e, 0x7e              // flags
	};

	unsigned char without_bitstuffing[] = {
		0x00, 0x71, 0x03,
		0x00, 0x00, 0x00, 0x00,
		0x01, 0xc0,
		0x01, 0x0c, 0x0c
	};

	unsigned char encoded_example[] = {
		0x7e, 0x7e, 0x7e,       // flags
		0x00, 0x8e, 0xc0,       // q922 header
		0x00, 0x00, 0x00, 0x00, // addresses
		0x80, 0x03,             // clientID and control byte
		0x80, 0x0c, 0x00,       // actual h281 information
		0xf0, 0xc3,             // real fcs
		0x7e, 0x7e              // flags
	};

	unsigned char wrong_fcs[] = {
		0x7e, 0x7e, 0x7e,
		0x00, 0x8e, 0xc0,
		0x00, 0x00, 0x00, 0x00,
		0x80, 0x03,
		0x80, 0x0c, 0x00,
		0x55, 0x55, // wrong fcs
		0x7e, 0x7e
	};

	unsigned char wrong_bitorder[] = {
		0x7e, 0x7e, 0x7e,
		0x00, 0x8e, 0xc0,
		0x00, 0xff, 0x00, 0x00, // 8 1's in a row
		0x80, 0x03,
		0x80, 0x0c, 0x00,
		0xf0, 0xc3,
		0x7e, 0x7e
	};

	unsigned char wrong_leading_flag[] = {
		// no flag
		0x00, 0x8e, 0xc0,
		0x00, 0x00, 0x00, 0x00,
		0x80, 0x03,
		0x80, 0x0c, 0x00,
		0xf0, 0xc3,
		0x7e, 0x7e
	};

	unsigned char wrong_trailing_flag[] = {
		0x7e, 0x7e, 0x7e,
		0x00, 0x8e, 0xc0,
		0x00, 0x00, 0x00, 0x00,
		0x80, 0x03,
		0x80, 0x0c, 0x00,
		0xf0, 0xc3,
		// no flag
	};

	void print_buffer(const unsigned char *buffer, unsigned long size) {
		for (unsigned long i = 0; i < (size - 1); i++) {
			printf("%02X:", buffer[i]);
		}
		printf("%02X\n", buffer[size - 1]);
	}

	TEST(H281FrameTest, TwoFlagsBuffer) {
		VS_H281Frame frame;

		ASSERT_EQ(true, frame.Decode(buffer1, sizeof(buffer1)));
		EXPECT_EQ(VS_H281Frame::eRequestType::StartAction, frame.GetRequestType());
		EXPECT_EQ(VS_H281Frame::eZoomDirection::NoZoom, frame.GetZoomDirection());
		EXPECT_EQ(VS_H281Frame::ePanDirection::NoPan, frame.GetPanDirection());
		EXPECT_EQ(VS_H281Frame::eTiltDirection::TiltUp, frame.GetTiltDirection());
		EXPECT_EQ(0x71, frame.GetLowOrderAddressOctet());
		EXPECT_EQ(0x03, frame.GetControlFieldOctet());
		EXPECT_EQ(9, frame.GetInformationFieldSize());

	}

	TEST(H281FrameTest, ThreeFlagsBuffer) {
		VS_H281Frame frame;

		ASSERT_EQ(true, frame.Decode(buffer2, sizeof(buffer2)));
		EXPECT_EQ(VS_H281Frame::eRequestType::StartAction, frame.GetRequestType());
		EXPECT_EQ(VS_H281Frame::ePanDirection::NoPan, frame.GetPanDirection());
		EXPECT_EQ(VS_H281Frame::eTiltDirection::TiltUp, frame.GetTiltDirection());
		EXPECT_EQ(9, frame.GetInformationFieldSize());
	}

	TEST(H281FrameTest, WrongFrames) {
		VS_H281Frame frame;

		ASSERT_EQ(false, frame.Decode(wrong_fcs, sizeof(wrong_fcs)));
		ASSERT_EQ(false, frame.Decode(wrong_bitorder, sizeof(wrong_bitorder)));
		ASSERT_EQ(false, frame.Decode(wrong_leading_flag, sizeof(wrong_leading_flag)));
		ASSERT_EQ(false, frame.Decode(wrong_trailing_flag, sizeof(wrong_trailing_flag)));
	}

	TEST(H281FrameTest, EncodeTest) {
		VS_H281Frame frame;

		// h281
		frame.SetRequestType(VS_H281Frame::eRequestType::StartAction);
		frame.SetTiltDirection(VS_H281Frame::eTiltDirection::TiltUp);
		ASSERT_EQ(9, frame.GetInformationFieldSize());

		// h224
		frame.SetHighPriority(true);
		frame.SetDestinationTerminalAddress(0x0000);
		frame.SetSourceTerminalAddress(0x0000);
		frame.SetClientID(0x01);
		frame.SetBS(true);
		frame.SetES(true);

		// q922
		frame.SetHighOrderAddressOctet(0x00);
		frame.SetLowOrderAddressOctet(0x71);
		frame.SetControlFieldOctet(0x03);

		unsigned long size = 270;
		unsigned char buffer[270];

		ASSERT_EQ(true, frame.Encode(buffer, size));
		ASSERT_EQ(0, memcmp(buffer, encoded_example, size));
	}

	TEST(H281FrameTest, EncodeDecodeEncode) {
		VS_H281Frame frame;

		// h281
		frame.SetRequestType(VS_H281Frame::eRequestType::StartAction);
		frame.SetTiltDirection(VS_H281Frame::eTiltDirection::TiltUp);
		ASSERT_EQ(9, frame.GetInformationFieldSize());

		// h224
		frame.SetHighPriority(true);
		frame.SetDestinationTerminalAddress(0x0000);
		frame.SetSourceTerminalAddress(0x0000);
		frame.SetClientID(0x01);
		frame.SetBS(true);
		frame.SetES(true);

		// q922
		frame.SetHighOrderAddressOctet(0x00);
		frame.SetLowOrderAddressOctet(0x71);
		frame.SetControlFieldOctet(0x03);

		unsigned long size = 270;
		unsigned char buffer[270];

		VS_H281Frame frame2;
		ASSERT_EQ(true, frame.Encode(buffer, size));
		ASSERT_EQ(true, frame2.Decode(buffer, size));
		ASSERT_EQ(true, frame2.Encode(buffer, size));
		ASSERT_EQ(0, memcmp(buffer, encoded_example, size));
	}

	TEST(H281FrameTest, SimpleCodec) {
		VS_H281Frame frame;

		ASSERT_EQ(true, frame.Decode(without_bitstuffing, sizeof(without_bitstuffing), VS_Q922Frame::SimpleCodec));
		EXPECT_EQ(VS_H281Frame::eRequestType::StartAction, frame.GetRequestType());
		EXPECT_EQ(VS_H281Frame::ePanDirection::NoPan, frame.GetPanDirection());
		EXPECT_EQ(VS_H281Frame::eZoomDirection::ZoomIn, frame.GetZoomDirection());
		EXPECT_EQ(9, frame.GetInformationFieldSize());
	}
}
