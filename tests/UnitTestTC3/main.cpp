#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tests/common/Utils.h"
#include "tools/Server/VS_Server.h"
#if defined(_WIN32) // Not ported yet
#include "acs/Lib/VS_AcsLib.h"
#endif
#include "SecureLib/VS_CryptoInit.h"
#include "std/cpplib/ThreadUtils.h"
#include "tests/common/ASIOEnvironment.h"

#if defined(_WIN32)
#include <Objbase.h>
#endif

extern std::string g_tr_endpoint_name;

class VSServerEnvironment : public ::testing::Environment
{
public:
	void SetUp()
	{
		// VS_Server::ServerMain
#if defined(_WIN32)
		ASSERT_TRUE(SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)));
#endif
		g_tr_endpoint_name = "unittest#vcs";

#if defined(_WIN32) // Not ported yet
		// VS_Server::Start from VCS
		ASSERT_TRUE(VS_AcsLibInitial());
#endif
	}

	void TearDown()
	{
#if defined(_WIN32)
		CoUninitialize();
#endif
	}
};

ASIOEnvironment* g_asio_environment;
int main(int argc, char* argv[])
{
	vs::FixThreadSystemMessages();
	::testing::InitGoogleMock(&argc, argv);
	vs::InitOpenSSL();
	test::InitRegistry();
	::testing::AddGlobalTestEnvironment(g_asio_environment = new ASIOEnvironment);
	::testing::AddGlobalTestEnvironment(new VSServerEnvironment);
	return RUN_ALL_TESTS();
}

bool VS_Server::Start() { return true; }
void VS_Server::Stop()  {}
const char* VS_Server::ShortName()      { return "Test"; }
const char* VS_Server::LongName()       { return "Test"; }
const char* VS_Server::RegistryKey()    { return "TrueConf\\Server"; }
const char* VS_Server::ServiceName()    { return "Test"; }
const char* VS_Server::ProductVersion() { return "Test"; }