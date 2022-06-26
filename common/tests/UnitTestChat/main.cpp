#include "chatlib/utils/chat_utils.h"
#include "std/cpplib/ThreadUtils.h"
#include "tests/common/Utils.h"

#include <sqlite3.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

int main(int argc, char* argv[])
{
	sqlite3_config(SQLITE_CONFIG_URI, 1);
	vs::FixThreadSystemMessages();
	::testing::InitGoogleMock(&argc, argv);
	test::InitRegistry();
	chat::SetUUIDGeneratorFunc([]() {
		return boost::uuids::to_string(boost::uuids::random_generator()()); });
	return RUN_ALL_TESTS();
}
