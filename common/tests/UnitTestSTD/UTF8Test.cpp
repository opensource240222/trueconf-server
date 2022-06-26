#include "std-generic/cpplib/utf8.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace utf8_test {

TEST(ASCII, IsASCII_UTF8)
{
	std::string str;
	str.reserve(128);
	for (int i = 32; i < 128; ++i)
	{
		str.push_back(static_cast<char>(i));
		EXPECT_TRUE(vs::IsASCIIString(str)) << "i = " << i << " '" << static_cast<char>(i) << '\'';
	}

	string_view str_2 = "Non ASCII:\xaa\xbb\xcc";
	EXPECT_TRUE(vs::IsASCIIString(str_2.substr(0, 10)));
	EXPECT_FALSE(vs::IsASCIIString(str_2.substr(0, 11)));
	EXPECT_TRUE(vs::IsASCIIString(str_2.substr(1, 9)));
	EXPECT_FALSE(vs::IsASCIIString(str_2.substr(1, 10)));
}

TEST(ASCII, IsASCII_UTF16)
{
	std::u16string str;
	str.reserve(128);
	for (int i = 32; i < 128; ++i)
	{
		str.push_back(static_cast<char16_t>(i));
		EXPECT_TRUE(vs::IsASCIIString(str)) << "i = " << i << " '" << static_cast<char>(i) << '\'';
	}

#if defined(_MSC_VER) && _MSC_VER < 1900
	u16string_view str_2 = reinterpret_cast<const char16_t*>(L"Non ASCII:строка");
#else
	u16string_view str_2 = u"Non ASCII:строка";
#endif
	EXPECT_TRUE(vs::IsASCIIString(str_2.substr(0, 10)));
	EXPECT_FALSE(vs::IsASCIIString(str_2.substr(0, 11)));
	EXPECT_TRUE(vs::IsASCIIString(str_2.substr(1, 9)));
	EXPECT_FALSE(vs::IsASCIIString(str_2.substr(1, 10)));
}

TEST(ASCII, UTF8_UTF16)
{
	using ::testing::ElementsAreArray;

	const char in[] = "Hello, World!";
#if defined(_MSC_VER) && _MSC_VER < 1900
	const char16_t expected[] = { L'H', L'e', L'l', L'l', L'o', L',', L' ', L'W', L'o', L'r', L'l', L'd', L'!', L'\0' };
#else
	const char16_t expected[] = { u'H', u'e', u'l', u'l', u'o', u',', u' ', u'W', u'o', u'r', u'l', u'd', u'!', u'\0' };
#endif
	auto out = vs::ASCIItoUTF16Convert(in);
	EXPECT_THAT(out, ElementsAreArray(std::begin(expected), std::end(expected) - 1));
}

TEST(ASCII, UTF16_UTF8)
{
	using ::testing::ElementsAreArray;

#if defined(_MSC_VER) && _MSC_VER < 1900
	const char16_t in[] = { L'H', L'e', L'l', L'l', L'o', L',', L' ', L'W', L'o', L'r', L'l', L'd', L'!', L'\0' };
#else
	const char16_t in[] = { u'H', u'e', u'l', u'l', u'o', u',', u' ', u'W', u'o', u'r', u'l', u'd', u'!', u'\0' };
#endif
	const char expected[] = "Hello, World!";
	auto out = vs::UTF16toASCIIConvert(in);
	EXPECT_THAT(out, ElementsAreArray(std::begin(expected), std::end(expected) - 1));
}

#define MAKE_PARAM(s) { u8 ## s, u ## s, U ## s, L ## s }
static const struct ConvertTestParam
{
	const char* u8;
	const char16_t* u16;
	const char32_t* u32;
	const wchar_t* w;
} convert_params[] = {
	MAKE_PARAM(""),
	MAKE_PARAM("ASCII only"),
	MAKE_PARAM("Кириллица"), // Russian (2 bytes per character)
	MAKE_PARAM("한국어"), // Korean (3 bytes per character)
	MAKE_PARAM("Mixed 부호화 текст"),
	{ "A\xCC\x82", u"A\x0302", U"A\x00000302", L"A\u0302" }, // Combining Circumflex Accent
	{ "A\xE1\xAA\xB2", u"A\x1AB2", U"A\x00001AB2", L"A\u1AB2" }, // Combining Infinity
	{ "A\xE1\xB7\x80", u"A\x1DC0", U"A\x00001DC0", L"A\u1DC0" }, // Combining Dotted Grave Accent
	{ "A\xE2\x83\x9E", u"A\x20DE", U"A\x000020DE", L"A\u20DE" }, // Combining Enclosing Square
	{ "A\xEF\xB8\xA2", u"A\xFE22", U"A\x0000FE22", L"A\uFE22" }, // Combining Double Tilde Left Half
	{ "A\xCC\x82\xCC\xB2", u"A\x0302\x0332", U"A\x00000302\x00000332", L"A\u0302\u0332" }, // Combining Circumflex Accent + Combining Low Line
	{ "\xF0\x9F\x98\x8E", u"\xD83D\xDE0E", U"\x0001F60E", L"\U0001F60E" }, // Smiling Face with Sunglasses
	{ "\xF0\x9D\x95\xB8\xF0\x9D\x95\xAC\xF0\x9D\x95\xBF\xF0\x9D\x95\xB3", u"\xD835\xDD78\xD835\xDD6C\xD835\xDD7F\xD835\xDD73", U"\x0001D578\x0001D56C\x0001D57F\x0001D573", L"\U0001D578\U0001D56C\U0001D57F\U0001D573" }, // "MATH" written in Mathematical Bold Fraktur symbols
};
std::ostream& operator<<(std::ostream& os, const ConvertTestParam& x)
{
	return os << '"' << x.u8 << '"';
}
#undef MAKE_PARAM
class ConvertTest : public ::testing::TestWithParam<ConvertTestParam> {};
INSTANTIATE_TEST_CASE_P(Unicode, ConvertTest, ::testing::ValuesIn(convert_params));

TEST_P(ConvertTest, UTF8_UTF16)
{
	std::u16string result;
	EXPECT_NO_THROW(result = vs::UTF8toUTF16Convert(GetParam().u8));
	EXPECT_EQ(GetParam().u16, result);
}

TEST_P(ConvertTest, UTF16_UTF8)
{
	std::string result;
	EXPECT_NO_THROW(result = vs::UTF16toUTF8Convert(GetParam().u16));
	EXPECT_EQ(GetParam().u8, result);
}

TEST_P(ConvertTest, UTF8_Wide)
{
	std::wstring result;
	EXPECT_NO_THROW(result = vs::UTF8ToWideCharConvert(GetParam().u8));
	EXPECT_EQ(GetParam().w, result);
}

TEST_P(ConvertTest, Wide_UTF8)
{
	std::string result;
	EXPECT_NO_THROW(result = vs::WideCharToUTF8Convert(GetParam().w));
	EXPECT_EQ(GetParam().u8, result);
}

TEST_P(ConvertTest, UTF8_UTF32)
{
	std::u32string result;
	EXPECT_NO_THROW(result = vs::UTF8ToUTF32Convert(GetParam().u8));
	EXPECT_EQ(GetParam().u32, result);
}

TEST(Unicode, UTF8_UTF16_NoExceptions)
{
	const char in[] = { 'a', 'b', 'c', char(0x80), 'x', 'y', 'z', 0 };
#if defined(_MSC_VER) && _MSC_VER < 1900
	std::u16string out({ L'_', 0 });
#else
	std::u16string out({ u'_', 0 });
#endif
	EXPECT_NO_THROW(out = vs::UTF8toUTF16Convert(in));
	EXPECT_TRUE(out.empty());
}

TEST(Unicode, UTF16_UTF8_NoExceptions)
{
#if defined(_MSC_VER) && _MSC_VER < 1900
	const char16_t in[] = { L'a', L'b', L'c', char16_t(0xd800), L'x', L'y', L'z', 0 };
#else
	const char16_t in[] = { u'a', u'b', u'c', char16_t(0xd800), u'x', u'y', u'z', 0 };
#endif
	std::string out({ '_', 0 });
	EXPECT_NO_THROW(out = vs::UTF16toUTF8Convert(in));
	EXPECT_TRUE(out.empty());
}

static const struct CaseConvertTestParam
{
	const char* orig;
	const char* lower;
	const char* upper;
} case_convert_params[] = {
	{ "", "", "" },
	{ "Text", "text", "TEXT" },
	{ "\xD0\xA1\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0", "\xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0", "\xD0\xA1\xD0\xA2\xD0\xA0\xD0\x9E\xD0\x9A\xD0\x90" }, // Строка
	{ "\xC3\x9C\x62\x65\x6C\x74\xC3\xA4\x74\x65\x72", "\xC3\xBC\x62\x65\x6C\x74\xC3\xA4\x74\x65\x72", "\xC3\x9C\x42\x45\x4C\x54\xC3\x84\x54\x45\x52" }, // Übeltäter
};
std::ostream& operator<<(std::ostream& os, const CaseConvertTestParam& x)
{
	return os << '"' << x.orig << '"';
}
class CaseConvertTest : public ::testing::TestWithParam<CaseConvertTestParam> {};
INSTANTIATE_TEST_CASE_P(Unicode, CaseConvertTest, ::testing::ValuesIn(case_convert_params));

TEST_P(CaseConvertTest, ToLower)
{
	EXPECT_EQ(vs::UTF8ToLower(GetParam().orig), GetParam().lower);
}

TEST_P(CaseConvertTest, ToUpper)
{
	EXPECT_EQ(vs::UTF8ToUpper(GetParam().orig), GetParam().upper);
}

TEST(Unicode, CountCodePoints_UTF8)
{
	EXPECT_EQ(12u, vs::CountCodePoints("ASCII string"));
	EXPECT_EQ(13u, vs::CountCodePoints("\xD0\xAE\xD0\xBD\xD0\xB8\xD0\xBA\xD0\xBE\xD0\xB4 \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0")); // "Юникод строка"
	EXPECT_EQ(2u, vs::CountCodePoints("A\xCC\x82")); // Combining Circumflex Accent
	EXPECT_EQ(2u, vs::CountCodePoints("A\xE1\xAA\xB2")); // Combining Infinity
	EXPECT_EQ(2u, vs::CountCodePoints("A\xE1\xB7\x80")); // Combining Dotted Grave Accent
	EXPECT_EQ(2u, vs::CountCodePoints("A\xE2\x83\x9E")); // Combining Enclosing Square
	EXPECT_EQ(2u, vs::CountCodePoints("A\xEF\xB8\xA2")); // Combining Double Tilde Left Half
	EXPECT_EQ(3u, vs::CountCodePoints("A\xCC\x82\xCC\xB2")); // Combining Circumflex Accent + Combining Low Line
}

}
