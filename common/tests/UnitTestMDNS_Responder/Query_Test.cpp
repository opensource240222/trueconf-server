#include <gtest/gtest.h>
#include "std-generic/compat/iterator.h"
#include "tests/common/GTestMatchers.h"
#include "mdnslib/Query.h"
#include <cstring>
#include "Packets.h"


TEST(Query_Test, AvahiQuery)
{
	const char dnsSdQueryName[] = "\011_services\007_dns-sd\004_udp\005local";
	const char serviceQueryName[] = "\014_workstation\004_tcp\005local";

	mdns::Query query;
	const char* standardAvahiQuery_ = reinterpret_cast<const char*>(standardAvahiQuery);
	EXPECT_TRUE(query.parse(standardAvahiQuery_, standardAvahiQuery_ + 12,
		vs::size(standardAvahiQuery)));
	EXPECT_THAT(query.name.data(),
		AsArray<char>(query.name.size(), ::testing::ElementsAreArray(dnsSdQueryName)));
	EXPECT_EQ(query.qtype, mdns::TYPE::PTR);
	EXPECT_EQ(query.qu, mdns::QU::NO);
	EXPECT_EQ(query.qclass, mdns::CLASS::INTERNET);
	uint32_t size = query.size();
	EXPECT_EQ(size, 34);

	query.clear();
	EXPECT_TRUE(query.parse(standardAvahiQuery_, standardAvahiQuery_ + 12 + 34,
		vs::size(standardAvahiQuery)));
	EXPECT_THAT(query.name.data(),
		AsArray<char>(query.name.size(), ::testing::ElementsAreArray(serviceQueryName)));
	EXPECT_EQ(query.qtype, mdns::TYPE::PTR);
	EXPECT_EQ(query.qu, mdns::QU::NO);
	EXPECT_EQ(query.qclass, mdns::CLASS::INTERNET);
	size = query.size();
	EXPECT_EQ(size, 24);
}

TEST(Query_Test, Asterisk)
{
	const char instanceName[] = "\033485D607CEE22@5KPlayer-ek-pc\005_raop\004_tcp\005local";
	mdns::Query query;
	const char* bigNameAsteriskQuery_ = reinterpret_cast<const char*>(bigNameAsteriskQuery);
	EXPECT_TRUE(query.parse(bigNameAsteriskQuery_, bigNameAsteriskQuery_ + 12,
		vs::size(bigNameAsteriskQuery)));
	EXPECT_THAT(query.name.data(),
		AsArray<char>(query.name.size(), ::testing::ElementsAreArray(instanceName)));
	EXPECT_EQ(query.qtype, mdns::TYPE::ANY);
	EXPECT_EQ(query.qu, mdns::QU::NO);
	EXPECT_EQ(query.qclass, mdns::CLASS::INTERNET);
	uint32_t size = query.size();
	EXPECT_EQ(size, 50);
}

TEST(Query_Test, ThreeLinkedQuestionsQuery)
{
	const char serviceName[] = "\015_apple-mobdev\004_tcp\005local";
	const char instanceName[] = "\010961c080c\004_sub\016_apple-mobdev2\004_tcp\005local";
	const char serviceNameTwo[] = "\017_apple-pairable\004_tcp\005local";
	mdns::Query query;
	const char* query3000LinksPtrUnicast_ = reinterpret_cast<const char*>(query3000LinksPtrUnicast);
	EXPECT_TRUE(query.parse(query3000LinksPtrUnicast_, query3000LinksPtrUnicast_ + 12,
		vs::size(query3000LinksPtrUnicast)));
	EXPECT_THAT(query.name.data(),
		AsArray<char>(query.name.size(), ::testing::ElementsAreArray(serviceName)));
	EXPECT_EQ(query.qtype, mdns::TYPE::PTR);
	EXPECT_EQ(query.qu, mdns::QU::YES);
	EXPECT_EQ(query.qclass, mdns::CLASS::INTERNET);
	uint32_t size = query.size();
	EXPECT_EQ(size, 30);

	query.clear();
	EXPECT_TRUE(query.parse(query3000LinksPtrUnicast_, query3000LinksPtrUnicast_ + 12 + 30,
		vs::size(query3000LinksPtrUnicast)));
	EXPECT_THAT(query.name.data(),
		AsArray<char>(query.name.size(), ::testing::ElementsAreArray(instanceName)));
	EXPECT_EQ(query.qtype, mdns::TYPE::PTR);
	EXPECT_EQ(query.qu, mdns::QU::YES);
	EXPECT_EQ(query.qclass, mdns::CLASS::INTERNET);
	size = query.size();
	EXPECT_EQ(size, 35);

	query.clear();
	EXPECT_TRUE(query.parse(query3000LinksPtrUnicast_, query3000LinksPtrUnicast_ + 12 + 30 + 35,
		vs::size(query3000LinksPtrUnicast)));
	EXPECT_THAT(query.name.data(),
		AsArray<char>(query.name.size(), ::testing::ElementsAreArray(serviceNameTwo)));
	EXPECT_EQ(query.qtype, mdns::TYPE::PTR);
	EXPECT_EQ(query.qu, mdns::QU::YES);
	EXPECT_EQ(query.qclass, mdns::CLASS::INTERNET);
	size = query.size();
	EXPECT_EQ(size, 22);
}

TEST(Query_Test, FormParcedData)
{
	mdns::Query query;
	const char* standardAvahiQuery_ = reinterpret_cast<const char*>(standardAvahiQuery);
	ASSERT_TRUE(query.parse(standardAvahiQuery_, standardAvahiQuery_ + 12,
		vs::size(standardAvahiQuery)));
//	Technically you don't expect for size_ to be intact so it gets saved
	size_t size = query.size();
	char* buffer = new char[size];
	ASSERT_TRUE(query.form(buffer, size));
	EXPECT_EQ(memcmp(buffer, standardAvahiQuery + 12, size), 0);
	query.clear();

	const char* bigNameAsteriskQuery_ = reinterpret_cast<const char*>(bigNameAsteriskQuery);
	ASSERT_TRUE(query.parse(bigNameAsteriskQuery_, bigNameAsteriskQuery_ + 12,
		vs::size(bigNameAsteriskQuery)));
	size = query.size();
	delete[] buffer;
	buffer = new char[size];
	ASSERT_TRUE(query.form(buffer, size));
	EXPECT_EQ(memcmp(buffer, bigNameAsteriskQuery_ + 12, size), 0);
	query.clear();

	const char* query3000LinksPtrUnicast_ = reinterpret_cast<const char*>(query3000LinksPtrUnicast);
	ASSERT_TRUE(query.parse(query3000LinksPtrUnicast_, query3000LinksPtrUnicast_ + 12,
		vs::size(query3000LinksPtrUnicast)));
	size = query.size();
	delete[] buffer;
	buffer = new char[size];
	ASSERT_TRUE(query.form(buffer, size));
	EXPECT_EQ(memcmp(buffer, query3000LinksPtrUnicast_ + 12, size), 0);
	delete[] buffer;
}

TEST(Query_Test, FormUserData)
{
	std::vector<char> name;
	name.insert(name.end(), userQueryData, userQueryData + 30);
	mdns::Query query(name, mdns::TYPE::SRV, mdns::QU::YES);
	char buffer[34];
	ASSERT_TRUE(query.form(buffer, 34));
	EXPECT_THAT(userQueryData,
		AsArray<char>(34, ::testing::ElementsAreArray(userQueryData)));
}
