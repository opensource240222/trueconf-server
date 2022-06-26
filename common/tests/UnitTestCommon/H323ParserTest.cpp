#if defined(_WIN32) // Not ported yet

#include "H323ParserTestBase.h"
#include "GTestPrinters.h"
#include "VS_ConferenceProtocolMock.h"
#include "tools/H323Gateway/Lib/src/VS_H235_DiffieHellman.h"
#include "tools/H323Gateway/Lib/h235/VS_H235CryptoEngine.h"
#include "tools/H323Gateway/Lib/h235/VS_H235CryptoEngineH323Plus.h"
#include "tools/SingleGatewayLib/VS_H235Session.cpp"
#include "FakeClient/VS_ConferenceInfo.h"
#include "TrueGateway/CallConfig/VS_IndentifierH323.h"

#include <gtest/gtest.h>

#include <string>
#include <thread>
#pragma warning (disable:4800)
namespace
{
	class H323ParserTest : public H323ParserTestBase
	{
	public:
		H323ParserTest()
			: H323ParserTestBase(net::address_v4::from_string("10.8.0.101"), 52881, net::address_v4::from_string("10.8.0.6"), 1720)
		{
		}
	};

#include "rawh323.h"

	TEST_F(H323ParserTest, tick_1786)
	{
		using ::testing::_;
		using ::testing::AtLeast;

		/* move into separte args */
		std::string to = "#h323:@10.8.0.6/12345";
		std::string from = "sgt@rua5n.trueconf.name";
		const char new_dialog[] = "02782DA9B688476D079DA823E2E203CE";
		///

		std::vector<char> snd_buf(0xffff);
		std::size_t sz(snd_buf.size());

		std::string dialog = h323->SetNewDialogTest(string_view{ new_dialog, sizeof(new_dialog) - 1 }, to, {}, VS_CallConfig{});
		ASSERT_TRUE(!dialog.empty());

		h323->SetPeerCSAddress(dialog, terminal_addr);

		h245_channel_fake->remote_addr = terminal_addr.addr;
		h245_channel_fake->remote_port = 12345;
		EXPECT_CALL(*h245_channel, Open(_, _, _, _, _, _)).Times(AtLeast(1));
		EXPECT_CALL(*h245_channel, Send_mocked(_)).Times(AtLeast(1));

		ASSERT_TRUE(h323->InviteMethod(dialog, from, to, VS_ConferenceInfo(false, false), "dn"));
		ASSERT_GT(h323->GetBufForSend(&snd_buf[0], sz, e_H225, {}, 0, {}, 0), 0);
		ASSERT_GT(h323->SetRecvBuf(allering_TCSFailed_tick_1786, sizeof(allering_TCSFailed_tick_1786), e_H225,
			terminal_addr.addr, terminal_addr.port, {}, 0), 0);
		sz = snd_buf.size();
		ASSERT_EQ(h323->GetBufForSend(&snd_buf[0], sz, e_H225, {}, 0, {}, 0), 0);

		ASSERT_GT(h323->SetRecvBuf(connect_TCSFailed_tick_1786, sizeof(connect_TCSFailed_tick_1786), e_H225,
			terminal_addr.addr, terminal_addr.port, {}, 0), 0);
		sz = snd_buf.size();
		ASSERT_EQ(h323->GetBufForSend(&snd_buf[0], sz, e_H225, {}, 0, {}, 0), 0);
		sz = snd_buf.size();
		h323->clock().add_diff(std::chrono::milliseconds(700));
		h323->Timeout();

		ASSERT_LE(1u, h245_channel_fake->out_queue.size());
		{
			auto& h245_data = h245_channel_fake->out_queue[0];
			EXPECT_LT(4u, h245_data.size());
			VS_PerBuffer buffer(h245_data.data<const unsigned char>() + 4, (h245_data.size() - 4) * 8);
			VS_H245MultimediaSystemControlMessage msg;
			ASSERT_TRUE(msg.Decode(buffer));
			ASSERT_EQ(msg.tag, VS_H245MultimediaSystemControlMessage::e_request);
			auto rsp = dynamic_cast<VS_H245RequestMessage*>(msg.choice);
			ASSERT_EQ(rsp->tag, VS_H245RequestMessage::e_terminalCapabilitySet);
		}

		h245_channel_fake->CallDataReceived(tcs_TCSFailed_tick_1786, sizeof(tcs_TCSFailed_tick_1786));
	}

	TEST_F(H323ParserTest, DISABLED_OLCPorts)
	{
		using ::testing::_;
		using ::testing::AtLeast;

		/* move into separte args */
		std::string to = "#h323:@10.8.0.6/12345";
		std::string from = "sgt@rua5n.trueconf.name";
		const char new_dialog[] = "D955EBF81E7981B3BECDAFEBC809769B";
		///

		std::vector<char> snd_buf(0xffff);
		std::size_t sz;

		std::string dialog = h323->SetNewDialogTest(string_view{ new_dialog, sizeof(new_dialog) - 1 }, to, {}, VS_CallConfig{});
		ASSERT_TRUE(!dialog.empty());

		h323->SetPeerCSAddress(dialog, terminal_addr);
			h323->UseACL(false);

		h245_channel_fake->remote_addr = terminal_addr.addr;
		h245_channel_fake->remote_port = 12345;
		EXPECT_CALL(*h245_channel, Open(_, _, _, _, _, _)).Times(AtLeast(1));
		EXPECT_CALL(*h245_channel, Send_mocked(_)).Times(AtLeast(1));

		ASSERT_TRUE(h323->InviteMethod(dialog, from, to, VS_ConferenceInfo(false, false), "dn"));
		sz = snd_buf.size();
		ASSERT_LT(0, h323->GetBufForSend(&snd_buf[0], sz, e_H225, {}, 0, {}, 0));
		ASSERT_LT(0, h323->SetRecvBuf(raw_hdx8000_h225_alerting, sizeof(raw_hdx8000_h225_alerting), e_H225,
			terminal_addr.addr, terminal_addr.port, {}, 0));
		ASSERT_LT(0, h323->SetRecvBuf(raw_hdx8000_h225_connect, sizeof(raw_hdx8000_h225_connect), e_H225, terminal_addr.addr, terminal_addr.port, {}, 0));
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_tcs, sizeof(raw_hdx8000_h245_tcs));
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_msd, sizeof(raw_hdx8000_h245_msd));
		auto ctx = h323->GetParserContext(dialog); // as the test was written before the MSD was actually implemented
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_tcsa, sizeof(raw_hdx8000_h245_tcsa));
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_msda, sizeof(raw_hdx8000_h245_msda));
		h245_channel_fake->out_queue.clear();

		EXPECT_CALL(*conf_protocol, SetMediaChannels(::testing::An<string_view>(), _, _, _))
			.Times(1);
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_olc_audio, sizeof(raw_hdx8000_h245_olc_audio));
		ASSERT_LE(1u, h245_channel_fake->out_queue.size());
		{
			auto& h245_data = h245_channel_fake->out_queue[0];
			EXPECT_LT(4u, h245_data.size());
			VS_PerBuffer buffer(h245_data.data<const unsigned char>() + 4, (h245_data.size() - 4) * 8);
			VS_H245MultimediaSystemControlMessage msg;
			ASSERT_TRUE(msg.Decode(buffer));
			ASSERT_EQ(VS_H245MultimediaSystemControlMessage::e_response, msg.tag);
			auto resp = static_cast<VS_H245ResponseMessage*>(msg.choice);
			ASSERT_EQ(VS_H245ResponseMessage::e_openLogicalChannelAck, resp->tag);
			auto olca = static_cast<VS_H245OpenLogicalChannelAck*>(resp->choice);
			net::address rtp_addr, rtcp_addr;
			net::port rtp_port, rtcp_port;
			auto gw_olca = static_cast<VS_GwH245OpenLogicalChannelAck*>(olca);
			ASSERT_TRUE(gw_olca->GetRtpRtcpIpPort(rtp_addr, rtp_port, rtcp_addr, rtcp_port));

			auto audio_channel_it = std::find_if(conf_protocol_fake.last_media_channels.begin(), conf_protocol_fake.last_media_channels.end(), [](const VS_MediaChannelInfo& x) {
				return x.type == SDPMediaType::audio && x.content == SDP_CONTENT_MAIN;
			});
			ASSERT_TRUE(audio_channel_it != conf_protocol_fake.last_media_channels.end());
			EXPECT_EQ(audio_channel_it->our_rtp_address.port(), rtp_port);
			EXPECT_EQ(audio_channel_it->our_rtcp_address.port(), rtcp_port);
		}
		h245_channel_fake->out_queue.clear();


		EXPECT_CALL(*conf_protocol, SetMediaChannels(::testing::An<string_view>(), _, _, _))
			.Times(1);
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_olc_video, sizeof(raw_hdx8000_h245_olc_video));
		ASSERT_LE(1u, h245_channel_fake->out_queue.size());
		{
			auto& h245_data = h245_channel_fake->out_queue[0];
			EXPECT_LT(4u, h245_data.size());
			VS_PerBuffer buffer(h245_data.data<const unsigned char>() + 4, (h245_data.size() - 4) * 8);
			VS_H245MultimediaSystemControlMessage msg;
			msg.Decode(buffer);
			ASSERT_EQ(VS_H245MultimediaSystemControlMessage::e_response, msg.tag);
			auto resp = static_cast<VS_H245ResponseMessage*>(msg.choice);
			ASSERT_EQ(VS_H245ResponseMessage::e_openLogicalChannelAck, resp->tag);
			auto olca = static_cast<VS_H245OpenLogicalChannelAck*>(resp->choice);
			auto gw_olca = static_cast<VS_GwH245OpenLogicalChannelAck*>(olca);
			net::address rtp_addr, rtcp_addr;
			net::port rtp_port, rtcp_port;
			ASSERT_TRUE(gw_olca->GetRtpRtcpIpPort(rtp_addr, rtp_port, rtcp_addr, rtcp_port));

			auto video_channel_it = std::find_if(conf_protocol_fake.last_media_channels.begin(), conf_protocol_fake.last_media_channels.end(), [](const VS_MediaChannelInfo& x) {
				return x.type == SDPMediaType::video && x.content == SDP_CONTENT_MAIN;
			});
			ASSERT_TRUE(video_channel_it != conf_protocol_fake.last_media_channels.end());
			EXPECT_EQ(video_channel_it->our_rtp_address.port(), rtp_port);
			EXPECT_EQ(video_channel_it->our_rtcp_address.port(), rtcp_port);
		}
		h245_channel_fake->out_queue.clear();
	}

	MATCHER_P2(MatchCodec, codec_type, payload_type, "") {
		return arg.CodecType == codec_type && arg.PayloadType == payload_type;
	}

	unsigned char GetPTFromOLC(VS_H245OpenLogicalChannel* olc)
	{
		if (olc->forwardLogicalChannelParameters.multiplexParameters.tag == VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::e_h2250LogicalChannelParameters)
		{
			auto h2250lcp = static_cast<VS_H245H2250LogicalChannelParameters*>(olc->forwardLogicalChannelParameters.multiplexParameters.choice);
			if (h2250lcp->dynamicRTPPayloadType.filled)
				return h2250lcp->dynamicRTPPayloadType.value;
			if (h2250lcp->mediaPacketization.tag == VS_H245H2250LogicalChannelParameters_MediaPacketization::e_rtpPayloadType)
			{
				auto h245_rtp_pt = static_cast<VS_H245RTPPayloadType*>(h2250lcp->mediaPacketization.choice);
				if (h245_rtp_pt->payloadType.filled)
					return h245_rtp_pt->payloadType.value;
			}
		}
		return 255;
	}

	TEST_F(H323ParserTest, DISABLED_CodecSelection)
	{
		using ::testing::_;
		using ::testing::AtLeast;
		using ::testing::Contains;
		using ::testing::Return;

		/* move into separte args */
		std::string to = "#h323:@10.8.0.6/12345";
		std::string from = "sgt@rua5n.trueconf.name";
		const char new_dialog[] = "D955EBF81E7981B3BECDAFEBC809769B";
		///

		std::vector<char> snd_buf(0xffff);
		std::size_t sz;

		std::string dialog = h323->SetNewDialogTest(string_view{ new_dialog, sizeof(new_dialog) - 1 }, to, {}, VS_CallConfig{});
		ASSERT_TRUE(!dialog.empty());

		// Set base logical channel number as used by older versions of TCS
		{
			auto ctx = h323->GetParserContext(dialog);
			ctx->SetBaseLCNumber();
		}

		h323->SetPeerCSAddress(dialog, terminal_addr);

		h245_channel_fake->remote_addr = terminal_addr.addr;
		h245_channel_fake->remote_port = 12345;
		EXPECT_CALL(*h245_channel, Open(_, _, _, _, _, _)).Times(AtLeast(1));
		EXPECT_CALL(*h245_channel, Send_mocked(_)).Times(AtLeast(1));

		EXPECT_CALL(*conf_protocol, SetMediaChannels(::testing::An<string_view>(), _, _, _))
			.Times(AtLeast(1));

		ASSERT_TRUE(h323->InviteMethod(dialog, from, to, VS_ConferenceInfo(false, false), "dn"));
		sz = snd_buf.size();
		ASSERT_LT(0, h323->GetBufForSend(&snd_buf[0], sz, e_H225, {}, 0, {}, 0));
		ASSERT_LT(0, h323->SetRecvBuf(raw_hdx8000_h225_alerting, sizeof(raw_hdx8000_h225_alerting), e_H225,
			terminal_addr.addr, terminal_addr.port, {}, 0));
		ASSERT_LT(0, h323->SetRecvBuf(raw_hdx8000_h225_connect, sizeof(raw_hdx8000_h225_connect), e_H225, terminal_addr.
			addr, terminal_addr.port, {}, 0));
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_tcs, sizeof(raw_hdx8000_h245_tcs));
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_msd, sizeof(raw_hdx8000_h245_msd));
		auto ctx = h323->GetParserContext(dialog); // as the test was written before the MSD was actually implemented
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_tcsa, sizeof(raw_hdx8000_h245_tcsa));
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_msda, sizeof(raw_hdx8000_h245_msda));
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_olc_audio, sizeof(raw_hdx8000_h245_olc_audio));
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_olc_video, sizeof(raw_hdx8000_h245_olc_video));
		h245_channel_fake->out_queue.clear();

		unsigned char snd_audio_pt = 0;
		unsigned char snd_video_pt = 0;
		h323->Timeout(); // Sends OLCs to terminal

		ASSERT_LE(2u, h245_channel_fake->out_queue.size());
		{
			auto& h245_data = h245_channel_fake->out_queue[0];
			EXPECT_LT(4u, h245_data.size());
			VS_PerBuffer buffer(h245_data.data<const unsigned char>() + 4, (h245_data.size() - 4) * 8);
			VS_H245MultimediaSystemControlMessage msg;
			ASSERT_TRUE(msg.Decode(buffer));
			ASSERT_EQ(VS_H245MultimediaSystemControlMessage::e_request, msg.tag);
			auto resp = static_cast<VS_H245RequestMessage*>(msg.choice);
			ASSERT_EQ(VS_H245RequestMessage::e_openLogicalChannel, resp->tag);
			auto olc = static_cast<VS_H245OpenLogicalChannel*>(resp->choice);
			snd_audio_pt = GetPTFromOLC(olc);
		}
		{
			auto& h245_data = h245_channel_fake->out_queue[1];
			EXPECT_LT(4u, h245_data.size());
			VS_PerBuffer buffer(h245_data.data<const unsigned char>() + 4, (h245_data.size() - 4) * 8);
			VS_H245MultimediaSystemControlMessage msg;
			ASSERT_TRUE(msg.Decode(buffer));
			ASSERT_EQ(VS_H245MultimediaSystemControlMessage::e_request, msg.tag);
			auto resp = static_cast<VS_H245RequestMessage*>(msg.choice);
			ASSERT_EQ(VS_H245RequestMessage::e_openLogicalChannel, resp->tag);
			auto olc = static_cast<VS_H245OpenLogicalChannel*>(resp->choice);
			snd_video_pt = GetPTFromOLC(olc);
		}
		h245_channel_fake->out_queue.clear();

		EXPECT_CALL(*conf_protocol, InviteReplay(_, e_call_ok, _, _, _))
			.WillOnce(Return(true));
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_olca_audio, sizeof(raw_hdx8000_h245_olca_audio));
		h245_channel_fake->CallDataReceived(raw_hdx8000_h245_olca_video, sizeof(raw_hdx8000_h245_olca_video));

		auto audio_channel_it = std::find_if(conf_protocol_fake.last_media_channels.begin(), conf_protocol_fake.last_media_channels.end(), [](const VS_MediaChannelInfo& x) {
			return x.type == SDPMediaType::audio && x.content == SDP_CONTENT_MAIN;
		});
		ASSERT_TRUE(audio_channel_it != conf_protocol_fake.last_media_channels.end());
		EXPECT_THAT(audio_channel_it->rcv_modes_audio, Contains(MatchCodec(e_rcvSIREN14_48, 115)));
		// starting from 4.3.8. Siren 48 has higher priority.
		EXPECT_EQ(e_rcvSIREN14_48, audio_channel_it->snd_mode_audio.CodecType);
		if (snd_audio_pt != 255)
			EXPECT_EQ(snd_audio_pt, audio_channel_it->snd_mode_audio.PayloadType);
		else
			EXPECT_GT(96, audio_channel_it->snd_mode_audio.PayloadType) << "dynamic payload type sent to transcoder, but not to H323 peer";

		auto video_channel_it = std::find_if(conf_protocol_fake.last_media_channels.begin(), conf_protocol_fake.last_media_channels.end(), [](const VS_MediaChannelInfo& x) {
			return x.type == SDPMediaType::video && x.content == SDP_CONTENT_MAIN;
		});
		ASSERT_TRUE(video_channel_it != conf_protocol_fake.last_media_channels.end());
		EXPECT_THAT(video_channel_it->rcv_modes_video, Contains(MatchCodec(e_videoH264, 109)));
		EXPECT_EQ(e_videoH264, video_channel_it->snd_mode_video.CodecType);
		if (snd_video_pt != 255)
			EXPECT_EQ(snd_video_pt, video_channel_it->snd_mode_video.PayloadType);
		else
			EXPECT_GT(96, video_channel_it->snd_mode_video.PayloadType) << "dynamic payload type sent to transcoder, but not to H323 peer";
	}


	struct H235Test : public ::testing::Test{
		const unsigned size = 16; // 128 bit
		/* A 128 bit key */
		uint8_t *key = (uint8_t *)"0123456789012345";
		std::vector<uint8_t> vkey;
		unsigned any_sync_flag = 115;
		std::set<encryption_mode> supported_modes;

		void SetUp(){
			vkey.assign(key, key + size);
			supported_modes.emplace(encryption_mode::AES_128CBC);
		}
	};


	TEST_F(H235Test, GenerateDHkeys){
		// verify generation of half key for Diffie-Hellman exchange from  params given in table D.4/H.235
		auto dh_map = VS_H235_DiffieHellman::CreateDiffieHellmanParams();
		for (auto it = dh_map.begin(), _end = dh_map.end(); it != _end; ++it)
		{
			dh_params &dh_ps = it->second;
			auto &df = std::get<1>(dh_ps);
			ASSERT_TRUE(df.GenerateHalfKey());
		}

		// verify default generation of params and half key for Diffie-Hellman exchange
		VS_H235_DiffieHellman df;
		ASSERT_TRUE(df.GenerateHalfKey());
	}

	TEST_F(H235Test, EncodingDecodingDH1024params){
		auto dh_map = VS_H235_DiffieHellman::CreateDiffieHellmanParams();
		auto it = dh_map.find(dh_oid::DH1024);
		ASSERT_TRUE(it != dh_map.end());

		unsigned char G_param[] = {0x2};
		VS_BitBuffer g_buff,p_buff;
		g_buff.AddBits(G_param, 8);

		auto &df = std::get<1>(it->second);
		VS_H235_DiffieHellman df_clone(df);
		df.Decode_G(g_buff);

		ASSERT_TRUE(df.Encode_P(p_buff));
		df.Decode_P(p_buff);

		ASSERT_FALSE(df.IsZeroDHGroup());
		ASSERT_TRUE(df.TestParamsEqual(df_clone));

		ASSERT_TRUE(df.GenerateHalfKey());
		ASSERT_TRUE(df_clone.GenerateHalfKey());
		df.SetRemoteKey(df_clone.GetPublicKey<const BIGNUM*>());
		df_clone.SetRemoteKey(df.GetPublicKey<const BIGNUM*>());

		std::vector<uint8_t> masterKey, masterKey_clone;
		df.ComputeMasterKey(masterKey);
		df_clone.ComputeMasterKey(masterKey_clone);
		ASSERT_TRUE(masterKey == masterKey_clone);
	}

	TEST_F(H235Test, EncryptAndDecrypt_AES128CBC){
		/* A 128 bit IV */
		uint8_t *iv = (uint8_t *)"0123456789012345";

		/* Message to be encrypted */
		unsigned char *plaintext =
			(unsigned char *)"The quick brown fox jumps over the lazy dog";

		std::vector<uint8_t> cryptogramm;
		std::vector<uint8_t> decr_text;
		std::vector<uint8_t> text(plaintext, plaintext + strlen((char*)plaintext));
		bool res(false), used_padding(false);

		VS_H235CryptoEngine crypter;
		ASSERT_TRUE(crypter.Init(vkey, encryption_mode::AES_128CBC));

		// test two implementation 1-with padding 2-use ciphertext stealing
		for (size_t i = 0; i < 2; ++i)
		{
			bool use_cts = i!=0;
			// verify encryption of VS_H235CryptoEngine
			std::tie(res, used_padding) = crypter.Encrypt(text, iv, cryptogramm, use_cts);
			ASSERT_TRUE(res);
			ASSERT_TRUE(crypter.Decrypt(cryptogramm, iv, used_padding, decr_text));
			ASSERT_TRUE(memcmp(text.data(), decr_text.data(), text.size()) == 0);
			///////////////////////////////////////////////
		}

		// verify key generation of VS_H235CryptoEngine
		auto new_key = crypter.GenerateKey(16);
		std::vector<uint8_t> protected_key, decr_key;
		std::tie(res, used_padding) = crypter.Encrypt(new_key, iv, protected_key);
		ASSERT_TRUE(res);
		ASSERT_TRUE(crypter.Decrypt(protected_key, iv, used_padding, decr_key));
		ASSERT_TRUE(memcmp(decr_key.data(), new_key.data(), std::min(decr_key.size(), new_key.size())) == 0);
		///////////////////////////////////////////////

		// verify encryption of H235CryptoEngineH323Plus
		H235CryptoEngineH323Plus crypter1(encryption_mode::AES_128CBC, vkey);
		bool rtp_padding(false);
		cryptogramm = crypter1.Encrypt(text.data(), text.size(), iv, rtp_padding);
		ASSERT_FALSE(cryptogramm.empty());

		decr_text = crypter1.Decrypt(cryptogramm.data(), cryptogramm.size(), iv, rtp_padding);
		ASSERT_TRUE(memcmp(text.data(), decr_text.data(), text.size()) == 0);
		//////////////////////////////////////////////////

		for (size_t i = 0; i < 2; ++i)
		{
			bool use_cts = i != 0;
			// verify encryption of H235CryptoEngineH323Plus and VS_H235CryptoEngine
			std::tie(res, used_padding) = crypter.Encrypt(text, iv, cryptogramm, use_cts);
			ASSERT_TRUE(res);
			decr_text = crypter1.Decrypt(cryptogramm.data(), cryptogramm.size(), iv, used_padding);
			ASSERT_TRUE(memcmp(text.data(), decr_text.data(), text.size()) == 0);

			cryptogramm = crypter1.Encrypt(text.data(), text.size(), iv, rtp_padding);
			ASSERT_FALSE(cryptogramm.empty());
			ASSERT_TRUE(crypter.Decrypt(cryptogramm, iv, rtp_padding, decr_text));
			ASSERT_TRUE(memcmp(text.data(), decr_text.data(), text.size()) == 0);
		}
	}

	TEST_F(H235Test, SerializeDeserialize){
		std::set<encryption_mode> e_modes = { encryption_mode::AES_128CBC, /*encryption_mode::AES_128EOFB,*/ encryption_mode::no_encryption};
		for (const auto& item : e_modes)
		{
			VS_H235SecurityCapability cap;
			cap.m = item;
			if (cap.m != encryption_mode::no_encryption) cap.h235_sessionKey = vkey;

			VS_Container cnt;
			ASSERT_TRUE(cap.Serialize(cnt, "sec_cap"));

			VS_H235SecurityCapability new_cap;
			ASSERT_TRUE(new_cap.Deserialize(cnt, "sec_cap"));

			ASSERT_TRUE(std::tie(cap.m, cap.syncFlag, cap.h235_sessionKey) == std::tie(new_cap.m, new_cap.syncFlag, new_cap.h235_sessionKey));

			int payload = 115;
			VS_GatewayMediaMode gw_m(payload, false);
			gw_m.sec_cap = new_cap;

			ASSERT_TRUE(gw_m.Serialize(cnt, "VS_GatewayMediaMode"));

			VS_GatewayMediaMode new_gwm(0, true);
			ASSERT_TRUE(new_gwm.Deserialize(cnt, "VS_GatewayMediaMode"));

			ASSERT_TRUE(std::tie(gw_m.PayloadType, gw_m.InitializeNAT) == std::tie(gw_m.PayloadType, gw_m.InitializeNAT));
			ASSERT_TRUE(gw_m.sec_cap == new_gwm.sec_cap);
		}
	}

	TEST_F(H235Test, EncryptionSync){
		VS_H235Authenticator h235auth;
		h235auth.SetSecurityEnabled(true);

		VS_H225ArrayOf_ClearToken  tokens;
		h235auth.PrepareTokens(tokens);
		ASSERT_TRUE(h235auth.ValidateTokens(tokens));

		for (const auto& item : supported_modes){
			VS_H245EncryptionSync sync;
			VS_H235SecurityCapability sec_cap(item, any_sync_flag), read_sec_cap(item, any_sync_flag);

			ASSERT_TRUE(h235auth.BuildEncryptionSync(sync, sec_cap));
			ASSERT_TRUE(h235auth.ReadEncryptionSync(sync, read_sec_cap));
			ASSERT_TRUE(sec_cap == read_sec_cap);
		}
	}

	TEST_F(H235Test, MediaEncryptionDecryption){
		uint8_t rtp_packet[] = { /* Packet 597 */
			0x80, 0x73, 0xf9, 0x55, 0x00, 0x00, 0xa6, 0x4e,
			0xf9, 0x17, 0x03, 0x8f, 0x59, 0x98, 0xb8, 0x50,
			0xa7, 0x81, 0x40, 0x18, 0x46, 0xaa, 0xb2, 0x1e,
			0xa5, 0xb4, 0x95, 0x02, 0xaa, 0x07, 0x38, 0x34,
			0x83, 0xe3, 0x94, 0x8a, 0x3b, 0x11, 0xbb, 0x29,
			0x52, 0xf5, 0x5f, 0x92, 0xe1, 0x98, 0x28, 0xa1,
			0x98, 0xd2, 0x17, 0x08, 0x52, 0x38, 0x09, 0x62,
			0x4b, 0x44, 0xe4, 0x9a, 0xb2, 0x85, 0xe7, 0x79,
			0xa8, 0x0b, 0xb3, 0x0c, 0x23, 0x8b, 0x7d, 0x53,
			0xa6, 0x6f, 0x55, 0x9b, 0x38, 0xa7, 0x46, 0x4a,
			0x1c, 0x36, 0xc7, 0xa2, 0x6c, 0xf1, 0xd3, 0x8b,
			0x70, 0x5a, 0x51, 0xff, 0x8c, 0xaa, 0xce, 0x96,
			0xa8, 0x9d, 0xb6, 0x3f, 0x29, 0x41, 0x25, 0x83,
			0x34, 0xc7, 0x1a, 0xbd, 0x48, 0x14, 0x65, 0x25,
			0x89, 0xc2, 0x99, 0x23, 0x6b, 0x82, 0x4c, 0xb8,
			0xa8, 0xf7, 0xd0, 0x73, 0x6c, 0x16, 0x0c, 0x86,
			0xc1, 0xfa, 0x24, 0x7f };

		uint8_t *seqNoAndTimeStamp = rtp_packet + 2;

		for (const auto& encryption : supported_modes){
			VS_H235SecurityCapability sec_cap(vkey, encryption, any_sync_flag);
			vs::SharedBuffer buf(sizeof(rtp_packet));
			memcpy(buf.data(), rtp_packet, sizeof(rtp_packet));

			VS_H235Session s(&sec_cap, &sec_cap,any_sync_flag);

			encryption_meta meta;
			vs::SharedBuffer encr_buf = std::move(s.EncryptPacket(buf, meta));
			ASSERT_TRUE(meta.succsess);
			s.DecryptPacket(encr_buf);

			RTPPacket p(rtp_packet, sizeof(rtp_packet));
			RTPPacket decr_p(encr_buf.data(), encr_buf.size());

			uint8_t IV[VS_H235CryptoEngine::IV_SEQUENCE_LEN];
			Fill_IV(IV, p);
			ASSERT_TRUE(memcmp(seqNoAndTimeStamp, IV, VS_H235CryptoEngine::IV_SEQUENCE_LEN) == 0);
			ASSERT_TRUE(decr_p.DataSize() == p.DataSize());
			ASSERT_TRUE(memcmp(p.Data(), decr_p.Data(), decr_p.DataSize()) == 0);

			size_t h_size = p.HeaderSize();
			auto pData = encr_buf.data();
			ASSERT_TRUE(memcmp(rtp_packet, pData, h_size) == 0);
		}

	}

	// polycom fills padding vector like this: "0000000000000N", where N - number of bytes used for padding
	// common way to fill padding vector is:   "NNNNNNNNNNNNNN"
	// to make sure we are able to decrypt packet from polycom this test was added for
	TEST_F(H235Test, DecryptionFromPolycom) {
		uint8_t ecnrypted_rtp_audio_from_polycom[] = { /* Packet 789  encrypted with AES_128CBC */
			0xa0, 0x6c, 0x00, 0x02, 0x00, 0x00, 0x05, 0x00,
			0x41, 0xbb, 0x7f, 0x8a, 0x48, 0x3a, 0x87, 0x11,
			0x20, 0x42, 0x5c, 0x08, 0x9a, 0x4c, 0x0b, 0xe8,
			0xcc, 0x55, 0xd9, 0xdb, 0x6b, 0x6e, 0xb1, 0xf2,
			0xbd, 0xf3, 0x49, 0x87, 0x44, 0x8a, 0x9c, 0xda,
			0x2e, 0x22, 0x8a, 0xf0, 0xdc, 0x9b, 0x1a, 0x01,
			0x1a, 0x77, 0x28, 0xc6, 0xa0, 0x2c, 0x10, 0x2c,
			0x56, 0x6b, 0x7a, 0xf2, 0x1f, 0x8f, 0x62, 0x38,
			0x82, 0x77, 0xbe, 0x6c, 0x22, 0xdc, 0xf0, 0xc6,
			0x40, 0x84, 0x4e, 0x0d, 0x73, 0xf9, 0x46, 0xec,
			0xb3, 0x17, 0x61, 0x2d, 0xcd, 0x67, 0xf1, 0x73,
			0x62, 0x0e, 0xb1, 0x62, 0x66, 0x42, 0x2c, 0xe0,
			0x29, 0x11, 0xeb, 0x9a, 0x6e, 0x75, 0xb9, 0x85,
			0xc8, 0xac, 0x4e, 0x94, 0xb9, 0x1e, 0x38, 0x5a,
			0x8f, 0xa1, 0x58, 0xbb, 0xa1, 0x06, 0x9e, 0x88,
			0x6b, 0x3b, 0xba, 0x8f, 0xb4, 0x91, 0x69, 0xe5,
			0x1e, 0x62, 0xa0, 0x83, 0x75, 0x62, 0x43, 0xa9,
			0x53, 0x4c, 0x04, 0xa1 };

		std::vector<uint8_t> session_key = { 0x07,0x26,0x36,0x2B,0x9E,0x6B,0x00,0x76,0xCA,0xCB,0x7E,0x66,0x05,0x13,0x15,0x83 };
		VS_H235SecurityCapability sec_cap(session_key, encryption_mode::AES_128CBC, 108);
		vs::SharedBuffer buf(sizeof(ecnrypted_rtp_audio_from_polycom));
		memcpy(buf.data(), ecnrypted_rtp_audio_from_polycom, sizeof(ecnrypted_rtp_audio_from_polycom));

		VS_H235Session s(&sec_cap, nullptr, any_sync_flag);
		encryption_meta meta = s.DecryptPacket(buf);
		ASSERT_TRUE(meta.succsess);
	}

	// MSD Related tests
	template<typename T>
	void pop_front(std::vector<T>& vec)
	{
		assert(!vec.empty());
		vec.erase(vec.begin());
	}

	class H323ParserH245Test : public H323ParserTestBase {
	public:
		H323ParserH245Test()
			: H323ParserTestBase(net::address_v4::from_string("10.8.0.101"), 52881, net::address_v4::from_string("10.8.0.6"), 1720)
		{

		}
	protected:
		void SetUp() override
		{
			H323ParserTestBase::SetUp();

			std::string to = "#h323:@10.8.0.6/12345";
			std::string from = "sgt@rua5n.trueconf.name";
			const char new_dialog[] = "02782DA9B688476D079DA823E2E203CE";

			std::vector<char> snd_buf(0xffff);
			std::size_t sz(snd_buf.size());

			dialog = h323->SetNewDialogTest(string_view{ new_dialog, sizeof(new_dialog) - 1 }, to, {}, VS_CallConfig{});

			ctx = h323->GetParserContext(dialog);
			ctx->clock().add_diff(std::chrono::steady_clock::now().time_since_epoch());

			h323->SetPeerCSAddress(dialog, terminal_addr);

			h245_channel_fake->remote_addr = terminal_addr.addr;
			h245_channel_fake->remote_port = terminal_addr.port;
			h245_channel_fake->local_addr = vcs_addr.addr;
			h245_channel_fake->local_port = vcs_addr.port;

			ASSERT_TRUE(h323->InviteMethod(dialog, from, to, VS_ConferenceInfo(false, false), "dn"));
			ASSERT_GT(h323->GetBufForSend(&snd_buf[0], sz, e_H225, {}, 0, {}, 0), 0);
			ASSERT_GT(h323->SetRecvBuf(allering_TCSFailed_tick_1786, sizeof(allering_TCSFailed_tick_1786), e_H225,
				terminal_addr.addr, terminal_addr.port, {}, 0), 0);
			sz = snd_buf.size();
			ASSERT_EQ(h323->GetBufForSend(&snd_buf[0], sz, e_H225, {}, 0, {}, 0), 0);

			ASSERT_GT(h323->SetRecvBuf(connect_TCSFailed_tick_1786, sizeof(connect_TCSFailed_tick_1786), e_H225,
				terminal_addr.addr, terminal_addr.port, {}, 0), 0);
			sz = snd_buf.size();
			ASSERT_EQ(h323->GetBufForSend(&snd_buf[0], sz, e_H225, {}, 0, {}, 0), 0);
			sz = snd_buf.size();
		}

		void TearDown() override
		{
			H323ParserTestBase::TearDown();
		}

		void SkipAllMessages() const
		{
			if (h245_channel_fake->out_queue.empty())
				return;

			h245_channel_fake->out_queue.clear();
		}

		bool GetH245MsgFromParser(VS_H245MultimediaSystemControlMessage *theMessage) {
			if (h245_channel_fake->out_queue.empty())
				return false;

			for (;;)
			{
				auto &h245_data = h245_channel_fake->out_queue[0];
				if (h245_data.size() <= 4)
					continue;

				VS_PerBuffer buffer(h245_data.data<const unsigned char>() + 4, (h245_data.size() - 4) * 8);
				if (theMessage->Decode(buffer)) {
					pop_front(h245_channel_fake->out_queue);
					return true;
				}
				pop_front(h245_channel_fake->out_queue);
			}
			return false;
		}

		void WaitTimeout(std::chrono::steady_clock::duration time) {
			ctx->clock().add_diff(ctx->clock().now().time_since_epoch() + time);
			h323->Timeout();
		}

		VS_H245RequestMessage* GetRequestFromOutQueue(VS_H245MultimediaSystemControlMessage *msg, int tag = -1) {
			while (GetH245MsgFromParser(msg)) {
				if (msg->tag == msg->e_request) {
					VS_H245RequestMessage *req = (VS_H245RequestMessage*)msg->choice;
					if (tag == -1 || req->tag == tag) {
						return req;
					}
				}
			}
			return nullptr;
		}

		VS_H245ResponseMessage* GetResponseFromOutQueue(VS_H245MultimediaSystemControlMessage *msg, int tag = -1) {
			bool found = false;
			while (GetH245MsgFromParser(msg)) {
				if (msg->tag == msg->e_response) {
					VS_H245ResponseMessage *resp = (VS_H245ResponseMessage*)msg->choice;
					if (tag == -1 || resp->tag == tag) {
						return resp;
					}
				}
			}
			return nullptr;
		}

		VS_H245IndicationMessage* GetIndicationFromOutQueue(VS_H245MultimediaSystemControlMessage *msg, int tag = -1) {
			bool found = false;
			while (GetH245MsgFromParser(msg)) {
				if (msg->tag == msg->e_indication) {
					VS_H245IndicationMessage *resp = (VS_H245IndicationMessage*)msg->choice;
					if (tag == -1 || resp->tag == tag) {
						return resp;
					}
				}
			}
			return nullptr;
		}

		int PutMSDRequest(int msd_type, int msd_num) {
			VS_H245MultimediaSystemControlMessage   mscm;
			VS_H245RequestMessage   *rm = new VS_H245RequestMessage;
			VS_H245MasterSlaveDetermination   *msd = new VS_H245MasterSlaveDetermination;

			msd->terminalType.value = msd_type;
			msd->terminalType.filled = true;
			msd->statusDeterminationNumber.value = msd_num;
			msd->statusDeterminationNumber.filled = true;
			msd->filled = true;
			*rm = msd;		mscm = rm;

			return h323->OnH245Message(mscm, dialog);
		}

		int PutMSDAck(int decision) {
			VS_H245MultimediaSystemControlMessage   mscm;
			VS_H245ResponseMessage   *rm = new VS_H245ResponseMessage;
			VS_H245MasterSlaveDeterminationAck   *msda = new VS_H245MasterSlaveDeterminationAck;

			msda->decision.tag = decision;

			msda->decision.filled = true;	msda->filled = true;
			VS_AsnNull * a = new VS_AsnNull;
			a->filled = true;
			msda->decision.choice = a;
			*rm = msda;		mscm = rm;

			return h323->OnH245Message(mscm, dialog);
		}

		int PutMSDReject(int cause) {
			VS_H245Data* h245params = ctx->GetH245Params();

			VS_H245MultimediaSystemControlMessage   mscm;
			VS_H245ResponseMessage   *rm = new VS_H245ResponseMessage;
			VS_H245MasterSlaveDeterminationReject *msdr = new VS_H245MasterSlaveDeterminationReject;
			msdr->cause.tag = cause;
			msdr->cause.filled = true;
			msdr->filled = true;

			msdr->cause.choice = new VS_AsnNull;
			msdr->cause.choice->filled = true;

			*rm = msdr;		mscm = rm;
			return h323->OnH245Message(mscm, dialog);
		}

		int PutMSDRelease(int cause) {
			VS_H245Data* h245params = ctx->GetH245Params();

			VS_H245MultimediaSystemControlMessage   mscm;
			VS_H245IndicationMessage   *im = new VS_H245IndicationMessage;
			VS_H245MasterSlaveDeterminationRelease *msdrel = new VS_H245MasterSlaveDeterminationRelease;
			msdrel->filled = true;

			*im = msdrel;		mscm = im;
			return h323->OnH245Message(mscm, dialog);
		}

		std::shared_ptr<TestableH323ParserInfo> ctx;
		std::string dialog;
	};

	TEST_F(H323ParserH245Test, MSDIdleState_SendRequest) {
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
		WaitTimeout(MSD_WAIT_TIMEOUT);
		VS_H245MultimediaSystemControlMessage msg;
		ASSERT_NE(nullptr, GetRequestFromOutQueue(&msg, VS_H245RequestMessage::e_masterSlaveDetermination));
		ASSERT_TRUE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDOutgoingAwaitingResponse);
	}

	TEST_F(H323ParserH245Test, MSDIdleState_ReceiveRequest_slave) {
		ASSERT_GT(PutMSDRequest(ctx->GetH245Params()->m_my_msd_type, ctx->GetH245Params()->m_my_msd_num + 1), 0);
		VS_H245MultimediaSystemControlMessage msg;
		VS_H245ResponseMessage *resp = GetResponseFromOutQueue(&msg, VS_H245ResponseMessage::e_masterSlaveDeterminationAck);
		ASSERT_NE(nullptr, resp);
		ASSERT_EQ(((VS_H245MasterSlaveDeterminationAck*)resp->choice)->decision.tag, VS_H245MasterSlaveDeterminationAck_Decision::e_slave);
		ASSERT_TRUE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIncomingAwaitingResponse);
	}

	TEST_F(H323ParserH245Test, MSDIdleState_ReceiveRequest_master) {
		ASSERT_GT(PutMSDRequest(ctx->GetH245Params()->m_my_msd_type, ctx->GetH245Params()->m_my_msd_num - 1), 0);
		VS_H245MultimediaSystemControlMessage msg;
		VS_H245ResponseMessage *resp = GetResponseFromOutQueue(&msg, VS_H245ResponseMessage::e_masterSlaveDeterminationAck);
		ASSERT_NE(nullptr, resp);
		ASSERT_EQ(((VS_H245MasterSlaveDeterminationAck*)resp->choice)->decision.tag, VS_H245MasterSlaveDeterminationAck_Decision::e_master);
		ASSERT_TRUE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIncomingAwaitingResponse);
	}

	TEST_F(H323ParserH245Test, MSDIdleState_ReceiveRequest_indeterminate1) {
		ASSERT_GT(PutMSDRequest(ctx->GetH245Params()->m_my_msd_type, ctx->GetH245Params()->m_my_msd_num), 0);
		VS_H245MultimediaSystemControlMessage msg;
		VS_H245ResponseMessage *resp = GetResponseFromOutQueue(&msg);
		ASSERT_NE(nullptr, resp);
		ASSERT_EQ(resp->tag, VS_H245ResponseMessage::e_masterSlaveDeterminationReject);
		ASSERT_EQ(((VS_H245MasterSlaveDeterminationReject*)resp->choice)->cause.tag, VS_H245MasterSlaveDeterminationReject_Cause::e_identicalNumbers);
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
	}

	TEST_F(H323ParserH245Test, MSDIdleState_ReceiveRequest_indeterminate2) {
		unsigned long num = ctx->GetH245Params()->m_my_msd_num + ((1 << 24) - (1 << 23));
		ASSERT_GT(PutMSDRequest(ctx->GetH245Params()->m_my_msd_type, num), 0);
		VS_H245MultimediaSystemControlMessage msg;
		VS_H245ResponseMessage *resp = GetResponseFromOutQueue(&msg);
		ASSERT_NE(nullptr, resp);
		ASSERT_EQ(resp->tag, VS_H245ResponseMessage::e_masterSlaveDeterminationReject);
		ASSERT_EQ(((VS_H245MasterSlaveDeterminationReject*)resp->choice)->cause.tag, VS_H245MasterSlaveDeterminationReject_Cause::e_identicalNumbers);
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
	}

	TEST_F(H323ParserH245Test, MSDIdleState_ReceiveAckRejectRelease) {
		ASSERT_GT(PutMSDAck(VS_H245MasterSlaveDeterminationAck_Decision::e_master), 0);
		ASSERT_EQ(ctx->MsdState(), MSDIdle);

		ASSERT_GT(PutMSDReject(0), 0);
		ASSERT_EQ(ctx->MsdState(), MSDIdle);

		ASSERT_GT(PutMSDRelease(0), 0);
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
	}

	TEST_F(H323ParserH245Test, MSDOutgoingState_ReceiveAck) {
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		ASSERT_GT(PutMSDAck(VS_H245MasterSlaveDeterminationAck_Decision::e_master), 0);
		VS_H245MultimediaSystemControlMessage msg;
		VS_H245ResponseMessage *resp = GetResponseFromOutQueue(&msg);
		ASSERT_NE(nullptr, resp);
		ASSERT_EQ(resp->tag, VS_H245ResponseMessage::e_masterSlaveDeterminationAck);
		ASSERT_EQ(((VS_H245MasterSlaveDeterminationAck*)resp->choice)->decision.tag, VS_H245MasterSlaveDeterminationAck_Decision::e_slave);
		ASSERT_FALSE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
	}

	TEST_F(H323ParserH245Test, MSDOutgoingState_ReceiveRequest_master) {
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		ASSERT_GT(PutMSDRequest(ctx->GetH245Params()->m_my_msd_type, ctx->GetH245Params()->m_my_msd_num - 1), 0);
		VS_H245MultimediaSystemControlMessage msg;
		VS_H245ResponseMessage *resp = GetResponseFromOutQueue(&msg, VS_H245ResponseMessage::e_masterSlaveDeterminationAck);
		ASSERT_NE(nullptr, resp);
		ASSERT_EQ(((VS_H245MasterSlaveDeterminationAck*)resp->choice)->decision.tag, VS_H245MasterSlaveDeterminationAck_Decision::e_master);
		ASSERT_TRUE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIncomingAwaitingResponse);
	}

	TEST_F(H323ParserH245Test, MSDOutgoingState_ReceiveRequest_slave) {
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		ASSERT_GT(PutMSDRequest(ctx->GetH245Params()->m_my_msd_type, ctx->GetH245Params()->m_my_msd_num + 1), 0);
		VS_H245MultimediaSystemControlMessage msg;
		VS_H245ResponseMessage *resp = GetResponseFromOutQueue(&msg, VS_H245ResponseMessage::e_masterSlaveDeterminationAck);
		ASSERT_NE(nullptr, resp);
		ASSERT_EQ(((VS_H245MasterSlaveDeterminationAck*)resp->choice)->decision.tag, VS_H245MasterSlaveDeterminationAck_Decision::e_slave);
		ASSERT_TRUE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIncomingAwaitingResponse);
	}

	TEST_F(H323ParserH245Test, MSDOutgoingState_ReceiveRequest_indeterminate1) {
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		unsigned long msd_num_before = ctx->GetH245Params()->m_my_msd_num;
		ASSERT_GT(PutMSDRequest(ctx->GetH245Params()->m_my_msd_type, ctx->GetH245Params()->m_my_msd_num), 0);
		ASSERT_NE(ctx->GetH245Params()->m_my_msd_num, msd_num_before) << "New number should be generated";
		VS_H245MultimediaSystemControlMessage msg;
		VS_H245RequestMessage *req = GetRequestFromOutQueue(&msg, VS_H245RequestMessage::e_masterSlaveDetermination);
		ASSERT_NE(nullptr, req);
		VS_H245MasterSlaveDetermination *r = (VS_H245MasterSlaveDetermination*)req->choice;
		ASSERT_NE(r->statusDeterminationNumber.value, msd_num_before) << "New number should be generated";
		ASSERT_TRUE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDOutgoingAwaitingResponse);
	}

	TEST_F(H323ParserH245Test, MSDOutgoingState_ReceiveRequest_indeterminate2) {
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		unsigned long msd_num_before = ctx->GetH245Params()->m_my_msd_num;
		unsigned long num = ctx->GetH245Params()->m_my_msd_num + ((1 << 24) - (1 << 23));
		ASSERT_GT(PutMSDRequest(ctx->GetH245Params()->m_my_msd_type, num), 0);
		ASSERT_NE(ctx->GetH245Params()->m_my_msd_num, msd_num_before) << "New number should be generated";
		VS_H245MultimediaSystemControlMessage msg;
		VS_H245RequestMessage *req = GetRequestFromOutQueue(&msg, VS_H245RequestMessage::e_masterSlaveDetermination);
		ASSERT_NE(nullptr, req);
		VS_H245MasterSlaveDetermination *r = (VS_H245MasterSlaveDetermination*)req->choice;
		ASSERT_NE(r->statusDeterminationNumber.value, msd_num_before) << "New number should be generated";
		ASSERT_TRUE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDOutgoingAwaitingResponse);
	}

	TEST_F(H323ParserH245Test, MSDOutgoingState_ReceiveRequest_indeterminate_counter_overflow) {
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		ctx->SetMSDCounterForTesting(MSD_COUNT_LIMIT - 1);
		EXPECT_CALL(*conf_protocol, Hangup(::testing::_)).Times(1);
		ASSERT_GT(PutMSDRequest(ctx->GetH245Params()->m_my_msd_type, ctx->GetH245Params()->m_my_msd_num), 0);
		VS_H245MultimediaSystemControlMessage msg;
		ASSERT_EQ(nullptr, GetRequestFromOutQueue(&msg, VS_H245RequestMessage::e_masterSlaveDetermination));
		ASSERT_FALSE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
		ASSERT_EQ(h323->GetParserContext(dialog), std::shared_ptr<VS_H323ParserInfo>());
	}

	TEST_F(H323ParserH245Test, MSDOutgoingState_ReceiveReject) {
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		unsigned long msd_num_before = ctx->GetH245Params()->m_my_msd_num;
		ASSERT_GT(PutMSDReject(0), 0);
		VS_H245MultimediaSystemControlMessage msg;
		VS_H245RequestMessage *req = GetRequestFromOutQueue(&msg, VS_H245RequestMessage::e_masterSlaveDetermination);
		VS_H245MasterSlaveDetermination *r = (VS_H245MasterSlaveDetermination*)req->choice;
		ASSERT_NE(r->statusDeterminationNumber.value, msd_num_before) << "New number should be generated";
		ASSERT_TRUE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDOutgoingAwaitingResponse);
	}

	TEST_F(H323ParserH245Test, MSDOutgoingState_ReceiveReject_counter_overflow) {
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		ctx->SetMSDCounterForTesting(MSD_COUNT_LIMIT - 1);
		EXPECT_CALL(*conf_protocol, Hangup(::testing::_)).Times(1);
		ASSERT_GT(PutMSDReject(0), 0);
		VS_H245MultimediaSystemControlMessage msg;
		ASSERT_EQ(nullptr, GetRequestFromOutQueue(&msg, VS_H245RequestMessage::e_masterSlaveDetermination));
		ASSERT_FALSE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
		ASSERT_EQ(h323->GetParserContext(dialog), std::shared_ptr<VS_H323ParserInfo>());
	}

	TEST_F(H323ParserH245Test, MSDOutgoingState_ReceiveRelease) {
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		EXPECT_CALL(*conf_protocol, Hangup(::testing::_)).Times(1);
		ASSERT_GT(PutMSDRelease(0), 0);
		ASSERT_FALSE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
		ASSERT_EQ(h323->GetParserContext(dialog), std::shared_ptr<VS_H323ParserInfo>());
	}

	TEST_F(H323ParserH245Test, MSDOutgoingState_TimerExpiry) {
		ctx->SetMsdState(MSDOutgoingAwaitingResponse);
		ctx->StartMSDTimer();
		WaitTimeout(MSDA_WAIT_TIMEOUT);
		VS_H245MultimediaSystemControlMessage msg;
		ASSERT_NE(nullptr, GetIndicationFromOutQueue(&msg, VS_H245IndicationMessage::e_masterSlaveDeterminationRelease));
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
		ASSERT_EQ(h323->GetParserContext(dialog), std::shared_ptr<VS_H323ParserInfo>());
	}

	TEST_F(H323ParserH245Test, MSDIncomingState_ReceiveAck_match) {
		VS_H245Data* h245params = ctx->GetH245Params();
		h245params->m_their_msd_type = h245params->m_my_msd_type;
		h245params->m_their_msd_num = h245params->m_my_msd_num - 1;

		ctx->SetMsdState(MSDIncomingAwaitingResponse);
		ASSERT_GT(PutMSDAck(VS_H245MasterSlaveDeterminationAck_Decision::e_slave), 0);
		ASSERT_FALSE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
		ASSERT_NE(h323->GetParserContext(dialog), std::shared_ptr<VS_H323ParserInfo>());
	}

	TEST_F(H323ParserH245Test, MSDIncomingState_ReceiveAck_notmatch) {
		VS_H245Data* h245params = ctx->GetH245Params();
		h245params->m_their_msd_type = h245params->m_my_msd_type;
		h245params->m_their_msd_num = h245params->m_my_msd_num + 1;

		ctx->SetMsdState(MSDIncomingAwaitingResponse);
		EXPECT_CALL(*conf_protocol, Hangup(::testing::_)).Times(1);
		ASSERT_GT(PutMSDAck(VS_H245MasterSlaveDeterminationAck_Decision::e_slave), 0);
		ASSERT_FALSE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
		ASSERT_EQ(h323->GetParserContext(dialog), std::shared_ptr<VS_H323ParserInfo>());
	}

	TEST_F(H323ParserH245Test, MSDIncomingState_ReceiveRequest) {
		VS_H245Data* h245params = ctx->GetH245Params();

		ctx->SetMsdState(MSDIncomingAwaitingResponse);
		EXPECT_CALL(*conf_protocol, Hangup(::testing::_)).Times(1);
		ASSERT_GT(PutMSDRequest(h245params->m_my_msd_type, 12456), 0);
		ASSERT_FALSE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
		ASSERT_EQ(h323->GetParserContext(dialog), std::shared_ptr<VS_H323ParserInfo>());
	}

	TEST_F(H323ParserH245Test, MSDIncomingState_ReceiveReject) {
		ctx->SetMsdState(MSDIncomingAwaitingResponse);
		EXPECT_CALL(*conf_protocol, Hangup(::testing::_)).Times(1);
		ASSERT_GT(PutMSDReject(0), 0);
		ASSERT_FALSE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
		ASSERT_EQ(h323->GetParserContext(dialog), std::shared_ptr<VS_H323ParserInfo>());
	}

	TEST_F(H323ParserH245Test, MSDIncomingState_ReceiveRelease) {
		ctx->SetMsdState(MSDIncomingAwaitingResponse);
		EXPECT_CALL(*conf_protocol, Hangup(::testing::_)).Times(1);
		ASSERT_GT(PutMSDRelease(0), 0);
		ASSERT_FALSE(ctx->IsMSDTimerStarted());
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
		ASSERT_EQ(h323->GetParserContext(dialog), std::shared_ptr<VS_H323ParserInfo>());
	}

	TEST_F(H323ParserH245Test, MSDIncomingState_TimerExpiry) {
		ctx->SetMsdState(MSDIncomingAwaitingResponse);
		ctx->StartMSDTimer();
		WaitTimeout(MSDA_WAIT_TIMEOUT);
		ASSERT_EQ(ctx->MsdState(), MSDIdle);
		ASSERT_EQ(h323->GetParserContext(dialog), std::shared_ptr<VS_H323ParserInfo>());
	}

	TEST_F(H323ParserTest, H323CallID_Parser)
	{
		VS_IndentifierH323::H323CallID c1("");
		ASSERT_TRUE(c1.GetCallId().empty());
		ASSERT_TRUE(c1.GetDTMF().empty());
		ASSERT_TRUE(c1.prefix.empty());
		ASSERT_TRUE(c1.name.empty());
		ASSERT_TRUE(c1.host.empty());
		ASSERT_TRUE(c1.dtmf.empty());

		VS_IndentifierH323::H323CallID c2("#h323:");
		ASSERT_EQ(c2.GetCallId(), "#h323:");
		ASSERT_TRUE(c2.GetDTMF().empty());
		ASSERT_EQ(c2.prefix, "#h323:");
		ASSERT_TRUE(c2.name.empty());
		ASSERT_TRUE(c2.host.empty());
		ASSERT_TRUE(c2.dtmf.empty());

		VS_IndentifierH323::H323CallID c3("#h323:user");
		ASSERT_EQ(c3.GetCallId(), "#h323:user");
		ASSERT_TRUE(c3.GetDTMF().empty());
		ASSERT_EQ(c3.prefix, "#h323:");
		ASSERT_EQ(c3.name, "user");
		ASSERT_TRUE(c3.host.empty());
		ASSERT_TRUE(c3.dtmf.empty());

		VS_IndentifierH323::H323CallID c4("#h323:\\e\\123");
		ASSERT_EQ(c4.GetCallId(), "#h323:\\e\\123");
		ASSERT_TRUE(c4.GetDTMF().empty());
		ASSERT_EQ(c4.prefix, "#h323:");
		ASSERT_EQ(c4.name, "\\e\\123");
		ASSERT_TRUE(c4.host.empty());
		ASSERT_TRUE(c4.dtmf.empty());

		VS_IndentifierH323::H323CallID c5("#h323:user@ip");
		ASSERT_EQ(c5.GetCallId(), "#h323:user@ip");
		ASSERT_TRUE(c5.GetDTMF().empty());
		ASSERT_EQ(c5.prefix, "#h323:");
		ASSERT_EQ(c5.name, "user");
		ASSERT_EQ(c5.host, "ip");
		ASSERT_TRUE(c5.dtmf.empty());

		VS_IndentifierH323::H323CallID c6("#h323:@ip");
		ASSERT_EQ(c6.GetCallId(), "#h323:@ip");
		ASSERT_TRUE(c6.GetDTMF().empty());
		ASSERT_EQ(c6.prefix, "#h323:");
		ASSERT_TRUE(c6.name.empty());
		ASSERT_EQ(c6.host, "ip");
		ASSERT_TRUE(c6.dtmf.empty());

		VS_IndentifierH323::H323CallID c7("#h323:@ip,*100#");
		ASSERT_EQ(c7.GetCallId(), "#h323:@ip");
		ASSERT_EQ(c7.GetDTMF(), "*100#");
		ASSERT_EQ(c7.prefix, "#h323:");
		ASSERT_TRUE(c7.name.empty());
		ASSERT_EQ(c7.host, "ip");
		ASSERT_EQ(c7.dtmf, "*100#");

		VS_IndentifierH323::H323CallID c8("#h323:@ip;*100#");
		ASSERT_EQ(c8.GetCallId(), "#h323:@ip");
		ASSERT_EQ(c8.GetDTMF(), "*100#");
		ASSERT_EQ(c8.prefix, "#h323:");
		ASSERT_TRUE(c8.name.empty());
		ASSERT_EQ(c8.host, "ip");
		ASSERT_EQ(c8.dtmf, "*100#");

		VS_IndentifierH323::H323CallID c9("#h323:user@ip,*100#");
		ASSERT_EQ(c9.GetCallId(), "#h323:user@ip");
		ASSERT_EQ(c9.GetDTMF(), "*100#");
		ASSERT_EQ(c9.prefix, "#h323:");
		ASSERT_EQ(c9.name, "user");
		ASSERT_EQ(c9.host, "ip");
		ASSERT_EQ(c9.dtmf, "*100#");

		VS_IndentifierH323::H323CallID c10("#h323:user@ip;*100#");
		ASSERT_EQ(c10.GetCallId(), "#h323:user@ip");
		ASSERT_EQ(c10.GetDTMF(), "*100#");
		ASSERT_EQ(c10.prefix, "#h323:");
		ASSERT_EQ(c10.name, "user");
		ASSERT_EQ(c10.host, "ip");
		ASSERT_EQ(c10.dtmf, "*100#");

		VS_IndentifierH323::H323CallID c11("#h323:user,*100#");
		ASSERT_EQ(c11.GetCallId(), "#h323:user");
		ASSERT_EQ(c11.GetDTMF(), "*100#");
		ASSERT_EQ(c11.prefix, "#h323:");
		ASSERT_EQ(c11.name, "user");
		ASSERT_TRUE(c11.host.empty());
		ASSERT_EQ(c11.dtmf, "*100#");

		VS_IndentifierH323::H323CallID c12("random");
		ASSERT_EQ(c12.GetCallId(), "random");
		ASSERT_TRUE(c12.GetDTMF().empty());
		ASSERT_TRUE(c12.prefix.empty());
		ASSERT_EQ(c12.name, "random");
		ASSERT_TRUE(c12.host.empty());
		ASSERT_TRUE(c12.dtmf.empty());

		VS_IndentifierH323::H323CallID c13("user@ip");
		ASSERT_EQ(c13.GetCallId(), "user@ip");
		ASSERT_TRUE(c13.GetDTMF().empty());
		ASSERT_TRUE(c13.prefix.empty());
		ASSERT_EQ(c13.name, "user");
		ASSERT_EQ(c13.host, "ip");
		ASSERT_TRUE(c13.dtmf.empty());
	}
}

#endif
