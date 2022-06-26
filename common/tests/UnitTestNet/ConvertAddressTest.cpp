#if defined(_WIN32)

#include "net/ConvertAddress.h"

#include <gtest/gtest.h>

namespace net_test {

static const char* params[] = {
	"0.0.0.0",
	"::0",
	"127.0.0.1",
	"::1",
	"192.168.10.20",
	"123.45.67.89",
	"fe80::abcd:0123",
};
class ConvertAddressTest : public ::testing::TestWithParam<const char*> {};
INSTANTIATE_TEST_CASE_P(ConvertAddress, ConvertAddressTest, ::testing::ValuesIn(params));

TEST_P(ConvertAddressTest, Test)
{
	const VS_IPPortAddress old_addr(GetParam());
	boost::system::error_code ec;
	const net::address new_addr = net::address::from_string(GetParam(), ec);
	ASSERT_TRUE(!ec) << "Incorrect test data: " << ec;

	EXPECT_EQ(old_addr, net::ConvertAddress(new_addr));
	EXPECT_EQ(new_addr, net::ConvertAddress(old_addr));
}

TEST(ConvertAddress, EmptyAddress)
{
	VS_IPPortAddress old_addr("0");
	net::address new_addr;

	EXPECT_EQ(old_addr, net::ConvertAddress(new_addr));
	EXPECT_EQ(new_addr, net::ConvertAddress(old_addr));
}

}

#endif
