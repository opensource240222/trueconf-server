#pragma once

#include <tests/common/ASIOEnvironment.h>

#include "TrueGateway/sip/VS_SIPParser.h"
#include "TrueGateway/CallConfig/VS_CallConfigStorage.h"
#include "TrueGateway/CallConfig/VS_IndentifierSIP.h"
#include "SIPParserLib/VS_SIPRequest.h"
#include "SIPParserLib/VS_SIPResponse.h"
#include "SIPParserLib/VS_SIPURI.h"
#include "std/cpplib/MakeShared.h"
#include "std/cpplib/VS_Policy.h"

boost::shared_ptr<VS_SIPURI> makeownURI(const std::string &to_name, const std::string &to_host);
boost::shared_ptr<VS_SIPParserInfo> makeInfo(const std::string& dialog_id, const std::string& from, const std::string& to, const std::string& display_name, boost::asio::io_service::strand& s);
bool MakeInviteRequest(std::shared_ptr<VS_SIPRequest> &req, boost::shared_ptr<VS_SIPParserInfo> &pCallInfo);

typedef std::pair<std::weak_ptr<VS_SIPParserInfo>, std::shared_ptr<VS_SIPMessage>> ctx_msg_pair;

struct DTMFTest : public testing::Test{
	boost::asio::io_service::strand strand;
	std::shared_ptr<VS_SIPParser> pParser;
	std::shared_ptr<VS_CallConfigStorage> storage;
	boost::shared_ptr<VS_IndentifierSIP> ident;
	net::Endpoint addr_local,addr_remote;

	static std::string dialog_id;
	static std::string from;
	static std::string to;
	static std::string to_name;
	static std::string to_host;

	static std::string display_name;
	static std::string server;

	std::shared_ptr<VS_SIPParserInfo> SimulateIncomingInvite();
	ctx_msg_pair SendDTMF();
	bool ReceiveResponseOnDTMFRequest(ctx_msg_pair dtmf_info_message);

	DTMFTest()
		: strand(g_asio_environment->IOService())
	{
	}
	virtual void SetUp(){
		const std::string userAgent = "serverVendor";
		pParser = vs::MakeShared<VS_SIPParser>(strand, userAgent, nullptr);
		storage = vs::MakeShared<VS_CallConfigStorage>();
		ident = boost::make_shared<VS_IndentifierSIP>(g_asio_environment->IOService(), userAgent);

		boost::system::error_code ec;
		addr_local.addr = net::address::from_string("127.0.0.1", ec);
		addr_local.port = 5060;
		addr_remote.addr = net::address::from_string("192.168.0.1", ec);
		addr_remote.port = 5060;

		storage->RegisterProtocol(ident);
		pParser->SetConfCallBack(pParser);
		pParser->SetCallConfigStorage(storage);
		pParser->SetMyCsAddress(addr_local);
		pParser->SetPolicy(boost::make_shared<VS_Policy>("SIP"));
	}
	virtual void TearDown(){
	}
};