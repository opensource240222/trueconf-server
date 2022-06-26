#include "tests/common/Utils.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>

void save_file(const char *file, const char *msg)
{
	std::ofstream stream;
	stream.open(file, std::ios::out);
	if(stream.is_open())
	{
		stream << msg;
	}
	else
	{
		throw std::runtime_error("Error open file: " + std::string(file));
	}
}

static const char MSG_CONFIG[] = "RegistryBackend = file_backend";
static const char CFG_FILE[] = "test_tc_server.cfg";

int main(int argc, char* argv[])
{
	::testing::InitGoogleMock(&argc, argv);
	test::InitRegistry();
	save_file(CFG_FILE, MSG_CONFIG);
	return RUN_ALL_TESTS();
}