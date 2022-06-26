#include "std/cpplib/VS_Utils.h"

#include <gtest/gtest.h>

static const struct ValidateHTMLTestParam
{
	const char* input;
	const char* expected_result;
} params[] = {
	// tests from https://projects.trueconf.com/bin/view/Projects/ChatV2#тесты
	{ R"(<a href="aaa"  onclick="dobadthings">text </a >)", "text </a>"},
	{ R"(<a  href="aaa"  > text</a >)", R"(<a  href="aaa"> text</a>)" },
	{ "<b>text</B>", "<b>text</B>" },
	{ "<b aaa>text</b  bbb>", "text" },
	{ "text<something", "text" },
	{ "<b>text</b>", "<b>text</b>"},
	{ "</br>", "</br>" },
};
std::ostream& operator<<(std::ostream& os, const ValidateHTMLTestParam& x)
{
	return os << "{ input=\"" << x.input << "\", expected_result=\"" << x.expected_result << "\" }";
}
class ValidateHTMLTest : public ::testing::TestWithParam<ValidateHTMLTestParam> {};
INSTANTIATE_TEST_CASE_P(ValidateHTML, ValidateHTMLTest, ::testing::ValuesIn(params));

TEST_P(ValidateHTMLTest, AllowValidTags)
{
	EXPECT_EQ(VS_ValidateHtml(GetParam().input), GetParam().expected_result);
}