#if defined(_WIN32) // Not ported yet

#include "H323ParserTestBase.h"

#include <gtest/gtest.h>

#include <string>
#include <cstdint>

#include "../../FakeClient/VS_ConferenceInfo.h"

#define BUFSZ (256 * 1024)

/* https://bug-tracking.trueconf.com/show_bug.cgi?id=29059 */
namespace sony_test {

#include "h225sony.h"
#include "h245sony.h"

	class  Sony_PCS_XG80_Test : public H323ParserTestBase
	{
	public:
		Sony_PCS_XG80_Test()
			: H323ParserTestBase(net::address_v4::from_string("153.0.0.193"), 53317, net::address_v4::from_string("153.0.0.192"), 2263)
		{
		}

		const char *to = "#h323:153.0.0.192";
		const char *from = "test@videoserver.elara.ru";
	};

	TEST_F(Sony_PCS_XG80_Test, DISABLED_WrongCodecAndPayloadTest)
	{
		using ::testing::_;
		using ::testing::AtLeast;

		VS_GatewayAudioMode audio_mode;
		const char new_dialog_id[] = "FD589D29B12904FC21719565BE9E6F09";
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
		ASSERT_TRUE(h323->InviteMethod(dialog, from, to, VS_ConferenceInfo(false,false), "dn")) << "InviteMethod() failed" << std::endl;
		ASSERT_GT(h323->GetBufForSend(tmpbuf.data(), sz, e_H225, {}, 0, {}, 0), 0);
		// receive Alerting
		ASSERT_GT(h323->SetRecvBuf(h225raw::peer1_0, sizeof(h225raw::peer1_0), e_H225, terminal_addr.addr, terminal_addr.port, {}, 0), 0);
		sz = tmpbuf.size();
		ASSERT_EQ(h323->GetBufForSend(tmpbuf.data(), sz, e_H225, {}, 0, {}, 0), 0);
		// receive Connect
		ASSERT_GT(h323->SetRecvBuf(h225raw::peer1_1, sizeof(h225raw::peer1_1), e_H225, terminal_addr.addr, terminal_addr.port, {}, 0), 0);
		sz = tmpbuf.size();
		ASSERT_EQ(h323->GetBufForSend(tmpbuf.data(), sz, e_H225, {}, 0, {}, 0), 0);

		// !!! H245 session !!!

		// receive TerminalCapabilitySet
		h245_channel_fake->CallDataReceived(h245raw::peer0_0, sizeof(h245raw::peer0_0));

		// check codec
		ASSERT_TRUE(h323->GetAudioMode(dialog, audio_mode));
		ASSERT_TRUE(audio_mode.CodecType == e_rcvG722_64k && audio_mode.PayloadType == SDP_PT_G722_64k)
			<< "Unexpected audio codec: codec - " << audio_mode.CodecType << ", payload type - " << audio_mode.PayloadType << std::endl;

		// ... //

		// send releaseComplete (H225)
		h323->Shutdown();
		sz = tmpbuf.size();
		ASSERT_GT(h323->GetBufForSend(tmpbuf.data(), sz, e_H225, {}, 0, {}, 0), 0);
	}
};

#endif
