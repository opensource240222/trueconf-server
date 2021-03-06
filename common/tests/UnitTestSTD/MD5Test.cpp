#include "std/cpplib/md5.h"
#include "std-generic/cpplib/string_view.h"
#include "MD5TestData.h"

#include <gtest/gtest.h>

#include <array>

namespace md5_test {

static const struct DigestTestParam
{
	string_view data;
	std::array<unsigned char, 16> digest;
} digest_params[] = {
	// Test cases from RFC1321
	{ "", { 0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04, 0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e, } },
	{ "a", { 0x0c, 0xc1, 0x75, 0xb9, 0xc0, 0xf1, 0xb6, 0xa8, 0x31, 0xc3, 0x99, 0xe2, 0x69, 0x77, 0x26, 0x61, } },
	{ "abc", { 0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0, 0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72, } },
	{ "message digest", { 0xf9, 0x6b, 0x69, 0x7d, 0x7c, 0xb7, 0x93, 0x8d, 0x52, 0x5a, 0x2f, 0x31, 0xaa, 0xf1, 0x61, 0xd0, } },
	{ "abcdefghijklmnopqrstuvwxyz", { 0xc3, 0xfc, 0xd3, 0xd7, 0x61, 0x92, 0xe4, 0x00, 0x7d, 0xfb, 0x49, 0x6c, 0xca, 0x67, 0xe1, 0x3b, } },
	{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", { 0xd1, 0x74, 0xab, 0x98, 0xd2, 0x77, 0xd9, 0xf5, 0xa5, 0x61, 0x1c, 0x2c, 0x9f, 0x41, 0x9d, 0x9f, } },
	{ "12345678901234567890123456789012345678901234567890123456789012345678901234567890", { 0x57, 0xed, 0xf4, 0xa2, 0x2b, 0xe3, 0xc9, 0x55, 0xac, 0x49, 0xda, 0x2e, 0x21, 0x07, 0xb6, 0x7a, } },
	// Exacly one block (64 bytes)
	{ "012345678901234567890123456789++++012345678901234567890123456789", { 0x4a, 0x96, 0xad, 0x8d, 0xae, 0xd2, 0x88, 0xeb, 0x55, 0x2d, 0x03, 0x60, 0xe3, 0x51, 0xa5, 0x1d, } },
	// One byte smaller that one block
	{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz=", { 0x8a, 0x79, 0x97, 0x14, 0xe1, 0xa7, 0x96, 0xfa, 0x0e, 0x2d, 0xe0, 0x0c, 0x43, 0x1a, 0x3f, 0x11, } },
	// One byte larger that one block
	{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz+-*", { 0x54, 0xb8, 0xa2, 0xcd, 0xd5, 0x82, 0x42, 0xfb, 0xf9, 0x07, 0xf7, 0x39, 0x50, 0x95, 0x3b, 0xa2, } },
	{ lorem_ipsum, { 0x75, 0x69, 0xd0, 0xde, 0x2c, 0xf9, 0x13, 0xd6, 0x23, 0xad, 0xc3, 0x79, 0xb9, 0xd4, 0x2a, 0x32, } },
};
std::ostream& operator<<(std::ostream& os, const DigestTestParam& x)
{
	return os << '"' << x.data << '"';
}
class MD5DigestTest : public ::testing::TestWithParam<DigestTestParam> {};
INSTANTIATE_TEST_CASE_P(MD5, MD5DigestTest, ::testing::ValuesIn(digest_params));

TEST_P(MD5DigestTest, OneBlock)
{
	MD5 md5;
	md5.Update(GetParam().data.data(), GetParam().data.size());
	md5.Final();
	std::array<unsigned char, 16> result;
	md5.GetBytes(result.data());
	EXPECT_EQ(result, GetParam().digest);
}

TEST_P(MD5DigestTest, TwoBlocks)
{
	MD5 md5;
	const auto mid_point = GetParam().data.size() / 2;
	md5.Update(GetParam().data.data(), mid_point);
	md5.Update(GetParam().data.data() + mid_point, GetParam().data.size() - mid_point);
	md5.Final();
	std::array<unsigned char, 16> result;
	md5.GetBytes(result.data());
	EXPECT_EQ(result, GetParam().digest);
}

TEST_P(MD5DigestTest, OneByteBlocks)
{
	MD5 md5;
	for (auto c : GetParam().data)
		md5.Update(&c, 1);
	md5.Final();
	std::array<unsigned char, 16> result;
	md5.GetBytes(result.data());
	EXPECT_EQ(result, GetParam().digest);
}

}

