#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "std-generic/compat/iterator.h"
#include "std-generic/compat/memory.h"
#include "tests/common/GTestMatchers.h"
#include "mdnslib/RRecord.h"
#include "Packets.h"

TEST(RRecord_Test, HugePackageTXT)
{
	const char instanceName[] = "\033485D607CEE22@5KPlayer-ek-pc\005_raop\004_tcp\005local";
	const char txtData[] = "\015am=AppleTV3,2\004ch=2\006cn=1,3\007da=true\010et=0,3,5\010md=0,1,2"
		"\006sf=0x4\010sr=44100\005ss=16\010sv=false\006tp=UDP\011txtvers=1"
		"\010vn=65537\004vv=2\011vs=220.68\022ft=0x5A7FFFF7,0x1E"
		"\103pk=482131eacf7e006792da125540724283fb3f2ba6a25cabe13b5f1543b3b234bd";

	mdns::RRecord record;
	const char* response0405_TXT_PTR_SRV_ = reinterpret_cast<const char*>(response0405_TXT_PTR_SRV);
	EXPECT_TRUE(record.parse(response0405_TXT_PTR_SRV_, response0405_TXT_PTR_SRV_ + 12,
		vs::size(response0405_TXT_PTR_SRV)));
	EXPECT_THAT(record.name.data(),
		AsArray<char>(record.name.size(), ::testing::ElementsAreArray(instanceName)));
	EXPECT_EQ(record.rtype, mdns::TYPE::TXT);
	EXPECT_EQ(record.cflush, mdns::CACHEFLUSH::YES);
	EXPECT_EQ(record.rclass, mdns::CLASS::INTERNET);
	EXPECT_EQ(record.ttl, 4500);
	EXPECT_EQ(record.rdLength, 211);
	EXPECT_EQ(memcmp(record.rData.data(), txtData, 211), 0);
	EXPECT_EQ(record.size(), 46 + 10 + 211);
}

TEST(RRecord_Test, HugePackagePTR)
{
	const char dnsSdName[] = "\011_services\007_dns-sd\004_udp\005local";

	mdns::RRecord record;
	const char* response0405_TXT_PTR_SRV_ = reinterpret_cast<const char*>(response0405_TXT_PTR_SRV);
	EXPECT_TRUE(record.parse(response0405_TXT_PTR_SRV_, response0405_TXT_PTR_SRV_ + 12 + 267,
		vs::size(response0405_TXT_PTR_SRV)));
	EXPECT_THAT(record.name.data(),
		AsArray<char>(record.name.size(), ::testing::ElementsAreArray(dnsSdName)));
	EXPECT_EQ(record.rtype, mdns::TYPE::PTR);
	EXPECT_EQ(record.cflush, mdns::CACHEFLUSH::NO);
	EXPECT_EQ(record.rclass, mdns::CLASS::INTERNET);
	EXPECT_EQ(record.ttl, 4500);
	EXPECT_EQ(record.rdLength, 2);
	EXPECT_EQ(record.size(), 25 + 10 + 2);
}

TEST(RRecord_Test, HugePackageSRV)
{
	const char instanceName[] = "\033485D607CEE22@5KPlayer-ek-pc\005_raop\004_tcp\005local";

	mdns::RRecord record;
	const char* response0405_TXT_PTR_SRV_ = reinterpret_cast<const char*>(response0405_TXT_PTR_SRV);
	EXPECT_TRUE(record.parse(response0405_TXT_PTR_SRV_, response0405_TXT_PTR_SRV_ + 330,
		vs::size(response0405_TXT_PTR_SRV)));
	EXPECT_THAT(record.name.data(),
		AsArray<char>(record.name.size(), ::testing::ElementsAreArray(instanceName))); // REAL SIZE == 2
	EXPECT_EQ(record.rtype, mdns::TYPE::SRV);
	EXPECT_EQ(record.cflush, mdns::CACHEFLUSH::YES);
	EXPECT_EQ(record.rclass, mdns::CLASS::INTERNET);
	EXPECT_EQ(record.ttl, 120);
	EXPECT_EQ(record.rdLength, 14);
	EXPECT_EQ(memcmp(record.rData.data(), "\0\0\0\0\x1f\364\005ek-pc\005local", 19), 0);//REAL SIZE == 14
	EXPECT_EQ(record.size(), 2 + 10 + 14);
}

TEST(RRecord_Test, DoubleLinkTest)
{
	const char teslenko[] = "\010TESLENKO\005local";

	mdns::RRecord record;
	const char* response0107_DoubleLink_ = reinterpret_cast<const char*>(response0107_DoubleLink);
	EXPECT_TRUE(record.parse(response0107_DoubleLink_, response0107_DoubleLink_ + 277,
		vs::size(response0107_DoubleLink)));
	EXPECT_THAT(record.name.data(),
		AsArray<char>(record.name.size(), ::testing::ElementsAreArray(teslenko)));
	EXPECT_EQ(record.rtype, mdns::TYPE::NSEC);
	EXPECT_EQ(record.cflush, mdns::CACHEFLUSH::YES);
	EXPECT_EQ(record.rclass, mdns::CLASS::INTERNET);
	EXPECT_EQ(record.ttl, 120);
	EXPECT_EQ(record.rdLength, 8);
	EXPECT_EQ(record.size(), 20);
}

TEST(RRecord_Test, FormParsedData)
{
	mdns::RRecord record;
	const char* response0405_TXT_PTR_SRV_ = reinterpret_cast<const char*>(response0405_TXT_PTR_SRV);
	ASSERT_TRUE(record.parse(response0405_TXT_PTR_SRV_, response0405_TXT_PTR_SRV_ + 12,
		vs::size(response0405_TXT_PTR_SRV)));
	size_t size = record.size();
	auto buffer = vs::make_unique<char[]>(size);
	ASSERT_TRUE(record.form(buffer.get(), size));
	EXPECT_EQ(memcmp(buffer.get(), response0405_TXT_PTR_SRV + 12, size), 0);
}

TEST(RRecord_Test, FormUserData)
{
	std::vector<char> name;
	std::vector<char> rData;
	name.insert(name.end(), userRecordData, userRecordData + 22);
	rData.insert(rData.end(), userRecordData + 32, userRecordData + 41);
	mdns::RRecord record(name, mdns::TYPE::PTR, 0xffff, rData);
	auto buffer = vs::make_unique<char[]>(41);
	ASSERT_TRUE(record.form(buffer.get(), 41));
//	EXPECT_EQ(memcmp(buffer, userRecordData, 41), 0); TODO: Make this supposed-to-be-correct test work!
}

// The only way to pass this test is to not run out of memory
TEST(RRecord_Test, CursedPacket)
{
	mdns::RRecord record;
	const char* cursedPacket_ = reinterpret_cast<const char*>(cursedPacket);
	ASSERT_FALSE(record.parse(cursedPacket_, cursedPacket_ + 254, vs::size(cursedPacket)));
}
