#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../TrueGateway/sip/VS_SIPParserInfo.h"
#include "../../SIPParserBase/VS_Const.h"
#include "../../SIPParserLib/VS_SIPRequest.h"
#include "../../SIPParserLib/VS_SIPInstantMessage.h"
#include "../../TrueGateway/sip/VS_SIPParser.h"
#include "../../TrueGateway/CallConfig/VS_CallConfigStorage.h"
#include "../../TrueGateway/CallConfig/VS_IndentifierSIP.h"
#include "VS_ConferenceProtocolMock.h"
#include "std/cpplib/MakeShared.h"

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <tests/common/ASIOEnvironment.h>


struct SIPChat : public testing::Test{
	boost::asio::io_service::strand strand;
	std::shared_ptr<VS_SIPParser> pParser;
	std::shared_ptr<VS_CallConfigStorage> storage;
	boost::shared_ptr<VS_IndentifierSIP> ident;
	std::shared_ptr<VS_ConferenceProtocolMock> conf_mock;
	VS_CallConfig config;

	const char *dialog_id = "123456789";
	const char *from = "from@host.org";
	const char *to = "#sip:to@host.org";
	const char *display_name = "display_name";
	const char *message = "Hello World!!!\n";

	net::Endpoint epLocal, epRemote;

	static boost::shared_ptr<VS_SIPParserInfo> MakeInfo(string_view dialogId, string_view from, string_view to, string_view displayName, boost::asio::io_service::strand &strand);
	std::shared_ptr<VS_SIPParserInfo> GetInternalCtx(VS_SIPParser *parser, string_view dialogId);

	SIPChat()
		: strand(g_asio_environment->IOService())
		, conf_mock(std::make_shared<VS_ConferenceProtocolMock>())
	{
	}

	void SetUp() override
	{
		const std::string userAgent = "serverVendor";
		pParser = vs::MakeShared<VS_SIPParser>(strand, userAgent, nullptr);
		storage = vs::MakeShared<VS_CallConfigStorage>();
		ident = boost::make_shared<VS_IndentifierSIP>(g_asio_environment->IOService(), userAgent);

		epLocal = net::Endpoint{ net::address::from_string("127.0.0.1"), 5060 , net::protocol::TCP };
		epRemote = net::Endpoint{ net::address::from_string("10.10.0.1"), 5060, net::protocol::TCP };

		storage->RegisterProtocol(ident);
		pParser->SetConfCallBack(conf_mock);
		pParser->SetCallConfigStorage(storage);
		pParser->SetMyCsAddress(epLocal);
		pParser->SetPolicy(boost::make_shared<VS_Policy>("test_policy"));
		config.Address = epRemote;
	}
	void TearDown() override
	{
	}

};
