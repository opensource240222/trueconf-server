#if defined(_WIN32) && !defined(_TRY_PORTED_)

#include "H323ParserTestBase.h"
#include "../../tools/H323Gateway/Lib/src/VS_Q931.h"
#include "../../FakeClient/VS_ConferenceInfo.h"

#include <gtest/gtest.h>

namespace
{
#include "rawh323.h"

	class H323GnuGkIPv6Test : public H323ParserTestBase
	{
	public:
		H323GnuGkIPv6Test()
			: H323ParserTestBase(net::address_v6::from_string("[fd24:800c:c8d7:881f::1]"), 61004,
				net::address_v6::from_string("[fd24:800c:c8d7:881f::2]"), 1720)
		{
		}



		const char *from = "b@brchk000.trueconf.ua";
		const char *to = "#h323:\\e\\777@[fd24:800c:c8d7:881f::2]";
		const char *dialogId = "23A2554344C956F778AE4C2C7FD4CF6A";

		VS_ConferenceProtocolMock confProtocol;
		VS_ConferenceProtocolFake confProtocol_fake;
	};

	TEST_F(H323GnuGkIPv6Test, DISABLED_Outgoing)
	{
		using ::testing::_;
		using ::testing::AtLeast;

		std::vector<uint8_t> buf(0xffff);
		std::size_t sz = buf.size();

		std::string dialog = h323->SetNewDialogTest(dialogId, to, {}, VS_CallConfig{});
		ASSERT_TRUE(!dialog.empty());

		h323->SetPeerCSAddress(dialog, terminal_addr);
		h323->SetMyMediaAddress(net::address::from_string("fd24:800c:c8d7:881f::1"));

		h245_channel_fake->remote_addr = terminal_addr.addr;
		h245_channel_fake->remote_port = 12345;
		EXPECT_CALL(*h245_channel, Open(_, _, _, _, _, _)).Times(AtLeast(1));
		EXPECT_CALL(*h245_channel, Send_mocked(_)).Times(AtLeast(1));

		ASSERT_TRUE(h323->InviteMethod(dialog, from, to, VS_ConferenceInfo(false,false), "dn"));
		ASSERT_GT(h323->GetBufForSend(&buf[0], sz, e_H225, {}, 0, {}, 0), 0);

		{
			VS_PerBuffer in_per_buff(&buf[4], (sz - 4) * 8); // skip 4 bytes of header
			VS_Q931 theQ931;
			ASSERT_TRUE(theQ931.DecodeMHeader(in_per_buff));
			ASSERT_EQ(theQ931.messageType, VS_Q931::e_setupMsg);

			unsigned char dn[82 + 1] = { 0 };
			unsigned char e164[50] = { 0 };
			ASSERT_TRUE(VS_Q931::GetUserUserIE(in_per_buff, dn, e164));
			VS_CsH323UserInformation ui;
			ASSERT_TRUE(ui.Decode(in_per_buff));
			VS_CsSetupUuie* setup = ui.h323UuPdu.h323MessageBody;
			ASSERT_EQ(setup->destCallSignalAddress.tag, VS_H225TransportAddress::e_ip6Address);
			net::address addr;
			net::port port;
			ASSERT_TRUE(get_ip_address(setup->destCallSignalAddress, addr, port));
			ASSERT_TRUE(addr.is_v6());
			auto &&addr_v4_bytes = addr.to_v6().to_bytes();
			ASSERT_EQ(addr_v4_bytes.front(), 0xFD);
			ASSERT_EQ(addr_v4_bytes.back(), 0x02);
		}

		ASSERT_GT(h323->SetRecvBuf(raw_gnugk_alerting, sizeof(raw_gnugk_alerting), e_H225, terminal_addr.addr, terminal_addr.port, {}, 0), 0);
		sz = buf.size();
		ASSERT_EQ(h323->GetBufForSend(&buf[0], sz, e_H225, {}, 0, {}, 0), 0);

		ASSERT_GT(h323->SetRecvBuf(raw_gnugk_connect, sizeof(raw_gnugk_connect), e_H225, terminal_addr.addr,
			terminal_addr.port, net::address{}, 0), 0);
		sz = buf.size();
		ASSERT_EQ(h323->GetBufForSend(&buf[0], sz, e_H225, {}, 0, {}, 0), 0);

		ASSERT_TRUE(h245_channel_fake->connected);
		h245_channel_fake->CallDataReceived(raw_gnugk_olc, sizeof(raw_gnugk_olc));

		ASSERT_LE(1u, h245_channel_fake->out_queue.size());
		{
			auto& h245_data = h245_channel_fake->out_queue[0];
			EXPECT_LT(4u, h245_data.size());
			VS_PerBuffer buffer(h245_data.data<const unsigned char>() + 4, (h245_data.size() - 4) * 8);
			VS_H245MultimediaSystemControlMessage msg;
			ASSERT_TRUE(msg.Decode(buffer));
			ASSERT_EQ(msg.tag, VS_H245MultimediaSystemControlMessage::e_response);
			auto rsp = dynamic_cast<VS_H245ResponseMessage*>(msg.choice);
			ASSERT_EQ(rsp->tag, VS_H245ResponseMessage::e_openLogicalChannelAck);
			VS_H245OpenLogicalChannelAck *ack = static_cast<VS_H245OpenLogicalChannelAck*>(rsp->choice);
			net::address addr1, addr2;
			net::port port1, port2;
			static_cast<VS_GwH245OpenLogicalChannelAck *>(ack)->GetRtpRtcpIpPort(addr1, port1, addr2, port2);
			ASSERT_TRUE(addr1.is_v6());
			ASSERT_TRUE(addr2.is_v6());
			auto ipv6addr = addr1.to_v6().to_bytes();
			ASSERT_EQ(ipv6addr.front(), 0xFD);
			ASSERT_EQ(ipv6addr.back(), 0x01);
			ipv6addr = addr2.to_v6().to_bytes();
			ASSERT_EQ(ipv6addr.front(), 0xFD);
			ASSERT_EQ(ipv6addr.back(), 0x01);
		}
	}

	TEST_F(H323GnuGkIPv6Test, Incoming)
	{
		using ::testing::_;
		using ::testing::DoAll;
		using ::testing::Return;
		using ::testing::WithArg;
		using ::testing::AtLeast;
		using ::testing::Invoke;

		std::string dialog_id;

		std::vector<uint8_t> buf(0xffff);
		std::size_t sz = buf.size();

		EXPECT_CALL(*conf_protocol, InviteMethod(_, _, _, _, _, _, _))
			.WillOnce(DoAll(WithArg<0>(Invoke([&dialog_id](string_view a) { dialog_id = std::string(a); })), Return(true)));

		h245_channel_fake->remote_addr = terminal_addr.addr;
		h245_channel_fake->remote_port = 12345;
		EXPECT_CALL(*h245_channel, Open(_, _, _, _, _, _)).Times(AtLeast(1));

		ASSERT_GT(h323->SetRecvBuf(raw_gnugk_setup, sizeof(raw_gnugk_setup), e_H225, terminal_addr.addr, terminal_addr.port, {}, 0), 0);
		ASSERT_GT(h323->GetBufForSend(&buf[0], sz, e_H225, {}, 0, {}, 0), 0);

		h323->InviteReplay(dialog_id.c_str(), e_call_ok, false);

		sz = buf.size();
		ASSERT_GT(h323->GetBufForSend(&buf[0], sz, e_H225, {}, 0, {}, 0), 0);

		{
			VS_PerBuffer in_per_buff(&buf[4], (sz - 4) * 8); // skip 4 bytes of header
			VS_Q931 theQ931;
			ASSERT_TRUE(theQ931.DecodeMHeader(in_per_buff));
			ASSERT_EQ(theQ931.messageType, VS_Q931::e_connectMsg);

			unsigned char dn[82 + 1] = { 0 };
			unsigned char e164[50] = { 0 };
			ASSERT_TRUE(VS_Q931::GetUserUserIE(in_per_buff, dn, e164));
			VS_CsH323UserInformation ui;
			ASSERT_TRUE(ui.Decode(in_per_buff));
			VS_CsConnectUuie* connect = ui.h323UuPdu.h323MessageBody;
			ASSERT_EQ(connect->h245Address.tag, VS_H225TransportAddress::e_ip6Address);
			net::address addr;
			net::port port;
			ASSERT_TRUE(get_ip_address(connect->h245Address, addr, port));
			ASSERT_TRUE(addr.is_v6());
			auto ipv6addr = addr.to_v6().to_bytes();
			ASSERT_EQ(ipv6addr.front(), 0xFD);
			ASSERT_EQ(ipv6addr.back(), 0x01);
		}
	}
}

#endif