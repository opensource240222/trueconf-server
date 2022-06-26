#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "std-generic/compat/iterator.h"
#include "tests/common/GTestMatchers.h"
#include "mdnslib/Header.cpp"
#include "Packets.h"

TEST(Header_Test, responsePtr1)
{
	mdns::Header header;

	EXPECT_TRUE(header.parse(reinterpret_cast<const char*>(response0600_Ptr_1),
		vs::size(response0600_Ptr_1)));
	EXPECT_EQ(header.id, 0);
	EXPECT_EQ(header.qr, mdns::QR::RESPONSE);
	EXPECT_EQ(header.opcode, mdns::OPCODE::QUERY);
	EXPECT_EQ(header.aa, mdns::AA::YES);
	EXPECT_EQ(header.tc, mdns::TC::NO);
	EXPECT_EQ(header.rd, mdns::RD::NO);
	EXPECT_EQ(header.ra, mdns::RA::NO);
	EXPECT_EQ(header.zero, 0);
	EXPECT_EQ(header.rcode, mdns::RCODE::ERROR_NONE);
	EXPECT_EQ(header.qd, 0);
	EXPECT_EQ(header.an, 6);
	EXPECT_EQ(header.ns, 0);
	EXPECT_EQ(header.ar, 0);
}

TEST(Header_Test, responsePtr2)
{
	mdns::Header header;
	EXPECT_TRUE(header.parse(reinterpret_cast<const char*>(response1210_AAAA_Cname_SOA_1),
		vs::size(response1210_AAAA_Cname_SOA_1)));
	EXPECT_EQ(header.id, 0x4d6e);
	EXPECT_EQ(header.qr, mdns::QR::RESPONSE);
	EXPECT_EQ(header.opcode, mdns::OPCODE::QUERY);
	EXPECT_EQ(header.aa, mdns::AA::NO);
	EXPECT_EQ(header.tc, mdns::TC::NO);
	EXPECT_EQ(header.rd, mdns::RD::YES);
	EXPECT_EQ(header.ra, mdns::RA::YES);
	EXPECT_EQ(header.zero, 0);
	EXPECT_EQ(header.rcode, mdns::RCODE::ERROR_NONE);
	EXPECT_EQ(header.qd, 1);
	EXPECT_EQ(header.an, 2);
	EXPECT_EQ(header.ns, 1);
	EXPECT_EQ(header.ar, 0);
}


TEST(Header_Test, FakeHeader1)
{
	mdns::Header header;
	EXPECT_TRUE(header.parse(reinterpret_cast<const char*>(fakeHeaderOnly), vs::size(fakeHeaderOnly)));
	EXPECT_EQ(header.id, 1);
	EXPECT_EQ(header.qr, mdns::QR::RESPONSE);
	EXPECT_EQ(header.opcode, mdns::OPCODE::UPDATE);
	EXPECT_EQ(header.aa, mdns::AA::YES);
	EXPECT_EQ(header.tc, mdns::TC::YES);
	EXPECT_EQ(header.rd, mdns::RD::YES);
	EXPECT_EQ(header.ra, mdns::RA::YES);
	EXPECT_EQ(header.zero, 0);
	EXPECT_EQ(header.rcode, mdns::RCODE::YX_RR_SET);
	EXPECT_EQ(header.qd, 0xf00f);
	EXPECT_EQ(header.an, 0xf00f);
	EXPECT_EQ(header.ns, 0xf00f);
	EXPECT_EQ(header.ar, 0xf00f);
}

TEST(Header_Test, StandardAvahiQuery)
{
	mdns::Header header;
	EXPECT_TRUE(header.parse(reinterpret_cast<const char*>(standardAvahiQuery),
		vs::size(standardAvahiQuery)));
	EXPECT_EQ(header.id, 0);
	EXPECT_EQ(header.qr, mdns::QR::QUERY);
	EXPECT_EQ(header.opcode, mdns::OPCODE::QUERY);
	EXPECT_EQ(header.aa, mdns::AA::NO);
	EXPECT_EQ(header.tc, mdns::TC::NO);
	EXPECT_EQ(header.rd, mdns::RD::NO);
	EXPECT_EQ(header.ra, mdns::RA::NO);
	EXPECT_EQ(header.zero, 0);
	EXPECT_EQ(header.rcode, mdns::RCODE::ERROR_NONE);
	EXPECT_EQ(header.qd, 2);
	EXPECT_EQ(header.an, 2);
	EXPECT_EQ(header.ns, 0);
	EXPECT_EQ(header.ar, 0);
}

TEST(Header_Test, FormParsedData)
{
	mdns::Header header;
	ASSERT_TRUE(header.parse(reinterpret_cast<const char*>(response0600_Ptr_1),
		vs::size(response0600_Ptr_1)));
	char buffer[mdns::Header::HEADER_SIZE];
	ASSERT_TRUE(header.form(buffer, mdns::Header::HEADER_SIZE));
	EXPECT_THAT(response0600_Ptr_1,
		AsArray<char>(mdns::Header::HEADER_SIZE, ::testing::ElementsAreArray(buffer)));

	header.clear();
	ASSERT_TRUE(header.parse(reinterpret_cast<const char*>(response1210_AAAA_Cname_SOA_1),
		vs::size(response1210_AAAA_Cname_SOA_1)));
	ASSERT_TRUE(header.form(buffer, mdns::Header::HEADER_SIZE));
	EXPECT_THAT(response1210_AAAA_Cname_SOA_1,
		AsArray<char>(mdns::Header::HEADER_SIZE, ::testing::ElementsAreArray(buffer)));

	header.clear();
	ASSERT_TRUE(header.parse(reinterpret_cast<const char*>(fakeHeaderOnly),
		vs::size(fakeHeaderOnly)));
	ASSERT_TRUE(header.form(buffer, mdns::Header::HEADER_SIZE));
	EXPECT_THAT(fakeHeaderOnly,
		AsArray<char>(mdns::Header::HEADER_SIZE, ::testing::ElementsAreArray(buffer)));

	header.clear();
	ASSERT_TRUE(header.parse(reinterpret_cast<const char*>(standardAvahiQuery),
		vs::size(standardAvahiQuery)));
	ASSERT_TRUE(header.form(buffer, mdns::Header::HEADER_SIZE));
	EXPECT_THAT(standardAvahiQuery,
		AsArray<char>(mdns::Header::HEADER_SIZE, ::testing::ElementsAreArray(buffer)));
}

TEST(Header_Test, FormUserData)
{
	mdns::Header header(1, 1, 0, 0);
	char buffer[mdns::Header::HEADER_SIZE];
	ASSERT_TRUE(header.form(buffer, mdns::Header::HEADER_SIZE));
	EXPECT_THAT(userDefinedHeader,
		AsArray<char>(mdns::Header::HEADER_SIZE, ::testing::ElementsAreArray(buffer)));
}
