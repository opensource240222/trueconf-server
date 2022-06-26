#include <gtest/gtest.h>
#include "std-generic/compat/iterator.h"
#include "mdnslib/Parser.cpp"
#include "Packets.h"

TEST(Parser_Test, General)
{
	mdns::Parser parser;
	EXPECT_TRUE(parser.parse(reinterpret_cast<const char*>(response0405_TXT_PTR_SRV),
		vs::size(response0405_TXT_PTR_SRV)));
	EXPECT_EQ(parser.queries.size(), 0);
	EXPECT_EQ(parser.responses.size(), 4);
	EXPECT_EQ(parser.authRecs.size(), 0);
	EXPECT_EQ(parser.additRecs.size(), 5);
}

TEST(Parser_Test, FilterStuff)
{
	mdns::Parser parser;
	parser.filter(
		mdns::FILTER::QUERY |
		mdns::FILTER::PTR |
		mdns::FILTER::SRV |
		mdns::FILTER::TXT
		);
	EXPECT_FALSE(parser.parse(reinterpret_cast<const char*>(response0405_TXT_PTR_SRV),
		vs::size(response0405_TXT_PTR_SRV)));
	parser.clear();
	parser.filter(
		mdns::FILTER::PTR |
		mdns::FILTER::A |
		mdns::FILTER::AAAA
		);
	EXPECT_TRUE(parser.parse(reinterpret_cast<const char*>(response0405_TXT_PTR_SRV),
		vs::size(response0405_TXT_PTR_SRV)));
}

TEST(Parser_Test, FindName)
{
	mdns::Parser parser;
	parser.filter(
		mdns::FILTER::PTR |
		mdns::FILTER::SRV |
		mdns::FILTER::TXT
		);
	parser.filterName("\033485D607CEE22@5KPlayer-ek-pc\005_raop\004_tcp\005local", 46);
	EXPECT_TRUE(parser.parse(reinterpret_cast<const char*>(response0405_TXT_PTR_SRV),
		vs::size(response0405_TXT_PTR_SRV)));
	parser.clear();
	parser.filter(
		mdns::FILTER::PTR |
		mdns::FILTER::SRV |
		mdns::FILTER::TXT
		);
	parser.filterName("\033485D607CEE22@5KPlayer-ek-pc\005_raop\004_tcp\005local", 46);
	EXPECT_FALSE(parser.parse(reinterpret_cast<const char*>(response0107_DoubleLink),
		vs::size(response0107_DoubleLink)));
}

TEST(Parser_Test, CursedPacket)
{
	mdns::Parser parser;
	ASSERT_TRUE(parser.parse(reinterpret_cast<const char*>(cursedPacket), vs::size(cursedPacket)));
}
