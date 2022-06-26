#if defined(_WIN32) // Not ported yet

#include "H323ParserTestBase.h"

#include <gtest/gtest.h>

#include <string>
#include <iostream>
#include <cstdint>

#include "../../FakeClient/VS_ConferenceInfo.h"

#define BUFSZ (256 * 1024)

/* https://bug-tracking.trueconf.com/show_bug.cgi?id=29042 */
namespace tandberg_test {

	#include "h225raw.h"
	#include "h245raw.h"

	class  TandbergTest : public H323ParserTestBase
	{
	public:
		TandbergTest()
			: H323ParserTestBase(net::address_v4::from_string("192.115.76.47"), 55992, net::address_v4::from_string("193.238.190.12"), 1720)
		{
		}

		const char *to = "#h323:193.238.190.12";
		const char *from = "info-ns@en84m.trueconf.name";
	};

	TEST_F(TandbergTest, DISABLED_WrongCodecAndPayloadTest)
	{
		using ::testing::_;
		using ::testing::AtLeast;

		VS_GatewayAudioMode audio_mode;
		const char new_dialog_id[] = "FA1F7102E66A1251D4EA3F8F0555305A";
		std::vector<uint8_t> tmpbuf(BUFSZ);
		std::size_t sz = tmpbuf.size();

		std::string dialog = h323->SetNewDialogTest(string_view{ new_dialog_id, sizeof(new_dialog_id) - 1 }, to, {}, VS_CallConfig{});
		ASSERT_FALSE(dialog.empty()) << "Can\'t create new dialog!" << std::endl;

		h323->SetPeerCSAddress(dialog, terminal_addr);

		h245_channel_fake->remote_addr = terminal_addr.addr;
		h245_channel_fake->remote_port = 12345;
		EXPECT_CALL(*h245_channel, Open(_, _, _, _, _, _)).Times(AtLeast(1));
		EXPECT_CALL(*h245_channel, Send_mocked(_)).Times(AtLeast(1));

		// !!! H225 session setup !!!
		// send Setup
		ASSERT_TRUE(h323->InviteMethod(dialog, from, to, VS_ConferenceInfo(false, false), "dn")) << "InviteMethod() failed" << std::endl;
		ASSERT_GT(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H225, {}, 0, {}, 0), 0);
		// receive Alerting
		ASSERT_GT(h323->SetRecvBuf(h225raw::peer1_0, sizeof(h225raw::peer1_0), e_H225, terminal_addr.addr, terminal_addr.port, {}, 0), 0);
		sz = tmpbuf.size();
		ASSERT_EQ(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H225, {}, 0, {}, 0), 0);
		// receive Connect
		ASSERT_GT(h323->SetRecvBuf(h225raw::peer1_1, sizeof(h225raw::peer1_1), e_H225, terminal_addr.addr, terminal_addr.port, {}, 0), 0);
		sz = tmpbuf.size();
		ASSERT_EQ(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H225, {}, 0, {}, 0), 0);

		// !!! H245 session !!!

		// receive TerminalCapabilitySet
		h245_channel_fake->CallDataReceived(h245raw::peer1_0, sizeof(h245raw::peer1_0));
		// send TerminalCapabilitySetAck and server TerminalCapabilitySet
		ASSERT_LE(1u, h245_channel_fake->out_queue.size());
		{
			auto& h245_data = h245_channel_fake->out_queue[0];
			EXPECT_LT(4u, h245_data.size());
			VS_PerBuffer buffer(h245_data.data<const unsigned char>() + 4, (h245_data.size() - 4) * 8);
			VS_H245MultimediaSystemControlMessage msg;
			ASSERT_TRUE(msg.Decode(buffer));
			ASSERT_EQ(VS_H245MultimediaSystemControlMessage::e_response, msg.tag);
			auto resp = static_cast<VS_H245ResponseMessage*>(msg.choice);
			ASSERT_EQ(VS_H245ResponseMessage::e_terminalCapabilitySetAck, resp->tag);
		}

		// check codec
		ASSERT_TRUE(h323->GetAudioMode(dialog, audio_mode));
		ASSERT_TRUE(audio_mode.CodecType == e_rcvG722132 && audio_mode.PayloadType == SDP_PT_DYNAMIC_G722132)
			<< "Unexpected audio codec: codec - " << audio_mode.CodecType << ", payload type - " << audio_mode.PayloadType << std::endl;

		// We actually do not need this...
		/*// receive TerminalCapabilitySetAck
		ASSERT_GT(h323->SetRecvBuf(h245raw::peer1_1, sizeof(h245raw::peer1_1), e_H245, terminal_addr), 0);

		// receive masterSlaveDetermination
		ASSERT_GT(h323->SetRecvBuf(h245raw::peer1_1, sizeof(h245raw::peer1_1), e_H245, terminal_addr), 0);
		// send masterSlaveDeterminationAck
		sz = tmpbuf.size();
		ASSERT_GT(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H245), 0);

		// receive roundTripDelayRequest
		ASSERT_GT(h323->SetRecvBuf(h245raw::peer1_2, sizeof(h245raw::peer1_2), e_H245, terminal_addr), 0);
		// send roundTripDelayResponse
		sz = tmpbuf.size();
		ASSERT_GT(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H245), 0);

		// receive masterSlaveDetermination
		ASSERT_GT(h323->SetRecvBuf(h245raw::peer1_3, sizeof(h245raw::peer1_3), e_H245, terminal_addr), 0);
		// receive userInput
		ASSERT_GT(h323->SetRecvBuf(h245raw::peer1_4, sizeof(h245raw::peer1_4), e_H245, terminal_addr), 0);
		// receive OpenLogicalChannel (g722-64k)
		ASSERT_GT(h323->SetRecvBuf(h245raw::peer1_5, sizeof(h245raw::peer1_5), e_H245, terminal_addr), 0);
		// send OpenLogicalChannelAck, OpenLogicalChannel(generic - audio), OpenLogicalChannel(GenericVideoCapability)
		sz = tmpbuf.size();
		ASSERT_GT(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H245), 0);

		// receive OpenLogicalChannel(GenericVideoCapability)
		ASSERT_GT(h323->SetRecvBuf(h245raw::peer1_6, sizeof(h245raw::peer1_6), e_H245, terminal_addr), 0);

		// send OpenLogicalChannelAck
		sz = tmpbuf.size();
		ASSERT_GT(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H245), 0);*/

		// ... //

		// send releaseComplete (H225)
		h323->Shutdown();
		sz = tmpbuf.size();
		ASSERT_GT(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H225, {}, 0, {}, 0), 0);
	}

	// Bug with Tandberg 770 mxp: wrong payload type in RTP (ticket 2260)
	namespace t2260 {
#include "h225_data2260.h"
#include "h245_data2260.h"
	}

	class  Tandberg770_MXP_Test : public H323ParserTestBase
	{
	public:
		Tandberg770_MXP_Test()
			: H323ParserTestBase(net::address_v4::from_string("10.144.40.70"), 54822, net::address_v4::from_string("10.144.53.10"), 1720)
		{
		}

		const char *to = "#h323:10.144.53.10";
		const char *from = "test@s1600-vks01.rf.rshbank.ru";
	};

	TEST_F(Tandberg770_MXP_Test, DISABLED_WrongPayload)
	{
		using ::testing::_;
		using ::testing::AtLeast;

		VS_GatewayAudioMode audio_mode;
		const char new_dialog_id[] = "D3C71FEFD78190208E39D0EBB0A20BB6";
		std::vector<uint8_t> tmpbuf(BUFSZ);
		std::size_t sz = tmpbuf.size();

		auto resetbuf = [&]() -> void {
			sz = tmpbuf.size();
			memset(&tmpbuf[0], 0, tmpbuf.size());
		};

		std::string dialog = h323->SetNewDialogTest(string_view{ new_dialog_id, sizeof(new_dialog_id) - 1 }, to, {}, VS_CallConfig{});
		ASSERT_FALSE(dialog.empty()) << "Can\'t create new dialog!" << std::endl;

		h323->SetPeerCSAddress(dialog, terminal_addr);

		h245_channel_fake->remote_addr = terminal_addr.addr;
		h245_channel_fake->remote_port = 12345;
		EXPECT_CALL(*h245_channel, Open(_, _, _, _, _, _)).Times(AtLeast(1));
		EXPECT_CALL(*h245_channel, Send_mocked(_)).Times(AtLeast(1));

		/*!!! H225 session !!!*/
		ASSERT_TRUE(h323->InviteMethod(dialog, from, to, VS_ConferenceInfo(false, false), "dn")) << "InviteMethod() failed" << std::endl;

		ASSERT_GT(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H225, terminal_addr.addr, terminal_addr.port, vcs_addr.addr, vcs_addr.port), 0);
		// receive Alerting
		ASSERT_GT(h323->SetRecvBuf(t2260::h225::peer1_0, sizeof(t2260::h225::peer1_0), e_H225, terminal_addr.addr, terminal_addr.port, vcs_addr.addr, vcs_addr.port), 0);
		resetbuf();
		ASSERT_EQ(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H225, terminal_addr.addr, vcs_addr.port, vcs_addr.addr, vcs_addr.port), 0);
		// receive Connect
		ASSERT_GT(h323->SetRecvBuf(t2260::h225::peer1_1, sizeof(t2260::h225::peer1_1), e_H225, terminal_addr.addr, terminal_addr.port, vcs_addr.addr, vcs_addr.port), 0);
		resetbuf();
		ASSERT_EQ(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H225, terminal_addr.addr, terminal_addr.port, vcs_addr.addr, vcs_addr.port), 0);
		resetbuf();

		/*!!! H235 session !!!*/
		// receive TerminalCapabilitySet
		h245_channel_fake->CallDataReceived(t2260::h245::peer1_0, sizeof(t2260::h245::peer1_0));
		h245_channel_fake->CallDataReceived(t2260::h245::peer1_1, sizeof(t2260::h245::peer1_1));

		// check codec
		ASSERT_TRUE(h323->GetAudioMode(dialog, audio_mode));
		ASSERT_TRUE(audio_mode.CodecType == e_rcvG722124) << "Wrong codec ID!" << std::endl;
		ASSERT_TRUE(audio_mode.PayloadType == SDP_PT_DYNAMIC_G722124) << "Wrong payload type!" << std::endl;

		// receive TCS Ack
		h245_channel_fake->CallDataReceived(t2260::h245::peer1_2, sizeof(t2260::h245::peer1_2));

		// receive masterSlaveDetermination terminalCapabilitySetAck
		h245_channel_fake->CallDataReceived(t2260::h245::peer1_3, sizeof(t2260::h245::peer1_3));

		// receive masterSlaveDeterminationAck
		h245_channel_fake->CallDataReceived(t2260::h245::peer1_4, sizeof(t2260::h245::peer1_4));

		// receive userInput roundTripDelayRequest
		h245_channel_fake->CallDataReceived(t2260::h245::peer1_5, sizeof(t2260::h245::peer1_5));

		// receive OLC
		h245_channel_fake->CallDataReceived(t2260::h245::peer1_6, sizeof(t2260::h245::peer1_6));

		// Generate and send OLC
		h323->Timeout();

		// receive miscellaneousIndication
		h245_channel_fake->CallDataReceived(t2260::h245::peer1_7, sizeof(t2260::h245::peer1_7));

		// receive openLogicalChannel (genericVideoCapability) openLogicalChannelAck openLogicalChannelAck flowControlCommand
		h245_channel_fake->CallDataReceived(t2260::h245::peer1_8, sizeof(t2260::h245::peer1_8));

		// It seems it is enough.
		/* ... */

		// Check codec
		{
			auto base_ctx = VS_ParserInterface::GetParserContextByDialogID(h323, string_view{ new_dialog_id, sizeof(new_dialog_id) - 1 });
			ASSERT_NE(base_ctx, nullptr) << "Can't get parser context!" << std::endl;
			auto ctx = dynamic_cast<VS_H323ParserInfo *>(base_ctx.get());
			ASSERT_NE(ctx, nullptr) << "Can't get parser context!" << std::endl;

			ASSERT_EQ(ctx->GetACDefault(), ctx->GetAudioIndex(audio_mode.CodecType))
				<< "Codec index and codec ID are not synchronised!" << std::endl;
		}

		// send releaseComplete (H225)
		h323->Shutdown();
		resetbuf();
		//ASSERT_GT(h323->GetBufForSend((void *)&tmpbuf[0], sz, e_H225, terminal_addr, vcs_addr), 0);
	}

};

#endif
