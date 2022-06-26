#include "SIPParserTestBase.h"
#include "tests/common/GMockOverride.h"
#include "../../SIPParserLib/VS_SIPField_RetryAfter.h"
#include "../../SIPParserLib/VS_UUID.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/scope_exit.h"
#include "SipRaw.h"
#include "TestSIPTransportLayer.h"

#include "../TrueGateway/sip/VS_SIPGetInfoImpl.h"
#include "../TrueGateway/sip/VS_SIPUpdateInfoImpl.h"
#include "../../FakeClient/VS_ConferenceInfo.h"
#include "../SIPParserLib/VS_SIPInstantMessage.h"
#include "../SIPParserLib/VS_SIPResponse.h"
#include "../tests/common/Utils.h"
#include "SIPParserLib/VS_SDPField_Origin.h"
#include <boost/algorithm/string/predicate.hpp>
#include "TrueGateway/sip/VS_ChatBotMessages.h"
#include "std/cpplib/MakeShared.h"
#include "std/cpplib/VS_Replace.h"
#include "tools/Server/VS_MediaChannelInfo.h"

#include "tests/common/GTestMatchers.h"

extern std::string g_tr_endpoint_name;

#pragma warning (disable:4800)
namespace
{
	#include "RawSIPMessages.h"

	const uint32_t _65KB = 66560; // 65KB in bytes

	struct OutgoingParams
	{
		const char *sipName;
		const char *from;
		const char *to;
		const char *trying;
		const char *ringing;
		const char *ok;

		bool isAudio;
		int  audioCodec;

		bool isVideo;
		int  videoCodec;

		friend std::ostream & operator<<(std::ostream &os, const OutgoingParams &param)
		{
			return os << "sip: " << param.sipName;
		}
	};

	struct IncomingParams
	{
		const char *sipName;
		const char *invite;
		const char *ack;

		bool isAudio;
		int  audioCodec;

		bool isVideo;
		int  videoCodec;

		friend std::ostream & operator<<(std::ostream &os, const IncomingParams &param)
		{
			return os << "sip: " << param.sipName;
		}
	};

	struct CallOriginGroupParam
	{
		const char *callName;
		const char *expectedOrigin;
		bool isGroupConf;
	};

	class SIPParserTest : public SIPParserTestBase< ::testing::Test> {};
	class SIPParserOutgoingTest : public SIPParserTestBase< ::testing::TestWithParam<OutgoingParams>> {};
	class SIPParserIncomingTest : public SIPParserTestBase< ::testing::TestWithParam<IncomingParams>> {};
	class SIPParserOriginSDPDirectionToSIPTest : public SIPParserTest, public ::testing::WithParamInterface<CallOriginGroupParam> {};
	class SIPParserOriginSDPDirectionFromSIPTest : public SIPParserTest, public ::testing::WithParamInterface<CallOriginGroupParam> {};


	TEST_F(SIPParserTest, Bug43000_Split_IPv6) //(Split ipv6)
	{
		const auto old_root = VS_RegistryKey::GetDefaultRoot();

		test::InitRegistry();

		VS_SCOPE_EXIT
		{
			VS_RegistryKey::SetDefaultRoot(old_root);
		};

		const struct final
		{
			const char *actual;
			const char *expected;
		} test_data[] =
		{
			{ "hello.com.ru", "hello.com.ru" },
			{ "192.168.61.168", "192.168.61.168"},
			{ "266.266.266.266", "" },
			{ "192.168.61.168:5060", ""},
			{ "[fd00:380:57::8ac6]", "[fd00:380:57::8ac6]"},
			{ "fd00:380:57::8ac6", "[fd00:380:57::8ac6]"},
			{ "0000:0000:0000:0000:0000:0000:0000:0001", "[0000:0000:0000:0000:0000:0000:0000:0001]"},
			{ "[0000:0000:0000:0000:0000:0000:0000:0001]", "[0000:0000:0000:0000:0000:0000:0000:0001]"}
		};

		VS_RegistryKey key{ false, CONFIGURATION_KEY, false, true };
		ASSERT_TRUE(key.IsValid());
		for(auto &item : test_data)
		{
			ASSERT_TRUE(key.SetString(item.actual, SIP_FROM_HOST));
			auto &&res = GetFromHost();
			EXPECT_STREQ(res.c_str(), item.expected);
		}
	}

	TEST_F(SIPParserTest, Bug23422) // (423 Interval Too Brief)
	{
		VS_CallConfig config;
		// config->sip.SkipOPTIONS = true;

		config.Address.addr = net::address_v4( 1 );
		config.Address.port =  1 ;
		config.Address.protocol = net::protocol::UDP;

		config.HostName = "213.133.168.206";
		config.IsValid = true;
		config.Login = "test";
		config.Password = "test";
		config.SignalingProtocol =VS_CallConfig::SIP;
		config.sip.RegistrationBehavior = VS_CallConfig::REG_REGISTER_ALWAYS;

		auto sip_call_config = vs::MakeShared<VS_CallConfigStorage>();
		sip_call_config->RegisterProtocol(boost::make_shared<VS_IndentifierSIP>(g_asio_environment->IOService(), "serverVendor"));
		sip->SetCallConfigStorage(sip_call_config);

		sip->SetRegistrationConfiguration( std::move(config) );
		sip->Timeout();

		auto msg = GetMessageFromParser( sip );
		ASSERT_TRUE(msg);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(MESSAGE_TYPE_REQUEST, msg->GetSIPMetaField()->iStartLine->GetMessageType());
		ASSERT_EQ(TYPE_REGISTER, msg->GetSIPMetaField()->iCSeq->GetType());
		ASSERT_GT(std::chrono::seconds(3000), msg->GetSIPMetaField()->iExpires->Value());

		std::string response = raw_sip_message_423_Interval_Too_Brief;
		strreplace(response, "__callid__", msg->CallID());
		ASSERT_GE(msg->GetSIPMetaField()->iVia.size(), std::size_t(1));
		ASSERT_NE(msg->GetSIPMetaField()->iVia[0], nullptr);
		strreplace(response, "__branch__", msg->GetSIPMetaField()->iVia[0]->Branch() );
		strreplace(response, "__cseq__", std::to_string(msg->GetCSeq()));

		ASSERT_TRUE(SetRecvBuf(&response[0], response.size()));
		sip->Timeout();

		msg = GetMessageFromParser( sip );
		ASSERT_TRUE(msg);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(MESSAGE_TYPE_REQUEST, msg->GetSIPMetaField()->iStartLine->GetMessageType());
		ASSERT_EQ(TYPE_REGISTER, msg->GetSIPMetaField()->iCSeq->GetType());
		ASSERT_LE(std::chrono::seconds(3200), msg->GetSIPMetaField()->iExpires->Value());
	};


	class CodecPriorityTest : public SIPParserTest
	{
	public:
		void TestCodecPriority(int _codec, const char *sdp)
		{
			// test may fail, if "SDP Enabled codecs" is specified
			using ::testing::_;
			using ::testing::AtLeast;
			using ::testing::InvokeArgument;
			using ::testing::DoAll;
			using ::testing::WithArg;
			using ::testing::Invoke;

			std::string dialog_id;

			EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, _, _))
				.WillOnce(DoAll(InvokeArgument<4>(false, VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE, ""), WithArg<0>(Invoke([&dialog_id](string_view a) { dialog_id = std::string(a); }))));
			EXPECT_CALL(*confProtocol, SetMediaChannels(_, _, _, _))
				.Times(AtLeast(1));

			std::string inv = CombineInviteAndSDP(raw_sip_message_CommonInvite, sdp);
			ASSERT_TRUE(SetRecvBuf(inv.c_str(), inv.size()));
			sip->InviteReplay(dialog_id, e_call_ok, false);

			auto audio_channel_it = std::find_if(confProtocol_fake.last_media_channels.begin(), confProtocol_fake.last_media_channels.end(), [](const VS_MediaChannelInfo& x) {
				return x.type == SDPMediaType::audio && x.content == SDP_CONTENT_MAIN;
			});
			ASSERT_TRUE(audio_channel_it != confProtocol_fake.last_media_channels.end());
			EXPECT_EQ(audio_channel_it->snd_mode_audio.CodecType, _codec);
		}
	};

	TEST_F(CodecPriorityTest, Bug23563) // (423 Select Right Codec in SDP)
	{
		TestCodecPriority(e_rcvG711Alaw64k, raw_sip_message_SDP_G729_PCMA);
	}

	TEST_F(CodecPriorityTest, Bug23563_2) // (G722 vs PCMA)
	{
		TestCodecPriority(e_rcvG722_64k, raw_sip_message_SDP_G722_PCMA);
	}

	TEST_F(SIPParserTest,  Bug23517) // sip timer extention
	{
		VS_CallConfig config;
		// config->sip.SkipOPTIONS = true;
		config.Address.addr = net::address_v4( 1 );
		config.Address.port =  1;
		config.Address.protocol = net::protocol::UDP;
		using ::testing::_;
		using ::testing::Return;

		EXPECT_CALL(*confProtocol, InviteReplay(_, e_call_ok, _, _, _))
			.WillOnce( Return( true ));

		EXPECT_CALL(*confProtocol, Hangup( _ ))
			.Times( 0 );

		sip->clock().add_diff(std::chrono::seconds(537));

		auto &&dialog_id = sip->NewDialogID("to_name", {}, config);
		sip->InviteMethod(dialog_id, "from_name", "to_name", VS_ConferenceInfo(false, false));

		auto iv_msg = GetMessageFromParser( sip );
		ASSERT_TRUE(iv_msg ->IsValid());
		ASSERT_TRUE(!iv_msg->GetSIPMetaField()->iVia.empty());
		std::string branch = iv_msg->GetSIPMetaField()->iVia.front()->Branch();

		sip->clock().add_diff(std::chrono::seconds(538));

		std::string resp_180 = raw_sip_message_Bug23517_180;
		strreplace(resp_180, "__callid__", dialog_id);
		strreplace(resp_180, "__branch__", branch);
		ASSERT_TRUE(SetRecvBuf(resp_180.c_str(), resp_180.size()));

		sip->clock().add_diff(std::chrono::seconds(542));

		std::string resp_200 = CombineInviteAndSDP(raw_sip_message_Bug23517_200, raw_sip_message_SDP_G722_PCMA);
		strreplace(resp_200, "__callid__", dialog_id);
		strreplace(resp_200, "__branch__", branch);
		ASSERT_TRUE(SetRecvBuf(resp_200.c_str(), resp_200.size()));

		auto ack = GetMessageFromParser( sip );
		ASSERT_TRUE(ack->IsValid());
		ASSERT_EQ(TYPE_ACK, ack->GetSIPMetaField()->iStartLine->GetRequestType());

		sip->clock().add_diff(std::chrono::seconds(1438));

		std::string req_invite = CombineInviteAndSDP(raw_sip_message_Bug23517_Invite, raw_sip_message_SDP_G722_PCMA);
		strreplace(req_invite, "__callid__", dialog_id);
		ASSERT_TRUE(SetRecvBuf(req_invite.c_str(), req_invite.size()));

		auto msg = GetMessageFromParser( sip );
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(200, msg->GetSIPMetaField()->iStartLine->GetResponseCode());

		sip->Timeout();

		sip->clock().add_diff(std::chrono::seconds(1530));

		sip->Timeout();
		sip->Timeout();

		auto msg_bye = GetMessageFromParser( sip );
		ASSERT_TRUE(!msg_bye || msg_bye->IsValid() == false || msg_bye->GetSIPMetaField()->iCSeq->GetType() != TYPE_BYE);
	}

	TEST_F(SIPParserTest,  realm_parse_spase)
	{
		VS_CallConfig config;

		config.Address.addr = net::address_v4( 1 );
		config.Address.port =  1;
		config.Address.protocol = net::protocol::UDP;

		config.HostName = "192.168.40.150";
		config.IsValid = true;
		config.Login = "107";
		config.Password = "1234";
		config.SignalingProtocol =VS_CallConfig::SIP;
		config.sip.RegistrationBehavior = VS_CallConfig::REG_REGISTER_ALWAYS;

		auto sip_call_config = vs::MakeShared<VS_CallConfigStorage>();
		sip_call_config->RegisterProtocol(boost::make_shared<VS_IndentifierSIP>(g_asio_environment->IOService(), "serverVendor"));
		sip->SetCallConfigStorage(sip_call_config);

		sip->SetRegistrationConfiguration( std::move(config) );
		sip->Timeout();

		auto msg = GetMessageFromParser( sip );
		ASSERT_TRUE(msg);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(MESSAGE_TYPE_REQUEST, msg->GetSIPMetaField()->iStartLine->GetMessageType());
		ASSERT_EQ(TYPE_REGISTER, msg->GetSIPMetaField()->iCSeq->GetType());

		std::string response = raw_sip_message_unauth;
		strreplace(response, "__callid__", msg->CallID());
		ASSERT_TRUE(!msg->GetSIPMetaField()->iVia.empty());
		strreplace(response, "__branch__", msg->GetSIPMetaField()->iVia[0]->Branch());

		ASSERT_TRUE(SetRecvBuf(response.c_str(), response.size()));
		sip->Timeout();

		msg = GetMessageFromParser( sip );
		ASSERT_TRUE(msg);
		ASSERT_TRUE(msg->IsValid());

		ASSERT_FALSE(msg->GetSIPMetaField()->iAuthHeader.empty());
		VS_SIPField_Auth* AuthHeader = msg->GetSIPMetaField()->iAuthHeader[0];
		ASSERT_TRUE(AuthHeader != NULL);
		ASSERT_TRUE(AuthHeader->GetAuthInfo() != NULL);
		EXPECT_STREQ("Registered Users", AuthHeader->GetAuthInfo()->realm().c_str());
		EXPECT_STREQ("19858fb688520a571d9005fcd19523c2", AuthHeader->GetAuthInfo()->response().c_str());
	}

	TEST(SIPParserTestS, ParseTest1)
	{
		boost::shared_ptr<VS_SIPMessage> msg = boost::make_shared<VS_SIPMessage>();
		msg->Decode(raw_sip_message_test1, strlen(raw_sip_message_test1));
				EXPECT_TRUE(msg->IsValid());
	}

	// Cisco 3845 using VoIPGateway
	TEST(SIPParserTestS, Parse_ru_ticket_6439)
	{
		boost::shared_ptr<VS_SIPMessage> msg = boost::make_shared<VS_SIPMessage>();
		msg->Decode(raw_sip_message_ru_ticket_6439, strlen(raw_sip_message_ru_ticket_6439));
		EXPECT_TRUE(msg->IsValid());
	}

	TEST_F(SIPParserTest,  ParseImageComCrash)
	{
		EXPECT_FALSE(SetRecvBuf(&raw_sip_message_ImageCom_crash[0], sizeof(raw_sip_message_ImageCom_crash)));
	}

	TEST(SIPParserTestS, DecodeUnsupportedMedia)
	{
		std::string inv = CombineInviteAndSDP(raw_sip_message_CommonInvite, raw_sdp_generic);
		boost::shared_ptr<VS_SIPMessage> msg = boost::make_shared<VS_SIPMessage>();

		msg->Decode(inv.c_str(), inv.length());
		ASSERT_TRUE(msg->IsValid());
		ASSERT_TRUE(msg->GetSIPMetaField()->IsValid());
		ASSERT_TRUE(msg->GetSDPMetaField()->IsValid());
		ASSERT_EQ(msg->GetSDPMetaField()->iDirection, SDP_MEDIACHANNELDIRECTION_INVALID);
		ASSERT_EQ(msg->GetSDPMetaField()->iMediaStreams.size(), 3); // audio, video, application
	}

	TEST_F(SIPParserTest,  ParseCiscoEX60RegisterAtPMO)
	{
		using ::testing::_;
		using ::testing::Return;

		ASSERT_TRUE(SetRecvBuf(&raw_cisco_ex60_at_PMO[0], sizeof(raw_cisco_ex60_at_PMO)));
	}

	TEST_F(SIPParserTest,  ParseCiscoC60_Crash)
	{
		ASSERT_TRUE(SetRecvBuf(&raw_cisco_c60_crash[0], sizeof(raw_cisco_c60_crash)));
	}

	TEST_F(SIPParserTest, Tandberg_722_INVITE) //bug 29361
	{
		using ::testing::_;
		using ::testing::Invoke;
		using ::testing::InvokeArgument;
		using ::testing::DoAll;
		using ::testing::WithArg;
		using ::testing::Invoke;

		net::Endpoint from{net::address::from_string("203.188.211.150"), 25776 , net::protocol::UDP};
		std::string dialog_id;

		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, _, _))
			.WillOnce(DoAll(InvokeArgument<4>(false, VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE, ""), WithArg<0>(Invoke([&dialog_id](string_view a) { dialog_id = std::string(a); })),
			Invoke([this](string_view, const gw::Participant&, string_view, const VS_ConferenceInfo&, const boost::function<void(bool redirect, VS_ConferenceProtocolInterface::ConferenceStatus status, const std::string &ip)> &, string_view, bool, bool){
			auto msg = GetMessageFromParser(sip);
			ASSERT_NE(msg, nullptr);
			ASSERT_EQ(msg->GetMessageType(), MESSAGE_TYPE_RESPONSE);
			ASSERT_EQ(msg->GetMethod(), TYPE_INVITE);
		})));

		ASSERT_TRUE(SetRecvBuf(&raw_TANDBERG_772_INVITE[0], sizeof(raw_TANDBERG_772_INVITE)));
	}

	TEST_F(SIPParserTest, BFCP_InCall_Client)
	{
		using ::testing::_;
		using ::testing::AnyNumber;
		using ::testing::AtLeast;
		using ::testing::DoAll;
		using ::testing::Invoke;
		using ::testing::InvokeArgument;
		using ::testing::WithArg;
		using ::testing::AllOf;
		using ::testing::Not;

		const net::address bfcp_remote_address = net::address::from_string("192.168.62.113");
		const net::port bfcp_remote_port = 34683;

		EXPECT_CALL(*identifier, CreateDefaultConfiguration_Impl(_, _, _, _))
			.WillRepeatedly(Invoke([](VS_CallConfig &cfg, const net::Endpoint& ep, VS_CallConfig::eSignalingProtocol protocol, string_view username) {
				cfg.SignalingProtocol = VS_CallConfig::SIP;
				cfg.Address = ep;
				cfg.Login = std::string(username);
				cfg.sip.BFCPEnabled = true;
				cfg.sip.BFCPRoles = SDP_FLOORCTRL_ROLE_C_ONLY;
				return true;
			}));

		std::string dialog_id;
		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, _, _))
			.WillOnce(DoAll(InvokeArgument<4>(false, VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE, ""), WithArg<0>(Invoke([&dialog_id](string_view a) { dialog_id = std::string(a); }))));

		auto bfcp_channel_fake = std::make_shared<VS_SignalChannelFake>();
		auto bfcp_channel = std::make_shared<VS_SignalChannelMock>();
		bfcp_channel->DelegateTo(bfcp_channel_fake.get(), bfcp_channel_fake);
		EXPECT_CALL(*bfcp_channel,
			Open(
				AllOf(
					AnyBitSet(VS_SignalChannel::CONNECT_TCP | VS_SignalChannel::CONNECT_UDP),
					Not(AnyBitSet(VS_SignalChannel::LISTEN_TCP | VS_SignalChannel::LISTEN_UDP))
				), _, _, _, _, _
			))
			.Times(AtLeast(1));
		VS_SignalChannel::SetFactory([&bfcp_channel](boost::asio::io_service&) { return bfcp_channel; });

		std::string inv = CombineInviteAndSDP(raw_sip_message_CommonInvite, raw_sdp_bfcp_c_s);
		ASSERT_TRUE(SetRecvBuf(inv.c_str(), inv.size()));
		sip->InviteReplay(dialog_id, e_call_ok, false);

		VS_SignalChannel::SetFactory(nullptr);
		ASSERT_TRUE(bfcp_channel_fake->connected);
		ASSERT_EQ(bfcp_remote_address, bfcp_channel_fake->remote_addr);
		ASSERT_EQ(bfcp_remote_port, bfcp_channel_fake->remote_port);

		auto msg = GetMessageFromParser(sip); //100 Trying
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(100, msg->GetSIPMetaField()->iStartLine->GetResponseCode());

		msg = GetMessageFromParser(sip); // 180 Ringing
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(180, msg->GetSIPMetaField()->iStartLine->GetResponseCode());

		auto msg_200ok = GetMessageFromParser(sip);
		ASSERT_NE(nullptr, msg_200ok->GetSIPMetaField());
		ASSERT_NE(nullptr, msg_200ok->GetSIPMetaField()->iStartLine);
		EXPECT_EQ(200, msg_200ok->GetSIPMetaField()->iStartLine->GetResponseCode());
		auto sdp = msg_200ok->GetSDPMetaField();
		ASSERT_NE(nullptr, sdp);
		auto bfcp_ms_it = std::find_if(sdp->iMediaStreams.begin(), sdp->iMediaStreams.end(), [](const VS_SDPField_MediaStream* ms) {
			return ms->GetMediaType() == SDPMediaType::application_bfcp;
		});
		ASSERT_TRUE(bfcp_ms_it != sdp->iMediaStreams.end());
		EXPECT_EQ(bfcp_channel_fake->local_port, (*bfcp_ms_it)->GetLocalPort());
		EXPECT_EQ(SDP_FLOORCTRL_ROLE_C_ONLY, (*bfcp_ms_it)->GetBFCPFloorCtrl());
	}

	TEST_F(SIPParserTest, BFCP_InCall_Server)
	{
		using ::testing::_;
		using ::testing::AnyNumber;
		using ::testing::AtLeast;
		using ::testing::DoAll;
		using ::testing::Invoke;
		using ::testing::InvokeArgument;
		using ::testing::WithArg;

		const net::address bfcp_remote_address = net::address::from_string("192.168.62.113");
		const net::port bfcp_remote_port = 34683;

		EXPECT_CALL(*identifier, CreateDefaultConfiguration_Impl(_, _, _, _))
			.WillRepeatedly(Invoke([](VS_CallConfig &cfg, const net::Endpoint& ep, VS_CallConfig::eSignalingProtocol protocol, string_view username) {
				cfg.SignalingProtocol = VS_CallConfig::SIP;
				cfg.Address = ep;
				cfg.Login = std::string(username);
				cfg.sip.BFCPEnabled = true;
				cfg.sip.BFCPRoles = SDP_FLOORCTRL_ROLE_S_ONLY;
				return true;
			}));

		std::string dialog_id;
		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, _, _))
			.WillOnce(DoAll(InvokeArgument<4>(false, VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE, ""), WithArg<0>(Invoke([&dialog_id](string_view a) { dialog_id = std::string(a); }))));

		auto bfcp_channel_fake = std::make_shared<VS_SignalChannelFake>();
		bfcp_channel_fake->remote_addr = bfcp_remote_address;
		bfcp_channel_fake->remote_port = bfcp_remote_port;
		auto bfcp_channel = std::make_shared<VS_SignalChannelMock>();
		bfcp_channel->DelegateTo(bfcp_channel_fake.get(), bfcp_channel_fake);
		EXPECT_CALL(*bfcp_channel,
			Open(
				AnyBitSet(VS_SignalChannel::LISTEN_TCP | VS_SignalChannel::LISTEN_UDP),
				_, _, _, _, _
			))
			.Times(AtLeast(1));
		VS_SignalChannel::SetFactory([&bfcp_channel](boost::asio::io_service&) { return bfcp_channel; });

		std::string inv = CombineInviteAndSDP(raw_sip_message_CommonInvite, raw_sdp_bfcp_c_s);
		ASSERT_TRUE(SetRecvBuf(inv.c_str(), inv.size()));
		sip->InviteReplay(dialog_id, e_call_ok, false);

		VS_SignalChannel::SetFactory(nullptr);
		ASSERT_TRUE(bfcp_channel_fake->connected);
		ASSERT_EQ(bfcp_remote_address, bfcp_channel_fake->remote_addr);
		ASSERT_EQ(bfcp_remote_port, bfcp_channel_fake->remote_port);

		auto msg = GetMessageFromParser(sip); //100 Trying
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(100, msg->GetSIPMetaField()->iStartLine->GetResponseCode());

		msg = GetMessageFromParser(sip); // 180 Ringing
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(180, msg->GetSIPMetaField()->iStartLine->GetResponseCode());

		msg = GetMessageFromParser(sip);
		auto sdp = msg->GetSDPMetaField();
		ASSERT_NE(nullptr, sdp);
		auto bfcp_ms_it = std::find_if(sdp->iMediaStreams.begin(), sdp->iMediaStreams.end(), [](const VS_SDPField_MediaStream* ms) {
			return ms->GetMediaType() == SDPMediaType::application_bfcp;
		});
		ASSERT_TRUE(bfcp_ms_it != sdp->iMediaStreams.end());
		EXPECT_EQ(bfcp_channel_fake->local_port, (*bfcp_ms_it)->GetLocalPort());
		EXPECT_EQ(SDP_FLOORCTRL_ROLE_S_ONLY, (*bfcp_ms_it)->GetBFCPFloorCtrl());
		EXPECT_NE(0, (*bfcp_ms_it)->GetBFCPConfID());
		EXPECT_NE(0, (*bfcp_ms_it)->GetBFCPUserID());
		EXPECT_NE(0, (*bfcp_ms_it)->GetBFCPFloorID());
	}

	TEST_F(SIPParserTest, BFCP_OutCall_Client)
	{
		using ::testing::_;
		using ::testing::AnyNumber;
		using ::testing::AtLeast;
		using ::testing::Return;
		using ::testing::AllOf;
		using ::testing::Not;
		using ::testing::StrEq;

		const net::address bfcp_remote_address = net::address::from_string("192.168.62.113");
		const net::port bfcp_remote_port = 34683;

		VS_CallConfig config;
		// config->sip.SkipOPTIONS = true;
		config.SignalingProtocol = VS_CallConfig::SIP;
		config.sip.BFCPEnabled = true;
		config.sip.BFCPRoles = SDP_FLOORCTRL_ROLE_C_ONLY;
		config.sip.DefaultBFCPProto = net::protocol::TCP;
		config.H224Enabled = false;

		auto &&dialog_id = sip->NewDialogID("to_name", {}, config);
		EXPECT_CALL(*confProtocol, InviteReplay(::testing::Eq(string_view(dialog_id)), e_call_ok, _, _, _))
			.WillOnce(Return(true));
		EXPECT_CALL(*confProtocol, Hangup(_))
			.Times(0);

		auto bfcp_channel_fake = std::make_shared<VS_SignalChannelFake>();
		auto bfcp_channel = std::make_shared<VS_SignalChannelMock>();
		bfcp_channel->DelegateTo(bfcp_channel_fake.get(), bfcp_channel_fake);
		EXPECT_CALL(*bfcp_channel,
			Open(
				AllOf(
					AnyBitSet(VS_SignalChannel::CONNECT_TCP | VS_SignalChannel::CONNECT_UDP),
					Not(AnyBitSet(VS_SignalChannel::LISTEN_TCP | VS_SignalChannel::LISTEN_UDP))
				), _, _, _, _, _
			))
			.Times(AtLeast(1));
		VS_SignalChannel::SetFactory([&bfcp_channel](boost::asio::io_service&) { return bfcp_channel; });

		sip->InviteMethod(dialog_id, "from_name", "to_name", VS_ConferenceInfo(false, false));

		auto msg_invite = GetMessageFromParser(sip);
		ASSERT_TRUE(msg_invite->IsValid());
		ASSERT_NE(nullptr, msg_invite->GetSIPMetaField());
		ASSERT_LE(1u, msg_invite->GetSIPMetaField()->iVia.size());
		std::string branch = msg_invite->GetSIPMetaField()->iVia[0]->Branch();

		auto sdp = msg_invite->GetSDPMetaField();
		ASSERT_NE(nullptr, sdp);
		auto bfcp_ms_it = std::find_if(sdp->iMediaStreams.begin(), sdp->iMediaStreams.end(), [](const VS_SDPField_MediaStream* ms) {
			return ms->GetMediaType() == SDPMediaType::application_bfcp;
		});
		ASSERT_TRUE(bfcp_ms_it != sdp->iMediaStreams.end());
		EXPECT_NE(0, (*bfcp_ms_it)->GetPort());
		EXPECT_EQ(SDP_FLOORCTRL_ROLE_C_ONLY, (*bfcp_ms_it)->GetBFCPFloorCtrl());

		std::string resp_180 = raw_sip_message_Common180Ringing;
		boost::replace_all(resp_180, string_view("__callid__"), string_view(dialog_id));
		boost::replace_all(resp_180, string_view("__branch__"), branch);
		ASSERT_TRUE(SetRecvBuf(resp_180.c_str(), resp_180.size()));

		std::string resp_200 = CombineInviteAndSDP(raw_sip_message_Common200OK, raw_sdp_bfcp_s_only);
		boost::replace_all(resp_200, string_view("__callid__"), string_view(dialog_id));
		boost::replace_all(resp_200, string_view("__branch__"), branch);
		ASSERT_TRUE(SetRecvBuf(resp_200.c_str(), resp_200.size()));

		VS_SignalChannel::SetFactory(nullptr);
		ASSERT_TRUE(bfcp_channel_fake->connected);
		ASSERT_EQ(bfcp_remote_address, bfcp_channel_fake->remote_addr);
		ASSERT_EQ(bfcp_remote_port, bfcp_channel_fake->remote_port);

		auto msg_ack = GetMessageFromParser(sip);
		ASSERT_TRUE(msg_ack->IsValid());
		ASSERT_NE(nullptr, msg_ack->GetSIPMetaField());
		ASSERT_NE(nullptr, msg_ack->GetSIPMetaField()->iStartLine);
		EXPECT_EQ(TYPE_ACK, msg_ack->GetSIPMetaField()->iStartLine->GetRequestType());
	}

	TEST_F(SIPParserTest, BFCP_OutCall_Server)
	{
		using ::testing::_;
		using ::testing::AnyNumber;
		using ::testing::AtLeast;
		using ::testing::Return;
		using ::testing::StrEq;

		const net::address bfcp_remote_address = net::address::from_string("192.168.62.113");
		const net::port bfcp_remote_port = 34683;

		VS_CallConfig config;
		// config->sip.SkipOPTIONS = true;
		config.SignalingProtocol = VS_CallConfig::SIP;
		config.sip.BFCPEnabled = true;
		config.sip.BFCPRoles = SDP_FLOORCTRL_ROLE_S_ONLY;
		config.sip.DefaultBFCPProto = net::protocol::TCP;

		auto dialog_id = sip->NewDialogID("to_name", {}, config);
		EXPECT_CALL(*confProtocol, InviteReplay(::testing::Eq(string_view(dialog_id)), e_call_ok, _, _, _))
			.WillOnce(Return(true));
		EXPECT_CALL(*confProtocol, Hangup(_))
			.Times(0);

		auto bfcp_channel_fake = std::make_shared<VS_SignalChannelFake>();
		bfcp_channel_fake->remote_addr = bfcp_remote_address;
		bfcp_channel_fake->remote_port = bfcp_remote_port;
		auto bfcp_channel = std::make_shared<VS_SignalChannelMock>();
		bfcp_channel->DelegateTo(bfcp_channel_fake.get(), bfcp_channel_fake);
		EXPECT_CALL(*bfcp_channel,
			Open(
				AnyBitSet(VS_SignalChannel::LISTEN_TCP | VS_SignalChannel::LISTEN_UDP),
				_, _, _, _, _
			))
			.Times(AtLeast(1));
		VS_SignalChannel::SetFactory([&bfcp_channel](boost::asio::io_service&) { return bfcp_channel; });

		sip->InviteMethod(dialog_id, "from_name", "to_name", VS_ConferenceInfo(false, false));

		VS_SignalChannel::SetFactory(nullptr);
		ASSERT_TRUE(bfcp_channel_fake->connected);
		ASSERT_EQ(bfcp_remote_address, bfcp_channel_fake->remote_addr);
		ASSERT_EQ(bfcp_remote_port, bfcp_channel_fake->remote_port);

		auto msg_invite = GetMessageFromParser(sip);
		ASSERT_TRUE(msg_invite->IsValid());
		ASSERT_NE(nullptr, msg_invite->GetSIPMetaField());
		ASSERT_LE(1u, msg_invite->GetSIPMetaField()->iVia.size());
		const std::string &branch = msg_invite->GetSIPMetaField()->iVia[0]->Branch();

		auto sdp = msg_invite->GetSDPMetaField();
		ASSERT_NE(nullptr, sdp);
		auto bfcp_ms_it = std::find_if(sdp->iMediaStreams.begin(), sdp->iMediaStreams.end(), [](const VS_SDPField_MediaStream* ms) {
			return ms->GetMediaType() == SDPMediaType::application_bfcp;
		});
		ASSERT_TRUE(bfcp_ms_it != sdp->iMediaStreams.end());
		EXPECT_EQ(bfcp_channel_fake->local_port, (*bfcp_ms_it)->GetLocalPort());
		EXPECT_EQ(SDP_FLOORCTRL_ROLE_S_ONLY, (*bfcp_ms_it)->GetBFCPFloorCtrl());
		EXPECT_NE(0, (*bfcp_ms_it)->GetBFCPConfID());
		EXPECT_NE(0, (*bfcp_ms_it)->GetBFCPUserID());
		EXPECT_NE(0, (*bfcp_ms_it)->GetBFCPFloorID());

		std::string resp_180 = raw_sip_message_Common180Ringing;
		boost::replace_all(resp_180, string_view("__callid__"), string_view(dialog_id));
		boost::replace_all(resp_180, string_view("__branch__"), branch);
		ASSERT_TRUE(SetRecvBuf(resp_180.c_str(), resp_180.size()));

		std::string resp_200 = CombineInviteAndSDP(raw_sip_message_Common200OK, raw_sdp_bfcp_c_only);
		boost::replace_all(resp_200, string_view("__callid__"), string_view(dialog_id));
		boost::replace_all(resp_200, string_view("__branch__"), branch);
		ASSERT_TRUE(SetRecvBuf(resp_200.c_str(), resp_200.size()));

		auto msg_ack = GetMessageFromParser(sip);
		ASSERT_TRUE(msg_ack->IsValid());
		ASSERT_NE(nullptr, msg_ack->GetSIPMetaField());
		ASSERT_NE(nullptr, msg_ack->GetSIPMetaField()->iStartLine);
		EXPECT_EQ(TYPE_ACK, msg_ack->GetSIPMetaField()->iStartLine->GetRequestType());
	}

	TEST_P(SIPParserOriginSDPDirectionToSIPTest, Bug42320_AddedFieldOriginToSDP_CallClientAndGroupClientAndServerToSIP)
	{
		auto orig_tr_endpoint_name = g_tr_endpoint_name;
		VS_SCOPE_EXIT { g_tr_endpoint_name = orig_tr_endpoint_name; };

		auto&& conf_param = GetParam();
		const auto to_name = "#sip:user1@vydr111.trueconf.name";
		const auto dtmf = "";
		g_tr_endpoint_name = "vydr111.trueconf.name";

		auto &&dialog_id = sip->NewDialogID(to_name, dtmf);
		sip->InviteMethod(dialog_id, conf_param.callName, to_name, VS_ConferenceInfo{ conf_param.isGroupConf, false });
		auto&& msg_invite = GetMessageFromParser(sip);
		ASSERT_TRUE(msg_invite->IsValid());
		VS_SIPBuffer buffer;
		ASSERT_EQ(msg_invite->GetSDPMetaField()->iOrigin->Encode(buffer), TSIPErrorCodes::e_ok);
		{
			std::unique_ptr<char[]> ptr;
			size_t ptr_sz = 0;
			ASSERT_EQ(buffer.GetNextBlockAllocConst(ptr, ptr_sz), TSIPErrorCodes::e_ok);
			ASSERT_TRUE((ptr && ptr_sz));
			boost::cmatch m;
			ASSERT_TRUE(boost::regex_match(ptr.get(), m, VS_SDPField_Origin::e));
			const std::size_t index_origin = 4;
			const std::string origin = m[index_origin];
			ASSERT_STREQ(origin.c_str(), conf_param.expectedOrigin);
		}
		ASSERT_EQ(msg_invite->GetSDPMetaField()->iOrigin->Decode(buffer), TSIPErrorCodes::e_ok);
		ASSERT_STREQ(msg_invite->GetSDPMetaField()->iOrigin->UserName().c_str(), conf_param.callName);
	}


	TEST_P(SIPParserOriginSDPDirectionFromSIPTest, Bug42320_AddedFieldOriginToSDP_CallSIPToClinetAndGroupConfCID)
	{
		using ::testing::_;
		using ::testing::InvokeArgument;
		using ::testing::DoAll;
		using ::testing::WithArg;
		using ::testing::Invoke;

		auto&& conf_param = GetParam();
		std::string dialog_id;
		ON_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, _, _))
			.WillByDefault(DoAll(InvokeArgument<4>(false, VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE, ""), WithArg<0>(Invoke([&dialog_id](string_view a) { dialog_id = std::string(a); }))));

		std::string invite_message{ raw_sip_message_Bug42320_Invite };
		const auto replace_str = "__invite__";
		boost::replace_all(invite_message, replace_str, conf_param.callName);

		invite_message = CombineInviteAndSDP(invite_message, raw_sdp_bfcp_c_s);
		ASSERT_TRUE(SetRecvBuf(invite_message.c_str(), invite_message.size()));
		sip->InviteReplay(dialog_id, e_call_ok, conf_param.isGroupConf);

		auto&& msg = GetMessageFromParser(sip);

		ASSERT_TRUE(msg);
		EXPECT_TRUE(msg->IsValid());
		ASSERT_EQ(100, msg->GetSIPMetaField()->iStartLine->GetResponseCode());

		msg = GetMessageFromParser(sip);

		ASSERT_TRUE(msg);
		EXPECT_TRUE(msg->IsValid());
		ASSERT_EQ(180, msg->GetSIPMetaField()->iStartLine->GetResponseCode());

		msg = GetMessageFromParser(sip);

		ASSERT_TRUE(msg);
		EXPECT_TRUE(msg->IsValid());
		ASSERT_EQ(200, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
		ASSERT_STREQ(msg->GetSDPMetaField()->iOrigin->UserName().c_str(), conf_param.expectedOrigin);
	}

	TEST_F(SIPParserTest, Register_NotOurNonce)		// Re-Registration  with re-calc digest
	{
		std::string response = raw_sip_message_not_our_nonce;
		ASSERT_TRUE(SetRecvBuf(&response[0], response.size()));
		auto msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		auto h = msg->GetSIPMetaField()->iStartLine;
		EXPECT_NE(nullptr, h);
		ASSERT_EQ(h->GetResponseCode() , 401);
	}

	struct TestSIPRegMaxExpires : public SIPParserTest
	{
		static const std::chrono::seconds MAX_EXPIRES;

		void SetUp() override
		{
			VS_RegistryKey key{ false, SIP_PEERS_KEY, false, true };
			ASSERT_TRUE(key.IsValid());
			const int32_t val = static_cast<int32_t>(MAX_EXPIRES.count());
			ASSERT_TRUE(key.SetValue(&val, sizeof(val), VS_REG_INTEGER_VT, REGISTRATION_MAX_EXPIRES));

			SIPParserTest::SetUp();
		}
		void TearDown() override
		{
			VS_RegistryKey key{ false, SIP_PEERS_KEY, false };
			EXPECT_TRUE(key.RemoveKey(REGISTRATION_MAX_EXPIRES));

			SIPParserTest::TearDown();
		}
	};

	const std::chrono::seconds TestSIPRegMaxExpires::MAX_EXPIRES{ 10 };

	TEST_F(TestSIPRegMaxExpires, RegMaxExpires_Bug55940)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ASSERT_TRUE(SetRecvBuf(raw_cisco_e30_register, sizeof(raw_cisco_e30_register) - 1));
		auto msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_FALSE(msg->GetSIPMetaField()->iAuthHeader.empty());
		VS_SIPField_Auth *AuthHeader = msg->GetSIPMetaField()->iAuthHeader[0];
		ASSERT_TRUE(AuthHeader != NULL);
		ASSERT_TRUE(AuthHeader->GetAuthInfo() != NULL);

		// add fake credentials
		VS_SIPObjectFactory *factory = VS_SIPObjectFactory::Instance();
		ASSERT_TRUE(factory != NULL);
		AuthHeader->GetAuthInfo()->login("user");
		AuthHeader->GetAuthInfo()->password("pass");
		AuthHeader->GetAuthInfo()->uri("sip:user@host");
		AuthHeader->GetAuthInfo()->method(VS_SIPObjectFactory::GetMethod(TYPE_REGISTER));
		factory->CalcDigestResponse(AuthHeader->GetAuthInfo().get());

		// compose a register message with Authorization header
		char buff[1024] = { 0 };
		size_t buff_sz = 1024;
		VS_SIPBuffer tmp_buff;
		AuthHeader->Encode(tmp_buff);
		tmp_buff.AddData("\r\n");
		tmp_buff.GetNextBlock(buff, buff_sz);

		std::string auth_header = "\r\n"; auth_header += buff; auth_header += "\r\n\r\n";
		auth_header.replace(auth_header.find("WWW-Authenticate"), strlen("WWW-Authenticate"), "Authorization");

		std::string authorization_mgs = raw_cisco_e30_register;
		authorization_mgs.replace(authorization_mgs.end() - 4, authorization_mgs.end(), auth_header);		// replace last "\r\n\r\n" with Authorization

		bool TriedToCheckDigest(false);
		sip->SetDigestChecker([&TriedToCheckDigest](const std::string &, const std::string &)
		{
			TriedToCheckDigest = true;
			return true;
		});

		EXPECT_CALL(*confProtocol, LoginUser(_, _, _, _, _, _, _, _))
			.WillOnce(Invoke([](string_view /*dialogId*/, string_view /*login*/, string_view /*password*/, std::chrono::steady_clock::time_point /*expireTime*/, string_view /*externalName*/,
				boost::function<void(bool) > result, boost::function<void(void)> /*logout*/, const std::vector<std::string> &/*h323Aliases*/)
				{
					result(true);			// logged in ok (without any check for password)
				}
		));

		ASSERT_TRUE(SetRecvBuf(&authorization_mgs[0], authorization_mgs.size()));
		ASSERT_TRUE(TriedToCheckDigest);
	
		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_TRUE(msg->GetSIPMetaField()->iExpires);

		ASSERT_EQ(msg->GetSIPMetaField()->iExpires->Value(), MAX_EXPIRES);
	}

	// bug#31588: when change sip password at Cisco E20, she re-registers from another port, but with nonce-value from previous registration
	TEST_F(SIPParserTest, Register_OurNonceFromAnotherContext)
	{
		using ::testing::_;
		using ::testing::Invoke;
		using ::testing::NiceMock;

		// get new OurNonce from parser
		std::string msg1 = raw_cisco_e30_register;
		ASSERT_TRUE(SetRecvBuf(&msg1[0], msg1.size()));
		auto msg2 = GetMessageFromParser(sip);
		ASSERT_TRUE(msg2->IsValid());
		ASSERT_FALSE(msg2->GetSIPMetaField()->iAuthHeader.empty());
		VS_SIPField_Auth* AuthHeader = msg2->GetSIPMetaField()->iAuthHeader[0];
		ASSERT_TRUE(AuthHeader != NULL);
		ASSERT_TRUE(AuthHeader->GetAuthInfo() != NULL);
		auto &&nonce = AuthHeader->GetAuthInfo()->nonce();

		// add fake credentials
		VS_SIPObjectFactory* factory = VS_SIPObjectFactory::Instance();
		AuthHeader->GetAuthInfo()->login("user");
		AuthHeader->GetAuthInfo()->password("pass");
		AuthHeader->GetAuthInfo()->uri("sip:user@host");
		AuthHeader->GetAuthInfo()->method(factory->GetMethod(TYPE_REGISTER));
		factory->CalcDigestResponse(AuthHeader->GetAuthInfo().get());

		// compose a register message with Authorization header
		char buff[1024] = { 0 };
		size_t buff_sz = 1024;
		VS_SIPBuffer tmp_buff;
		AuthHeader->Encode(tmp_buff);
		tmp_buff.AddData("\r\n");
		tmp_buff.GetNextBlock(buff, buff_sz);

		std::string auth_header = "\r\n"; auth_header += buff; auth_header += "\r\n\r\n";
		auth_header.replace(auth_header.find("WWW-Authenticate"), strlen("WWW-Authenticate"), "Authorization");

		std::string msg3 = raw_cisco_e30_register;
		msg3.replace(msg3.end()-4, msg3.end(), auth_header);		// replace last "\r\n\r\n" with Authorization

		// make another sip context
		auto sip2 = vs::MakeShared<VS_SIPParser>(strand, "serverVendor", nullptr);
		sip2->UseACL(false);
		net::Endpoint myaddr{ net::address_v4(1) , 1, net::protocol::UDP };
		sip2->SetMyCsAddress(myaddr);
		sip2->SetPeerCSAddress("", myaddr);
		auto call_config = vs::MakeShared<VS_CallConfigStorage>();
		call_config->RegisterProtocol(identifier);
		sip2->SetCallConfigStorage(call_config);
		sip2->SetConfCallBack(confProtocol);
		sip2->SetPolicy(boost::make_shared<VS_Policy>("SIP"));
		bool TriedToCheckDigest(false);
		sip2->SetDigestChecker([&TriedToCheckDigest](const std::string&, const std::string&)
		{
			TriedToCheckDigest = true;
			return true;
		});

		sip_transport->ResetParser(sip2);
		ASSERT_TRUE(SetRecvBuf(&msg3[0], msg3.size()));
		ASSERT_TRUE(TriedToCheckDigest);	// the goal of a UnitTest: second parser should accept old our nonce and call func to check digest responce
	}

	// todo(kt): disabled because need to test VS_TransportConnection + VS_TranscoderControl (not just SIPParser)
	TEST_F(SIPParserTest, DISABLED_ReRegister_Test1)		// Re-Registration with re-calc digest
	{
		using ::testing::_;
		using ::testing::AnyNumber;
		using ::testing::DoAll;
		using ::testing::Invoke;
		using ::testing::InvokeArgument;
		using ::testing::Return;
		using ::testing::SaveArg;
		using ::testing::StrEq;

		//std::string response = raw_sip_message_re_register_1;

		//sip->SetRecvBuf(&response[0], response.size(), sip->GetDefaultChannelID(), net::Endpoint(), net::Endpoint());

		//auto msg = GetMessageFromParser(sip);
		//ASSERT_TRUE(msg->IsValid());

		//VS_SIPField_Auth* AuthHeader = msg->GetSIPMetaField()->iAuthHeader;
		//ASSERT_TRUE(AuthHeader != NULL);
		//ASSERT_TRUE(AuthHeader->GetAuthInfo() != NULL);
		//VS_SIPAuthInfo* auth_info = AuthHeader->GetAuthInfo();
		//ASSERT_NE(nullptr, AuthHeader->GetAuthInfo()->nonce());
		//std::string nonce = AuthHeader->GetAuthInfo()->nonce();
		//
		VS_SIPObjectFactory* factory = VS_SIPObjectFactory::Instance();
		ASSERT_NE(nullptr, factory);
		//auth_info->method(factory->GetMethod(TYPE_REGISTER));
		//ASSERT_TRUE(factory->CalcDigestResponse(auth_info));

		auto &&dialog_id = sip->NewDialogID("to_name", {});
		VS_SIPParserInfo info("serverVendor");
		info.SetSIPRemoteTarget(std::string("user@host"));
		info.SetMyCsAddress({ net::address::from_string("1.1.1.1"), 5060, net::protocol::TCP });
		info.SetAliasMy("user@host");
		info.SetAliasRemote("user@host");
		info.SIPDialogID(dialog_id);
		info.SetExpires(std::chrono::seconds(300));
		VS_SIPRequest req;

		const VS_SIPGetInfoImpl get_info(info);
		VS_SIPUpdateInfoImpl update_info(info);

		ASSERT_TRUE(req.MakeREGISTER(get_info, update_info, VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization));
		std::string data;
		ASSERT_EQ(req.Encode(data), TSIPErrorCodes::e_ok);
		EXPECT_NE(0, data.length());
		ASSERT_TRUE(SetRecvBuf(data.c_str(), data.length()));

		std::string nonce;
		{// save auth to ctx
			// should get "401 Unauthorized" with nonce value
			auto msg = GetMessageFromParser(sip);
			ASSERT_TRUE(msg->IsValid());
			{// todo(kt): check for "401 UnAuth" and for nonce value exist
				auto h = msg->GetSIPMetaField()->iStartLine;
				EXPECT_NE(nullptr, h);
			}

			ASSERT_FALSE(msg->GetSIPMetaField()->iAuthHeader.empty());
			auto h = msg->GetSIPMetaField()->iAuthHeader[0];
			EXPECT_NE(nullptr, h);
			EXPECT_NE(nullptr, h->GetAuthInfo());
			nonce = h->GetAuthInfo()->nonce();
			auto d = std::make_shared<VS_SIPAuthDigest>();
			*((VS_SIPAuthInfo*)d.get()) = *(h->GetAuthInfo());

			// set fake nonce
//			d->nonce(std::string(nonce + "fake").c_str());

			info.SetAuthScheme(d);
			EXPECT_NE(nullptr, info.GetAuthScheme());

			// add credentials
			info.GetAuthScheme()->login("user");
			info.GetAuthScheme()->password("pass");
			info.GetAuthScheme()->uri("sip:user@host");
			info.GetAuthScheme()->method(factory->GetMethod(TYPE_REGISTER));

			// Register with auth_info
			ASSERT_TRUE(req.MakeREGISTER(get_info, update_info, VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization));
			ASSERT_EQ(req.Encode(data), TSIPErrorCodes::e_ok);
			EXPECT_NE(0, data.length());
		}

		EXPECT_CALL(*confProtocol, LoginUser(_, _, _, _, _, _, _, _))
			.WillOnce(Invoke([](string_view dialogId, string_view login, string_view password, std::chrono::steady_clock::time_point expireTime, string_view externalName,
				boost::function<void(bool) > result, boost::function<void(void)> logout, const std::vector<std::string>& h323Aliases)
			{
				result(true);			// logged in ok (without any check for password)
			}
		));
		ASSERT_TRUE(SetRecvBuf(data.c_str(), data.length()));

		auto msg2 = GetMessageFromParser(sip);
		ASSERT_TRUE(msg2->IsValid());

		{ // check that auth with incorrect nonce is not valid (error code = 403)
			auto h = msg2->GetSIPMetaField()->iStartLine;
			EXPECT_NE(nullptr, h);
			ASSERT_TRUE(h->GetResponseCode() != 403);
		}
	}

	TEST_F(SIPParserTest, tcc2sip_invite_with_session_timer)
	{
		using ::testing::AtLeast;
		using ::testing::StrEq;

		VS_CallConfig config;
		config.Address.addr = net::address_v4(1);
		config.Address.port = 1;
		config.Address.protocol = net::protocol::TCP;
		config.sip.SessionTimers.Enabled = true;

		config.HostName = "hostname1";
		config.sip.FromDomain = "hostname3";
		auto &&dialog_id = sip->NewDialogID("to_name", {}, config);

		VS_SIPParserInfo info("serverVendor");
		info.SetSIPRemoteTarget(std::string("user@host"));
		info.SetMyCsAddress({net::address::from_string("1.1.1.1"), 5060, net::protocol::TCP });
		info.SetContactHost("192.168.0.1");
		info.SetAliasMy("qwe");
		info.SetTagMy("rty");
		info.SetDisplayNameMy("uio");
		info.SetAliasRemote("asd");
		info.SetTagSip("fgh");
		info.SetDisplayNameSip("jkl");
		info.SetExpires(std::chrono::seconds(1800));
		info.SetContentType(CONTENTTYPE_SDP);
		info.SIPDialogID(dialog_id);
		info.SetViaHost("192.168.0.0");
		info.SetMyMediaAddress(net::address::from_string("1.2.3.4"));
		info.IsRequest(true);
		info.GetTimerExtention().refresher = REFRESHER::REFRESHER_UAC;
		info.GetTimerExtention().refreshPeriod = std::chrono::seconds(1800); //Registry Key - "Refresh Period", default - 1800
		info.GetTimerExtention().lastUpdate = sip->clock().now();
		info.SetListenPort(5060);
		info.SetConfig(config);
		std::string data;
		VS_SIPRequest req;

		const VS_SIPGetInfoImpl get_info(info);
		VS_SIPUpdateInfoImpl update_info(info);

		ASSERT_TRUE(req.MakeINVITE(get_info, update_info));
		req.FillInfoByRequest(get_info, update_info);
		ASSERT_EQ(req.Encode(data), TSIPErrorCodes::e_ok);
		EXPECT_NE(0, data.length());
		ASSERT_TRUE(strstr(data.c_str(), "Session-Expires"));

		info.UseSessionTimer();
		//info.EnableSessionTimer();
		req.InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, get_info); //for insert header Via (see sip::TransportLayer)

		sip->InviteMethod(dialog_id, "from_name", "to_name", VS_ConferenceInfo(false, false));
		auto iv_msg = GetMessageFromParser(sip);
		req.GetSIPMetaField()->iVia[0]->Branch(std::string(iv_msg->Branch()));
		//INVITE
		ASSERT_TRUE(iv_msg->IsValid());
		ASSERT_EQ(iv_msg->Encode(data), TSIPErrorCodes::e_ok);
		ASSERT_NE(iv_msg->GetSIPMetaField()->iSessionExpires, nullptr);
		auto inviteReq = dynamic_cast<VS_SIPRequest*>(iv_msg.get());
		ASSERT_NE(inviteReq, nullptr);
		auto rsp = std::make_shared<VS_SIPResponse>();
		rsp->MakeOnInviteResponseOK(inviteReq, get_info, update_info);
		rsp->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, get_info);
		//200 OK
		ASSERT_TRUE(rsp->IsValid());
		ASSERT_EQ(rsp->Encode(data), TSIPErrorCodes::e_ok);
		ASSERT_TRUE(strstr(data.c_str(), "200 OK"));

		sip->SetRecvMsg_SIP(rsp, { net::address::from_string("212.13.98.248"), 5060, net::protocol::TCP });
		iv_msg = GetMessageFromParser(sip);
		//ACK
		ASSERT_TRUE(iv_msg->IsValid());
		ASSERT_EQ(iv_msg->Encode(data), TSIPErrorCodes::e_ok);
		ASSERT_TRUE(strstr(data.c_str(), "ACK"));

		iv_msg = GetMessageFromParser(sip);
		ASSERT_FALSE(iv_msg);
		auto timeDelta = std::chrono::seconds(1000);
		//refresh period = 1800s, see VS_SIPParser::Timeout()
		sip->clock().add_diff(timeDelta);
		ASSERT_TRUE(timeDelta > info.GetTimerExtention().refreshPeriod / 2); //if the condition is true in Timeout() is called new InviteMethod;
																			 //otherwise iv_msg->IsValid() throw exception
		sip->Timeout();
		iv_msg = GetMessageFromParser(sip);
		//Refresh Invite
		ASSERT_TRUE(iv_msg->IsValid());
		ASSERT_EQ(iv_msg->Encode(data), TSIPErrorCodes::e_ok);
		ASSERT_TRUE(strstr(data.c_str(), "INVITE"));
		ASSERT_TRUE(strstr(data.c_str(), "Session-Expires"));
		//call termination testing
		EXPECT_CALL(*confProtocol, Hangup(::testing::Eq<string_view>(dialog_id)))
			.Times(AtLeast(1));
		timeDelta = std::chrono::seconds(1800);
		sip->clock().add_diff(timeDelta);
		ASSERT_TRUE(timeDelta > info.GetTimerExtention().refreshPeriod / 2);
		sip->Timeout();
		iv_msg = GetMessageFromParser(sip);
		ASSERT_TRUE(iv_msg->IsValid());
		ASSERT_EQ(iv_msg->Encode(data), TSIPErrorCodes::e_ok);
		ASSERT_TRUE(strstr(data.c_str(), "BYE"));
	}


	TEST_F(SIPParserTest, Crash_Bug38067)
	{
		auto &&dialog_id = sip->NewDialogID("to_name", {});
		VS_SIPParserInfo info( "serverVendor");
		info.SetSIPRemoteTarget(std::string("user@host"));
		info.SetMyCsAddress({net::address::from_string("1.1.1.1"), 5060, net::protocol::TCP });
		info.SetAliasMy("user@host");
		info.SetAliasRemote("user@host");
		info.SIPDialogID(dialog_id);
		info.SetExpires(std::chrono::seconds(300));

		auto digest_info = std::make_shared<VS_SIPAuthDigest>();
		digest_info->nonce("nonce");
		digest_info->login("login");
		digest_info->realm("realm");
		digest_info->password("password");
		digest_info->uri("uri");

		info.SetAuthScheme(digest_info);

		VS_SIPRequest req;
		const VS_SIPGetInfoImpl get_info(info);
		VS_SIPUpdateInfoImpl update_info(info);

		ASSERT_TRUE(req.MakeREGISTER(get_info, update_info, VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization));

		digest_info.reset();		// this is actual test: shoud not crash after deleted

		std::string data;
		ASSERT_EQ(req.Encode(data), TSIPErrorCodes::e_ok);
		EXPECT_NE(0, data.length());
	}

	TEST_F(SIPParserTest, Bug32183)
	{
		ASSERT_TRUE(SetRecvBuf(raw_rockwareC9_options, sizeof(raw_rockwareC9_options)));

		auto msg = GetMessageFromParser(sip);
		ASSERT_NE(msg, nullptr);
		ASSERT_EQ(msg->GetMessageType(), MESSAGE_TYPE_RESPONSE);
		ASSERT_EQ(msg->GetMethod(), TYPE_OPTIONS);
	}

	void EncodeAndCheckHeader(const VS_SIPMessage& msg, std::string& data)
	{
		data = "";
		ASSERT_EQ(msg.Encode(data), TSIPErrorCodes::e_ok);
		EXPECT_NE(0, data.length());
		ASSERT_TRUE(strstr(data.c_str(), "User-Agent"));
	}

	TEST_F(SIPParserTest, UserAgentHeader)
	{
		auto &&dialog_id = sip->NewDialogID("to_name", {}, {});
		VS_SIPParserInfo info( "serverVendor");
		info.SetSIPRemoteTarget(std::string("user@host"));
		info.SetMyCsAddress({net::address::from_string("1.1.1.1"), 5060, net::protocol::TCP });
		info.SetAliasMy("user@host");
		info.SetAliasRemote("user@host");
		info.SetMyMediaAddress(net::address::from_string("1.2.3.4"));
		info.SIPDialogID(dialog_id);
		info.SetViaHost("192.168.0.0");
		info.SetExpires(std::chrono::seconds(300));

		std::string data;
		data.reserve(3000);
		VS_SIPRequest req;
		VS_SIPResponse rsp;
		const VS_SIPGetInfoImpl get_info(info);
		VS_SIPUpdateInfoImpl update_info(info);

		//requests

		ASSERT_TRUE(req.MakeACK(get_info, update_info,false));
		EncodeAndCheckHeader(req, data);

		ASSERT_TRUE(req.MakeBYE(get_info, update_info));
		EncodeAndCheckHeader(req, data);

		ASSERT_TRUE(req.MakeCANCEL(get_info, update_info));
		EncodeAndCheckHeader(req, data);

		ASSERT_TRUE(req.MakeINFO_DTMF(get_info, update_info,'c'));
		EncodeAndCheckHeader(req, data);

		ASSERT_TRUE(req.MakeINFO_FastUpdatePicture(get_info, update_info));
		EncodeAndCheckHeader(req, data);

		ASSERT_TRUE(req.MakeMESSAGE(get_info, update_info,"message"));
		EncodeAndCheckHeader(req, data);

		ASSERT_TRUE(req.MakeNOTIFY(get_info, update_info));
		EncodeAndCheckHeader(req, data);

		ASSERT_TRUE(req.MakeOPTIONS(get_info, update_info,true));
		EncodeAndCheckHeader(req, data);

		ASSERT_TRUE(req.MakeRefreshINVITE(get_info, update_info));
		EncodeAndCheckHeader(req, data);

		ASSERT_TRUE(req.MakeREGISTER(get_info, update_info));
		EncodeAndCheckHeader(req, data);

		ASSERT_TRUE(req.MakeINVITE(get_info, update_info));
		EncodeAndCheckHeader(req, data);
		//for valid response
		req.FillInfoByRequest(get_info, update_info);
		req.InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, get_info);
		sip->InviteMethod(dialog_id, "from_name", "to_name", VS_ConferenceInfo(false, false));
		auto iv_msg = GetMessageFromParser(sip);
		req.GetSIPMetaField()->iVia[0]->Branch(std::string(iv_msg->Branch()));

		//responses
		ASSERT_TRUE(rsp.MakeMovedPermanently(get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnByeResponseOK(get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnCancelResponseOK(get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnInfoResponseOK(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnInfoResponseUnsupported(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnInviteResponseBusyHere(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnInviteResponseOK(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnInviteResponseRinging(get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnInviteResponseUnauthorized(&req, get_info, update_info, std::string()));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnMessageResponseAccepted(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnMessageResponseNotFound(get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnMessageResponseOK(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnOptionsResponseOK(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnRegisterResponseForbidden(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnRegisterResponseOK(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnRegisterResponseUnauthorized(&req, get_info, update_info, std::string()));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnSubscribeResponseOK(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeOnUpdateResponseOK(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeRequestTimeout(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeResponseUnsupported(&req, get_info, update_info));
		EncodeAndCheckHeader(rsp, data);

		ASSERT_TRUE(rsp.MakeResponseUseProxy(&req, get_info, update_info, std::string()));
		EncodeAndCheckHeader(rsp, data);
	}
#ifdef _WIN32
	TEST_F(SIPParserTest, IPv6CallResolve)
	{
		const std::string userAgent = "serverVendor";
		bool before = VS_GatewayStarter::IsVCS();
		VS_GatewayStarter::SetIsVCS(true);

		auto cfgStor = vs::MakeShared<VS_CallConfigStorage>();
		VS_UserData user;
		user.m_name = "user";
		cfgStor->RegisterProtocol(VS_Indentifier::GetCommonIndentifierChain(g_asio_environment->IOService(), userAgent));
		cfgStor->UpdateSettings();

		WSAData wsaData;
		WSAStartup(0x0202, &wsaData);

		VS_CallConfig config;
		bool res_resolve = cfgStor->Resolve(config, "#h323:hdx8000@[fd00:7:495:0:2e0:dbff:fe08:aac0]", &user);

		ASSERT_TRUE(res_resolve);
		ASSERT_EQ(VS_CallConfig::H323, config.SignalingProtocol);
		auto address = config.Address;
		ASSERT_TRUE(address.addr.is_v6());
		ASSERT_EQ(1720, address.port);

		cfgStor->RegisterProtocol(boost::make_shared<VS_IndentifierSIP>(g_asio_environment->IOService(), userAgent));
		config = {}; //reset
		res_resolve = cfgStor->Resolve(config, "#sip:hdx8000@[fd00:7:495:0:2e0:dbff:fe08:aac0]", &user);
		WSACleanup();

		ASSERT_TRUE(res_resolve);
		ASSERT_EQ(VS_CallConfig::SIP, config.SignalingProtocol);
		address = config.Address;
		ASSERT_TRUE(address.addr.is_v6());
		ASSERT_EQ(5060, address.port);

		VS_GatewayStarter::SetIsVCS(before);
	}
#endif

	TEST_F(SIPParserTest, IPv6Outgoing)
	{
		using ::testing::_;
		using ::testing::Return;

		VS_CallConfig config;
		// config->sip.SkipOPTIONS = true;
		config.SignalingProtocol = VS_CallConfig::SIP;
		std::string dialogId = sip->NewDialogID("#sip:hdx8000@[fd00:7:495:0:2e0:dbff:fe08:aac0]", "", config);

		EXPECT_CALL(*confProtocol, InviteReplay(testing::Eq(dialogId), e_call_ok, _, _, _))
			.WillOnce(Return(true));
		EXPECT_CALL(*confProtocol, Hangup(_))
			.Times(0);

		sip->InviteMethod(dialogId, "b@brchk000.trueconf.ua", "#sip:hdx8000@[fd00:7:495:0:2e0:dbff:fe08:aac0]", VS_ConferenceInfo(false, false));
		auto msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(MESSAGE_TYPE_REQUEST, msg->GetSIPMetaField()->iStartLine->GetMessageType());
		ASSERT_EQ(TYPE_INVITE, msg->GetSIPMetaField()->iCSeq->GetType());

		std::string response = raw_hdx8000_ipv6_ok;
		strreplace(response, "__callid__", msg->CallID());
		ASSERT_TRUE(!msg->GetSIPMetaField()->iVia.empty());
		strreplace(response, "__branch__", msg->GetSIPMetaField()->iVia[0]->Branch());
		ASSERT_TRUE(SetRecvBuf(&response[0], response.length()));

		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(TYPE_ACK, msg->GetSIPMetaField()->iCSeq->GetType());
	}

	TEST_F(SIPParserTest, IPv6Incoming)
	{
		using ::testing::_;
		using ::testing::InvokeArgument;
		using ::testing::DoAll;
		using ::testing::WithArg;
		using ::testing::Invoke;

		std::string dialog_id;

		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, _, _))
			.WillOnce(DoAll(InvokeArgument<4>(false, VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE, ""), WithArg<0>(Invoke([&dialog_id](string_view a) { dialog_id = std::string(a); }))));

		ASSERT_TRUE(SetRecvBuf(&raw_hdx8000_ipv6_invite[0], sizeof(raw_hdx8000_ipv6_invite)));
		sip->InviteReplay(dialog_id, e_call_ok, false);

		auto msg = GetMessageFromParser(sip); //100 Trying
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(100, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(180, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(200, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
	}

	TEST_F(SIPParserTest, Bug30235_1) // no registration, neither "fromdomain" nor "SIP From Host" set +  Administrator call
	{
		auto orig_tr_endpoint_name = g_tr_endpoint_name;
		VS_SCOPE_EXIT { g_tr_endpoint_name = orig_tr_endpoint_name; };

		VS_CallConfig config;
		config.Address.addr = net::address_v4(1);
		config.Address.port = 1;
		config.Address.protocol = net::protocol::UDP;

		config.HostName = "hostname1";

		std::string from_name = "from_name1";
		std::string from_domain = "hostname1";

		VS_RegistryKey cfg(false, CONFIGURATION_KEY, false);
		cfg.RemoveValue(SIP_FROM_HOST);

		for (const auto& server_name_lost : {true,false})
		{
			if (server_name_lost) g_tr_endpoint_name = "";
			else g_tr_endpoint_name = our_endpoint;
			for (const auto& admin_call : { true,false })
			{
				std::string from = admin_call ? "" : from_name + "@";
				auto &&dialog_id = sip->NewDialogID("to_name", {}, config);
				sip->InviteMethod(dialog_id, from + from_domain, "to_name", VS_ConferenceInfo(false, false));

				auto msg = GetMessageFromParser(sip);
				ASSERT_NE(nullptr, msg);
				auto uri = msg->GetSIPMetaField()->iFrom->GetURI();
				ASSERT_NE(nullptr, uri);

				if(admin_call)	ASSERT_STREQ(uri->User().c_str(), "Administrator");
				else			ASSERT_STREQ(uri->User().c_str(), "from_name1");
				ASSERT_TRUE(!uri->Host().empty());

				if (server_name_lost)	ASSERT_STREQ(uri->Host().c_str(), from_domain.c_str());				// if server_name was lost some how take host from call_id
				else					ASSERT_STREQ(uri->Host().c_str(), our_endpoint.c_str());
			}
		}
	}


	TEST_F(SIPParserTest, Bug30235_2) // no registration "SIP From Host" set
	{
		VS_CallConfig config;
		config.Address.addr = net::address_v4(1);
		config.Address.port = 1;
		config.Address.protocol = net::protocol::UDP;

		config.HostName = "hostname1.com";

		VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
		cfg.SetString("hostname2.com", SIP_FROM_HOST);

		auto &&dialog_id = sip->NewDialogID("to_name", {}, config);
		sip->InviteMethod(dialog_id, "from_name1@hostname1.com", "to_name", VS_ConferenceInfo(false, false));

		auto msg = GetMessageFromParser(sip);
		ASSERT_NE(nullptr, msg);
		auto uri = msg->GetSIPMetaField()->iFrom->GetURI();
		ASSERT_NE(nullptr, uri);

		ASSERT_STREQ(uri->User().c_str(), "from_name1");
		ASSERT_STREQ(uri->Host().c_str(), "hostname2.com");
	}

	TEST_F(SIPParserTest, Bug30235_3) // no registration, "fromdomain" and "SIP From Host" set
	{
		VS_CallConfig config;
		config.Address.addr = net::address_v4(1);
		config.Address.port = 1;
		config.Address.protocol = net::protocol::UDP;

		config.HostName = "hostname1";

		config.sip.FromUser = "from_name3";
		config.sip.FromDomain = "hostname3";

		VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
		cfg.SetString("hostname2", SIP_FROM_HOST);
		VS_SCOPE_EXIT { cfg.RemoveValue(SIP_FROM_HOST); };

		auto &&dialog_id = sip->NewDialogID("to_name", {}, config);
		sip->InviteMethod(dialog_id, "from_name1@hostname1", "to_name", VS_ConferenceInfo(false, false));

		auto msg = GetMessageFromParser(sip);
		ASSERT_NE(nullptr, msg);
		auto uri = msg->GetSIPMetaField()->iFrom->GetURI();
		ASSERT_NE(nullptr, uri);

		ASSERT_STREQ(uri->User().c_str(), "from_name3");
		ASSERT_STREQ(uri->Host().c_str(), "hostname3");
	}

	TEST_F(SIPParserTest, Bug30235_4) // no registration, "fromdomain" w/o "fromuser" and "SIP From Host" set
	{
		VS_CallConfig config;
		config.Address.addr = net::address_v4(1);
		config.Address.port = 1;
		config.Address.protocol = net::protocol::UDP;

		config.HostName = "hostname1";

		config.sip.FromDomain = "hostname3";

		VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
		cfg.SetString("hostname2", SIP_FROM_HOST);
		VS_SCOPE_EXIT { cfg.RemoveValue(SIP_FROM_HOST); };

		auto &&dialog_id = sip->NewDialogID("to_name", {}, config);
		sip->InviteMethod(dialog_id, "from_name1@hostname1", "to_name", VS_ConferenceInfo(false, false));

		auto msg = GetMessageFromParser(sip);
		ASSERT_NE(nullptr, msg);
		auto uri = msg->GetSIPMetaField()->iFrom->GetURI();
		ASSERT_NE(nullptr, uri);

		ASSERT_STREQ(uri->User().c_str(), "from_name1");
		ASSERT_STREQ(uri->Host().c_str(), "hostname3");
	}

	TEST_F(SIPParserTest, Bug30235_5) // with registration, "fromdomain" and "SIP From Host" set
	{
		VS_CallConfig config;
		config.Address.addr = net::address_v4(1);
		config.Address.port = 1;
		config.Address.protocol = net::protocol::UDP;

		config.Login = "username1";
		config.HostName = "hostname";

		config.sip.FromUser = "from_name3";
		config.sip.FromDomain = "hostname3";

		VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
		cfg.SetString("hostname2", SIP_FROM_HOST);
		VS_SCOPE_EXIT { cfg.RemoveValue(SIP_FROM_HOST); };

		auto &&dialog_id = sip->NewDialogID("to_name", {}, config);
		sip->InviteMethod(dialog_id, "from_name1@hostname", "to_name", VS_ConferenceInfo(false, false));

		auto msg = GetMessageFromParser(sip);
		ASSERT_NE(nullptr, msg);
		auto uri = msg->GetSIPMetaField()->iFrom->GetURI();
		ASSERT_NE(nullptr, uri);

		ASSERT_STREQ(uri->User().c_str(), "username1");
		ASSERT_STREQ(uri->Host().c_str(), "hostname");
	}

	TEST_F(SIPParserTest, Bug_33043)
	{
		VS_CallConfig config;
		config.Login = "201$vpbx471200081";
		config.HostName = "188.187.220.27";
		config.Password = "sqj2lnmi";

		config.sip.RegistrationBehavior = VS_CallConfig::REG_REGISTER_ALWAYS;
		config.sip.AuthName = "201";

		sip->UpdateRegisterContext(config);

		auto msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg);

		std::string resp = bug_33043_unauthorized;

		auto &&branch = msg->Branch();
		ASSERT_TRUE(!branch.empty());
		strreplace(resp, "__branch__", branch);

		auto &&from_tag = msg->GetSIPMetaField()->iFrom->GetURI()->Tag();
		ASSERT_TRUE(!from_tag.empty());
		strreplace(resp, "__fromtag__", from_tag);

		auto &&call_id = msg->CallID();
		strreplace(resp, "__callid__", call_id);
		ASSERT_TRUE(SetRecvBuf(resp.c_str(), resp.length()));

		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg);

		auto info = msg->GetAuthInfo();
		ASSERT_TRUE(info != nullptr);
		ASSERT_STREQ(info->login().c_str(), "201");
		ASSERT_STREQ(info->uri().c_str(), "sip:201$vpbx471200081@188.187.220.27");
		ASSERT_STREQ(info->nonce().c_str(), "AD014D7D");
		ASSERT_STREQ(info->response().c_str(), "5e274f2f072430d143d144255676988b");
	}

	TEST_F(SIPParserTest, UpdateTagAndEpid) {
		const char* to = "#sip:vsx7000@192.168.62.41";
		const char* from = "b@brchk000.trueconf.ua";
		VS_CallConfig config; config.SignalingProtocol = VS_CallConfig::SIP;
		auto &&dialogId = sip->NewDialogID(to, {}, config);

		sip->InviteMethod(dialogId, from, to, VS_ConferenceInfo(false, false));
		auto msg = GetMessageFromParser(sip);
		ASSERT_NE(msg, nullptr);
		ASSERT_TRUE(msg->IsValid());
		EXPECT_EQ(MESSAGE_TYPE_REQUEST, msg->GetSIPMetaField()->iStartLine->GetMessageType());
		EXPECT_EQ(TYPE_INVITE, msg->GetSIPMetaField()->iCSeq->GetType());

		std::string ringing = raw_vsx7000_ringing;
		VS_ReplaceAll(ringing, "tag=plcm_100717000-17925415", "tag=old_tag;epid=123");

		std::string _200ok = raw_vsx7000_ok;
		VS_ReplaceAll(_200ok, "tag=plcm_100717000-17925415", "tag=new_tag;epid=321");

		std::string response;
		ASSERT_TRUE(!msg->GetSIPMetaField()->iVia.empty());
		ConstructResponse(response, ringing, msg->CallID(), msg->GetSIPMetaField()->iVia[0]->Branch());
		ASSERT_TRUE(SetRecvBuf(&response[0], response.length()));

		ConstructResponse(response, _200ok, msg->CallID(), msg->GetSIPMetaField()->iVia[0]->Branch());
		ASSERT_TRUE(SetRecvBuf(&response[0], response.length()));

		auto ctxs = GetInternalContextsTest();
		auto it = ctxs.find(dialogId);
		ASSERT_NE(it, ctxs.end());
		EXPECT_STREQ(it->second->GetTagSip().c_str(), "new_tag");
		EXPECT_STREQ(it->second->GetEpidSip().c_str(), "321");
	}

	TEST_P(SIPParserOutgoingTest, Outgoing)
	{
		using ::testing::_;
		using ::testing::Return;

		auto param = GetParam();

		// signal part

		VS_CallConfig config;
		// config->sip.SkipOPTIONS = true;
		config.SignalingProtocol = VS_CallConfig::SIP;
		std::string dialogId = sip->NewDialogID(param.to, {}, config);

		EXPECT_CALL(*confProtocol, InviteReplay(testing::Eq(dialogId), e_call_ok, _, _, _))
			.WillOnce(Return(true));
		EXPECT_CALL(*confProtocol, Hangup(_))
			.Times(0);

		sip->InviteMethod(dialogId, param.from, param.to, VS_ConferenceInfo(false, false));
		auto msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(MESSAGE_TYPE_REQUEST, msg->GetSIPMetaField()->iStartLine->GetMessageType());
		ASSERT_EQ(TYPE_INVITE, msg->GetSIPMetaField()->iCSeq->GetType());

		std::string response;

		if (param.trying)
		{
			ConstructResponse(response, param.trying, msg->CallID(), msg->GetSIPMetaField()->iVia[0]->Branch());
			ASSERT_TRUE(SetRecvBuf(&response[0], response.length()));
		}

		if (param.ringing)
		{
			ASSERT_TRUE(!msg->GetSIPMetaField()->iVia.empty());
			ConstructResponse(response, param.ringing, msg->CallID(), msg->GetSIPMetaField()->iVia[0]->Branch());
			ASSERT_TRUE(SetRecvBuf(&response[0], response.length()));
		}

		ConstructResponse(response, param.ok, msg->CallID(), msg->GetSIPMetaField()->iVia[0]->Branch());
		ASSERT_TRUE(SetRecvBuf(&response[0], response.length()));

		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(TYPE_ACK, msg->GetSIPMetaField()->iCSeq->GetType());

		// media part

		TestMediaPart(param.isAudio, param.audioCodec, param.isVideo, param.videoCodec);
	}

	TEST_P(SIPParserIncomingTest, Incoming)
	{
		using ::testing::_;
		using ::testing::InvokeArgument;
		using ::testing::DoAll;
		using ::testing::WithArg;
		using ::testing::Invoke;

		std::string dialog_id;

		auto param = GetParam();

		// signal part

		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, _, _))
			.WillOnce(DoAll(InvokeArgument<4>(false, VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE, ""), WithArg<0>(Invoke([&dialog_id](string_view a) { dialog_id = std::string(a); }))));

		ASSERT_TRUE(SetRecvBuf(param.invite, strlen(param.invite)));
		sip->InviteReplay(dialog_id, e_call_ok, false);

		auto msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(100, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(180, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(200, msg->GetSIPMetaField()->iStartLine->GetResponseCode());

		ASSERT_TRUE(SetRecvBuf(param.ack, strlen(param.ack)));

		// media part

		TestMediaPart(param.isAudio, param.audioCodec, param.isVideo, param.videoCodec);
	}

	INSTANTIATE_TEST_CASE_P(Default, SIPParserOutgoingTest,
		testing::Values(
		OutgoingParams{ "VSX7000", "b@brchk000.trueconf.ua", "#sip:vsx7000@192.168.62.41", nullptr, raw_vsx7000_ringing, raw_vsx7000_ok,
			true, e_rcvSIREN14_32, true, e_videoH263plus },
		OutgoingParams{ "HDX8000", "b@brchk000.trueconf.ua", "#sip:hdx8000@192.168.62.42", nullptr, raw_hdx8000_ringing, raw_hdx8000_ok,
			true, e_rcvSIREN14_48, true, e_videoH264 },
		OutgoingParams{ "HD9030", "b@brchk000.trueconf.ua", "#sip:@192.168.62.42", raw_hd9030_trying, raw_hd9030_ringing, raw_hd9030_ok,
			true, e_rcvG722_64k, true, e_videoH264 },
		OutgoingParams{ "VC400", "a@matvey.trueconf.loc", "#sip:@192.168.62.48", raw_vc400_trying, raw_vc400_ringing, raw_vc400_ok,
			true, e_rcvSIREN14_32, true, e_videoH264 },
		OutgoingParams{ "AVER EVC 130", "a@matvey.trueconf.loc", "#sip:@192.168.62.89", raw_aver_evc_130_trying, raw_aver_evc_130_ringing, raw_aver_evc_130_ok,
			true, e_rcvG722_64k, true, e_videoH264 },
		OutgoingParams{ "Sony XG77", "a@matvey.trueconf.loc", "#sip:@192.168.62.153", raw_sony_xg77_trying, raw_sony_xg77_ringing, raw_sony_xg77_ok,
			true, e_rcvG722_64k, true, e_videoH264 },
		OutgoingParams{ "Cisco E20", "b@brchk000.trueconf.ua", "#sip:@192.168.62.43", nullptr, raw_e20_ringing, raw_e20_ok,
			true, e_rcvG722132, true, e_videoH264 },
		OutgoingParams{ "MC850", "b@brchk000.trueconf.ua", "#sip:@192.168.62.44", raw_mc850_trying, raw_mc850_ringing, raw_mc850_ok,
			false, 0, true, e_videoH264 }
		));

	INSTANTIATE_TEST_CASE_P(CallToSIP, SIPParserOriginSDPDirectionToSIPTest, ::testing::Values(
		CallOriginGroupParam { "user@vydr111.trueconf.name", "user@vydr111.trueconf.name", false},
		CallOriginGroupParam { "user*1&f?us1@domen@vydr111.trueconf.name", "user*1&f?us1%40domen@vydr111.trueconf.name", false },
		CallOriginGroupParam { "user@vydr111.trueconf.name", "user@vydr111.trueconf.name", true },
		CallOriginGroupParam { "user@domen@vydr111.trueconf.name", "user%40domen@vydr111.trueconf.name", true },
		CallOriginGroupParam { u8"\u043F\u043E\u043B\u044C\u0437\u043E\u0432\u0430\u0442\u0435\u043B\u044C@vydr111.trueconf.name",
			u8"%D0%BF%D0%BE%D0%BB%D1%8C%D0%B7%D0%BE%D0%B2%D0%B0%D1%82%D0%B5%D0%BB%D1%8C@vydr111.trueconf.name", true },
		CallOriginGroupParam { u8"\u043F\u043E\u043B\u044C\u0437\u043E\u0432\u0430\u0442\u0435\u043B\u044C@vydr111.trueconf.name",
			u8"%D0%BF%D0%BE%D0%BB%D1%8C%D0%B7%D0%BE%D0%B2%D0%B0%D1%82%D0%B5%D0%BB%D1%8C@vydr111.trueconf.name", false }
	));

	INSTANTIATE_TEST_CASE_P(CallFromSIP, SIPParserOriginSDPDirectionFromSIPTest, ::testing::Values(
		CallOriginGroupParam{ "user@vydr111.trueconf.name", "user@vydr111.trueconf.name", false },
		CallOriginGroupParam{ "%D0%BF%D0%BE%D0%BB%D1%8C%D0%B7%D0%BE%D0%B2%D0%B0%D1%82%D0%B5%D0%BB%D1%8C@vydr111.trueconf.name",
			u8"\u043F\u043E\u043B\u044C\u0437\u043E\u0432\u0430\u0442\u0435\u043B\u044C@vydr111.trueconf.name", false },
		CallOriginGroupParam{ "\\c\\2546251488@vydr111.trueconf.name", "\\c\\2546251488", true },
		CallOriginGroupParam{ "%D0%BF%D0%BE%D0%BB%D1%8C%D0%B7%D0%BE%D0%B2%D0%B0%D1%82%D0%B5%D0%BB%D1%8C@vydr111.trueconf.name",
			u8"\u043F\u043E\u043B\u044C\u0437\u043E\u0432\u0430\u0442\u0435\u043B\u044C", true }
	));

	INSTANTIATE_TEST_CASE_P(Default, SIPParserIncomingTest,
		testing::Values(
		IncomingParams{ "HD9030", raw_hd9030_invite, raw_hd9030_ack,
			true, e_rcvG722_64k, true, e_videoH264 },
		IncomingParams{ "VC400", raw_vc400_invite, raw_vc400_ack,
			true, e_rcvSIREN14_32, true, e_videoH264 },
		IncomingParams{ "AVER EVC 130", raw_aver_evc_130_invite, raw_aver_evc_130_ack,
			true, e_rcvSIREN14_32, true, e_videoH264 },
		IncomingParams{ "Sony XG77", raw_sony_xg77_invite, raw_sony_xg77_ack,
			true, e_rcvG722_64k, true, e_videoH264 }
	));

	TEST(CompactHeader, DecodeCompactHeader)
	{
		VS_SIPBuffer buf(raw_compact_header, sizeof(raw_compact_header));
		VS_SIPMetaField header;
		ASSERT_EQ(header.Decode(buf), TSIPErrorCodes::e_ok);
		ASSERT_EQ(header.iVia.size(), 1);
		ASSERT_EQ(header.iVia[0]->ConnectionType(), net::protocol::UDP);
		ASSERT_EQ(header.iVia[0]->Port(), 5060);
		ASSERT_STREQ(header.iVia[0]->Host().c_str(), "192.168.0.100");
		ASSERT_STREQ(header.iVia[0]->Branch().c_str(), "z9hG4bKPjxaGHm7W.9fqkrTP.wF8lmS.kKCrVoRnD");

		ASSERT_NE(header.iFrom, nullptr);
		ASSERT_STREQ(header.iFrom->GetURI()->Host().c_str(), "192.168.0.100");
		ASSERT_STREQ(header.iFrom->GetURI()->Tag().c_str(), "ofmgUL-c4JP1y4RIkzHZ.xcskWlx9Btw");

		ASSERT_NE(header.iTo, nullptr);
		ASSERT_STREQ(header.iTo->GetURI()->Host().c_str(), "192.168.0.103");

		ASSERT_NE(header.iContact, nullptr);
		ASSERT_EQ(header.iContact->GetLastURI()->Port(), 5060);
		ASSERT_STREQ(header.iContact->GetLastURI()->Host().c_str(), "192.168.0.100");

		ASSERT_NE(header.iCallID, nullptr);
		ASSERT_STREQ(header.iCallID->Value().c_str(), ".RmpN7NE0I4NeKZzwrdgGE8v.34hrvyX");

		ASSERT_NE(header.iCSeq, nullptr);
		ASSERT_NE(header.iUserAgent, nullptr);

		ASSERT_NE(header.iSupported, nullptr);

		ASSERT_NE(header.iExpires, nullptr);
		ASSERT_EQ(header.iExpires->Value(), std::chrono::seconds(1800));

		ASSERT_NE(header.iContentType, nullptr);
		ASSERT_EQ(header.iContentType->GetContentType(), CONTENTTYPE_SDP);

		ASSERT_NE(header.iContentLength, nullptr);
		ASSERT_EQ(header.iContentLength->Value(), 597);

		ASSERT_NE(header.iEvent, nullptr);
		ASSERT_EQ(header.iEvent->Event(), SIP_EVENT_PRESENCE);
	}

	TEST(CompactHeader, EncodeCompactHeader) {
		VS_SIPParserInfo info("serverVendor");

		VS_CallConfig config;
		config.sip.CompactHeader = true;
		info.SetAliasMy("qwe");
		info.SetTagMy("rty");
		info.SetDisplayNameMy("uio");
		info.SetAliasRemote("asd");
		info.SetTagSip("fgh");
		info.SetDisplayNameSip("jkl");
		info.SIPDialogID(".RmpN7NE0I4NeKZzwrdgGE8v.34hrvyX");
		info.SetExpires(std::chrono::seconds(1800));
		info.SetContentType(CONTENTTYPE_SDP);
		info.IsRequest(true);
		info.SetListenPort(5060);
		info.SetConfig(std::move(config));

		info.SetMyCsAddress({net::address_v4::from_string("192.168.0.100"), 5060, net::protocol::UDP});

		char data[512];

#define CREATE_AND_ENCODE(TYPE, INFO, DATA) {	\
		VS_SIPBuffer buf;						\
		TYPE field;								\
		field.Init(INFO);						\
		ASSERT_EQ(field.Encode(buf), TSIPErrorCodes::e_ok);		\
		buf.GetData(data, buf.GetWriteIndex()); \
	}
		const VS_SIPGetInfoImpl get_info{ info };

		CREATE_AND_ENCODE(VS_SIPField_Via, get_info, data);
			ASSERT_EQ(strstr(data, "v: SIP/2.0/UDP"), data);
			ASSERT_NE(strstr(data, "192.168.0.100"), nullptr);

		CREATE_AND_ENCODE(VS_SIPField_From, get_info, data);
			ASSERT_EQ(strstr(data, "f:"), data);
			ASSERT_NE(strstr(data, "\"uio\""), nullptr);
			ASSERT_NE(strstr(data, "<sip:qwe>"), nullptr);
			ASSERT_NE(strstr(data, "tag=rty"), nullptr);

		CREATE_AND_ENCODE(VS_SIPField_To, get_info, data);
			ASSERT_EQ(strstr(data, "t:"), data);
			ASSERT_NE(strstr(data, "<sip:asd>"), nullptr);
			ASSERT_NE(strstr(data, "tag=fgh"), nullptr);

		CREATE_AND_ENCODE(VS_SIPField_Contact, get_info, data);
			ASSERT_EQ(strstr(data, "m:"), data);
			ASSERT_NE(strstr(data, "<sip:192.168.0.100:5060"), nullptr);

		CREATE_AND_ENCODE(VS_SIPField_CallID, get_info, data);
			ASSERT_EQ(strstr(data, "i:"), data);
			ASSERT_NE(strstr(data, ".RmpN7NE0I4NeKZzwrdgGE8v.34hrvyX"), nullptr);

		CREATE_AND_ENCODE(VS_SIPField_Expires, get_info, data);
			ASSERT_EQ(strstr(data, "Expires:"), data);
			ASSERT_NE(strstr(data, "1800"), nullptr);

		CREATE_AND_ENCODE(VS_SIPField_ContentType, get_info, data);
			ASSERT_EQ(strstr(data, "c:"), data);
			ASSERT_NE(strstr(data, "application/sdp"), nullptr);

		CREATE_AND_ENCODE(VS_SIPField_ContentLength, get_info, data);
			ASSERT_EQ(strstr(data, "l:"), data);

		CREATE_AND_ENCODE(VS_SIPField_Event, get_info, data);
			ASSERT_EQ(strstr(data, "o:"), data);

#undef CREATE_AND_ENCODE

	}

	TEST(SIPInputMessageQueue, Grandstream_STUN) {
		unsigned char stun_msg1[] = { 0x00, 0x01, 0x00, 0x00, 0xb1, 0xe6, 0xbe, 0xd9, 0x97, 0x32, 0xd7, 0x24, 0x04, 0xd6, 0x64, 0xae, 0x47, 0x9b, 0x41, 0xf5 };
		unsigned char stun_msg2[] = { 0x00, 0x03, 0x00, 0x08, 0x21, 0x12, 0xa4, 0x42, 0x6f, 0x61, 0x50, 0x44, 0x7a, 0x43, 0x31, 0x47,  0x33, 0x38, 0x64, 0x6a, 0x00, 0x19, 0x00, 0x04, 0x11, 0x00, 0x00, 0x00 };
		unsigned char stun_msg3[] = { 0x00, 0x03, 0x00, 0x00, 0x21, 0x12, 0xA4, 0x42, 0x04, 0xd6, 0x64, 0xae, 0x47, 0x9b, 0x41, 0xf5, 0x04, 0xd6, 0x64, 0xae };

		VS_SIPInputMessageQueue queue(_65KB);
		ASSERT_FALSE(queue.PutMessageWithFilters(stun_msg1, sizeof(stun_msg1), e_SIP_CS, VS_SIPInputMessageQueue::FLT_STUN));
		ASSERT_FALSE(queue.PutMessageWithFilters(stun_msg2, sizeof(stun_msg2), e_SIP_CS, VS_SIPInputMessageQueue::FLT_STUN));
		ASSERT_FALSE(queue.PutMessageWithFilters(stun_msg3, sizeof(stun_msg3), e_SIP_CS, VS_SIPInputMessageQueue::FLT_STUN));
		ASSERT_TRUE(queue.PutMessageWithFilters((unsigned char *)raw_rockwareC9_options, sizeof(raw_rockwareC9_options), e_SIP_CS));

		uint32_t sz;
		ASSERT_NE(nullptr, queue.GetChannelMessage(sz, e_SIP_CS));
	}

	TEST(SIPInputMessageQueue, SIP_Message_Limit) {
		bool put_mess_res(false);
		uint32_t sz;

		//LIMIT = 10 000. tests for control: limit is more than packets
		//single packet tests
		VS_SIPInputMessageQueue sip_in_mess_limit_10000(10000);
		put_mess_res = sip_in_mess_limit_10000.PutMessage((unsigned char*)raw_sip_message_over_3000b, strlen(raw_sip_message_over_3000b), e_SIP_CS);
		EXPECT_TRUE(put_mess_res);
		EXPECT_NE(nullptr, sip_in_mess_limit_10000.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_EQ(nullptr, sip_in_mess_limit_10000.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_10000.PutMessage((unsigned char*)raw_2_sip_messages_over_3000b_and_less_1000b, strlen(raw_2_sip_messages_over_3000b_and_less_1000b), e_SIP_CS);
		EXPECT_TRUE(put_mess_res);
		EXPECT_NE(nullptr, sip_in_mess_limit_10000.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_NE(nullptr, sip_in_mess_limit_10000.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_EQ(nullptr, sip_in_mess_limit_10000.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_10000.PutMessage((unsigned char*)raw_2_sip_messages_content_part_over_3000b_and_full_less_1000b, strlen(raw_2_sip_messages_content_part_over_3000b_and_full_less_1000b), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_10000.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_10000.PutMessage((unsigned char*)raw_2_sip_messages_content_part_less_300b_and_full_less_1000b, strlen(raw_2_sip_messages_content_part_less_300b_and_full_less_1000b), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_10000.GetChannelMessage(sz, e_SIP_CS));

		////////////////////////////////////////////////////////////////////////////////////////////////
		//LIMIT = 1500. tests for using limit: limit is less than part packets and more than other part
		//single packet tests
		VS_SIPInputMessageQueue sip_in_mess_limit_1500(1500);
		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_sip_message_less_1000b, strlen(raw_sip_message_less_1000b), e_SIP_CS);
		EXPECT_TRUE(put_mess_res);
		EXPECT_NE(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_sip_message_over_3000b, strlen(raw_sip_message_over_3000b), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_2_sip_messages_over_3000b_and_less_1000b, strlen(raw_2_sip_messages_over_3000b_and_less_1000b), e_SIP_CS);
		EXPECT_TRUE(put_mess_res);
		EXPECT_NE(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_2_sip_messages_content_part_over_3000b_and_full_less_1000b, strlen(raw_2_sip_messages_content_part_over_3000b_and_full_less_1000b), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_2_sip_messages_content_part_less_300b_and_full_less_1000b, strlen(raw_2_sip_messages_content_part_less_300b_and_full_less_1000b), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_4_sip_messages_content_part_less_300b_and_3_full_less_1000b, strlen(raw_4_sip_messages_content_part_less_300b_and_3_full_less_1000b), e_SIP_CS);
		EXPECT_TRUE(put_mess_res);
		EXPECT_NE(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_NE(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		//////////////////////////////////////////////////////////////////////////////////
		//few packet tests
		//LIMIT = 1500. tests for using limit for header+content in diff packets
		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_sip_message_without_content_over_2500b_p1, strlen(raw_sip_message_without_content_over_2500b_p1), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);//because save first part and waiting for content
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_sip_message_content_over_plus_other_mess_less_1000b_p2, strlen(raw_sip_message_content_over_plus_other_mess_less_1000b_p2), e_SIP_CS);
		EXPECT_TRUE(put_mess_res);//delete big packet headers+content and get nornal packet
		EXPECT_NE(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		//////////////////////////////////////////////////////////////////////////////////
		//one message with content that is more than limit in 2 packets: 1 - part of headers wit ContentLen, 2 - other part of headers with ContentLen and content and small packet
		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_part_of_headers_sip_message_over_2000b_w_c_l, strlen(raw_part_of_headers_sip_message_over_2000b_w_c_l), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_part_of_headers_and_content_less_500b_wo_c_l_and_mess_less_1000, strlen(raw_part_of_headers_and_content_less_500b_wo_c_l_and_mess_less_1000), e_SIP_CS);
		EXPECT_TRUE(put_mess_res);
		EXPECT_NE(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		///////////////////////////////////////////////////////////////////////////////
		//one message with content that is more than limit in 2 packets: 1 - part of headers without ContentLen, 2 - other part of headers with ContentLen and content and small packet
		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_part_of_headers_sip_message_over_2000b_wo_c_l, strlen(raw_part_of_headers_sip_message_over_2000b_wo_c_l), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_part_of_headers_and_content_less_500b_w_c_l_and_mess_less_1000, strlen(raw_part_of_headers_and_content_less_500b_w_c_l_and_mess_less_1000), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		//////////////////////////////////////////////////////////////////////////////
		//one message with content that is more than limit in 3 packets 1 - headers + content part, 2 - content part + small packet
		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_part_of_headers_sip_message_over_2000b_part_of_content, strlen(raw_part_of_headers_sip_message_over_2000b_part_of_content), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_part_of_content_less_500b_and_mess_less_1000, strlen(raw_part_of_content_less_500b_and_mess_less_1000), e_SIP_CS);
		EXPECT_TRUE(put_mess_res);
		EXPECT_NE(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		//////////////////////////////////////////////////////////////
		//one message with content that is more than limit in 3 packets 1 - headers + content part, 2 - content part, 3 - content part + small packet
		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_part_of_headers_sip_message_over_2000b_part_of_content, strlen(raw_part_of_headers_sip_message_over_2000b_part_of_content), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_part_of_content_less_100b, strlen(raw_part_of_content_less_100b), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_1500.PutMessage((unsigned char*)raw_part_of_content_less_300b_and_mess_less_1000, strlen(raw_part_of_content_less_300b_and_mess_less_1000), e_SIP_CS);
		EXPECT_TRUE(put_mess_res);
		EXPECT_NE(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_EQ(nullptr, sip_in_mess_limit_1500.GetChannelMessage(sz, e_SIP_CS));

		//////////////////////////////////////////////////////////////
		//LIMIT = 2650. tests for using limit for header+content
		VS_SIPInputMessageQueue sip_in_mess_limit_2650(2650);
		put_mess_res = sip_in_mess_limit_2650.PutMessage((unsigned char*)raw_sip_message_less_1000b, strlen(raw_sip_message_less_1000b), e_SIP_CS);
		EXPECT_TRUE(put_mess_res);
		EXPECT_NE(nullptr, sip_in_mess_limit_2650.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_EQ(nullptr, sip_in_mess_limit_2650.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_2650.PutMessage((unsigned char*)raw_sip_message_over_3000b, strlen(raw_sip_message_over_3000b), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_2650.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_2650.PutMessage((unsigned char*)raw_2_sip_messages_over_3000b_and_less_1000b, strlen(raw_2_sip_messages_over_3000b_and_less_1000b), e_SIP_CS);
		EXPECT_TRUE(put_mess_res);
		EXPECT_NE(nullptr, sip_in_mess_limit_2650.GetChannelMessage(sz, e_SIP_CS));
		EXPECT_EQ(nullptr, sip_in_mess_limit_2650.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_2650.PutMessage((unsigned char*)raw_2_sip_messages_content_part_over_3000b_and_full_less_1000b, strlen(raw_2_sip_messages_content_part_over_3000b_and_full_less_1000b), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_2650.GetChannelMessage(sz, e_SIP_CS));

		put_mess_res = sip_in_mess_limit_2650.PutMessage((unsigned char*)raw_2_sip_messages_content_part_less_300b_and_full_less_1000b, strlen(raw_2_sip_messages_content_part_less_300b_and_full_less_1000b), e_SIP_CS);
		EXPECT_FALSE(put_mess_res);
		EXPECT_EQ(nullptr, sip_in_mess_limit_2650.GetChannelMessage(sz, e_SIP_CS));
	}


	TEST_F(SIPParserTest, Ticket_EN_1393_test1)	// parse Record-Route header with many values
	{
		std::string str("Record-Route: <sip:proxy-call-id=9fba18e5-b614-4b7a-88de-af8fa2579823@10.100.128.23:5060;transport=tcp;lr>,<sip:proxy-call-id=9fba18e5-b614-4b7a-88de-af8fa2579823@10.100.128.23:5061;transport=tls;lr>,<sip:proxy-call-id=41d3cebc-31fe-407d-a2ee-77b68c0dfa57@10.100.128.41:7011;transport=tls;lr>,<sip:proxy-call-id=41d3cebc-31fe-407d-a2ee-77b68c0dfa57@150.254.210.29:5060;transport=tcp;lr>\r\n");
		VS_SIPBuffer buff;
		buff.AddData(str);
        VS_SIPField_RecordRoute field;
		ASSERT_EQ(field.Decode(buff), TSIPErrorCodes::e_ok);
		VS_SIPBuffer out_buff;
		ASSERT_EQ(field.Encode(out_buff), TSIPErrorCodes::e_ok);
		out_buff.AddData("\r\n");
		std::unique_ptr<char[]> out_ptr;
		ASSERT_EQ(out_buff.GetAllDataAllocConst(out_ptr), TSIPErrorCodes::e_ok);
		ASSERT_STREQ(str.c_str(), out_ptr.get());
	}

	TEST_F(SIPParserTest, Ticket_EN_1393_test2)	// parse two Record-Route header (first with many <uri>'s)
	{
		std::string str("Record-Route: <sip:proxy-call-id=9fba18e5-b614-4b7a-88de-af8fa2579823@10.100.128.23:5060;transport=tcp;lr>,<sip:proxy-call-id=9fba18e5-b614-4b7a-88de-af8fa2579823@10.100.128.23:5061;transport=tls;lr>,<sip:proxy-call-id=41d3cebc-31fe-407d-a2ee-77b68c0dfa57@10.100.128.41:7011;transport=tls;lr>,<sip:proxy-call-id=41d3cebc-31fe-407d-a2ee-77b68c0dfa57@150.254.210.29:5060;transport=tcp;lr>\r\n");
		str += "Record-Route: <sip:user@host>\r\n";
		VS_SIPBuffer buff;
		buff.AddData(str);
		VS_SIPBuffer out_buff;
		TSIPErrorCodes err(TSIPErrorCodes::e_ok);
		while (err == TSIPErrorCodes::e_ok && buff.GetReadIndex() < buff.GetWriteIndex())
		{
			VS_SIPField_RecordRoute field;
			ASSERT_EQ(field.Decode(buff), TSIPErrorCodes::e_ok);
			ASSERT_EQ(field.Encode(out_buff), TSIPErrorCodes::e_ok);
			out_buff.AddData("\r\n");
		}
		std::unique_ptr<char[]> out_ptr;
		ASSERT_EQ(out_buff.GetAllDataAllocConst(out_ptr), TSIPErrorCodes::e_ok);
		ASSERT_STREQ(str.c_str(), out_ptr.get());
	}

	TEST_F(SIPParserTest, Ticket_EN_1393_test3)
	{
		VS_CallConfig config;
		config.Address.addr = net::address_v4(1);
		config.Address.port = 1;
		config.Address.protocol = net::protocol::UDP;
		using ::testing::_;
		using ::testing::Return;

		EXPECT_CALL(*confProtocol, InviteReplay(_, e_call_ok, _, _, _))
			.WillOnce(Return(true));

		EXPECT_CALL(*confProtocol, Hangup(_))
			.Times(0);

		auto &&dialog_id = sip->NewDialogID("to_name", {}, config);
		sip->InviteMethod(dialog_id, "from_name", "to_name", VS_ConferenceInfo(false, false));

		auto iv_msg = GetMessageFromParser(sip);
		ASSERT_TRUE(iv_msg);
		ASSERT_TRUE(iv_msg->IsValid());
		ASSERT_TRUE(!iv_msg->GetSIPMetaField()->iVia.empty());
		std::string branch = (*(iv_msg->GetSIPMetaField()->iVia.begin()))->Branch();

		sip->clock().add_diff(std::chrono::seconds(538));

		std::string resp_200ok = raw_ticket_en1393_200ok;
		strreplace(resp_200ok, "__callid__", dialog_id);
		strreplace(resp_200ok, "__branch__", branch);
		SetRecvBuf(resp_200ok.c_str(), resp_200ok.size());

		auto msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg);
		std::string out;
		msg->Encode(out);
		// check that we have four Route headers (in reverse order)
		ASSERT_TRUE(msg->IsValid());
	};

	TEST_F(SIPParserTest, SIP_Field_RetryAfter_CodingTest)
	{
		std::chrono::seconds value(10);
		VS_SIPField_RetryAfter field;

		VS_SIPBuffer buff;
		auto separator = "\r\n";
		auto pCallInfo = boost::make_shared<VS_SIPParserInfo>("serverVendor");
		pCallInfo->SetRetryAfterValue(value);

		const VS_SIPGetInfoImpl get_info{ *pCallInfo };

		ASSERT_EQ(TSIPErrorCodes::e_ok, field.Init(get_info));
		ASSERT_EQ(TSIPErrorCodes::e_ok, field.Encode(buff));

		buff.AddData(separator, strlen(separator));
		ASSERT_EQ(TSIPErrorCodes::e_ok, field.Decode(buff));
		ASSERT_EQ(value, field.Value());
	};

	TEST_F(SIPParserTest, SIP_Field_RetryAfter_GetMessageAndDuration)
	{
		auto separator = "\r\n";
		std::string data = "Retry-After: 120 (I'm in a meeting);duration=3600";
		VS_SIPBuffer buff;
		VS_SIPField_RetryAfter field;

		buff.AddData(data);
		buff.AddData(separator, strlen(separator));
		ASSERT_EQ(TSIPErrorCodes::e_ok, field.Decode(buff));
		ASSERT_STREQ(field.Comment().c_str(),"I'm in a meeting");
		ASSERT_EQ(std::chrono::seconds(3600), field.Duration());

	};

	struct SIPParserSniffTest : public SIPParserTest{
		void ResetEndpoints(const net::Endpoint& myEp, const net::Endpoint& peerEp) {
			vcs_addr = myEp;
			terminal_addr = peerEp;
			SIPParserTestBase::SetUp();
		}
		std::shared_ptr<VS_SIPParserInfo> InviteFromParser(string_view from, string_view to, std::string &OUT_dialog_id, std::string& OUT_branch, const VS_CallConfig &cfg = {}){

			auto &&dialog_id = sip->NewDialogID(to, {}, cfg);
			OUT_dialog_id = dialog_id;

			if (!sip->InviteMethod(dialog_id, from, to, VS_ConferenceInfo(false, false))) return nullptr;
			auto msg_ctx = sip->GetMsgForSend_SIP(dialog_id);
			if (!msg_ctx.first || !msg_ctx.second) return nullptr;
			sip_transport->ProcessMessage(msg_ctx.first, msg_ctx.second);
			auto branch = msg_ctx.second->Branch();
			auto internal_context = msg_ctx.first;
			if (branch.empty())
				return nullptr;
			OUT_branch = std::string(branch);

			return msg_ctx.first;
		}

		std::shared_ptr<VS_SIPMessage> ReceivePacket(const char* packet, const net::Endpoint &remoteEp){
			auto msg = std::make_shared<VS_SIPMessage>();

			msg->Decode(packet, strlen(packet));
			sip->SetRecvMsg_SIP(msg, remoteEp);
			return msg;
		}

		std::shared_ptr<VS_SIPMessage> SendRequest() {
			auto msg_ctx = sip->GetMsgForSend_SIP();
			if (!msg_ctx.first || !msg_ctx.second || msg_ctx.second->GetMessageType() != MESSAGE_TYPE_REQUEST) return nullptr;
			return msg_ctx.second;
		}

		std::shared_ptr<VS_SIPMessage> SendResponse() {
			auto msg_ctx = sip->GetMsgForSend_SIP();
			if (!msg_ctx.first || !msg_ctx.second || msg_ctx.second->GetMessageType() != MESSAGE_TYPE_RESPONSE) return nullptr;
			return msg_ctx.second;
		}

		void SendACK(){
			auto req = SendRequest();
			ASSERT_NE(req, nullptr);
			ASSERT_EQ(TYPE_ACK, req->GetMethod());
		}
		std::shared_ptr<VS_SIPMessage> VerifyInviteSendedInTimeout(){
			sip->Timeout();																		// InviteMethod must be called
			auto msg_ctx = sip->GetMsgForSend_SIP();
			if (!msg_ctx.first || !msg_ctx.second) return nullptr;
			if (TYPE_INVITE != msg_ctx.second->GetMethod()) 		return nullptr;				// verify that InviteMethod really has been called
			sip_transport->ProcessMessage(msg_ctx.first, msg_ctx.second);
			return msg_ctx.second;
		}

		std::pair<std::weak_ptr<VS_SIPParserInfo>, std::shared_ptr<VS_SIPMessage>> SendRegister() {
			auto msg_ctx = sip->GetMsgForSend_SIP();
			auto &register_request = msg_ctx.second;
			if (register_request == nullptr) return std::make_pair(std::weak_ptr<VS_SIPParserInfo>(), nullptr);
			if(TYPE_REGISTER != register_request->GetSIPMetaField()->iStartLine->GetRequestType()) return std::make_pair(std::weak_ptr<VS_SIPParserInfo>(), nullptr);
			sip_transport->ProcessMessage(msg_ctx.first, msg_ctx.second);

			return msg_ctx;
		}
		bool MakeNTLMRegistration() {
			auto msg_ctx = SendRegister();
			auto ctx = msg_ctx.first.lock();
			if (!ctx || !msg_ctx.second) return false;

			auto &register_request = msg_ctx.second;
			char unautorized_packet[1024] = { 0 };
			sprintf(unautorized_packet, sipraw::unautorized_ntlm_kerberros_tlsdsk, ctx->SIPDialogID().c_str(), register_request->GetCSeq(), std::string(register_request->Branch()).c_str());
			auto unautorized = ReceivePacket(unautorized_packet, terminal_addr);
			if (!unautorized) return false;

			msg_ctx = SendRegister();
			ctx = msg_ctx.first.lock();
			if (!ctx || !msg_ctx.second) return false;

			auto &register_ntlm = msg_ctx.second;
			char unautorized_packet_gss_api[1024] = { 0 };
			sprintf(unautorized_packet_gss_api, sipraw::unautorized_ntlm_with_gss, ctx->SIPDialogID().c_str(), register_ntlm->GetCSeq(), std::string(register_ntlm->Branch()).c_str());
			unautorized = ReceivePacket(unautorized_packet_gss_api, terminal_addr);
			if (!unautorized) return false;

			return true;
		}

		VS_CallConfig MakeSkypeConfig(const std::string login) {
			VS_CallConfig skype_reg_config;
			skype_reg_config.Login = login;
			skype_reg_config.HostName = "192.168.74.4";
			skype_reg_config.Password = "qweASD123";
			skype_reg_config.sip.RegistrationBehavior = VS_CallConfig::REG_REGISTER_ALWAYS;
			skype_reg_config.Address = { terminal_addr.addr, terminal_addr.port, terminal_addr.protocol };
			skype_reg_config.SignalingProtocol = VS_CallConfig::SIP;
			skype_reg_config.sip.SRTPEnabled = true;
			return skype_reg_config;
		}

		VS_CallConfig MakeConfigurationForSkype(const std::string login = "user6") {
			terminal_addr.addr = net::address_v4::from_string("192.168.74.4");
			terminal_addr.port = 5061;
			terminal_addr.protocol = vcs_addr.protocol = net::protocol::TLS;
			sip->SetMyCsAddress(vcs_addr);

			auto skype_reg_config = MakeSkypeConfig(login);
			boost::shared_ptr<VS_IndentifierSIP_TestImpl> fake_ident = 	boost::make_shared<VS_IndentifierSIP_TestImpl>();

			fake_ident->AddHostConfiguration(skype_reg_config);
			call_config->RegisterProtocol(fake_ident);
			call_config->UpdateSettings();
			sip->SetRegistrationConfiguration(skype_reg_config);

			return skype_reg_config;
		}
	};
	TEST_F(SIPParserSniffTest, SpecifyEpidInAnswer) {
		terminal_addr.addr = net::address_v4::from_string("192.168.74.4");
		terminal_addr.port = 5061;
		terminal_addr.protocol = vcs_addr.protocol = net::protocol::TLS;
		ResetEndpoints(vcs_addr, terminal_addr);

		// 1. Emulate config in registry and init NTLM authentication process
		auto skype_reg_config = MakeConfigurationForSkype("service.trueconf");
		ASSERT_TRUE(MakeNTLMRegistration());
		auto msg_ctx = SendRegister();	// send final register and expect sucsessful SA verification

		auto invite = ReceivePacket(sipraw::InviteWithExtendedRecordRoute, terminal_addr);
		ASSERT_NE(invite, nullptr);
		auto sip_meta = invite->GetSIPMetaField();
		ASSERT_NE(sip_meta, nullptr); ASSERT_NE(sip_meta->iFrom, nullptr);

		auto uri = sip_meta->iFrom->GetURI();	ASSERT_NE(uri, nullptr);
		auto from_epid = uri->Epid();

		auto trying = GetMessageFromParser(sip);
		EXPECT_NE(trying, nullptr);

		auto busy_here = GetMessageFromParser(sip);
		EXPECT_NE(busy_here, nullptr);

		auto invite_msg = GetMessageFromParser(sip);
		ASSERT_NE(invite_msg, nullptr);
		sip_meta = invite_msg->GetSIPMetaField();
		ASSERT_NE(sip_meta, nullptr); ASSERT_NE(sip_meta->iTo, nullptr);

		uri = sip_meta->iTo->GetURI();	ASSERT_NE(uri, nullptr);
		auto to_epid = uri->Epid();
		EXPECT_STREQ(to_epid.c_str(), from_epid.c_str());	// make sure we send invite with message to same endpoint id from which we received invitation to call
	}

	// test that we save sip uri from Contact header and use it in request uri for requests in-dialog
	TEST_F(SIPParserSniffTest, SaveSipRemoteTargetFromContact) {
		// 1. Receive message with contact header
		auto invite = ReceivePacket(sipraw::InviteWithExtendedRecordRoute, terminal_addr);
		ASSERT_NE(invite, nullptr);

		auto dialog = invite->CallID();
		ASSERT_FALSE(dialog.empty());

		const auto contexts = GetInternalContextsTest();
		auto it = contexts.find(dialog);
		ASSERT_NE(it, contexts.end());

		auto& ctx = it->second;
		ASSERT_NE(ctx, nullptr);

		// 2. Get uri that was in contact
		auto remote_target = ctx->GetSIPRemoteTarget();
		ASSERT_TRUE(!remote_target.empty());
		auto target_from_contact = std::string(remote_target);

		// 3. Receive new message in-dialog
		std::string msg_in_dialog(sipraw::WebinarCommand);
		VS_ReplaceAll(msg_in_dialog, "7c08edfc64074ec880fd8a546be21f61", dialog);
		auto command = ReceivePacket(msg_in_dialog.c_str(), terminal_addr);
		EXPECT_NE(command, nullptr);

		// 4. Verify that uri was not overwritten by from header
		remote_target = ctx->GetSIPRemoteTarget();
		EXPECT_TRUE(remote_target == target_from_contact);
	}

	TEST_F(SIPParserSniffTest, TwoRecordRoutes) {
		// 1. Init dialog, where we have two records route
		auto invite = ReceivePacket(sipraw::invite_with_two_records_route, terminal_addr);
		ASSERT_NE(invite, nullptr);

		auto msg = SendResponse();
		ASSERT_NE(msg, nullptr);
		EXPECT_EQ(msg->GetResponseCode(), 100);

		auto _200ok = SendResponse();
		ASSERT_NE(_200ok, nullptr);
		EXPECT_EQ(_200ok->GetResponseCode(), 200);

		// 1.1. Dialog is inited
		auto ack = ReceivePacket(sipraw::ack_to_invite_with_two_records_route, terminal_addr);
		ASSERT_NE(ack, nullptr);

		// 2. Send request in-dialog
		auto callID = ack->CallID();
		sip->Chat(callID, "from", "to", {}, "TestMessage");
		auto msg_ctx = sip->GetMsgForSend_SIP();
		auto chat_mess = msg_ctx.second;
		ASSERT_NE(chat_mess, nullptr);

		auto pctx = msg_ctx.first;
		ASSERT_NE(pctx, nullptr);

		std::vector<std::shared_ptr<VS_SIPURI>> initial_set;
		for (auto record_route : invite->GetSIPMetaField()->iRouteSet) {
			auto uris = record_route->GetURIs();
			initial_set.insert(initial_set.end(), uris.begin(), uris.end());

		}
		auto route_size = pctx->GetSIPRouteSetSize();
		EXPECT_EQ(route_size, initial_set.size());
		pctx->ResetSIPRouteIndex();

		std::vector<const VS_SIPURI*> new_set;
		for (size_t i = 0; i < route_size; ++i) new_set.emplace_back(pctx->GetNextSIPRouteFromSet());
		EXPECT_EQ(new_set.size(), initial_set.size());

		// 3. Expect routes in reverse order for request
		std::reverse(new_set.begin(), new_set.end());
		for (size_t i = 0; i < new_set.size(); ++i)	{
			EXPECT_EQ(*new_set[i], *initial_set[i]);
		}
	}

	TEST_F(SIPParserSniffTest, GetAliasFromRegContext) {
		auto skype_reg_config = MakeConfigurationForSkype();
		ASSERT_TRUE(MakeNTLMRegistration());

		auto msg_ctx = SendRegister();	// send final register
		auto reg_ctx = msg_ctx.first.lock();
		ASSERT_NE(reg_ctx, nullptr) << "context for final register is nullptr";
		ASSERT_NE(msg_ctx.second, nullptr);

		const char *from_id = "user6@skype2015.loc";
		const char *to_id = "user3@skype2015.loc";
		std::string generated_dialog_id, generated_branch;
		auto invite_context = InviteFromParser(from_id, to_id, generated_dialog_id, generated_branch, skype_reg_config);
		ASSERT_TRUE(invite_context);

		EXPECT_TRUE(invite_context->GetAliasMy() == reg_ctx->GetAliasMy());
	}

	TEST_F(SIPParserSniffTest, RedirectToHomeServer) {
		// 1. Emulate config in registry and init NTLM authentication process
		auto skype_reg_config = MakeConfigurationForSkype();
		ASSERT_TRUE(MakeNTLMRegistration());

		auto msg_ctx = SendRegister();	// send final register and expect sucsessful SA verification
		auto ctx = msg_ctx.first.lock();
		ASSERT_NE(ctx, nullptr) << "context for final register is nullptr";
		ASSERT_NE(msg_ctx.second, nullptr);

		// but instead receive 301 redirect
		auto &register_ntlm = msg_ctx.second;
		char redirect_to_home_server[1024] = { 0 };
		sprintf(redirect_to_home_server, sipraw::redirect_to_home_server, ctx->SIPDialogID().c_str(), std::string(register_ntlm->Branch()).c_str());
		auto redirect301 = ReceivePacket(redirect_to_home_server, terminal_addr);
		EXPECT_NE(redirect301, nullptr);


		sip->Timeout();							// process redirection (Resolve new address and send REGISTER request to it)
		EXPECT_TRUE(MakeNTLMRegistration());	// expect successfull registration on new server
	}

	TEST_F(SIPParserSniffTest, RejectNotExistedCID) {

		using ::testing::_;
		using ::testing::AtLeast;
		using ::testing::Return;

		UserStatusInfo user;
		user.info = UserStatusInfo::Conf{ true };
		EXPECT_CALL(*confProtocol, GetUserStatus(_)).WillRepeatedly(Return(user));

		// 1. Make fake config to emulate config in registry with values we need
		auto skype_reg_config = MakeConfigurationForSkype();

		// 2. Make sure NTLM authentification process not broken
		ASSERT_TRUE(MakeNTLMRegistration());
		SendRequest();

		// 3. Receive /call \c\1234
		ASSERT_TRUE(ReceivePacket(sipraw::call_with_conference_CID, terminal_addr));

		// specification for text here https://projects.trueconf.com/bin/view/Projects/S4BIntegration#Вне_конференции
		// 4. Make sure we began to send "Joining \c\CID"
		auto _internal_ctxs = GetInternalContextsTest();
		auto it = std::find_if(_internal_ctxs.begin(), _internal_ctxs.end(), [](const std::pair<std::string, std::shared_ptr<VS_SIPParserInfo>> &p) -> bool {
			if (!p.second) return false;
			auto pending_messages = p.second->PopPendingMessages();
			return std::find_if(pending_messages.begin(), pending_messages.end(), [](const std::tuple<std::string ,eContentType> &m) {return boost::starts_with(std::get<0>(m), "Joining"); }) != pending_messages.end();
		});
		ASSERT_TRUE(it != _internal_ctxs.end());
		ASSERT_TRUE(SendResponse());	// 200 OK for call_with_conference_CID
		ASSERT_TRUE(SendRequest());		// invite to SFB
		ASSERT_TRUE(SendRequest());		// INVITE with m=message

		// 5. Get dialog id of context created by chat bot
		auto internal_ctxs = GetInternalContextsTest();
		it = std::find_if(internal_ctxs.begin(), internal_ctxs.end(), [](const std::pair<std::string, std::shared_ptr<VS_SIPParserInfo>> &p) {
			return p.second && p.second->CreatedByChatBot();
		});
		ASSERT_NE(it, internal_ctxs.end());
		auto &&dialog = it->second->SIPDialogID();

		// 6. Reply negatively to invite request
		sip->InviteReplay(dialog, e_call_busy, true, "\\c\\1234");

		// 7. Make sure we began to send "Rejected" message to SFB
		ASSERT_TRUE(SendRequest());	// INVITE with m=message
		auto updated_internal_ctxs = GetInternalContextsTest();
		it = std::find_if(updated_internal_ctxs.begin(), updated_internal_ctxs.end(), [](const std::pair<std::string, std::shared_ptr<VS_SIPParserInfo>> &p) -> bool {
			if(!p.second && p.second->CreatedByChatBot()) return false;
			auto pending_messages = p.second->PopPendingMessages();
			return std::find_if(pending_messages.begin(), pending_messages.end(), [](const std::tuple<std::string, eContentType> &m) {return std::get<0>(m) == "Rejected"; }) != pending_messages.end();
		});
		ASSERT_TRUE(it != updated_internal_ctxs.end());
	}

	TEST_F(SIPParserSniffTest, DeleteContextAfterChatViaTLS) {
		// 1. Make fake config to emulate config in registry with values we need
		auto skype_reg_config = MakeConfigurationForSkype();

		// 2. Make sure NTLM authentification process not broken
		ASSERT_TRUE(MakeNTLMRegistration());
		SendRequest();

		// 3. Send Chat
		auto dialog_id = sip->NewDialogID("user3@skype2015.loc", "", skype_reg_config);
		sip->Chat(dialog_id, "user6@skype2015.loc", "user3@skype2015.loc", "user6DN", "hello\n");

		// 4. Make sure we have sended invite with m=message media channel
		auto chat_invite = SendRequest();
		ASSERT_NE(chat_invite, nullptr);
		EXPECT_EQ(chat_invite->CallID(), dialog_id);

		auto pSDP = chat_invite->GetSDPMetaField();
		ASSERT_NE(pSDP, nullptr);
		ASSERT_TRUE(std::any_of(pSDP->iMediaStreams.begin(), pSDP->iMediaStreams.end(), [](VS_SDPField_MediaStream *ms) {return ms && ms->GetMediaType() == SDPMediaType::message; }));

		const auto& internal_ctxs = GetInternalContextsTest();
		auto ctx_it = internal_ctxs.find(dialog_id);
		ASSERT_NE(ctx_it, internal_ctxs.end());

		ASSERT_NE(ctx_it->second, nullptr);
		ctx_it->second->SetRingingStartTick(std::chrono::steady_clock::now() - std::chrono::minutes(3));


		// 5. We don't receive 200 ok, make sure ctx will be deleted
		sip->Timeout();
		const auto& internal_ctxs1 = GetInternalContextsTest();
		ctx_it = internal_ctxs1.find(dialog_id);
		ASSERT_NE(ctx_it->second, nullptr);
		ASSERT_TRUE(ctx_it->second->GetByeTick() != std::chrono::steady_clock::time_point());
		ctx_it->second->SetByeTick(ctx_it->second->GetByeTick() - std::chrono::minutes(3));

		sip->Timeout();
		const auto& internal_ctxs2 = GetInternalContextsTest();
		ASSERT_EQ(internal_ctxs2.find(dialog_id), internal_ctxs2.end());
	}

	TEST_F(SIPParserSniffTest, DeriveSRTPKeyFromSameTLSConnection) {
		using ::testing::Return;
		using ::testing::_;

		terminal_addr.addr = net::address_v4::from_string("192.168.74.4");
		terminal_addr.port = 5061;
		terminal_addr.protocol = vcs_addr.protocol = net::protocol::TLS;
		ResetEndpoints(vcs_addr, terminal_addr);

		UserStatusInfo user; boost::get<UserStatusInfo::User>(user.info).status = VS_UserPresence_Status::USER_LOGOFF;
		ON_CALL(*confProtocol, GetUserStatus(_)).WillByDefault(Return(user));

		// 1. Make fake config to emulate config in registry with values we need
		auto skype_reg_config = MakeConfigurationForSkype();

		// 2. Make sure NTLM authentification process not broken
		ASSERT_TRUE(MakeNTLMRegistration());

		// 3. Add two fake TLS connections: one with necessary bind and peer addresses, second with other peer address
		vcs_addr.protocol = net::protocol::TLS;

		//auto fake_conn = boost::make_shared<VS_ConnectionFake>(vcs_addr, terminal_addr);
		//sip_transport->Accept(fake_conn, sipraw::call_command, sizeof(sipraw::call_command));

		net::Endpoint addr1{ net::address::from_string("192.168.74.12"), 5061 ,net::protocol::TLS };
		//auto fake_conn1 = boost::make_shared<VS_ConnectionFake>(tmp_vcs_addr, addr1);
		//sip_transport->Accept(fake_conn1, sipraw::call_command, sizeof(sipraw::call_command));

		ReceivePacket(sipraw::call_command, terminal_addr);

		// 4. Make sure SRTP key for created ctx was get from apropriate TLS connection, hope that assert in sip::TransportLayer::GetSRTPKey won't be removed
		const auto &bind_addr = vcs_addr;
		const auto &peer_addr = terminal_addr;

		const auto internalContexts = GetInternalContextsTest();
		auto it = std::find_if(internalContexts.begin(), internalContexts.end(), [](const std::pair<std::string, std::shared_ptr<VS_SIPParserInfo>> &p) {
			return (p.second && !p.second->GetSRTPKey().empty());
		});

		ASSERT_TRUE(it != internalContexts.end());
		ASSERT_NE(it->second, nullptr);
		auto &cfg_address = it->second->GetConfig().Address;
		ASSERT_TRUE(it->second->GetMyCsAddress() == bind_addr && cfg_address == peer_addr);

		auto msg_ctx = sip->GetMsgForSend_SIP();
		auto chat_bot_msg_ctx = msg_ctx.first;
		ASSERT_NE(chat_bot_msg_ctx, nullptr);
		auto &chat_bot_cfg_addr = chat_bot_msg_ctx->GetConfig().Address;
		ASSERT_TRUE(chat_bot_msg_ctx->GetMyCsAddress() == bind_addr && chat_bot_cfg_addr == peer_addr);
	}

	TEST_F(SIPParserSniffTest, sip2tcc_invite_with_empty_refresher)
	{
		using ::testing::AtLeast;
		using ::testing::StrEq;
		using ::testing::_;
		using ::testing::InvokeArgument;
		using ::testing::DoAll;
		using ::testing::WithArg;
		using ::testing::Invoke;

		auto config = boost::make_shared<VS_CallConfig>();
		config->Address.addr = net::address_v4(1);
		config->Address.port = 1;
		config->Address.protocol = net::protocol::TCP;
		config->sip.SessionTimers.Enabled = true;
		config->HostName = "hostname1";
		config->sip.FromDomain = "hostname3";
		std::string data;
		auto req = std::make_shared<VS_SIPRequest>();

		std::string dialog_id;
		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, _, _))
			.WillOnce(DoAll(InvokeArgument<4>(false, VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE, ""), WithArg<0>(Invoke([&dialog_id](string_view a) { dialog_id = std::string(a); }))));
		std::string str = sipraw::Invite_with_session_expires;
		strreplace(str, "Session-Expires: 100;refresher=uas", "Session-Expires: 100");
		ASSERT_EQ(req->Decode(str.c_str(), str.length()), TSIPErrorCodes::e_ok);
		ASSERT_EQ(req->GetSIPMetaField()->iSessionExpires->GetRefresher(), REFRESHER::REFRESHER_INVALID);

		sip->SetRecvMsg_SIP(req, { net::address_v4::from_string("212.13.98.248"), 5060, net::protocol::TCP });
		sip->InviteReplay(dialog_id, e_call_ok, false);
		auto msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(100, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(180, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(200, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
		ASSERT_EQ(REFRESHER::REFRESHER_UAC, msg->GetSIPMetaField()->iSessionExpires->GetRefresher());
		auto internalContexts = GetInternalContextsTest();
		ASSERT_EQ(internalContexts[dialog_id]->GetTimerExtention().refresher,REFRESHER::REFRESHER_UAC);

	}

	TEST_F(SIPParserSniffTest, sip2tcc_invite_with_session_timer)
	{
		using ::testing::AtLeast;
		using ::testing::StrEq;
		using ::testing::_;
		using ::testing::InvokeArgument;
		using ::testing::DoAll;
		using ::testing::WithArg;
		using ::testing::Invoke;

		const net::address remote_addr = net::address_v4::from_string("212.13.98.248");
		const net::port remote_port = 5060;
		auto remote_type = net::protocol::TCP;
		std::string data;
		auto req = std::make_shared<VS_SIPRequest>();

		std::string dialog_id;
		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, _, _))
			.WillOnce(DoAll(InvokeArgument<4>(false, VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE, ""), WithArg<0>(Invoke([&dialog_id](string_view a) { dialog_id = std::string(a); }))));
		ASSERT_EQ(req->Decode(sipraw::Invite_with_session_expires, strlen(sipraw::Invite_with_session_expires)),TSIPErrorCodes::e_ok);

		sip->SetRecvMsg_SIP(req, { remote_addr, remote_port, remote_type });
		sip->InviteReplay(dialog_id, e_call_ok, false);
		auto msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(100, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(180, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(200, msg->GetSIPMetaField()->iStartLine->GetResponseCode());
		ASSERT_NE(nullptr, msg->GetSIPMetaField()->iRequire);
		ASSERT_EQ(msg->Encode(data), TSIPErrorCodes::e_ok);
		ASSERT_TRUE(strstr(data.c_str(), "Require"));
		auto internalContexts = GetInternalContextsTest();

		VS_SIPUpdateInfoImpl update_info(*internalContexts[dialog_id].get());
		const VS_SIPGetInfoImpl get_info(*internalContexts[dialog_id].get());

		req->MakeACK(get_info, update_info,false);
		req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, get_info); //for insert header Via (see sip::TransportLayer)
		req->GetSIPMetaField()->iVia[0]->Branch(std::string(msg->Branch()));
		req->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, get_info);
		ASSERT_EQ(TSIPErrorCodes::e_ok, req->Encode(data));
		sip->SetRecvMsg_SIP(req, { remote_addr, remote_port, remote_type });
		msg = GetMessageFromParser(sip);
		ASSERT_FALSE(msg);
		internalContexts[dialog_id]->GetTimerExtention().refresher = REFRESHER::REFRESHER_UAS;
		internalContexts[dialog_id]->GetTimerExtention().refreshPeriod = std::chrono::seconds(1800);

		auto timeDelta = std::chrono::seconds(1500);

		sip->clock().add_diff(timeDelta);

		ASSERT_TRUE(timeDelta > internalContexts[dialog_id]->GetTimerExtention().refreshPeriod / 2); //if the condition is true in Timeout() is called new InviteMethod
		sip->Timeout();
		msg = GetMessageFromParser(sip);
		//Refresh Invite
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(msg->Encode(data), TSIPErrorCodes::e_ok);
		ASSERT_TRUE(strstr(data.c_str(), "INVITE"));
		ASSERT_TRUE(strstr(data.c_str(), "Session-Expires"));

		//call termination testing
		EXPECT_CALL(*confProtocol, Hangup(::testing::Eq<string_view>(dialog_id)))
			.Times(AtLeast(1));
		timeDelta = std::chrono::seconds(1800);

		sip->clock().add_diff(timeDelta);

		ASSERT_TRUE(timeDelta > internalContexts[dialog_id]->GetTimerExtention().refreshPeriod / 2);
		sip->Timeout();
		msg = GetMessageFromParser(sip);
		ASSERT_TRUE(msg->IsValid());
		ASSERT_EQ(msg->Encode(data), TSIPErrorCodes::e_ok);
		ASSERT_TRUE(strstr(data.c_str(), "BYE"));
	}

	TEST_F(SIPParserSniffTest, RetryAfter_RawTest)
	{
		const char *from_id = "vialalekseev@rugjr.trueconf.name";
		const char *to_id = "@212.13.98.248";
		char busy_here_packet[1024] = { 0 };
		net::Endpoint remoteEp{ net::address_v4::from_string("212.13.98.248") , 5060, net::protocol::TCP };
		std::string generated_dialog_id;
		std::string generated_branch;

		auto pInternal_context = InviteFromParser(from_id, to_id, generated_dialog_id, generated_branch);
		ASSERT_TRUE(pInternal_context);

		// 486 busy here with "Retry-After 5"
		sprintf(busy_here_packet, sipraw::busy_here_packet_prototype, generated_branch.c_str(), generated_dialog_id.c_str());
		auto rsp = ReceivePacket(busy_here_packet, remoteEp);
		ASSERT_TRUE(rsp);

		ASSERT_EQ(TSIPErrorCodes::e_ok, rsp->Decode(busy_here_packet, strlen(busy_here_packet)));
		ASSERT_TRUE(sip->SetRecvMsg_SIP(rsp, remoteEp));
		ASSERT_GT(rsp->GetRetryAfterInterval(), std::chrono::steady_clock::duration());
		ASSERT_TRUE(pInternal_context->RetryAfterTime() > std::chrono::steady_clock::now());	// verify that reinvite time was changed to future

		SendACK();

		// reinvite part
		pInternal_context->RetryAfterTime(std::chrono::steady_clock::now());					// not need to wait in test
		sip->Timeout();																			// InviteMethod must be called
		auto msg_ctx = sip->GetMsgForSend_SIP();
		auto ctx = msg_ctx.first;
		ASSERT_TRUE(ctx); ASSERT_TRUE(msg_ctx.second);
		ASSERT_EQ(TYPE_INVITE, msg_ctx.second->GetMethod());									// verify that InviteMethod really was called
	};

	TEST_F(SIPParserSniffTest, MovedTemporarily){
		const char from_id[] = "vialalekseev@rugjr.trueconf.name";
		const char to_id[] = "@192.168.62.170";
		net::Endpoint remoteEp{ net::address_v4::from_string("192.168.62.170") , 5060 , net::protocol::UDP };
		std::string generated_dialog_id;
		std::string generated_branch;

		ASSERT_TRUE(InviteFromParser(from_id, to_id, generated_dialog_id, generated_branch));

		// SIP/2.0 302 Moved Temporarily
		char test_packet[1024] = { 0 };
		sprintf(test_packet, sipraw::MovedTemporarily_packet, generated_branch.c_str(), generated_dialog_id.c_str());
		auto rsp = ReceivePacket(test_packet, remoteEp);
		ASSERT_TRUE(rsp);

		auto sip_meta = rsp->GetSIPMetaField();
		ASSERT_TRUE(sip_meta); ASSERT_TRUE(sip_meta->iContact);
		auto contact_uri = sip_meta->iContact->GetLastURI();
		ASSERT_TRUE(contact_uri);

		std::string received_contact_uri;
		ASSERT_TRUE(contact_uri->GetRequestURI(received_contact_uri));

		SendACK();

		auto req = VerifyInviteSendedInTimeout();
		ASSERT_TRUE(req);

		std::string req_uri = req->GetTo();
		ASSERT_STREQ(req_uri.c_str(), received_contact_uri.c_str());
	}

	TEST_F(SIPParserSniffTest, MovedTemporarily_SeveralContacts){
		const char from_id[] = "vialalekseev@rugjr.trueconf.name";
		const char to_id[] = "@192.168.62.170";
		net::Endpoint remoteEp{ net::address_v4::from_string("192.168.62.170") , 5060 , net::protocol::UDP };
		std::string generated_dialog_id;
		std::string generated_branch;

		ASSERT_TRUE(InviteFromParser(string_view{ from_id, sizeof(from_id) - 1 }, string_view{ to_id, sizeof(to_id) -1 }
			, generated_dialog_id, generated_branch));
		char test_packet[1024] = { 0 };

		// SIP/2.0 302 Moved Temporarily
		sprintf(test_packet, sipraw::MovedTemporarilySeveralContacts_packet, generated_branch.c_str(), generated_dialog_id.c_str());
		auto rsp = ReceivePacket(test_packet, remoteEp);
		ASSERT_TRUE(rsp);

		SendACK();

		auto req = VerifyInviteSendedInTimeout();
		ASSERT_TRUE(req);

		generated_branch = std::string(req->Branch());

		// 404 Not Found
		sprintf(test_packet, sipraw::NotFound_packet, generated_branch.c_str(), generated_dialog_id.c_str());
		auto rsp1 = ReceivePacket(test_packet, remoteEp);
		ASSERT_TRUE(rsp1);

		SendACK();

		VerifyInviteSendedInTimeout();
	}

	TEST_F(SIPParserSniffTest, WebinarChatCommand) {
		using ::testing::_;
		using ::testing::Eq;
		using ::testing::Return;

		UserStatusInfo user; boost::get<UserStatusInfo::User>(user.info).status = VS_UserPresence_Status::USER_LOGOFF;
		ON_CALL(*confProtocol, GetUserStatus(_)).WillByDefault(Return(user));

		net::Endpoint remoteEp{ net::address_v4::from_string("192.168.74.4") , 5070 , net::protocol::TLS };
		const char remote_call_id[] = "#sip:user12@skype2015.loc";

		// 0. Receive command via sip char '/webinar us2 "Internal"'
		EXPECT_CALL(*confProtocol, S4B_InitBeforeCall(::testing::An<string_view>(), Eq(remote_call_id), _)).Times(1);
		auto webinar_command = ReceivePacket(sipraw::WebinarCommand, remoteEp);

		// 1. Verify we have parsed webinar topic succesfully
		const auto internalContexts = GetInternalContextsTest();
		auto webinar_ctx_it = std::find_if(internalContexts.begin(), internalContexts.end(), [](const std::pair<std::string, std::shared_ptr<VS_SIPParserInfo>> &p) {
			return p.second->IsGroupConf() && p.second->IsPublicConf();
		});
		ASSERT_NE(webinar_ctx_it, internalContexts.end());
		auto webinar_ctx = webinar_ctx_it->second;
		ASSERT_NE(webinar_ctx, nullptr);
		ASSERT_STREQ(webinar_ctx->GetConfTopic().c_str(), "Internal");

		// 2. Send 200 OK
		auto _200ok = SendResponse();
		ASSERT_NE(_200ok, nullptr);
		ASSERT_EQ(_200ok->GetResponseCode(), 200);

		SendRequest();	// req Invite to skype

		// 3. Send answer for chat command
		auto m = SendRequest();
		ASSERT_NE(m, nullptr);
		ASSERT_EQ(m->GetContentType(), CONTENTTYPE_TEXT_PLAIN);

		auto sip_message = m->GetSIPInstantMessage();
		ASSERT_NE(sip_message, nullptr);
		auto text = sip_message->GetMessageText();
		ASSERT_TRUE(text.length() > 0);
	}

	TEST_F(SIPParserTest, DecodeUnauthorized) {
		{
			boost::shared_ptr<VS_SIPMessage> msg = boost::make_shared<VS_SIPMessage>();
			TSIPErrorCodes err = msg->Decode(raw_lync_unauthorized, strlen(raw_lync_unauthorized));
			ASSERT_TRUE(err == TSIPErrorCodes::e_ok);

			auto auth_info = msg->GetAuthInfo();
			ASSERT_TRUE(auth_info != nullptr);
		}

		{
			boost::shared_ptr<VS_SIPMessage> msg = boost::make_shared<VS_SIPMessage>();
			TSIPErrorCodes err = msg->Decode(raw_lync_unauthorized2, strlen(raw_lync_unauthorized2));
			ASSERT_TRUE(err == TSIPErrorCodes::e_ok);

			auto auth_info = msg->GetAuthInfo();
			ASSERT_TRUE(auth_info != nullptr);
			auto auth = std::dynamic_pointer_cast<VS_SIPAuthGSS>(auth_info);
			ASSERT_TRUE(auth);
			ASSERT_TRUE(auth->gssapi_data().is_initialized());
			ASSERT_TRUE(auth->gssapi_data()->length() > 0);
			ASSERT_STREQ(auth->opaque().c_str(), "78D29AC8");
			ASSERT_STREQ(auth->realm().c_str(), "SIP Communications Service");
			ASSERT_STREQ(auth->targetname().c_str(), "LYNC2013-SERVER.lync.loc");
		}
	}

	TEST_F(SIPParserTest, DecodeOk) {
		auto msg = boost::make_shared<VS_SIPMessage>();
		TSIPErrorCodes err = msg->Decode(raw_lync_ok, strlen(raw_lync_ok));
		ASSERT_TRUE(err == TSIPErrorCodes::e_ok);

		auto *sdp_meta = msg->GetSDPMetaField();
		ASSERT_NE(sdp_meta, nullptr);

		auto media_streams = sdp_meta->iMediaStreams;
		ASSERT_EQ(media_streams.size(), 2);

		auto *s1 = media_streams[0], *s2 = media_streams[1];
		ASSERT_STREQ(s1->GetRemoteIceUfrag().c_str(), "0sfO");
		ASSERT_STREQ(s1->GetRemoteIcePwd().c_str(), "OjyZ/pr8oLuDPWcmj9gDMEav");
		ASSERT_STREQ(s1->GetRemoteCryptoKey().c_str(), "9+s+/+675CaW+4oOF+C4ViBZc2nIjsFEVVoJbL0J");

		ASSERT_TRUE(s2->GetRemoteCodecs().size() == 1);
		ASSERT_STREQ(s2->GetRemoteIceUfrag().c_str(), "bjVF");
		ASSERT_STREQ(s2->GetRemoteIcePwd().c_str(), "Gqh3DXwZsRGEbbnx5TuaOYYk");
		ASSERT_STREQ(s2->GetRemoteCryptoKey().c_str(), "5tBMMHlqa8Ms9KaXJhU8CpES7IVilFxRZq2w6kzP");
	}
	TEST_F(SIPParserTest, DecodeImage) {
		auto msg = boost::make_shared<VS_SIPMessage>();
		TSIPErrorCodes err = msg->Decode(raw_sip_image_ok, strlen(raw_sip_image_ok));
		ASSERT_TRUE(err == TSIPErrorCodes::e_ok);

		auto *sdp_meta = msg->GetSDPMetaField();
		ASSERT_NE(sdp_meta, nullptr);

		auto media_streams = sdp_meta->iMediaStreams;
		ASSERT_EQ(media_streams.size(), 2);

		auto *s1 = media_streams[0], *s2 = media_streams[1];
		ASSERT_EQ(s1->GetMediaType(), SDPMediaType::audio);
		ASSERT_STREQ(s1->GetHost().c_str(), "192.168.120.1");

		ASSERT_EQ(s2->GetMediaType(), SDPMediaType::invalid);
	}

	TEST_F(SIPParserTest, DecodeMultipartSDP) {
		auto msg = boost::make_shared<VS_SIPMessage>();
		TSIPErrorCodes err = msg->Decode(raw_lync_invite, strlen(raw_lync_invite));
		ASSERT_TRUE(err == TSIPErrorCodes::e_ok);
		ASSERT_EQ(msg->GetContentType(), CONTENTTYPE_SDP);

		auto *sdp_meta = msg->GetSDPMetaField();
		ASSERT_NE(sdp_meta, nullptr);

		auto media_streams = sdp_meta->iMediaStreams;
		ASSERT_EQ(media_streams.size(), 2);

		auto *s1 = media_streams[0], *s2 = media_streams[1];
		ASSERT_EQ(s1->GetMediaType(), SDPMediaType::audio);
		ASSERT_EQ(s1->GetPort(), 11650);

		ASSERT_EQ(s2->GetMediaType(), SDPMediaType::video);
		ASSERT_EQ(s2->GetPort(), 4778);
	}

	TEST(UUID, GenUUID) {
		const char test_epid[] = "01010101";
		const char test_uuid[] = "4b1682a8-f968-5701-83fc-7c6741dc6697";

		{	VS_UUID uuid(test_uuid);
			std::string str = static_cast<std::string>(uuid);

			ASSERT_STREQ(str.c_str(), test_uuid);
		}

		{
			std::string str = VS_UUID::GenUUID(test_epid);
			ASSERT_STREQ(str.c_str(), test_uuid);
		}
	}

	TEST(UUID, GenEpid) {
		EXPECT_STREQ("ff518848d1", VS_UUID::GenEpid("user2@skype2015.loc", "tc", "10.110.14.101").c_str());
	}

	struct SkypeSniffTest : public SIPParserSniffTest {
		std::string dialog_id = "a23ba67b2b0b4db5b10db2b1e7b5ca7c";	// call_id from sipraw::InvitePacket

		void MakeInviteFromSkype() {
			auto incoming_invite_from_skype = std::make_shared<VS_SIPMessage>();
			ASSERT_TRUE(incoming_invite_from_skype->Decode(sipraw::InvitePacket, sizeof(sipraw::InvitePacket)) == TSIPErrorCodes::e_ok);

			net::Endpoint remoteEp{ net::address_v4::from_string("192.168.74.12"), 5070, net::protocol::TLS };
			ASSERT_TRUE(sip->SetRecvMsg_SIP(incoming_invite_from_skype, remoteEp));
		}
	};

	// to make sure XH264UC is present in DEFAULT_ENABLED_CODECS and after removing we'll see it
	TEST_F(SkypeSniffTest, XH264UC_in_default) {
		MakeInviteFromSkype();

		sip->InviteReplay(dialog_id,(VS_CallConfirmCode)1, false);

		//Trying
		auto m = sip->GetMsgForSend_SIP(dialog_id);
		auto ctx = m.first;
		auto msg = m.second;
		ASSERT_NE(ctx, nullptr);
		ASSERT_NE(msg, nullptr);
		ASSERT_EQ(100, msg->GetResponseCode());

		//Ringing
		m = sip->GetMsgForSend_SIP(dialog_id);
		ctx = m.first;
		msg = m.second;
		ASSERT_NE(ctx, nullptr);
		ASSERT_NE(msg, nullptr);
		ASSERT_EQ(200, msg->GetResponseCode());

		// emulate sip::TransportLayer::FillMsgOutgoing
		ctx->SetMyMediaAddress(net::address::from_string("1.2.3.4"));
		EXPECT_TRUE(msg->UpdateOrInsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Connection, static_cast<VS_SIPGetInfoImpl>(*ctx)));
		EXPECT_TRUE(msg->UpdateOrInsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Origin, static_cast<VS_SIPGetInfoImpl>(*ctx)));

		std::string response200ok;
		ASSERT_EQ(msg->Encode(response200ok), TSIPErrorCodes::e_ok);
		ASSERT_NE(response200ok.find("X-H264UC"),std::string::npos);
	}

	// test to recognize VideoSourceRequest i.e. 'src' in sdp: "a=rtcp-fb:* x-message app send:src,x-pli recv:src,x-pli"
	TEST_F(SkypeSniffTest, Recognize_VideoSourceRequest_In_SDP) {
		using ::testing::_;
		using ::testing::SaveArg;
		using ::testing::Return;

		MakeInviteFromSkype();

		std::vector<VS_MediaChannelInfo> channels;
		EXPECT_CALL(*confProtocol, SetMediaChannels(::testing::An<string_view>(), _, _, _)).WillOnce(::testing::DoAll(SaveArg<1>(&channels), Return(true)));
		sip->InviteReplay(dialog_id, (VS_CallConfirmCode)1, false);

		auto video_ch_it = std::find_if(channels.cbegin(), channels.cend(), [](const VS_MediaChannelInfo &i) { return i.type == SDPMediaType::video; });
		ASSERT_NE(video_ch_it, channels.cend());
		ASSERT_FALSE(std::none_of(video_ch_it->rcv_modes_video.cbegin(), video_ch_it->rcv_modes_video.cend(), [](const VS_GatewayVideoMode& x) { return x.IsFIRSupported; }));

	}

	TEST_F(SIPParserTest, DecodeSeveralContacts){
		VS_SIPMessage m; m.Decode(sipraw::MovedTemporarilySeveralContacts1_packet, strlen(sipraw::MovedTemporarilySeveralContacts1_packet));
		auto pMeta = m.GetSIPMetaField();
		ASSERT_TRUE(pMeta && pMeta->iContact);

		std::vector<std::string> contacts;
		pMeta->iContact->GetURIs(contacts);

		/*
			"Contact: <sip:192.168.11.150:5060;transport=UDP>, <sip:hdx8000@192.168.62.42:5060>\r\n"
			"Contact: <sip:1.2.3.4:5060;transport=UDP>, <sip:gvc3200@192.168.62.170:5060>\r\n"
		*/
		ASSERT_EQ(contacts.size(), 4);
	}

	TEST_F(SIPParserTest, DecodeContactWithGruu) {
		VS_SIPMessage m; m.Decode(sipraw::_200ok_contact_with_gruu, strlen(sipraw::_200ok_contact_with_gruu));
		auto pMeta = m.GetSIPMetaField();
		ASSERT_TRUE(pMeta && pMeta->iContact);

		auto gruu = pMeta->iContact->LastGruu();
		EXPECT_STREQ(gruu.c_str(), "sip:user6@skype2015.loc;opaque=user:epid:a1De4AWnzFmb-tN1kxQSMQAA;gruu");
	}

	TEST_F(SIPParserTest, SIP_URI_with_ms_extensions) {
		VS_SIPMessage m;
		ASSERT_EQ(m.Decode(sipraw::InviteWithExtendedRecordRoute, strlen(sipraw::InviteWithExtendedRecordRoute)), TSIPErrorCodes::e_ok);

		auto sip_fields = m.GetSIPMetaField();
		ASSERT_NE(sip_fields, nullptr);

		EXPECT_FALSE(sip_fields->iRouteSet.empty());

		for (auto& record_route : sip_fields->iRouteSet)
		{
			auto sip_uris = record_route->GetURIs();
			EXPECT_FALSE(sip_uris.empty());
			for (auto& route_uri : sip_uris)
			{
				ASSERT_NE(route_uri, nullptr);
				VS_SIPBuffer encoded_uri;
				EXPECT_EQ(route_uri->Encode(encoded_uri), TSIPErrorCodes::e_ok);
				encoded_uri.AddData("\r\n");

				char uri[1024] = {};
				size_t size(1024);
				EXPECT_EQ(encoded_uri.GetNextBlock(uri, size), TSIPErrorCodes::e_ok);
				string_view uri_view(uri, size);
				EXPECT_FALSE(uri_view.empty());

				EXPECT_TRUE(uri_view.find("ms-fe=") != string_view::npos);
				EXPECT_TRUE(uri_view.find("ms-role-rs-to") != string_view::npos);
				EXPECT_TRUE(uri_view.find("ms-role-rs-from") != string_view::npos);
				EXPECT_TRUE(uri_view.find("ms-ent-dest") != string_view::npos);
				EXPECT_TRUE(uri_view.find("ms-opaque=") != string_view::npos);
				EXPECT_TRUE(uri_view.find("ms-key-info=") != string_view::npos);
				EXPECT_TRUE(uri_view.find("ms-identity=") != string_view::npos);
			}
		}
	}

	TEST_F(SIPParserTest, DecodeChatInvite) {
		auto msg = boost::make_shared<VS_SIPMessage>();
		TSIPErrorCodes err = msg->Decode(raw_lync_chat_invite, strlen(raw_lync_chat_invite));
		ASSERT_TRUE(err == TSIPErrorCodes::e_ok);
		ASSERT_EQ(msg->GetContentType(), CONTENTTYPE_SDP);

		auto *sdp_meta = msg->GetSDPMetaField();
		ASSERT_NE(sdp_meta, nullptr);

		auto media_streams = sdp_meta->iMediaStreams;
		ASSERT_EQ(media_streams.size(), 1);

		const auto *ms = media_streams[0];
		ASSERT_EQ(ms->GetMediaType(), SDPMediaType::message);
		ASSERT_EQ(ms->GetPort(), 5060);
		ASSERT_EQ(ms->GetProto(), SDP_PROTO_SIP);
		ASSERT_STREQ(ms->GetMessageURL().c_str(), "null");
	}

	TEST_F(SIPParserTest, DecodeChatOk) {
		auto msg = boost::make_shared<VS_SIPMessage>();
		TSIPErrorCodes err = msg->Decode(raw_lync_chat_ok, strlen(raw_lync_chat_ok));
		ASSERT_TRUE(err == TSIPErrorCodes::e_ok);
		ASSERT_EQ(msg->GetContentType(), CONTENTTYPE_SDP);

		auto *sdp_meta = msg->GetSDPMetaField();
		ASSERT_NE(sdp_meta, nullptr);

		auto media_streams = sdp_meta->iMediaStreams;
		ASSERT_EQ(media_streams.size(), 1);

		const auto *ms = media_streams[0];
		ASSERT_EQ(ms->GetMediaType(), SDPMediaType::message);
		ASSERT_EQ(ms->GetPort(), 5060);
		ASSERT_EQ(ms->GetProto(), SDP_PROTO_SIP);
		ASSERT_STREQ(ms->GetMessageURL().c_str(), "sip:user1@skype2015.loc");
	}

	TEST_F(SIPParserTest, Invite_with_tel_event) {
		using ::testing::_;
		using ::testing::AtLeast;
		using ::testing::Invoke;
		using ::testing::InvokeArgument;
		using ::testing::DoAll;
		using ::testing::SaveArg;

		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, _, _))
			.WillOnce(DoAll(InvokeArgument<4>(false, VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE, ""),
			Invoke([this](string_view , const gw::Participant&, string_view, const VS_ConferenceInfo&, const boost::function<void(bool redirect, VS_ConferenceProtocolInterface::ConferenceStatus status, const std::string &ip0)> &, string_view, bool, bool) {
			auto msg = GetMessageFromParser(sip);
			ASSERT_NE(msg, nullptr);
			ASSERT_EQ(msg->GetMessageType(), MESSAGE_TYPE_RESPONSE);
			ASSERT_EQ(msg->GetMethod(), TYPE_INVITE);
		})));

		ASSERT_TRUE(SetRecvBuf(raw_invite_with_tel_event, strlen(raw_invite_with_tel_event)));
	}

	TEST_F(SIPParserTest, TransportSelectionFromCallID) {
		// TLS
		{
			const char to_callid[] = "#sip:@192.168.41.156;transport=tls";
			VS_CallConfig config;
			auto &&dialog_id = sip->NewDialogID(string_view{ to_callid, sizeof(to_callid) - 1 }, {}, config);

			config.SignalingProtocol = VS_CallConfig::SIP;
			config.Address.addr = net::address_v4::from_string("192.168.41.156");

			ASSERT_TRUE(identifier_impl->PostResolve(config, to_callid, nullptr, false));
			ASSERT_EQ(config.ConnectionTypeSeq.size(), 1);
			ASSERT_EQ(config.ConnectionTypeSeq[0], net::protocol::TLS);
			ASSERT_EQ(config.Address.protocol, config.ConnectionTypeSeq[0]);
		}
		// TCP
		{
			const char to_callid[] = "#sip:@192.168.41.156;transport=tcp";
			VS_CallConfig config;
			auto &&dialog_id = sip->NewDialogID(string_view{ to_callid, sizeof(to_callid) - 1 }, {}, config);

			config.SignalingProtocol = VS_CallConfig::SIP;
			config.Address.addr = net::address_v4::from_string("192.168.41.156");

			ASSERT_TRUE(identifier_impl->PostResolve(config, to_callid, nullptr, false));
			ASSERT_EQ(config.ConnectionTypeSeq.size(), 1);
			ASSERT_EQ(config.ConnectionTypeSeq[0], net::protocol::TCP);
			ASSERT_EQ(config.Address.protocol, config.ConnectionTypeSeq[0]);
		}
		// UDP
		{
			const char to_callid[] = "#sip:@192.168.41.156;transport=udp";
			VS_CallConfig config;
			auto &&dialog_id = sip->NewDialogID(string_view{ to_callid, sizeof(to_callid) - 1 }, {}, config);

			config.SignalingProtocol = VS_CallConfig::SIP;
			config.Address.addr = net::address_v4::from_string("192.168.41.156");

			ASSERT_TRUE(identifier_impl->PostResolve(config, to_callid, nullptr, false));
			ASSERT_EQ(config.ConnectionTypeSeq.size(), 1);
			ASSERT_EQ(config.ConnectionTypeSeq[0], net::protocol::UDP);
			ASSERT_EQ(config.Address.protocol, config.ConnectionTypeSeq[0]);
		}
		// default: TCP, UDP, TLS_5061
		{
			const char to_callid[] = "#sip:@192.168.41.156";
			VS_CallConfig config;
			auto &&dialog_id = sip->NewDialogID(string_view{ to_callid, sizeof(to_callid) - 1 }, {}, config);

			config.SignalingProtocol = VS_CallConfig::SIP;
			config.Address.addr = net::address_v4::from_string("192.168.41.156");

			ASSERT_TRUE(identifier_impl->PostResolve(config, to_callid, nullptr, false));
			ASSERT_EQ(config.ConnectionTypeSeq.size(), 3);
			ASSERT_EQ(config.ConnectionTypeSeq[0], net::protocol::TCP);
			ASSERT_EQ(config.ConnectionTypeSeq[1], net::protocol::UDP);
			ASSERT_EQ(config.ConnectionTypeSeq[2], net::protocol::TLS);
		}
	}

	struct SIPParserTestUnicode :SIPParserTest, testing::WithParamInterface<std::string> {};

	TEST_P(SIPParserTestUnicode, InviteWithUnicodeUserName) {
		const VS_CallConfig::EscapeMethod availavle_ethods[] = { VS_CallConfig::EscapeMethod::Unicode, VS_CallConfig::EscapeMethod::URI };
		for (const auto& method : availavle_ethods)
		{
			const char *to_callid = GetParam().c_str();
			VS_CallConfig config;
			auto &&dialog_id = sip->NewDialogID(to_callid, {}, config);

			config.sip.UriEscapeMethod = method;
			config.SignalingProtocol = VS_CallConfig::SIP;
			config.Address.addr = net::address_v4::from_string("192.168.41.195");

			ASSERT_TRUE(identifier_impl->PostResolve(config, to_callid, nullptr, false));
			ASSERT_TRUE(sip->InviteMethod(dialog_id, "from@10.10.11.30,", config.resolveResult.NewCallId, VS_ConferenceInfo(false, false)));

			auto msg = GetMessageFromParser(sip);
			ASSERT_NE(msg, nullptr);
			ASSERT_EQ(msg->GetMessageType(), MESSAGE_TYPE_REQUEST);
			ASSERT_EQ(msg->GetMethod(), TYPE_INVITE);
		}

	}

	INSTANTIATE_TEST_CASE_P(WithValues,
		SIPParserTestUnicode,
		::testing::Values(														// names was taken from here https://www.behindthename.com/names/usage/turkish
			"\xD0\xB2\xD0\xB0\xD1\x81\xD1\x8F@192.168.41.195",					// russian вася
			"\x41\x41\x52\xC3\x93\x4E@192.168.41.195",							// spanish sounds like 'AARON'
			"\xE6\x84\x9B\xE8\x97\x8D@192.168.41.195",							// japanese sounds like 'AI'
			"\xE7\x99\xBD\xE7\x99\xBE\xE6\x9F\x8F@192.168.41.195",				// chinese sounds like 'BAI'
			"\x41\x42\x44\xC3\x9C\x4C\x48\x41\x4D\xC4\xB0\x54@192.168.41.195"	// turkish sounds like 'abduhalmit'
		));

	struct SIPParserMock : public  SIPParserExtends {
	public:
		MOCK_METHOD5_OVERRIDE(Chat, void (string_view, const std::string&, const std::string&, const std::string&, const char*));
		MOCK_METHOD1_OVERRIDE(FindActiveCallCtx, std::shared_ptr<VS_SIPParserInfo> (const std::shared_ptr<VS_SIPParserInfo>&));
		std::string ProcessConfCommand(const std::shared_ptr<VS_SIPParserInfo>& ctx, string_view from, const std::vector<std::string>& args) override {
			new_ctx = ctx;
			return VS_SIPParser::ProcessConfCommand(ctx, from, args);
		}

		std::shared_ptr<VS_SIPParserInfo> new_ctx;

	protected:
		SIPParserMock(boost::asio::io_service::strand& strand)
			: SIPParserExtends(strand, "serverVendor", nullptr)
		{
		}

		static void PostConstruct(std::shared_ptr<SIPParserMock> &) { /*stub*/ }
	};

	struct ChatBotTest : SIPParserTest {
		std::shared_ptr<VS_SIPParserInfo> ctx;
		VS_CallConfig* pCfg = nullptr;

		void SetUp() override
		{
			SIPParserTestBase::SetUp();

			sip = vs::MakeShared<SIPParserMock>(strand);

			sip->SetMyCsAddress(vcs_addr);
			sip->SetPeerCSAddress({}, terminal_addr);
			sip->SetCallConfigStorage(call_config);
			sip->SetConfCallBack(confProtocol);
			sip->SetPolicy(boost::make_shared<VS_Policy>("SIP"));

			ctx = std::make_shared<VS_SIPParserInfo>("serverVendor");
			ctx->SetConfig(VS_CallConfig{});
			pCfg = &ctx->GetConfig();
			verify_invitation = true;
		}

		UserStatusInfo MakeUser(const std::string &real_id, VS_UserPresence_Status st) {
			UserStatusInfo user;
			user.real_id = real_id;
			boost::get<UserStatusInfo::User>(user.info).status = st;
			return user;
		}

		enum verification_type {
			equal,
			begin_with
		} v;

		std::string expected_chat_answer;
		ChatBotTest& SetExpectedAnswer(const std::string &answer, verification_type v_ = equal) { expected_chat_answer = answer; v = v_; return *this; }
		ChatBotTest& SetExpectedAnswerBeginWith(const std::string &answer) {
			return SetExpectedAnswer(answer, begin_with);
		}

		std::string user_call_to;
		VS_UserPresence_Status user_status = VS_UserPresence_Status::USER_STATUS_UNDEF;
		ChatBotTest& SetCallToUser(const std::string &s, VS_UserPresence_Status st) {
			user_call_to = s;
			user_status = st;
			return *this;
		}

		bool verify_invitation = true;
		ChatBotTest& DisablePendingInvitationVerifying() { verify_invitation = false; return *this;}

		std::string chat_command;
		ChatBotTest& SetChatCommand(const std::string &s) { chat_command = s; return *this; }

		ChatBotTest& SetPublicConfContext() {
			ctx->SetGroupConf(true);
			ctx->SetPublicConf(true);
			return *this;
		}

		ChatBotTest& SetSymmetricConfContext() {
			ctx->SetGroupConf(true);
			ctx->SetPublicConf(false);
			return *this;
		}

		ChatBotTest& SetMyTribuneRole(const VS_Participant_Role r) {
			using ::testing::Return;
			using ::testing::_;
			ON_CALL(*confProtocol, GetMyTribuneRole(_)).WillByDefault(Return(r));
			return *this;
		}

		ChatBotTest& SetMyConferenceRole(const VS_Participant_Role r) {
			using ::testing::Return;
			using ::testing::_;
			ON_CALL(*confProtocol, GetMyConferenceRole(_)).WillByDefault(Return(r));
			return *this;
		}

		bool verify_conf_topic = false;
		std::string expected_topic;
		ChatBotTest& SetExpectedConfTopic(const string_view topic) {
			verify_conf_topic = true;
			expected_topic = static_cast<std::string>(topic);
			return *this;
		}

		void MakeTest() {
			using ::testing::_;
			using ::testing::Return;
			using ::testing::StrEq;
			using ::testing::StartsWith;
			using ::testing::An;

			UserStatusInfo user = MakeUser(user_call_to, user_status);
			ON_CALL(*confProtocol, GetUserStatus(_)).WillByDefault(Return(user));
			ON_CALL(*std::static_pointer_cast<SIPParserMock>(sip), FindActiveCallCtx(_)).WillByDefault(Return(ctx));

			switch (v)
			{
			case ChatBotTest::equal:
				EXPECT_CALL(*std::static_pointer_cast<SIPParserMock>(sip), Chat(An<string_view>(), _, _, _, StrEq(expected_chat_answer)));
				break;
			case ChatBotTest::begin_with:
				EXPECT_CALL(*std::static_pointer_cast<SIPParserMock>(sip), Chat(An<string_view>(), _, _, _, StartsWith(expected_chat_answer)));
				break;
			default:
				ASSERT_TRUE(false);
				break;
			}

			sip->ProcessChatCommand(ctx,"from", chat_command);
			if (verify_conf_topic) {
				ASSERT_NE(std::static_pointer_cast<SIPParserMock>(sip)->new_ctx, nullptr);
				EXPECT_STREQ(expected_topic.c_str(), std::static_pointer_cast<SIPParserMock>(sip)->new_ctx->GetConfTopic().c_str());
			}

			if (!verify_invitation) return;

			// for available users or CID, verify invitation
			if (boost::get<UserStatusInfo::User>(user.info).status != VS_UserPresence_Status::USER_AVAIL && !boost::starts_with(user_call_to,"\\c\\")) return;

			auto in_ctxs = GetInternalContextsTest();
			auto it = std::find_if(in_ctxs.cbegin(), in_ctxs.cend(), [&user](const std::pair<std::string, std::shared_ptr<VS_SIPParserInfo>> &p) -> bool{
				if (!p.second) return false;

				auto pending_invites = p.second->PopPendingInvites();
				return std::find(pending_invites.begin(), pending_invites.end(), user.real_id) != pending_invites.end();
			});

			ASSERT_TRUE(it != in_ctxs.end()) << "Pending invite for '" << user.real_id << "' wasn't done\n";
		}


	};

	TEST_F(ChatBotTest, CallToSIPBlocking) {
		this->SetCallToUser("#sip:@1.2.3.4", VS_UserPresence_Status::USER_AVAIL).DisablePendingInvitationVerifying()
			.SetChatCommand("/call #sip:@1.2.3.4")
			.SetExpectedAnswer("User #sip:@1.2.3.4 not found\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, CallToSIPAuthorized) {
		pCfg->isAuthorized = true;
		this->SetCallToUser("#sip:@1.2.3.4", VS_UserPresence_Status::USER_AVAIL)
			.SetChatCommand("/call #sip:@1.2.3.4")
			.SetExpectedAnswer("Inviting #sip:@1.2.3.4\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, ConfToSIPBlocking) {
		this->SetCallToUser("#sip:@1.2.3.4", VS_UserPresence_Status::USER_AVAIL).DisablePendingInvitationVerifying()
			.SetChatCommand("/conf #sip:@1.2.3.4")
			.SetExpectedAnswer("User #sip:@1.2.3.4 not found\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, ConfToSIPAuthorized) {
		pCfg->isAuthorized = true;
		this->SetCallToUser("#sip:@1.2.3.4", VS_UserPresence_Status::USER_AVAIL)
			.SetChatCommand("/conf #sip:@1.2.3.4")
			.SetExpectedAnswer("Inviting #sip:@1.2.3.4\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, AcceptReqInvite) {
		ctx->AddPendingReqInviteUser("us1@srv.server.name");
		this->SetChatCommand("/y").SetExpectedAnswerBeginWith("Accepting us1@srv.server.name").DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, AcceptReqInviteByShortName) {
		ctx->AddPendingReqInviteUser("us1@srv.server.name");
		this->SetChatCommand("/y u").SetExpectedAnswerBeginWith("Accepting us1@srv.server.name").DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, RejectReqInvite) {
		ctx->AddPendingReqInviteUser("us1@srv.server.name");
		this->SetChatCommand("/n").SetExpectedAnswerBeginWith("Rejecting us1@srv.server.name").DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, RejectReqInviteByShortName) {
		ctx->AddPendingReqInviteUser("us1@srv.server.name");
		this->SetChatCommand("/n u").SetExpectedAnswerBeginWith("Rejecting us1@srv.server.name").DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, CantFindUniqueName) {
		ctx->AddPendingReqInviteUser("us1@srv.server.name");
		ctx->AddPendingReqInviteUser("us2@srv.server.name");
		this->SetChatCommand("/y").SetExpectedAnswer(bot::USER_NOT_RECOGNIZED_ANSWER).DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, CantFindUniqueNameByShortName) {
		ctx->AddPendingReqInviteUser("us1@srv.server.name");
		ctx->AddPendingReqInviteUser("us2@srv.server.name");
		this->SetChatCommand("/y us").SetExpectedAnswer(bot::USER_NOT_RECOGNIZED_ANSWER).DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, FindUniqueNameByShortName) {
		ctx->AddPendingReqInviteUser("us1@srv.server.name");
		ctx->AddPendingReqInviteUser("us2@srv.server.name");
		this->SetChatCommand("/y us2").SetExpectedAnswerBeginWith("Accepting us2@srv.server.name").DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, InviteWithLeaderRights) {
		using ::testing::_;

		const char call[] = "us1@my.server.name#vcs";

		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, ::testing::Eq(string_view{ call, sizeof(call) - 1 }), _, _, _, false, _));

		this->SetChatCommand("/invite us1")
			.SetCallToUser(call, VS_UserPresence_Status::USER_AVAIL)
			.SetExpectedAnswerBeginWith("Inviting " + std::string(call) + "\n")
			.SetSymmetricConfContext()
			.SetMyConferenceRole(PR_LEADER)
			.DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, InviteWithCommonRights) {
		using ::testing::_;
		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, false, _)).Times(0);

		this->SetChatCommand("/invite us1")
			.SetCallToUser("us1@my.server.name#vcs", VS_UserPresence_Status::USER_AVAIL)
			.SetExpectedAnswerBeginWith("You do not have rights for that in this conference")
			.SetSymmetricConfContext()
			.SetMyConferenceRole(PR_COMMON)
			.DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, RemoveWithCommonRights) {
		using ::testing::_;
		EXPECT_CALL(*confProtocol, KickFromConference(_, _, _)).Times(0);

		this->SetChatCommand("/remove us1")
			.SetCallToUser("us1@my.server.name#vcs", VS_UserPresence_Status::USER_BUSY)
			.SetExpectedAnswerBeginWith("You do not have rights for that in this conference")
			.SetSymmetricConfContext()
			.SetMyConferenceRole(PR_COMMON)
			.DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, RemoveWithLeaderRights) {
		using ::testing::_;

		const char call[] = "us1@my.server.name#vcs";

		EXPECT_CALL(*confProtocol, KickFromConference(_, _, ::testing::Eq(string_view{ call, sizeof(call) - 1 })));

		this->SetChatCommand("/remove us1")
			.SetCallToUser(call, VS_UserPresence_Status::USER_BUSY)
			.SetExpectedAnswer("Kicking " +  std::string(call) + "\n")
			.SetSymmetricConfContext()
			.SetMyConferenceRole(PR_LEADER)
			.DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, TakeInPublicConference) {
		using ::testing::_;

		// when tribune role is PR_COMMON we must succesfully take tribune
		EXPECT_CALL(*confProtocol, TakeTribune(_));
		this->SetChatCommand("/take")
			.SetExpectedAnswer("")
			.SetPublicConfContext()
			.SetMyTribuneRole(PR_COMMON)
			.MakeTest();
	}

	TEST_F(ChatBotTest, TakeWhenWeAlreayReporting) {
		using ::testing::_;

		// when tribune role is PR_REPORTER we must NOT take tribune again
		EXPECT_CALL(*confProtocol, TakeTribune(_)).Times(0);

		this->SetChatCommand("/take")
			.SetExpectedAnswer("You are broadcasting already\n")
			.SetPublicConfContext()
			.SetMyTribuneRole(PR_REPORTER)
			.MakeTest();
	}

	TEST_F(ChatBotTest, LeaveInPublicConference) {
		using ::testing::_;
		using ::testing::Return;

		// when tribune role is PR_REPORTER we must succesfully leave tribune
		EXPECT_CALL(*confProtocol, LeaveTribune(_));

		this->SetChatCommand("/leave")
			.SetExpectedAnswer("")
			.SetPublicConfContext()
			.SetMyTribuneRole(PR_REPORTER)
			.MakeTest();
	}

	TEST_F(ChatBotTest, LeaveWhenWeAlreadyNotBrodcating) {
		using ::testing::_;

		// when tribune role is PR_COMMON we must NOT leave tribune again
		EXPECT_CALL(*confProtocol, LeaveTribune(_)).Times(0);
		this->SetChatCommand("/leave")
			.SetExpectedAnswer("You are not broadcasting now\n")
			.SetMyTribuneRole(PR_COMMON)
			.SetPublicConfContext()
			.MakeTest();
	}

	TEST_F(ChatBotTest, TakeInSymmetricConference) {
		this->SetChatCommand("/take")
			.SetExpectedAnswerBeginWith("Not supported in this type of conference")
			.SetSymmetricConfContext()
			.MakeTest();
	}

	TEST_F(ChatBotTest, LeaveInSymmetricConference) {
		this->SetChatCommand("/leave")
			.SetExpectedAnswerBeginWith("Not supported in this type of conference")
			.SetSymmetricConfContext()
			.MakeTest();
	}

	TEST_F(ChatBotTest, PodiumInSymmetricConference) {
		this->SetChatCommand("/podium us1")
			.SetExpectedAnswerBeginWith("Not supported in this type of conference")
			.SetSymmetricConfContext()
			.MakeTest();
	}

	TEST_F(ChatBotTest, FreeInSymmetricConference) {
		this->SetChatCommand("/free us1")
			.SetExpectedAnswerBeginWith("Not supported in this type of conference")
			.SetSymmetricConfContext()
			.MakeTest();
	}

	TEST_F(ChatBotTest, CallCommandSuccess) {
		this->SetCallToUser("us1@my.server.name#vcs", VS_UserPresence_Status::USER_AVAIL)
			.SetChatCommand("/call us1")
			.SetExpectedAnswer("Inviting us1@my.server.name#vcs\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, CallWithMailForm) {	// skype can send "us1@my.server.name#vcs <mailto:us1@my.server.name> "  instead of just "us1@my.server.name" even if you don't type it
		this->SetCallToUser("us1@my.server.name#vcs", VS_UserPresence_Status::USER_AVAIL)
			.SetChatCommand("/call us1@my.server.name <mailto:us1@my.server.name#vcs> ")
			.SetExpectedAnswer("Inviting us1@my.server.name#vcs\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, CallWithFileForm) {
		this->SetCallToUser("us1@my.server.name#vcs", VS_UserPresence_Status::USER_AVAIL)
			.SetChatCommand("/call us1@my.server.name <file://us1@my.server.name> ")
			.SetExpectedAnswer("Inviting us1@my.server.name#vcs\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, CallCommandBadArg) {
		// veify that '/call' with two arguments is not recognized
		this->SetCallToUser("us1@my.server.name#vcs", VS_UserPresence_Status::USER_AVAIL)
			.SetChatCommand("/call us1 us2")
			.SetExpectedAnswer("Command not recognized\n")
			.DisablePendingInvitationVerifying()
			.MakeTest();
	}

	TEST_F(ChatBotTest, CallCommandUserBusy) {
		this->SetCallToUser("us1@my.server.name#vcs", VS_UserPresence_Status::USER_BUSY)
			.SetChatCommand("/call us1")
			.SetExpectedAnswer("User us1@my.server.name#vcs is busy\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, CallCommandUserUnavailable) {
		this->SetCallToUser("us1@my.server.name#vcs", VS_UserPresence_Status::USER_LOGOFF)
			.SetChatCommand("/call us1")
			.SetExpectedAnswer("User us1@my.server.name#vcs is not available\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, CallCommandToConf) {
		using ::testing::_;
		using ::testing::AtLeast;
		using ::testing::Return;

		UserStatusInfo user;
		user.info = UserStatusInfo::Conf{ true };
		EXPECT_CALL(*confProtocol, GetUserStatus(_)).WillRepeatedly(Return(user));
		this->SetChatCommand("/call \\c\\12345")
			.SetExpectedAnswer("Joining \\c\\12345\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, ConfCommandToConf) {
		using ::testing::_;
		using ::testing::AtLeast;
		using ::testing::Return;

		UserStatusInfo user;
		user.info = UserStatusInfo::Conf{ true };
		EXPECT_CALL(*confProtocol, GetUserStatus(_)).WillRepeatedly(Return(user));
		this->SetChatCommand("/conf \\c\\12345")
			.SetExpectedAnswer("Joining \\c\\12345\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, Webinar) {
		this->SetChatCommand("/webinar us1 \"Webinar 1\"")
			.SetExpectedAnswer("Inviting us1@my.server.name#vcs\n")
			.SetExpectedConfTopic("Webinar 1")
			.SetCallToUser("us1@my.server.name#vcs", VS_UserPresence_Status::USER_AVAIL)
			.MakeTest();
	}

	TEST_F(ChatBotTest, ConfCommandToConfAndToUser) {
		this->SetChatCommand("/conf some_user \\c\\12345")
			.SetExpectedAnswer("Command not recognized\n")
			.MakeTest();
	}

	TEST_F(ChatBotTest, EmptyInvite) {
		this->SetChatCommand("/invite")
			.SetExpectedAnswerBeginWith("Username required.")
			.MakeTest();
	}

	TEST_F(ChatBotTest, EmptyRemove) {
		this->SetChatCommand("/remove")
			.SetExpectedAnswerBeginWith("Username required.")
			.MakeTest();
	}

	struct SIPParserFake : public  SIPParserExtends {
	public:
		std::shared_ptr<VS_SIPParserInfo> GetParserContext(string_view /*dialog_id*/, bool /*create*/ = false) override {
			auto ctx = std::make_shared<VS_SIPParserInfo>("serverVendor");
			ctx->IsAnswered(false);
			ctx->SetConfig(VS_CallConfig{});
			ctx->AddPendingInviteUser("#sip:user@1.2.3.4");
			return ctx;
		}
	protected:
		SIPParserFake(boost::asio::io_service::strand& strand)
			: SIPParserExtends(strand, "serverVendor", nullptr)
		{
		}

		static void PostConstruct(std::shared_ptr<SIPParserFake>& ){ /*stub*/ }
	};

	struct InviteToCallFromChatBotTest : ChatBotTest {
		void SetUp() override
		{
			SIPParserTestBase::SetUp();

			sip = vs::MakeShared<SIPParserFake>(strand);

			sip->SetMyCsAddress(vcs_addr);
			sip->SetPeerCSAddress({}, terminal_addr);
			sip->SetCallConfigStorage(call_config);
			sip->SetConfCallBack(confProtocol);
			sip->SetPolicy(boost::make_shared<VS_Policy>("SIP"));

			ctx = std::make_shared<VS_SIPParserInfo>("serverVendor");
			ctx->SetConfig(VS_CallConfig{});
		}
	};

	TEST_F(InviteToCallFromChatBotTest, MakeInviteWithoutSessionCreation) {
		using ::testing::_;

		auto rsp = std::make_shared<VS_SIPResponse>();
		rsp->Decode(raw_lync_ok, strlen(raw_lync_ok));

		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, false, _));	// 7 arg = false means that we do not create new RTP session
		CallOnResponse_Code200(rsp);
	}

	TEST_F(SIPParserTest, FillMediaChannelsTest)
	{
		using ::testing::_;
		using ::testing::AnyNumber;
		using ::testing::DoAll;
		using ::testing::Invoke;
		using ::testing::InvokeArgument;
		using ::testing::WithArg;

		std::string dialog_id;
		EXPECT_CALL(*confProtocol, AsyncInvite(_, _, _, _, _, _, _, _))
			.WillOnce(DoAll(InvokeArgument<4>(false, VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE, ""), WithArg<0>(Invoke([&dialog_id](string_view a) { dialog_id = std::string(a); }))));

		EXPECT_CALL(*confProtocol, SetMediaChannels(_, _, _,_))
			.WillRepeatedly(Invoke([](string_view _dialog_id, const std::vector<VS_MediaChannelInfo>& _channels, string_view peer, std::int32_t _bandw_rcv)
		{
			//EXPECT_TRUE(!(_channels.empty()));
			for (auto&& channel : _channels)
			{

				//////Serialize/Deserialize tests
				VS_MediaChannelInfo media_channel_info(123456);
				VS_Container cnt;
				std::string mci_name = "media_channel_info";
				channel.Serialize(cnt, mci_name);
				media_channel_info.Deserialize(cnt, mci_name.c_str());
				bool channelsEq = channel == media_channel_info;
				EXPECT_TRUE(channelsEq);
				////////////////////////////////

				if (channel.IsRecv())
				{
					if (channel.type == SDPMediaType::audio)
					{
						EXPECT_TRUE(!(channel.rcv_modes_audio.empty()));
						for (auto&& mode : channel.rcv_modes_audio)
						{
							EXPECT_EQ(vs::GetCodecClockRateByCodecType(SDPMediaType::audio, mode.CodecType), mode.ClockRate);
						}
					}
					else if (channel.type == SDPMediaType::video)
					{
						EXPECT_TRUE(!(channel.rcv_modes_video.empty()));
						for (auto&& mode : channel.rcv_modes_video)
						{
							EXPECT_EQ(vs::GetCodecClockRateByCodecType(SDPMediaType::video, mode.CodecType), mode.ClockRate);
						}
					}
				}

				else if (channel.IsSend())
				{
					if (channel.type == SDPMediaType::audio)
					{
						EXPECT_EQ(vs::GetCodecClockRateByCodecType(SDPMediaType::audio, channel.snd_mode_audio.CodecType), channel.snd_mode_audio.ClockRate);
					}
					else if (channel.type == SDPMediaType::video)
					{
						EXPECT_EQ(vs::GetCodecClockRateByCodecType(SDPMediaType::video, channel.snd_mode_video.CodecType), channel.snd_mode_video.ClockRate);

					}
				}
				else
				{
					//ERROR: invalid type
					EXPECT_TRUE(false);//grustniy kostyl`- need fix
				}
			}
			return true;
		}
		));

		ASSERT_TRUE(SetRecvBuf(&raw_TANDBERG_772_INVITE[0], sizeof(raw_TANDBERG_772_INVITE)));

		sip->InviteReplay(dialog_id, e_call_ok, false);

	}

	TEST_F(SIPParserSniffTest, UseRegCtx) {

		net::Endpoint remoteEp{ net::address_v4::from_string("192.168.56.1"), 5060, net::protocol::TCP };

		// 1. Registration on our server
		ReceivePacket(sipraw::registerSIP, remoteEp);	// sip register -> tc
		auto unauth = SendResponse();						// tc unauth -> sip
		ASSERT_NE(unauth, nullptr);

		auto auth = unauth->GetAuthInfo();
		ASSERT_NE(auth, nullptr);
		auto nonce = auth->nonce();

		char registerMsgBuf[1024] = {};
		sprintf(registerMsgBuf, sipraw::registerSIP_Nonce, nonce.c_str());
		ReceivePacket(registerMsgBuf, remoteEp);			// sip register -> tc

		// 2. Set values in reg context
		auto &ctxs = GetInternalContextsTest();
		auto it = ctxs.find(string_view("9d9e89325be34fc2868bcab4d9562bb9"));
		ASSERT_NE(it, ctxs.end());

		auto regCtx = it->second;
		ASSERT_NE(regCtx, nullptr);
		regCtx->SetRegisterTick(sip->clock().now());
		regCtx->GetConfig().Address.protocol = net::protocol::TCP;

		// 3. Make invite and verify that invite ctx and reg ctx are connected
		const char* to = "#sip:us1@10.0.2.15:50869;transport=tcp";
		VS_CallConfig config;
		auto dialog_id = sip->NewDialogID(to, {}, config);
		sip->InviteMethod(dialog_id, "from", to, VS_ConferenceInfo(false, false));
		it = ctxs.find(dialog_id);
		ASSERT_NE(it, ctxs.end());
		auto inviteCtx = it->second;
		ASSERT_NE(inviteCtx, nullptr);
		EXPECT_STREQ(regCtx->SIPDialogID().c_str(), inviteCtx->GetRegCtxDialogID().c_str());
	}

	TEST(Contact, CommaInMethods) {
		std::vector<std::pair<std::string/*contact*/, int /*number of URIs*/>> testData =
		{
			std::make_pair("Contact:sip:test2@192.168.41.111:5070;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\"\r\n", 1),
			std::make_pair("Contact:<sip:test2@192.168.41.111:5070>;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\"\r\n",1),
			std::make_pair("Contact:<sip:test2@192.168.41.111:5070>;methods=\"INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\", <sip:test3@192.168.41.111:5070>\r\n",2),
			std::make_pair("Contact:<sip:test2@192.168.41.111:5070;transport=tcp>;methods=\"INVITE,ACK,BYE,CANCEL,OPTIONS,INFO,MESSAGE,SUBSCRIBE,NOTIFY,PRACK,UPDATE,REFER\";reg-id=1;+sip.instance=\"<urn:uuid:f6d821d1-e96f-404e-b24e-5aaad7420d86>\"\r\n",1)
		};
		for (auto& contactHeader : testData)
		{
			VS_SIPBuffer b; b.AddData(contactHeader.first);
			VS_SIPField_Contact contact;
			EXPECT_EQ(contact.Decode(b), TSIPErrorCodes::e_ok);

			std::vector<std::string> parsedUris;
			contact.GetURIs(parsedUris);
			EXPECT_EQ(parsedUris.size(), contactHeader.second);	// expect number of URIs to be parsed
		}

	}

}  // namespace