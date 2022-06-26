#include "std/cpplib/ThreadUtils.h"
#include "tests/common/Utils.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

int main(int argc, char* argv[])
{
	vs::FixThreadSystemMessages();
	::testing::InitGoogleMock(&argc, argv);
	test::InitRegistry();
	return RUN_ALL_TESTS();
}
