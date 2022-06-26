#include "std/cpplib/ForEachHost.h"

#include <gtest/gtest.h>

namespace foreachhost_test {

struct HostPortPair
{
	string_view host;
	string_view port;
};

using expected_pairs_t = std::vector<HostPortPair>;

void CheckAddresses(string_view addrs, const expected_pairs_t & expected)
{
	size_t i = 0;
	ForEachHost(addrs, [&i, &expected] (string_view host, string_view port) {
		ASSERT_LT(i, expected.size());
		EXPECT_EQ(std::string(host), std::string(expected[i].host));
		EXPECT_EQ(std::string(port), std::string(expected[i].port));
		++i;
	});
}

TEST(ForEachHostTest, IPv4)
{

	CheckAddresses("192.168.0.1:4307,127.0.0.1:4307", expected_pairs_t({
		{"192.168.0.1", "4307"},
		{"127.0.0.1", "4307"}
	}));

}

TEST(ForEachHostTest, IPv4andIPv6)
{

	CheckAddresses("192.168.0.1:80,[fd00:110::14:823c]:4307,127.0.0.1:4307", expected_pairs_t({
		{"192.168.0.1", "80"},
		{"[fd00:110::14:823c]", "4307"},
		{"127.0.0.1", "4307"}
	}));

}

TEST(ForEachHostTest, WithoutPort)
{

	CheckAddresses("192.168.0.1:80,[fd00:110::14:823c],127.0.0.1,[::1]:1234", expected_pairs_t({
		{"192.168.0.1", "80"},
		{"[fd00:110::14:823c]", ""},
		{"127.0.0.1", ""},
		{"[::1]", "1234"}
	}));

}

TEST(ForEachHostTest, OneIP)
{

	CheckAddresses("192.168.0.1:80", expected_pairs_t({
		{"192.168.0.1", "80"}
	}));

	CheckAddresses("192.168.0.1", expected_pairs_t({
		{"192.168.0.1", ""}
	}));

	CheckAddresses("[fd00:110::14:823c]:1234", expected_pairs_t({
		{"[fd00:110::14:823c]", "1234"}
	}));

	CheckAddresses("[fd00:110::14:823c]", expected_pairs_t({
		{"[fd00:110::14:823c]", ""}
	}));

}

}
