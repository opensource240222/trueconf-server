#include "ServerServices/VS_RTSPRequestParser.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace rtsp_stream_parser_test {

using Action = VS_RTSPRequestParser::Action;

struct RTSPRequestParserTest : ::testing::Test
{
	VS_RTSPRequestParser parser;
	::testing::MockFunction<Action (string_view part)> method_handler;
	::testing::MockFunction<Action (string_view part)> uri_handler;
	::testing::MockFunction<Action (string_view part)> version_handler;
	::testing::MockFunction<Action (string_view part)> header_name_handler;
	::testing::MockFunction<void (string_view part)> header_value_handler;
	::testing::MockFunction<void ()> headers_end_handler;
	::testing::MockFunction<void (string_view::size_type pos)> error_handler;
	std::string data;
	std::string all_data;

	RTSPRequestParserTest()
	{
		using ::testing::_;
		using ::testing::Return;
		ON_CALL(method_handler, Call(_)).WillByDefault(Return(Action::skip));
		ON_CALL(uri_handler, Call(_)).WillByDefault(Return(Action::skip));
		ON_CALL(version_handler, Call(_)).WillByDefault(Return(Action::skip));
		ON_CALL(header_name_handler, Call(_)).WillByDefault(Return(Action::skip));
	}

	bool Parse(string_view data)
	{
		return parser.Parse(data,
			[this](string_view part)           { all_data += part; return method_handler.Call(part); },
			[this](string_view part)           { all_data += part; return uri_handler.Call(part); },
			[this](string_view part)           { all_data += part; return version_handler.Call(part); },
			[this](string_view part)           { all_data += part; return header_name_handler.Call(part); },
			[this](string_view part)           { all_data += part; return header_value_handler.Call(part); },
			[this]()                           { headers_end_handler.Call(); },
			[this](string_view part)           { all_data += part; this->data += part; },
			[this](string_view::size_type pos) { error_handler.Call(pos); }
		);
	}

	void AdvanceToHeaderParsing()
	{
		using ::testing::_;
		using ::testing::Return;
		EXPECT_CALL(method_handler, Call(_)).Times(1).WillOnce(Return(Action::skip_line));
		EXPECT_TRUE(Parse("SETUP rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\n"));
		EXPECT_EQ(all_data, "SETUP rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\n");
		data.clear();
		all_data.clear();
	}
};

TEST_F(RTSPRequestParserTest, MethodName)
{
	using ::testing::_;
	using ::testing::Return;
	EXPECT_TRUE(Parse("SE"));
	EXPECT_CALL(method_handler, Call(string_view("SETUP"))).Times(1).WillOnce(Return(Action::parse));
	EXPECT_TRUE(Parse("TUP "));
	EXPECT_EQ(all_data, "SETUP ");
}

TEST_F(RTSPRequestParserTest, URI)
{
	using ::testing::_;
	using ::testing::Return;
	EXPECT_CALL(method_handler, Call(_)).Times(1).WillOnce(Return(Action::parse));
	EXPECT_TRUE(Parse("SETUP rtsp"));
	EXPECT_CALL(uri_handler, Call(string_view("rtsp://1.2.3.4/c/conf_name"))).Times(1).WillOnce(Return(Action::parse));
	EXPECT_TRUE(Parse("://1.2.3.4/c/conf_name "));
	EXPECT_EQ(all_data, "SETUP rtsp://1.2.3.4/c/conf_name ");
}

TEST_F(RTSPRequestParserTest, Version)
{
	using ::testing::_;
	using ::testing::Return;
	EXPECT_CALL(method_handler, Call(_)).Times(1).WillOnce(Return(Action::parse));
	EXPECT_CALL(uri_handler, Call(_)).Times(1).WillOnce(Return(Action::parse));
	EXPECT_TRUE(Parse("SETUP rtsp://1.2.3.4/c/conf_name RTS"));
	EXPECT_CALL(version_handler, Call(string_view("RTSP/8.9"))).Times(1).WillOnce(Return(Action::parse));
	EXPECT_TRUE(Parse("P/8.9\r\n"));
	EXPECT_EQ(all_data, "SETUP rtsp://1.2.3.4/c/conf_name RTSP/8.9\r\n");
}

TEST_F(RTSPRequestParserTest, ReqeustLineSkipped)
{
	using ::testing::_;
	using ::testing::Return;
	EXPECT_CALL(method_handler, Call(_)).Times(1).WillOnce(Return(Action::skip_line));
	EXPECT_CALL(uri_handler, Call(_)).Times(0);
	EXPECT_TRUE(Parse("SETUP rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\nCSeq"));
	EXPECT_EQ(data, " rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\n");
	EXPECT_EQ(all_data, "SETUP rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\n");
}

TEST_F(RTSPRequestParserTest, HeaderName)
{
	using ::testing::_;
	using ::testing::Return;
	AdvanceToHeaderParsing();
	EXPECT_TRUE(Parse("CS"));
	EXPECT_CALL(header_name_handler, Call(string_view("CSeq"))).Times(1).WillOnce(Return(Action::parse));
	EXPECT_TRUE(Parse("eq:"));
	EXPECT_EQ(all_data, "CSeq:");
}

TEST_F(RTSPRequestParserTest, HeaderValue)
{
	using ::testing::_;
	using ::testing::Return;
	AdvanceToHeaderParsing();
	EXPECT_TRUE(Parse("CS"));
	EXPECT_CALL(header_name_handler, Call(_)).Times(1).WillOnce(Return(Action::parse));
	EXPECT_TRUE(Parse("eq: 12"));
	EXPECT_CALL(header_value_handler, Call(string_view(" 1234"))).Times(1);
	EXPECT_TRUE(Parse("34\r\n"));
	EXPECT_EQ(all_data, "CSeq: 1234\r\n");
}

TEST_F(RTSPRequestParserTest, NextHeader)
{
	using ::testing::_;
	using ::testing::Return;
	AdvanceToHeaderParsing();
	EXPECT_CALL(header_name_handler, Call(string_view("CSeq"))).Times(1).WillOnce(Return(Action::skip_line));
	EXPECT_TRUE(Parse("CSeq: 1234"));
	EXPECT_CALL(header_name_handler, Call(string_view("Connection"))).Times(1).WillOnce(Return(Action::skip_line));
	EXPECT_TRUE(Parse("\r\nConnection:"));
	EXPECT_EQ(all_data, "CSeq: 1234\r\nConnection:");
}

TEST_F(RTSPRequestParserTest, HeaderSkipped)
{
	using ::testing::_;
	using ::testing::Return;
	AdvanceToHeaderParsing();
	EXPECT_TRUE(Parse("CS"));
	EXPECT_CALL(header_name_handler, Call(_)).Times(1).WillOnce(Return(Action::skip_line));
	EXPECT_CALL(header_value_handler, Call(_)).Times(0);
	EXPECT_TRUE(Parse("eq: 1234\r\nConnection"));
	EXPECT_EQ(data, ": 1234\r\n");
	EXPECT_EQ(all_data, "CSeq: 1234\r\n");
}

TEST_F(RTSPRequestParserTest, RequestSkipped)
{
	using ::testing::_;
	using ::testing::Return;
	EXPECT_CALL(method_handler, Call(_)).Times(1).WillOnce(Return(Action::skip));
	EXPECT_CALL(uri_handler, Call(_)).Times(0);
	EXPECT_CALL(version_handler, Call(_)).Times(0);
	EXPECT_CALL(header_name_handler, Call(_)).Times(0);
	EXPECT_CALL(header_value_handler, Call(_)).Times(0);
	EXPECT_CALL(headers_end_handler, Call()).Times(0);
	EXPECT_TRUE(Parse("SETUP rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\nCSeq:1\r\n\r\nPLAY"));
	EXPECT_EQ(data, " rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\nCSeq:1\r\n\r\n");
	EXPECT_EQ(all_data, "SETUP rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\nCSeq:1\r\n\r\n");
}

TEST_F(RTSPRequestParserTest, Body)
{
	using ::testing::_;
	using ::testing::Return;
	EXPECT_CALL(method_handler, Call(_)).Times(1).WillOnce(Return(Action::skip));
	EXPECT_TRUE(Parse("SETUP rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\nContent-Length:13\r\n\r\n"));
	data.clear();
	EXPECT_TRUE(Parse("Hello, "));
	EXPECT_TRUE(Parse("World!"));
	EXPECT_EQ(data, "Hello, World!");
	EXPECT_EQ(all_data, "SETUP rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\nContent-Length:13\r\n\r\nHello, World!");
}

TEST_F(RTSPRequestParserTest, InvalidContentLength)
{
	using ::testing::_;
	using ::testing::Return;
	EXPECT_CALL(method_handler, Call(_)).Times(1).WillOnce(Return(Action::skip));
	EXPECT_CALL(error_handler, Call(58)).Times(1);
	EXPECT_FALSE(Parse("SETUP rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\nContent-Length:foo\r\n\r\n"));
}

TEST_F(RTSPRequestParserTest, NextRequest)
{
	using ::testing::_;
	using ::testing::Return;
	EXPECT_CALL(method_handler, Call(string_view("SETUP"))).Times(1).WillOnce(Return(Action::skip));
	EXPECT_TRUE(Parse("SETUP rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\nCSeq:1\r\n"));
	EXPECT_CALL(method_handler, Call(string_view("PLAY"))).Times(1).WillOnce(Return(Action::skip));
	EXPECT_TRUE(Parse("\r\nPLAY "));
	EXPECT_EQ(all_data, "SETUP rtsp://1.2.3.4/c/conf_name RTSP/1.0\r\nCSeq:1\r\n\r\nPLAY ");
}

TEST_F(RTSPRequestParserTest, InterleavedData)
{
	using ::testing::_;
	using ::testing::Return;
	std::string block_1("$\x00\x00\x05Hello", 9);
	EXPECT_CALL(method_handler, Call(_)).Times(0);
	EXPECT_CALL(uri_handler, Call(_)).Times(0);
	EXPECT_CALL(version_handler, Call(_)).Times(0);
	EXPECT_CALL(header_name_handler, Call(_)).Times(0);
	EXPECT_CALL(header_value_handler, Call(_)).Times(0);
	EXPECT_CALL(headers_end_handler, Call()).Times(0);
	EXPECT_TRUE(Parse(block_1));
	EXPECT_EQ(data, block_1);
	EXPECT_CALL(method_handler, Call(string_view("SETUP"))).Times(1).WillOnce(Return(Action::skip));
	EXPECT_TRUE(Parse("SETUP "));
	EXPECT_EQ(all_data, block_1 + "SETUP ");
}

}
