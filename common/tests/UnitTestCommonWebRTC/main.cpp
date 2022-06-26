#include "SecureLib/VS_CryptoInit.h"
#include "std/cpplib/ThreadUtils.h"
#include "tests/common/Utils.h"
#include "tools/Server/VS_Server.h"
#include "tests/common/ASIOEnvironment.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

ASIOEnvironment* g_asio_environment;

int main(int argc, char* argv[])
{
	vs::FixThreadSystemMessages();
	::testing::InitGoogleMock(&argc, argv);
	vs::InitOpenSSL();
	test::InitRegistry();
	::testing::AddGlobalTestEnvironment(g_asio_environment = new ASIOEnvironment);
	return RUN_ALL_TESTS();
}

bool VS_Server::Start() { return true; }
void VS_Server::Stop()  {}
const char* VS_Server::ShortName()      { return "Test"; }
const char* VS_Server::LongName()       { return "Test"; }
const char* VS_Server::RegistryKey()    { return "TrueConf\\Server"; }
const char* VS_Server::ServiceName()    { return "Test"; }
const char* VS_Server::ProductVersion() { return "Test"; }
