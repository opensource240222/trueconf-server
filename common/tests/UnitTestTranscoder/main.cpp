#include "std/cpplib/ThreadUtils.h"
#include "tests/common/ASIOEnvironment.h"
#include "tests/common/Utils.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

ASIOEnvironment* g_asio_environment;

int main(int argc, char* argv[])
{
	vs::FixThreadSystemMessages();
	::testing::InitGoogleMock(&argc, argv);
	test::InitRegistry();
	::testing::AddGlobalTestEnvironment(g_asio_environment = new ASIOEnvironment);
	return RUN_ALL_TESTS();
}
