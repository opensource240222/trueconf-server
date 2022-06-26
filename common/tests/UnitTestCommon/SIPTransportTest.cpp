#include "TrueGateway/sip/VS_SIPParser.h"
#include "VS_ConferenceProtocolMock.h"
#include "std/cpplib/VS_Policy.h"
#include "std/cpplib/MakeShared.h"
#include <tests/common/ASIOEnvironment.h>

#include <gtest/gtest.h>

#include <boost/make_shared.hpp>
#include <boost/asio/io_service.hpp>

#include "SIPParserLib/VS_SIPField_Contact.h"
#include "SIPParserLib/VS_SIPField_To.h"
#include "SIPParserLib/VS_SIPField_Via.h"
#include "SIPParserLib/VS_SIPMetaField.h"
#include "TrueGateway/sip/VS_SIPParserInfo.h"

#include "SIPParserLib/VS_SIPRequest.h"
#include "SIPParserLib/VS_SIPResponse.h"
#include "SIPParserLib/VS_SIPURI.h"

#include "TrueGateway/CallConfig/VS_CallConfigStorage.h"
#include "TrueGateway/CallConfig/VS_IndentifierSIP.h"

#include "TestSIPTransportLayer.h"

#include "../TrueGateway/sip/VS_SIPGetInfoImpl.h"
#include "../TrueGateway/sip/VS_SIPUpdateInfoImpl.h"

extern char raw_sony_xg77_invite[];

namespace {

	class SIPTransportTest : public ::testing::Test {
	public:
		SIPTransportTest()
			: strand(g_asio_environment->IOService())
			, m_conf_protocol(std::make_shared<VS_ConferenceProtocolMock>())
		{
		}

		virtual void SetUp() {
			const std::string userAgent = "serverVendor";
			boost::system::error_code ec;

			myEp.addr = net::address::from_string("1.2.3.4", ec);
			myEp.port  = 5060;
			myEp.protocol = net::protocol::UDP;

			peerEp.addr = net::address::from_string("21.22.23.24", ec);
			peerEp.port = 5060;
			peerEp.protocol = net::protocol::UDP;

			m_sip_parser = vs::MakeShared<VS_SIPParser>(strand, userAgent, nullptr);
			m_sip_parser->SetConfCallBack(m_conf_protocol);
			m_sip_parser->SetPolicy(boost::make_shared<VS_Policy>("SIP"));

			auto call_config_storage = vs::MakeShared<VS_CallConfigStorage>();
			sip_transport = std::make_shared<test::SIPTransportLayer>(strand,m_sip_parser, call_config_storage, myEp, peerEp);

			ctx = std::make_shared<VS_SIPParserInfo>(userAgent);
			ctx->SetExpires(std::chrono::seconds(60));

			auto identifier = boost::make_shared<VS_IndentifierSIP>(g_asio_environment->IOService(), "serverVendor");

			auto call_config = vs::MakeShared<VS_CallConfigStorage>();
			call_config->RegisterProtocol(identifier);
			m_sip_parser->SetCallConfigStorage(call_config);

			ctx->SetConfig(call_config->GetConfiguration(peerEp, VS_CallConfig::SIP));
			ctx->SetSIPRemoteTarget(std::string("21.22.23.24"));

			ctx->SetAliasMy("userqwe");
			ctx->SetUser("qwe");
			ctx->SetDomain("asdf");

			ctx->SetAliasRemote("yuio");
			ctx->SetTagSip("ofmgUL-c4JP1y4RIkzHZ.xcskWlx9Btw");
			ctx->SetDisplayNameSip("hjk");

			ctx->MyBranch("7W.9fqkrTP.wF8lmS.kKCrVoRnD");

			auto &&config = ctx->GetConfig();
			config.Address = peerEp;
			config.Login = "qwe";
			config.HostName = "asdf";

			ctx->SIPDialogID("cc4c785e-e014-11d3-8141-421b47e33a7f");
		}

		void TearDown() override
		{

		}

		bool FillMsgOutgoing(const std::shared_ptr<VS_SIPMessage> &msg)
		{
			return sip_transport->FillMsgOutgoing(ctx, msg);
		}

		boost::asio::io_service::strand strand;
		std::shared_ptr<test::SIPTransportLayer> sip_transport;
		std::shared_ptr<VS_SIPParserInfo> ctx;
		boost::asio::io_service m_ios;
	protected:
		std::shared_ptr<VS_SIPParser> m_sip_parser;
		std::shared_ptr<VS_ConferenceProtocolMock> m_conf_protocol;
		net::Endpoint myEp, peerEp;
	};

	TEST_F(SIPTransportTest, FillRequest) {
		std::vector<std::shared_ptr<VS_SIPRequest>> msgs(10);
		std::generate(msgs.begin(), msgs.end(), &std::make_shared<VS_SIPRequest>);

		const VS_SIPGetInfoImpl get_info(*ctx);
		VS_SIPUpdateInfoImpl update_info(*ctx);

		ASSERT_TRUE(msgs[0]->MakeINVITE(get_info, update_info));
		ASSERT_TRUE(msgs[1]->MakeREGISTER(get_info, update_info));
		ASSERT_TRUE(msgs[2]->MakeNOTIFY(get_info, update_info));
		ASSERT_TRUE(msgs[3]->MakeRefreshINVITE(get_info, update_info));
		ASSERT_TRUE(msgs[4]->MakeACK(get_info, update_info, true));
		ASSERT_TRUE(msgs[5]->MakeBYE(get_info, update_info));
		ASSERT_TRUE(msgs[6]->MakeCANCEL(get_info, update_info));
		ASSERT_TRUE(msgs[7]->MakeINFO_FastUpdatePicture(get_info, update_info));
		ASSERT_TRUE(msgs[8]->MakeINFO_DTMF(get_info, update_info, 1));
		ASSERT_TRUE(msgs[9]->MakeNOTIFY(get_info, update_info));

		for (auto &msg : msgs) {
			ASSERT_TRUE(msg->GetSIPMetaField()->iVia.empty());
			ASSERT_EQ(msg->GetSIPMetaField()->iContact, nullptr);
			ASSERT_EQ(msg->GetSIPMetaField()->iTo, nullptr);

			ASSERT_TRUE(FillMsgOutgoing(msg));

			ASSERT_FALSE(msg->GetSIPMetaField()->iVia.empty());
			ASSERT_NE(msg->GetSIPMetaField()->iVia[0], nullptr);
			ASSERT_STREQ(msg->GetSIPMetaField()->iVia[0]->Host().c_str(), "1.2.3.4");
			ASSERT_EQ(msg->GetSIPMetaField()->iVia[0]->Port(), 5060);
			ASSERT_EQ(msg->GetSIPMetaField()->iVia[0]->ConnectionType(), net::protocol::UDP);

			eStartLineType m = msg->GetMethod();
			if (m != TYPE_BYE && m != TYPE_CANCEL) {
				ASSERT_NE(msg->GetSIPMetaField()->iContact, nullptr);
				ASSERT_NE(msg->GetSIPMetaField()->iContact->GetLastURI(), nullptr);
				ASSERT_STREQ(msg->GetSIPMetaField()->iContact->GetLastURI()->Host().c_str(), "1.2.3.4");
				ASSERT_EQ(msg->GetSIPMetaField()->iContact->GetLastURI()->Port(), 5060);
				ASSERT_EQ(msg->GetSIPMetaField()->iContact->GetLastURI()->Transport(), net::protocol::UDP);
			}

			ASSERT_NE(msg->GetSIPMetaField()->iTo, nullptr);
			ASSERT_NE(msg->GetSIPMetaField()->iTo->GetURI(), nullptr);
			ASSERT_STREQ(msg->GetSIPMetaField()->iTo->GetURI()->Host().c_str(), "yuio");

			char* out = 0;
			size_t out_sz = 0;

			ASSERT_EQ(TSIPErrorCodes::e_buffer, msg->Encode(out, out_sz));
			ASSERT_GT(out_sz, 0u);

			out = new char[out_sz + 1];
			std::unique_ptr<char[]> _(out);
			ASSERT_EQ(TSIPErrorCodes::e_ok, msg->Encode(out, out_sz));
		}
	}

	TEST_F(SIPTransportTest, AcceptAndCloseUDP) {
		using ::testing::_;
		using ::testing::Return;
		using ::testing::AtLeast;

		net::Endpoint myEp, peerEp;
		boost::system::error_code ec;
		myEp.addr = net::address::from_string("1.2.3.4", ec);
		myEp.port = 5060;
		myEp.protocol = net::protocol::UDP;

		peerEp.addr = net::address::from_string("5.6.7.8", ec);
		peerEp.port = 5060;
		peerEp.protocol = net::protocol::UDP;

		boost::asio::io_service ios;
		test::asio::udp_socket_mock udpMock{ ios };
		ON_CALL(*udpMock.impl_, local_endpoint(_)).WillByDefault(Return(boost::asio::ip::udp::endpoint(myEp.addr, myEp.port)));
		ON_CALL(*udpMock.impl_, remote_endpoint(_)).WillByDefault(Return(boost::asio::ip::udp::endpoint(peerEp.addr, peerEp.port)));
		EXPECT_CALL(*udpMock.impl_, local_endpoint(_)).Times(AtLeast(1));
		EXPECT_CALL(*udpMock.impl_, remote_endpoint(_)).Times(AtLeast(1));

		sip_transport->Accept<test::asio::udp_socket_mock, acs::Handler::packet_buffer, test::FakeChannel>(std::move(udpMock), acs::Handler::packet_buffer(), net::protocol::UDP);
		auto info = sip_transport->GetCSInfo(peerEp);
		EXPECT_TRUE(info.isAccepted);
		EXPECT_EQ(info.bindEp, myEp);
		EXPECT_EQ(info.peerEp, peerEp);

		EXPECT_TRUE(sip_transport->CloseChannel(peerEp));
		sip_transport->Timeout();
		info = sip_transport->GetCSInfo(peerEp);
		EXPECT_FALSE(info.isAccepted);
		EXPECT_NE(info.bindEp, myEp);
		EXPECT_NE(info.peerEp, peerEp);
	}

	TEST_F(SIPTransportTest, AcceptAndCloseTCP) {
		using ::testing::_;
		using ::testing::Return;
		using ::testing::AtLeast;

		net::Endpoint myEp, peerEp;
		boost::system::error_code ec;
		myEp.addr = net::address::from_string("1.2.3.4", ec);
		myEp.port = 5060;
		myEp.protocol = net::protocol::TCP;

		peerEp.addr = net::address::from_string("5.6.7.8", ec);
		peerEp.port = 5060;
		peerEp.protocol = net::protocol::TCP;

		boost::asio::io_service ios;
		test::asio::tcp_socket_mock tcpMock{ ios };
		ON_CALL(*tcpMock.impl_, local_endpoint(_)).WillByDefault(Return(boost::asio::ip::tcp::endpoint(myEp.addr, myEp.port)));
		ON_CALL(*tcpMock.impl_, remote_endpoint(_)).WillByDefault(Return(boost::asio::ip::tcp::endpoint(peerEp.addr, peerEp.port)));
		EXPECT_CALL(*tcpMock.impl_, local_endpoint(_)).Times(AtLeast(1));
		EXPECT_CALL(*tcpMock.impl_, remote_endpoint(_)).Times(AtLeast(1));

		sip_transport->Accept<test::asio::tcp_socket_mock, acs::Handler::stream_buffer, test::FakeChannel>(std::move(tcpMock), acs::Handler::stream_buffer(), net::protocol::TCP);
		auto info = sip_transport->GetCSInfo(peerEp);
		EXPECT_TRUE(info.isAccepted);
		EXPECT_EQ(info.bindEp, myEp);
		EXPECT_EQ(info.peerEp, peerEp);

		EXPECT_TRUE(sip_transport->CloseChannel(peerEp));
		sip_transport->Timeout();
		info = sip_transport->GetCSInfo(peerEp);
		EXPECT_FALSE(info.isAccepted);
		EXPECT_NE(info.bindEp, myEp);
		EXPECT_NE(info.peerEp, peerEp);
	}
#
	TEST_F(SIPTransportTest, SameDialogDifferentAddresses) {
		using ::testing::_;
		using ::testing::Return;

		net::Endpoint myEp, peerEp, peerEp1;
		boost::system::error_code ec;
		myEp.addr = net::address::from_string("1.2.3.4", ec);
		myEp.port = 5060;
		myEp.protocol = net::protocol::TCP;

		peerEp.addr = net::address::from_string("5.6.7.8", ec);
		peerEp.port = 5060;
		peerEp.protocol = net::protocol::TCP;

		peerEp1.addr = net::address::from_string("4.3.2.1", ec);
		peerEp1.port = 5060;
		peerEp1.protocol = net::protocol::TCP;

		boost::asio::io_service ios;
		test::asio::tcp_socket_mock tcpMock{ ios };
		ON_CALL(*tcpMock.impl_, local_endpoint(_)).WillByDefault(Return(boost::asio::ip::tcp::endpoint(myEp.addr, myEp.port)));
		ON_CALL(*tcpMock.impl_, remote_endpoint(_)).WillByDefault(Return(boost::asio::ip::tcp::endpoint(peerEp.addr, peerEp.port)));

		test::asio::tcp_socket_mock tcpMock1{ ios };
		ON_CALL(*tcpMock1.impl_, local_endpoint(_)).WillByDefault(Return(boost::asio::ip::tcp::endpoint(myEp.addr, myEp.port)));
		ON_CALL(*tcpMock1.impl_, remote_endpoint(_)).WillByDefault(Return(boost::asio::ip::tcp::endpoint(peerEp1.addr, peerEp1.port)));

		acs::Handler::stream_buffer invite(raw_sony_xg77_invite, raw_sony_xg77_invite + strlen(raw_sony_xg77_invite));
		sip_transport->Accept<test::asio::tcp_socket_mock, acs::Handler::stream_buffer, test::FakeChannel>(std::move(tcpMock), std::move(invite), net::protocol::TCP);
		auto info = sip_transport->GetCSInfo("cc4c785e-e014-11d3-8141-421b47e33a7f");
		EXPECT_TRUE(info.isAccepted);
		EXPECT_EQ(info.bindEp, myEp);
		EXPECT_EQ(info.peerEp, peerEp);
		EXPECT_EQ(info.numDialogs, 1);

		acs::Handler::stream_buffer invite1(raw_sony_xg77_invite, raw_sony_xg77_invite + strlen(raw_sony_xg77_invite));
		sip_transport->Accept<test::asio::tcp_socket_mock, acs::Handler::stream_buffer, test::FakeChannel>(std::move(tcpMock1), std::move(invite1), net::protocol::TCP);
		info = sip_transport->GetCSInfo("cc4c785e-e014-11d3-8141-421b47e33a7f");
		EXPECT_TRUE(info.isAccepted);
		EXPECT_EQ(info.bindEp, myEp);
		EXPECT_EQ(info.peerEp, peerEp1);
		EXPECT_EQ(info.numDialogs, 1);

		info = sip_transport->GetCSInfo(peerEp);
		EXPECT_EQ(info.numDialogs, 0);
	}
	TEST_F(SIPTransportTest, UseRegCtxChannel) {
		const std::string regDialog = "regDialog";
		// 1. Fake channel is created at  fake sip_transport startup
		// 2. Add reg dialog to fake channel
		sip_transport->AddDialogToFakeCh(regDialog);
		auto &cfg = ctx->GetConfig();
		cfg.Address = peerEp;
		cfg.Address.addr = net::address::from_string("1.2.3.4");	// set other address then peerEp
		ctx->SetSIPRemoteTarget("1.2.3.4");

		EXPECT_NE(peerEp, cfg.Address);
		ctx->SetRegCtxDialogID(regDialog);

		// 3. Make invite
		auto msg = std::make_shared<VS_SIPRequest>();
		const VS_SIPGetInfoImpl get_info(*ctx);
		VS_SIPUpdateInfoImpl update_info(*ctx);
		msg->MakeINVITE(get_info, update_info);
		ctx->GetConfig().ConnectionTypeSeq.push_back(net::protocol::UDP);
		EXPECT_TRUE(sip_transport->Write(ctx, msg));
		sip::TransportChannel chInfo;
		EXPECT_TRUE(sip_transport->FindChannel(peerEp, chInfo));
		EXPECT_EQ(chInfo.queueOut.size(), 1);
	}
}