#include "std-generic/clib/sha1.h"
#include "std-generic/cpplib/string_view.h"

#include <gtest/gtest.h>

#include <array>

namespace sha1_test {

static const struct DigestTestParam
{
	string_view data;
	std::array<unsigned char, 20> digest;
} digest_params[] = {
	{ "", { 0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09, } },
	{ "A", { 0x6d, 0xcd, 0x4c, 0xe2, 0x3d, 0x88, 0xe2, 0xee, 0x95, 0x68, 0xba, 0x54, 0x6c, 0x00, 0x7c, 0x63, 0xd9, 0x13, 0x1c, 0x1b, } },
	// Exacly one block (64 bytes)
	{ "012345678901234567890123456789++++012345678901234567890123456789", { 0x7d, 0x7c, 0x89, 0xa6, 0xc1, 0x44, 0x6c, 0x52, 0x0a, 0x88, 0x68, 0x44, 0x85, 0x3d, 0xa3, 0x69, 0x98, 0x57, 0xe3, 0x5d, } },
	// One byte smaller that one block
	{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz=", { 0xa9, 0x67, 0x21, 0xd4, 0x7b, 0x8e, 0x23, 0x0c, 0x96, 0xe1, 0x15, 0xa4, 0x7c, 0xa6, 0x96, 0xce, 0x63, 0x62, 0x05, 0x16, } },
	// One byte larger that one block
	{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz+-*", { 0x35, 0xf2, 0x3c, 0x47, 0x8b, 0xd8, 0x98, 0xd1, 0xbf, 0x92, 0xae, 0x2f, 0x79, 0xa8, 0x45, 0x0e, 0x75, 0x8b, 0xb1, 0xe1, } },
	// Test case from FIPS PUB 180-1 Appendix A
	{ "abc", { 0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a, 0xba, 0x3e, 0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c, 0x9c, 0xd0, 0xd8, 0x9d, } },
	// Test case from FIPS PUB 180-1 Appendix B
	{ "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", { 0x84, 0x98, 0x3e, 0x44, 0x1c, 0x3b, 0xd2, 0x6e, 0xba, 0xae, 0x4a, 0xa1, 0xf9, 0x51, 0x29, 0xe5, 0xe5, 0x46, 0x70, 0xf1, } },
};
std::ostream& operator<<(std::ostream& os, const DigestTestParam& x)
{
	return os << '"' << x.data << '"';
}
class SHA1DigestTest : public ::testing::TestWithParam<DigestTestParam> {};
INSTANTIATE_TEST_CASE_P(SHA1, SHA1DigestTest, ::testing::ValuesIn(digest_params));

TEST_P(SHA1DigestTest, OneBlock)
{
	SHA1 sha;
	sha.Update(GetParam().data.data(), GetParam().data.size());
	sha.Final();
	std::array<unsigned char, 20> result;
	sha.GetBytes(result.data());
	EXPECT_EQ(result, GetParam().digest);
}

TEST_P(SHA1DigestTest, TwoBlocks)
{
	SHA1 sha;
	const auto mid_point = GetParam().data.size() / 2;
	sha.Update(GetParam().data.data(), mid_point);
	sha.Update(GetParam().data.data() + mid_point, GetParam().data.size() - mid_point);
	sha.Final();
	std::array<unsigned char, 20> result;
	sha.GetBytes(result.data());
	EXPECT_EQ(result, GetParam().digest);
}

TEST_P(SHA1DigestTest, OneByteBlocks)
{
	SHA1 sha;
	for (auto c : GetParam().data)
		sha.Update(&c, 1);
	sha.Final();
	std::array<unsigned char, 20> result;
	sha.GetBytes(result.data());
	EXPECT_EQ(result, GetParam().digest);
}

TEST(SHA1, FIPS_PUB_180_1_AppendixC)
{
	// Test case from FIPS PUB 180-1 Appendix C: 1000000 'a' characters.
	const unsigned N = 1000000;
	char data[127 /*co-prime with the block size to test all possible offsets*/];
	memset(data, 'a', sizeof(data));
	const std::array<unsigned char, 20> expected_result { 0x34, 0xaa, 0x97, 0x3c, 0xd4, 0xc4, 0xda, 0xa4, 0xf6, 0x1e, 0xeb, 0x2b, 0xdb, 0xad, 0x27, 0x31, 0x65, 0x34, 0x01, 0x6f, };

	SHA1 sha;
	unsigned n = 0;
	for ( ; n <= N - sizeof(data); n += sizeof(data))
		sha.Update(data, sizeof(data));
	sha.Update(data, N - n);
	sha.Final();

	std::array<unsigned char, 20> result;
	sha.GetBytes(result.data());
	EXPECT_EQ(result, expected_result);
}

}
