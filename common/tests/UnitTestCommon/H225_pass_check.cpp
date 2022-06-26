#if defined(_WIN32) // Not ported yet

#include "H323RASParserTestBase.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>

/*
void dumpmem(const void *ptr, const size_t size)
{
	const size_t BYTES_PER_LINE = 16;
	size_t offset, readen;
	FILE *out = stdout;

	uint8_t *p = (uint8_t *)ptr;
	const uint8_t *maxp = (p + size);
	for (offset = readen = 0; offset != size; offset += readen)
	{
		uint8_t buf[BYTES_PER_LINE];

		for (readen = 0; readen != BYTES_PER_LINE && (&p[offset + readen]) < maxp; readen++)
		{
			buf[readen] = p[offset + readen];
		}

		if (readen == 0)
			return;

		fprintf(out, "%.8x: ", offset);

		// raw data
		for (size_t i = 0; i < readen; i++)
		{
			fprintf(out, " %.2x", buf[i]);
			if (BYTES_PER_LINE > 8 && BYTES_PER_LINE % 2 == 0 && i == (BYTES_PER_LINE / 2 - 1))
				fprintf(out, " ");
		}

		// ASCII
		if (readen < BYTES_PER_LINE)
		{
			for (size_t i = readen; i < BYTES_PER_LINE; i++)
			{
				fprintf(out, "  ");
				fprintf(out, " ");
				if (BYTES_PER_LINE > 8 && BYTES_PER_LINE % 2 == 0 && i == (BYTES_PER_LINE / 2 - 1))
					fprintf(out, " ");
			}
		}
		fprintf(out, " ");
		for (size_t i = 0; i < readen; i++)
		{
			if (buf[i] <= 31 || buf[i] >= 127) // ignore control and non-ASCII characters
				fprintf(out, ".");
			else
				fprintf(out, "%c", buf[i]);
		}

		fprintf(out, "\n");
	}
}
*/

class H225PassCheck : public H323RASParserTestBase {
public:
	H225PassCheck() :
		H323RASParserTestBase(net::address{}, 0, net::address{}, 0)
	{

	}
protected:
	void SetUp() override
	{
		H323RASParserTestBase::SetUp();
	}

	void TearDown() override
	{
		H323RASParserTestBase::TearDown();
	}
};
/*
****************************************************************************
4.3.8 (as expected):
****************************************************************************
tokenOID:
00000000:  00 01 00                                         ...

generalID:
00000000:  04 00 63 00 39 00 00                             ..c.9..

password:
00000000:  06 00 63 00 39 00 39 00  00                      ..c.9.9..

timeStamp:
00000000:  c0 59 dc d1 04                                   .Y...

Total (VS_H235PwdCertToken):
00000000:  61 00 01 00 c0 59 dc d1  04 06 00 63 00 39 00 39 a....Y.....c.9.9
00000010:  00 00 04 00 63 00 39 00  00                      ....c.9..

****************************************************************************

****************************************************************************
4.3.9 (wrong):
****************************************************************************
tokenOID:
00000000:  00 01 00                                         ...

generalID:
00000000:  04 00 63 00 39 00 00                             ..c.9..

password:
00000000:  06 00 63 00 39 00 39 00  00                      ..c.9.9..

timeStamp:
00000000:  c0 59 dc d1 04                                   .Y...

Total (VS_H235PwdCertToken):
00000000:  61 00 01 00 60 59 dc d1  04 06 00 63 00 39 00 39 a...`Y.....c.9.9
00000010:  00 00 04 00 63 00 39 00  00                      ....c.9..

****************************************************************************
*/

TEST_F(H225PassCheck, pass_check)
{
	const char name[] = "c9";
	const char password[] = "c99";
	auto md5 = ras->MakeEncryptedToken_MD5_String(string_view{ name, sizeof(name) - 1 }, string_view{ password, sizeof(password) - 1}, 1507643653);
	// was:"4C6ADFBE295FF57B4C52EB2110A6A9BF", expected: "49898586951E171B7BFD204AFA8B24F2"
	ASSERT_STREQ(md5.c_str(), "49898586951E171B7BFD204AFA8B24F2");
}

#endif
