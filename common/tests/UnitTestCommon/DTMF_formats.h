#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "TrueGateway/CallConfig/VS_IndentifierSIP.h"
#include "std/cpplib/VS_WideStr.h"
#include "std/cpplib/VS_UserData.h"
#include <tests/common/ASIOEnvironment.h>

typedef std::pair<std::string, std::string> sipto_dtmf_pair;

struct DTMF_formats : public testing::Test{
	boost::shared_ptr<VS_Indentifier> identSIP;
	VS_CallConfig cfg;
	boost::shared_ptr<VS_UserData> from_user;

	const char* disp_n = "display name";
	const char* from = "from";

	sipto_dtmf_pair GetDTMFTest(std::string call_string);
	bool MakeFormatTest(std::string call_string, std::string expected_call_string, std::string expected_dtmf);

	void SetUp(){
		identSIP = boost::make_shared<VS_IndentifierSIP>(g_asio_environment->IOService(), "serverVendor");
		cfg.SignalingProtocol = VS_CallConfig::SIP;
		from_user = boost::make_shared<VS_UserData>(from, disp_n);
	}
	void TearDown(){
	}
};