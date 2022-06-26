#include <gtest/gtest.h>
#include <iostream>
#include "std/cpplib/netutils.h"

namespace netutils_test
{
	using namespace netutils;
	using namespace std;

	class Netutils_Tests : public testing::Test {
	protected:
		virtual void SetUp() {}

		virtual void TearDown() {}
	private:
	};

	TEST_F(Netutils_Tests, StringToIP)
	{
		IPAddress ip;

		// IPv4
		ASSERT_TRUE(StringToIPAddress("192.168.1.2", ip)) << "Can't convert address to string" << endl;
		ASSERT_EQ(ip.type, IP_ADDR_V4);
		ASSERT_EQ(ip.addr.ipv4, MakeAddress_IPv4(192, 168, 1, 2)) << "Non expected address." << endl;

		// IPv6
		{
			uint16_t a[] = { 0x8329, 0xFE1E, 0xB3FF, 0x0202, 0x0000, 0x0000, 0x0000, 0xFE80 };
			ASSERT_TRUE(StringToIPAddress("FE80:0000:0000:0000:0202:B3FF:FE1E:8329", ip)) << "Can't convert address to string" << endl;
			ASSERT_EQ(ip.type, IP_ADDR_V6);

			ASSERT_EQ(memcmp(a, ip.addr.ipv6, sizeof(a)), 0);
		}

		ASSERT_FALSE(StringToIPAddress("gibberishv4.1.1", ip));
		ASSERT_FALSE(StringToIPAddress("FE:gibberishv6:2000", ip));
	}

	TEST_F(Netutils_Tests, GetMaskV4)
	{
		// 32 bit mask
		ASSERT_TRUE(GetMaskValue_IPv4(32) == ~0);

		// 16 bit mask
		ASSERT_TRUE(GetMaskValue_IPv4(16) == 0xFFFF0000LU);

		// 17 bit mask
		ASSERT_TRUE(GetMaskValue_IPv4(17) == 0xFFFF8000LU);

		// 1 bit mask
		ASSERT_TRUE(GetMaskValue_IPv4(1) == 0x80000000LU);

		// 0 bit mask
		ASSERT_TRUE(GetMaskValue_IPv4(0) == 0x0LU);

		// too big mask
		ASSERT_TRUE(GetMaskValue_IPv4(33) == ~0);
	}

	TEST_F(Netutils_Tests, GetMaskV6)
	{
		uint32_t res[4];

		// 128 bit mask
		{
			uint32_t m[] = { 0xFFFFFFFFLU, 0xFFFFFFFFLU, 0xFFFFFFFFLU, 0xFFFFFFFFLU };
			GetMaskValue_IPv6(res, 128);

			ASSERT_EQ(memcmp(m, res, sizeof(res)), 0);
		}

		// 49 bit mask
		{
			uint32_t m[] = { 0x00000000LU, 0x00000000LU, 0xFFFF0100LU, 0xFFFFFFFFLU };
			GetMaskValue_IPv6(res, 49);

			ASSERT_EQ(memcmp(m, res, sizeof(res)), 0);
		}

		// 64 bit mask
		{
			uint32_t m[] = { 0, 0, 0xFFFFFFFFLU, 0xFFFFFFFFLU };
			GetMaskValue_IPv6(res, 64);

			ASSERT_EQ(memcmp(m, res, sizeof(res)), 0);
		}

		// 1 bit mask
		{
			uint32_t m[] = { 0, 0, 0, 0x1000000 };
			GetMaskValue_IPv6(res, 1);

			ASSERT_EQ(memcmp(m, res, sizeof(res)), 0);
		}
		// 0 bit mask
		{
			uint32_t m[] = { 0, 0, 0, 0 };
			GetMaskValue_IPv6(res, 0);

			ASSERT_EQ(memcmp(m, res, sizeof(res)), 0);
		}

		// too big mask
		{
			uint32_t m[] = { 0xFFFFFFFFLU, 0xFFFFFFFFLU, 0xFFFFFFFFLU, 0xFFFFFFFFLU };
			GetMaskValue_IPv6(res, 256);

			ASSERT_EQ(memcmp(m, res, sizeof(res)), 0);
		}
	}

	TEST_F(Netutils_Tests, IsPrivateAddressV4)
	{
		// 16 bit private block
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(192, 168, 0, 1)));
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(192, 168, 128, 128)));
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(192, 168, 255, 254)));

		// 16 bit zeroconf private block
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(169, 254, 0, 1)));
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(169, 254, 128, 128)));
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(169, 254, 255, 254)));

		ASSERT_FALSE(IsPrivateAddress_IPv4(MakeAddress_IPv4(169, 254, 255, 254), true));

		// 20 bit private block
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(172, 16, 0, 1)));
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(172, 24, 128, 128)));
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(172, 31, 255, 254)));

		// 24 bit private block
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(10, 0, 0, 1)));
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(10, 128, 128, 128)));
		ASSERT_TRUE(IsPrivateAddress_IPv4(MakeAddress_IPv4(10, 255, 255, 254)));

		// global addresses
		ASSERT_FALSE(IsPrivateAddress_IPv4(MakeAddress_IPv4(172, 32, 255, 254)));
		ASSERT_FALSE(IsPrivateAddress_IPv4(MakeAddress_IPv4(200, 43, 1, 112)));
		ASSERT_FALSE(IsPrivateAddress_IPv4(MakeAddress_IPv4(173, 194, 122, 248)));

		// string tests
		ASSERT_FALSE(IsPrivateAddress_IPv4("200.43.1.112"));
		ASSERT_TRUE(IsPrivateAddress_IPv4("192.168.1.1"));
	}

	TEST_F(Netutils_Tests, IsAddressInRangeV4)
	{
		ASSERT_TRUE(IsAddressInRange_IPv4(MakeAddress_IPv4(192, 168, 1, 1), MakeAddress_IPv4(192, 168, 0, 0), GetMaskValue_IPv4(16)));
		ASSERT_FALSE(IsAddressInRange_IPv4(MakeAddress_IPv4(192, 169, 1, 1), MakeAddress_IPv4(192, 168, 0, 0), GetMaskValue_IPv4(16)));

		// alignment tests
		ASSERT_TRUE(IsAddressInRange_IPv4(MakeAddress_IPv4(169, 254, 1, 1),
			MakeAddress_IPv4(169, 254, 1, 1), // unaligned subnet address
			GetMaskValue_IPv4(16)));

		ASSERT_FALSE(IsAddressInRange_IPv4(MakeAddress_IPv4(169, 254, 1, 1),
			MakeAddress_IPv4(169, 254, 1, 1), // unaligned subnet address
			GetMaskValue_IPv4(16), false));
	}

	TEST_F(Netutils_Tests, IsAddressInRangeV6)
	{
		IPAddress ip;
		IPAddress subnet;
		uint32_t mask[4];

		// subnetting
		{
			IPAddress ip2;
			StringToIPAddress("2001:0db8:85a3::8a2e:0370:7334", ip);
			StringToIPAddress("2001:0db8:85a3::0", subnet);
			GetMaskValue_IPv6(mask, 48);

			ASSERT_TRUE(IsAddressInRange_IPv6(ip.addr.ipv6, subnet.addr.ipv6, mask));

			StringToIPAddress("E80:0000:0000:0000:0202:B3FF:FE1E:8329", ip2);
			ASSERT_FALSE(IsAddressInRange_IPv6(ip2.addr.ipv6, subnet.addr.ipv6, mask));
		}

		// alignment tests
		{
			StringToIPAddress("2001:0db8:85a3::8a2e:1000:8000", ip);
			StringToIPAddress("2001:0db8:85a3::8a2e:1:1", subnet);
			GetMaskValue_IPv6(mask, 48);

			// aligned test
			ASSERT_TRUE(IsAddressInRange_IPv6(ip.addr.ipv6, subnet.addr.ipv6, mask)); // align mask before check
			ASSERT_FALSE(IsAddressInRange_IPv6(ip.addr.ipv6, subnet.addr.ipv6, mask, false)); // do not align mask before check
		}
	}

	TEST_F(Netutils_Tests, DomainNameValidator)
	{
		// valid domain names
		ASSERT_TRUE(IsValidDomainName("localhost"));
		ASSERT_TRUE(IsValidDomainName("a.gmail.google.com"));
		ASSERT_TRUE(IsValidDomainName("a.gmail.google.com"));
		ASSERT_TRUE(IsValidDomainName("a.gmail.google.com."));
		ASSERT_TRUE(IsValidDomainName("a.gmail.google.com"));
		ASSERT_TRUE(IsValidDomainName("a0.gmail.goo-gle.com"));
		ASSERT_TRUE(IsValidDomainName("a0.gmail.goo-gle.com."));

		// invalid domain names
		// regular expression check would give wrong results on this...
		ASSERT_FALSE(IsValidDomainName(""));
		ASSERT_FALSE(IsValidDomainName(".localhost."));
		ASSERT_FALSE(IsValidDomainName("localhost.."));
		ASSERT_FALSE(IsValidDomainName("a.gmail.google..com"));
		ASSERT_FALSE(IsValidDomainName(".a.gmail.google.com"));
		ASSERT_FALSE(IsValidDomainName("-a.gmail.google.com"));
		ASSERT_FALSE(IsValidDomainName("0a.gmail.google.com"));

		ASSERT_FALSE(IsValidDomainName("user.trueconf.name#vcs"));

		// additional checks
		ASSERT_FALSE(IsValidDomainName("localhost "));
		ASSERT_TRUE(IsValidDomainName("localhost ", false, true));
		ASSERT_FALSE(IsValidDomainName("aa.domain_name.zz", false));
		ASSERT_TRUE(IsValidDomainName("aa.domain_name.zz", true));
		ASSERT_TRUE(IsValidDomainName("aa.domain_name.zz ", true, true));
		// punycode (видеозвонок.рф)
		ASSERT_TRUE(IsValidDomainName("xn--b1aafekdsxgcb.xn--p1ai"));
	}
}
