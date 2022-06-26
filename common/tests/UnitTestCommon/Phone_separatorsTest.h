#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "TrueGateway/CallConfig/VS_IndentifierSIP.h"
#include "std/cpplib/VS_WideStr.h"
#include "std/cpplib/VS_UserData.h"
#include <tests/common/ASIOEnvironment.h>

struct PhoneSeparators : public testing::Test{
	boost::shared_ptr<VS_Indentifier> identSIP;
	VS_CallConfig cfg;
	boost::shared_ptr<VS_UserData> from_user;

	const char* disp_n = "display name";
	const char* from = "from";

	bool MakeTest(std::string input, std::string expected_output);

	void SetUp(){
		identSIP = boost::make_shared<VS_IndentifierSIP>(g_asio_environment->IOService(), "serverVendor");
		identSIP->SetVoipProtocol(VS_CallConfig::SIP);
		cfg.SignalingProtocol = VS_CallConfig::SIP;
		from_user = boost::make_shared<VS_UserData>(from, disp_n);
	}
	void TearDown(){
	}
};
