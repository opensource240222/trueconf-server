#if defined(_WIN32) // Not ported yet

#include "../../SIPParserLib/VS_SIPURI.h"

#include <sstream>
#include <gtest/gtest.h>

namespace SIPURI_tel_test
{
	class SIPURI_TelTest :
		public ::testing::Test
	{
	protected:
		SIPURI_TelTest()
		{}

		virtual void SetUp()
		{}

		virtual void TearDown()
		{}
	};

	TEST_F(SIPURI_TelTest, Test)
	{
		// OK: correct tel: uri
		{
			std::string tel("tel:+380679387708 SIP/2.0\r\n");
			VS_SIPURI uri;
			VS_SIPBuffer buf(tel.c_str(), tel.length() + 1);

			EXPECT_EQ(uri.Decode(buf), TSIPErrorCodes::e_ok);
			EXPECT_EQ(uri.URIType(), SIPURI_TEL);
		}

		// OK: correct tel: uri with parens and hyphens
		{
			std::string tel("tel:+38(095)00-87-971 SIP/2.0\r\n");
			VS_SIPURI uri;
			VS_SIPBuffer buf(tel.c_str(), tel.length() + 1);

			EXPECT_EQ(uri.Decode(buf), TSIPErrorCodes::e_ok);
			EXPECT_EQ(uri.URIType(), SIPURI_TEL);
		}

		// OK: correct tel: uri with parens and hyphens without a plus sign
		{
			std::string tel("tel:8(800)937-99-92 SIP/2.0\r\n");
			VS_SIPURI uri;
			VS_SIPBuffer buf(tel.c_str(), tel.length() + 1);

			EXPECT_EQ(uri.Decode(buf), TSIPErrorCodes::e_ok);
			EXPECT_EQ(uri.URIType(), SIPURI_TEL);
		}

		// OK: correct sip: uri which contains only a tel. number.
		{
			std::string tel("sip:+380950087971 SIP/2.0\r\n");
			VS_SIPURI uri;
			VS_SIPBuffer buf(tel.c_str(), tel.length() + 1);

			EXPECT_EQ(uri.Decode(buf), TSIPErrorCodes::e_ok);
			EXPECT_EQ(uri.URIType(), SIPURI_TEL);
		}

		// OK: correct sip: uri which a telephone number as user part.
		{
			std::string tel("sip:+380950087971@192.168.62.11 SIP/2.0\r\n");
			VS_SIPURI uri;
			VS_SIPBuffer buf(tel.c_str(), tel.length() + 1);

			EXPECT_EQ(uri.Decode(buf), TSIPErrorCodes::e_ok);
			EXPECT_EQ(uri.URIType(), SIPURI_SIP);
		}

		// WRONG: data from sniff 2 (Bug45440).
		{
			std::string tel("sip:#tel:+380950087971 SIP/2.0\r\n");
			VS_SIPURI uri;
			VS_SIPBuffer buf(tel.c_str(), tel.length() + 1);

			EXPECT_EQ(uri.Decode(buf), TSIPErrorCodes::e_ok);
			EXPECT_EQ(uri.URIType(), SIPURI_SIP);
		}
	}
}

#endif
