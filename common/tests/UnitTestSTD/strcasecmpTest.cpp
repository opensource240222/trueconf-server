#include "std-generic/clib/strcasecmp.h"

#include <gtest/gtest.h>

#if defined(_WIN32)
#	include <string.h>
#	define os_strcasecmp _stricmp
#	define os_strncasecmp _strnicmp
#elif defined(__unix__)
#	include <strings.h>
#	define os_strcasecmp strcasecmp
#	define os_strncasecmp strncasecmp
#endif

#undef strcasecmp
#undef strcnasecmp

namespace strcasecmp_test {

static int GetSign(int x)
{
	return x < 0 ? -1 : x > 0 ? 1 : 0;
}

TEST(strcasecmp, ComparisonOrder)
{
	char l_str[2] = { 0, 0 };
	char r_str[2] = { 0, 0 };

	unsigned n_errors = 0;
	for (int l = 0; l < 256; ++l)
	{
		l_str[0] = l;
		for (int r = 0; r < 256; ++r)
		{
			r_str[0] = r;
			int res = GetSign(vs_strcasecmp(l_str, r_str));
			int os_res = GetSign(os_strcasecmp(l_str, r_str));
			if (res != os_res)
			{
				if (++n_errors <= 100)
				{
					ADD_FAILURE() << "Different comparison results for characters "
						"'" << static_cast<char>(l) << "' (" << l << ")"
						" and "
						"'" << static_cast<char>(r) << "' (" << r << ")"
						": our=" << res << ", os=" << os_res;
				}
				else
				{
					GTEST_FAIL() << "Too many errors, aborting the test.";
				}
			}
		}
	}
}

TEST(strncasecmp, ComparisonOrder)
{
	char l_str[2] = { 0, 0 };
	char r_str[2] = { 0, 0 };

	unsigned n_errors = 0;
	for (int l = 0; l < 256; ++l)
	{
		l_str[0] = l;
		for (int r = 0; r < 256; ++r)
		{
			r_str[0] = r;
			int res = GetSign(vs_strncasecmp(l_str, r_str, 1));
			int os_res = GetSign(os_strncasecmp(l_str, r_str, 1));
			if (res != os_res)
			{
				if (++n_errors <= 100)
				{
					ADD_FAILURE() << "Different comparison results for characters "
						"'" << static_cast<char>(l) << "' (" << l << ")"
						" and "
						"'" << static_cast<char>(r) << "' (" << r << ")"
						": our=" << res << ", os=" << os_res;
				}
				else
				{
					GTEST_FAIL() << "Too many errors, aborting the test.";
				}
			}
		}
	}
}

}
