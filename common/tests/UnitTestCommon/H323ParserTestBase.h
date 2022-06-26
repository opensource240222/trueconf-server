#include "VS_ConferenceProtocolMock.h"
#include "VS_ConferenceProtocolFake.h"
#include "VS_SignalChannelMock.h"
#include "VS_SignalChannelFake.h"
#include "TrueGateway/h323/VS_H323ParserInfo.h"
#include "TrueGateway/h323/VS_H323Parser.h"
#include "TrueGateway/VS_GatewayStarter.h"
#include "TrueGateway/CallConfig/VS_Indentifier.h"
#include "TrueGateway/CallConfig/VS_CallConfigStorage.h"

#include <gtest/gtest.h>
#include <tests/common/ASIOEnvironment.h>
#include "std/cpplib/MakeShared.h"


struct TestableH323ParserInfo final : public VS_H323ParserInfo
{
	TestableH323ParserInfo() : VS_H323ParserInfo() {}
	using VS_H323ParserInfo::clock;
};

class TestableH323Parser : public VS_H323Parser {
public:
	std::shared_ptr<TestableH323ParserInfo> GetParserContext(string_view dialogId) {
		return VS_H323Parser::GetParserContext<TestableH323ParserInfo>(dialogId);
	}
	int OnH245Message(VS_H245MultimediaSystemControlMessage &aMessage, string_view dialogId) {
		auto &&ctx = std::static_pointer_cast<VS_H323ParserInfo>(GetParserContext(dialogId));
		return VS_H323Parser::OnH245Message(&aMessage, ctx);
	}

	using VS_H323Parser::clock;

protected:
	TestableH323Parser(boost::asio::io_service::strand& strand)
		: VS_H323Parser(strand, nullptr)
	{
	}

	static void PostConstruct(std::shared_ptr<TestableH323Parser>& ) { /*stub*/ }

	std::shared_ptr<VS_ParserInfo> GetParserContextBase(string_view dialogId, bool create) override
	{
		return VS_H323Parser::GetParserContext<TestableH323ParserInfo>(dialogId, create);
	}
};

class H323ParserTestBase : public ::testing::Test
{
protected:
	H323ParserTestBase(net::address vcsAddr, net::port vcsPort, net::address vcsTerminalAddr, net::port vcsTerminalPort)
		: vcs_addr{ std::move(vcsAddr), vcsPort, net::protocol::TCP }
		, terminal_addr{ std::move(vcsTerminalAddr), vcsTerminalPort, net::protocol::TCP }
		, strand(g_asio_environment->IOService())
		, h323(vs::MakeShared<TestableH323Parser>(strand))
		, conf_protocol(std::make_shared<VS_ConferenceProtocolMock>())
	{
		auto call_config = vs::MakeShared<VS_CallConfigStorage>();
		call_config->RegisterProtocol(VS_Indentifier::GetCommonIndentifierChain(g_asio_environment->IOService(), "serverVendor"));
		h323->UseACL(false);
		h323->SetCallConfigStorage(call_config);
		h323->SetMyCsAddress(vcs_addr);
		h323->SetPeerCSAddress({}, terminal_addr);
		h323->SetConfCallBack(conf_protocol);

		conf_protocol_fake.parser = h323.get();
		conf_protocol->DelegateTo(&conf_protocol_fake);

		h245_channel_fake = std::make_shared<VS_SignalChannelFake>();
		conf_protocol_fake.SetOurAddr(vcs_addr);

		conf_protocol_fake.SetRemoteAddr(terminal_addr);
		h245_channel = std::make_shared<VS_SignalChannelMock>();
		h245_channel->DelegateTo(h245_channel_fake.get(), h245_channel_fake);
		VS_SignalChannel::SetFactory([this](boost::asio::io_service&) { return h245_channel; });
	}

	~H323ParserTestBase()
	{
		VS_SignalChannel::SetFactory(nullptr);
	}

public:
	static void SetUpTestCase()
	{
		VS_GatewayStarter::SetIsVCS(true);
	}

	void ClearSendQueue(VS_ChannelID channel_id, unsigned max_length = 10)
	{
		for (unsigned n = 1; n <= max_length; ++n)
		{
			char buf[8 * 1024];
			std::size_t size = sizeof(buf);
			if (!h323->GetBufForSend(buf, size, channel_id, {}, 0, {}, 0))
				return;
			EXPECT_LT(0u, size);
		}
		ADD_FAILURE() << "Didn't manage to clear send queue (longer than " << max_length << ")\n";
	}

	void WaitTimeout(std::chrono::steady_clock::duration time, boost::shared_ptr<TestableH323ParserInfo> &ctx) {
		ctx->clock().add_diff(ctx->clock().now().time_since_epoch() + time);
		h323->Timeout();
	}

	net::Endpoint vcs_addr, terminal_addr;
	boost::asio::io_service::strand strand;
	std::shared_ptr<TestableH323Parser> h323;
	std::shared_ptr<VS_ConferenceProtocolMock> conf_protocol;
	VS_ConferenceProtocolFake conf_protocol_fake;
	std::shared_ptr<VS_SignalChannelMock> h245_channel;
	std::shared_ptr<VS_SignalChannelFake> h245_channel_fake;
};
