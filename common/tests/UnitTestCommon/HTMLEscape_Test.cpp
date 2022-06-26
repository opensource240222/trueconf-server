#include <std/cpplib/escape_unsafe_html_tags.h>
#include <gtest/gtest.h>

class HtmlEscaperTest :
	public ::testing::Test
{
protected:
	HtmlEscaperTest()
	{}

	virtual void SetUp()
	{}

	virtual void TearDown()
	{}
};

static const char *tiny_page_example =
"<html>\n"
"<header><title>This is title</title></header>\n"
"<body>\n"
"<h1></i><b>Hello!</b></i><h1><br>\n"
"<b>Please visit <a href='https://chaoticlab.io'>my homepage</a>!</b><br>\n"
"</body>\n"
"</html>\n";

static const char *tiny_page_example_quoted =
"&lt;html&gt;\n"
"&lt;header&gt;&lt;title&gt;This is title&lt;/title&gt;&lt;/header&gt;\n"
"&lt;body&gt;\n"
"&lt;h1&gt;&lt;/i&gt;<b>Hello!</b>&lt;/i&gt;&lt;h1&gt;<br>\n"
"<b>Please visit <a href='https://chaoticlab.io'>my homepage</a>!</b><br>\n"
"&lt;/body&gt;\n"
"&lt;/html&gt;\n";


TEST_F(HtmlEscaperTest, Basic)
{
	// NULL
	{
		std::string out;
		EXPECT_FALSE(vs::escape_unsafe_html_tags(nullptr, out));
	}

	// empty message
	{
		std::string out;
		std::string msg("");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == msg);
	}


	// message without any tags
	{
		std::string out;
		std::string msg("Hello, how are you?");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == msg);
	}

	// Messages with some not allowed symbols
	{
		std::string out;
		std::string msg("1 < 2 & 5 < 6");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "1 &lt; 2 &amp; 5 &lt; 6");
	}

	{
		std::string out;
		std::string msg("a < b & c < d");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "a &lt; b &amp; c &lt; d");
	}

	{
		std::string out;
		std::string msg("a <b > c < d < /b>"); // hard: it is almost valid <b></b> tag
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "a &lt;b &gt; c &lt; d &lt; /b&gt;");
	}


	// message with not allowed symbols
	{
		std::string out;
		std::string msg("Escaped symbols:\"'&<>");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "Escaped symbols:&quot;&apos;&amp;&lt;&gt;");
	}

	// message with <u>
	{
		std::string out;
		std::string msg("<u>Hello, how are you?</u>");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == msg);
	}

	// message with <i>
	{
		std::string out;
		std::string msg("<i>Hello, how are you?</i>");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == msg);
	}

	// message with <b>
	{
		std::string out;
		std::string msg("<b>Hello, how are you?</b>");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == msg);
	}

	// message with <br>
	{
		std::string out;
		std::string msg("Hello!<br>How are you?");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == msg);
	}

	// messages with extra formatting
	{
		std::string out;
		std::string msg("Hello!<i>How</i> <b><u>are</u></b> <u>you</u>?");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == msg);
	}

	// message with web-link
	{
		std::string out;
		std::string msg("<b>Hello, how are you?</b>Could you visit <a href=\"https://chaoticlab.io\">my web site</a>?");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == msg);
	}

	// message with web-link which tries to use subtags inside a tag body
	{
		std::string out;
		std::string msg("<b>Hello, how are you?</b>Could you visit <a href=\"https://chaoticlab.io\"><u>my web site</u></a>?");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "<b>Hello, how are you?</b>Could you visit <a href=\"https://chaoticlab.io\">&lt;u&gt;my web site&lt;/u&gt;</a>?");
	}


	// message with tel-link
	{
		std::string out;
		std::string msg("<b>Hello, how are you?</b>Could you call <a href='tel:+38(067)9387708'>me</a>?");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == msg);
	}

	// message with wrong tel-link
	{
		std::string out;
		std::string msg("<b>Hello, how are you?</b>Could you call <a href=\"tel:+38(067)9387708'>me</a>?"); // double quote and single quote mismatch
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "<b>Hello, how are you?</b>Could you call &lt;a href=&quot;tel:+38(067)9387708&apos;&gt;me&lt;/a&gt;?");
	}

	// Message with unmatched closing tag
	{
		std::string out;
		std::string msg("<i>Hello, how are you?");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "&lt;i&gt;Hello, how are you?");
	}

	// Message with unmatched opening tag
	{
		std::string out;
		std::string msg("Hello, how are you?</i>");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "Hello, how are you?&lt;/i&gt;");
	}

	// subtags
	{
		std::string out;
		std::string msg("Hello!<br><i><u><b>How are you</b></u></i>?"); // subtags
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == msg);
	}

	// subtags (wrong order)
	{
		std::string out;
		std::string msg("Hello!<br><i><u><b>How are you</i></u></b>?"); // notice the order mismatch between opening and closing tags
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "Hello!<br><i>&lt;u&gt;&lt;b&gt;How are you</i>&lt;/u&gt;&lt;/b&gt;?");
	}

	// messages with some missing tags
	{
		std::string out;
		std::string msg("Hello!<br><b><i>How</i> <b><u>are</u></b> you</u>?");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "Hello!<br>&lt;b&gt;<i>How</i> <b><u>are</u></b> you&lt;/u&gt;?");
	}

	// escape span tag and last incomplete <u>
	{
		std::string out;
		std::string msg("<span><b><i>hello</i><br></b><br></span><u>");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "&lt;span&gt;<b><i>hello</i><br></b><br>&lt;/span&gt;&lt;u&gt;");
	}

	// escape span tag and last incomplete </u>
	{
		std::string out;
		std::string msg("<span><u><b><i>hello</i><br></b><br></u></span></u>");
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == "&lt;span&gt;<u><b><i>hello</i><br></b><br></u>&lt;/span&gt;&lt;/u&gt;");
	}

	// tiny web page quotation
	{
		std::string out;
		std::string msg(tiny_page_example);
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == tiny_page_example_quoted);
	}

	// try to escape already escaped text
	{
		std::string out;
		std::string msg(tiny_page_example_quoted);
		EXPECT_TRUE(vs::escape_unsafe_html_tags(msg.c_str(), out));
		EXPECT_TRUE(out == tiny_page_example_quoted);
	}
}
