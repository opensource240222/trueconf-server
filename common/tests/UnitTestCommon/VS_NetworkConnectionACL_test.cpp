#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <cstring>

#include "TrueGateway/VS_NetworkConnectionACL.h"
#include "std/cpplib/netutils.h"

using namespace std;
using namespace netutils;

struct ACLTestDataString {
	const char *entry; // address string
	const bool res; // true if access is allowed in whitelist mode
};

static const char *masks_str[] = {
	"192.168.1.12/32",
	"192.168.10.12/32",
	"168.131.0.0/16",
	"2001:0db8:85a3::0/49",
	"FE80:0000:0000:0000:0202:B3FF:FE1E:8329/128"
};

static ACLTestDataString TestData_str[] = {
	{ "192.168.1.12",  true  },
	{ "192.168.10.12", true  },
	{ "168.131.1.1",   true  },
	{ "168.132.1.14",  false },
	{ "168.139.1.11",  false },
	{ "2001:db8:85a3:0:0:8a2e:370:7334",         true  },
	{ "FE80:0000:0000:0000:0202:B3FF:FE1E:8329", true  },
	{ "FE80:0000:0000:00FF:0202:B3FF:FE1E:8320", false }
};

static bool ParseEntry(const char *entry_string, IPAddress &ip, size_t &mask)
{
	const char *p;

	p = strrchr(entry_string, '/');
	if (p == NULL)
		return false;

	{
		string saddr(entry_string, p - entry_string);

		mask = atoi(p + 1);

		return StringToIPAddress(saddr.c_str(), ip);
	}
}

namespace network_connection_acl_test {

	class VS_NetworkConnectionACL_Tests : public testing::Test {
	protected:
		virtual void SetUp() {}

		virtual void TearDown() {}
	private:
	};

	TEST_F(VS_NetworkConnectionACL_Tests, TestGeneral)
	{
		VS_NetworkConnectionACL acl;

		acl.SetMode(VS_NetworkConnectionACL::ACL_NONE);

		ASSERT_EQ(VS_NetworkConnectionACL::ACL_NONE, acl.GetMode()) << "Illegal ACL mode: ACL_NONE expected." << endl;

		acl.SetMode(VS_NetworkConnectionACL::ACL_WHITELIST);

		ASSERT_TRUE(acl.AddEntry("192.168.0.0/16"));

		ASSERT_FALSE(acl.IsAllowed("192.168.1.12")) << "Access check before finaliztion failed." << endl;
		acl.Finalize();

		ASSERT_FALSE(acl.AddEntry("192.168.1.1/32")) << "ACL modifiaction after finalization is possible." << endl;;

		ASSERT_TRUE(acl.IsAllowed("192.168.1.12"));
		ASSERT_FALSE(acl.IsAllowed("220.232.1.9"));
	}

	TEST_F(VS_NetworkConnectionACL_Tests, TestExtremeRanges)
	{
		{
			VS_NetworkConnectionACL acl;
			acl.SetMode(VS_NetworkConnectionACL::ACL_WHITELIST);

			acl.AddEntry("0.0.0.0/32");
			acl.AddEntry("::0/128");
			acl.Finalize();

			ASSERT_FALSE(acl.IsAllowed("192.168.1.1"));
			ASSERT_FALSE(acl.IsAllowed("FE80:0000:0000:0000:0202:B3FF:FE1E:8329"));
		}

		{
			VS_NetworkConnectionACL acl;
			acl.SetMode(VS_NetworkConnectionACL::ACL_WHITELIST);

			acl.AddEntry("0.0.0.0/0");
			acl.AddEntry("::0/0");
			acl.Finalize();

			ASSERT_TRUE(acl.IsAllowed("192.168.1.1"));
			ASSERT_TRUE(acl.IsAllowed("FE80:0000:0000:0000:0202:B3FF:FE1E:8329"));
		}
	}

	// String Based Tests
	TEST_F(VS_NetworkConnectionACL_Tests, TestWhitelist)
	{
		VS_NetworkConnectionACL acl;

		acl.SetMode(VS_NetworkConnectionACL::ACL_WHITELIST);
		for (auto &v : masks_str)
		{
			ASSERT_TRUE(acl.AddEntry(v)) << "Can't add entry: " << v << endl;
		}

		acl.Finalize();

		for (auto &v : TestData_str)
		{
			ASSERT_TRUE(acl.IsAllowed(v.entry) == v.res) << "Bad result for " << v.entry << endl;
		}
	}

	TEST_F(VS_NetworkConnectionACL_Tests, TestBlacklist)
	{
		VS_NetworkConnectionACL acl;

		acl.SetMode(VS_NetworkConnectionACL::ACL_BLACKLIST);
		for (auto &v : masks_str)
		{
			ASSERT_TRUE(acl.AddEntry(v)) << "Can't add entry: " << v << endl;
		}

		acl.Finalize();

		for (auto &v : TestData_str)
		{
			ASSERT_FALSE(acl.IsAllowed(v.entry) == v.res) << "Bad result for " << v.entry << endl;
		}
	}

	// Integer Based Tests
	TEST_F(VS_NetworkConnectionACL_Tests, TestWhitelist_IntegerAddresses)
	{
		VS_NetworkConnectionACL acl;

		acl.SetMode(VS_NetworkConnectionACL::ACL_WHITELIST);
		for (auto &v : masks_str)
		{
			IPAddress ip;
			size_t mask_bits;

			ASSERT_TRUE(ParseEntry(v, ip, mask_bits)) << "Can\'t parse entry: " << v << endl;

			if (ip.type == IP_ADDR_V4)
			{
				ASSERT_TRUE(acl.AddEntry(ip.addr.ipv4, GetMaskValue_IPv4(mask_bits))) << "Can't add IPv4 entry: " << v << endl;
			}
			else
			{
				uint32_t mask[4];

				GetMaskValue_IPv6(mask, mask_bits);
				ASSERT_TRUE(acl.AddEntry(ip.addr.ipv6, mask)) << "Can\'t add IPv6 entry: " << v << endl;
			}
		}

		acl.Finalize();

		for (auto &v : TestData_str)
		{
			ASSERT_TRUE(acl.IsAllowed(v.entry) == v.res) << "Bad result for " << v.entry << endl;
		}
	}

	TEST_F(VS_NetworkConnectionACL_Tests, TestBlacklist_IntegerAddresses)
	{
		VS_NetworkConnectionACL acl;

		acl.SetMode(VS_NetworkConnectionACL::ACL_BLACKLIST);
		for (auto &v : masks_str)
		{
			IPAddress ip;
			size_t mask_bits;

			ASSERT_TRUE(ParseEntry(v, ip, mask_bits)) << "Can\'t parse entry: " << v << endl;

			if (ip.type == IP_ADDR_V4)
			{
				ASSERT_TRUE(acl.AddEntry(ip.addr.ipv4, GetMaskValue_IPv4(mask_bits))) << "Can't add IPv4 entry: " << v << endl;
			}
			else
			{
				uint32_t mask[4];

				GetMaskValue_IPv6(mask, mask_bits);
				ASSERT_TRUE(acl.AddEntry(ip.addr.ipv6, mask)) << "Can\'t add IPv6 entry: " << v << endl;
			}
		}

		acl.Finalize();

		for (auto &v : TestData_str)
		{
			ASSERT_FALSE(acl.IsAllowed(v.entry) == v.res) << "Bad result for " << v.entry << endl;
		}
	}

	TEST_F(VS_NetworkConnectionACL_Tests, TestWhitelist_VS_Addresses)
	{
		VS_NetworkConnectionACL acl;

		acl.SetMode(VS_NetworkConnectionACL::ACL_WHITELIST);
		for (auto &v : masks_str)
		{
			IPAddress ip;
			size_t mask_bits;

			ASSERT_TRUE(ParseEntry(v, ip, mask_bits)) << "Can\'t parse entry: " << v << endl;

			if (ip.type == IP_ADDR_V4)
			{
				ASSERT_TRUE(acl.AddEntry(ip.addr.ipv4, GetMaskValue_IPv4(mask_bits))) << "Can't add IPv4 entry: " << v << endl;
			}
			else
			{
				uint32_t mask[4];

				GetMaskValue_IPv6(mask, mask_bits);
				ASSERT_TRUE(acl.AddEntry(ip.addr.ipv6, mask)) << "Can\'t add IPv6 entry: " << v << endl;
			}
		}

		acl.Finalize();

		for (auto &v : TestData_str)
		{
			boost::system::error_code ec;
			auto addr = net::address::from_string(v.entry, ec);
			EXPECT_FALSE(ec);
			ASSERT_TRUE(acl.IsAllowed(addr) == v.res) << "Bad result for " << v.entry << endl;
		}
	}

	TEST_F(VS_NetworkConnectionACL_Tests, TestBlacklist_VS_Addresses)
	{
		VS_NetworkConnectionACL acl;

		acl.SetMode(VS_NetworkConnectionACL::ACL_BLACKLIST);
		for (auto &v : masks_str)
		{
			IPAddress ip;
			size_t mask_bits;

			ASSERT_TRUE(ParseEntry(v, ip, mask_bits)) << "Can\'t parse entry: " << v << endl;

			if (ip.type == IP_ADDR_V4)
			{
				ASSERT_TRUE(acl.AddEntry(ip.addr.ipv4, GetMaskValue_IPv4(mask_bits))) << "Can't add IPv4 entry: " << v << endl;
			}
			else
			{
				uint32_t mask[4];

				GetMaskValue_IPv6(mask, mask_bits);
				ASSERT_TRUE(acl.AddEntry(ip.addr.ipv6, mask)) << "Can\'t add IPv6 entry: " << v << endl;
			}
		}

		acl.Finalize();

		for (auto &v : TestData_str)
		{
			boost::system::error_code ec;
			auto addr = net::address::from_string(v.entry, ec);
			EXPECT_FALSE(ec);
			ASSERT_FALSE(acl.IsAllowed(addr) == v.res) << "Bad result for " << v.entry << endl;
		}
	}
}
