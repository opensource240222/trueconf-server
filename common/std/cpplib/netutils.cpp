#include "netutils.h"
#include "std-generic/cpplib/hton.h"
#include "../clib/int128utils.h"

#include <algorithm>
#include <cstring>
#include <string>

#if defined(_WIN32)
#	include <ws2tcpip.h>
#elif defined(__linux__)
#	include <netinet/in.h> // in_addr, in6_addr
#	include <arpa/inet.h> // inet_pton
#endif

using namespace int128_utils;

uint32_t netutils::GetMaskValue_IPv4(size_t nbits)
{
	size_t left;
	nbits = (nbits > 32 ? 32 : nbits); // normalize

	left = 32 - nbits;

	return (left == 0 ? (~(uint32_t)0) : ((1 << nbits) - 1) << left);
}

bool netutils::IsPrivateAddress_IPv4(const uint32_t ip, bool ignore_zeroconf)
{
	// 16 bit private block
	if ((ip & GetMaskValue_IPv4(32 - 16)) == MakeAddress_IPv4(192, 168, 0, 0))
		return true;

	// 16 bit private block - zeroconf link-local adresses range
	if (!ignore_zeroconf && ((ip & GetMaskValue_IPv4(32 - 16)) == MakeAddress_IPv4(169, 254, 0, 0)))
		return true;

	// 20 bit private block
	if ((ip & GetMaskValue_IPv4(32 - 20)) == MakeAddress_IPv4(172, 16, 0, 0))
		return true;

	// 24 bit private block
	if ((ip & GetMaskValue_IPv4(32 - 24)) == MakeAddress_IPv4(10, 0, 0, 0))
		return true;

	return false;
}

bool netutils::IsPrivateAddress_IPv4(const char *str, bool ignore_zeroconf)
{
	in_addr addr;
	uint32_t ip;

	if (inet_pton(AF_INET, str, &addr) <= 0)
		return false;

	ip = vs_ntohl(addr.s_addr); // convert IP address into native host endianess

	return IsPrivateAddress_IPv4(ip, ignore_zeroconf);
}

bool netutils::IsSubdomain(const char *a_name, const char *a_domain)
{
	std::string name = a_name;
	std::string domain = a_domain;

	std::transform(name.begin(), name.end(), name.begin(), tolower);
	std::transform(domain.begin(), domain.end(), domain.begin(), tolower);

	{
		size_t len = domain.size();
		const char *p = strstr(name.c_str(), domain.c_str());

		if (p == NULL)
			return false;

		// check if match is in the end of name
		if (p[len] != '\0' &&
			(p[len] != '.' && p[len + 1] != '\0')) // handle optional dot properly
			return false;

		if (p != name && p[-1] != '.')
			return false;

		return true;
	}
}

void netutils::GetMaskValue_IPv6(uint32_t mask[], size_t nbits)
{
	__m128i res = int128_set_lowest_bits(nbits);

	int128_store_array(res, mask);
}

void netutils::GetMaskValue_IPv6(__m128i &mask, size_t nbits)
{
	mask = int128_set_lowest_bits(nbits);
}

bool netutils::IsAddressInRange_IPv6(const __m128i addr, const __m128i subnet, const __m128i mask, bool align_subnet)
{
	// TODO: algin IPv6 subnet number
	__m128i subnet_addr;
	__m128i subnet_aligned;
	__m128i cmp_res;
	uint32_t res[4] = { 0 };

	// align subnet number (just in case)
	subnet_aligned = (align_subnet == true ? _mm_and_si128(subnet, mask) : subnet);

	// get subnet addr
	subnet_addr = _mm_and_si128(addr, mask);
	// compare as four 32 bit integers
	cmp_res = _mm_cmpeq_epi32(subnet_addr, subnet_aligned);
	// get result
	_mm_storeu_si128((__m128i *)res, cmp_res);

	// check result
	if (res[0] && res[1] && res[2] && res[3])
		return true;

	return false;
}


bool netutils::IsAddressInRange_IPv6(const uint32_t addr[], const uint32_t subnet[], const uint32_t mask[], bool align_subnet)
{
	__m128i iaddr = int128_load_array(addr),
		isubnet = int128_load_array(subnet),
		imask = int128_load_array(mask);

	return IsAddressInRange_IPv6(iaddr, isubnet, imask, align_subnet);
}

// reverse bytes
static void datarev(void *data, const size_t nbytes)
{
	uint8_t *p = (uint8_t *)data;
	uint8_t a, b;
	for (size_t i = 0; i < nbytes / 2; i++)
	{
		a = p[i];
		b = p[(nbytes - 1) - i];

		if (a != b)
		{
			a = a ^ b;
			b = a ^ b;
			a = a ^ b;
		}

		p[i] = a;
		p[(nbytes - 1) - i] = b;
	}
}

bool netutils::StringToIPAddress(const char *addr, IPAddress &res)
{
	struct in_addr ip_addr_v4;
	struct in6_addr ip_addr_v6;

	// try to handle as IPv4
	if (inet_pton(AF_INET, addr, (void *)&ip_addr_v4) > 0)
	{
		res.type = IP_ADDR_V4;
		res.addr.ipv4 = vs_ntohl(ip_addr_v4.s_addr);
		return true;
	}
	else if (inet_pton(AF_INET6, addr, (void *)&ip_addr_v6) > 0) // try to handle as IPv6
	{
		res.type = IP_ADDR_V6;
		// save IPv6 address
		memcpy((void *)res.addr.ipv6, ip_addr_v6.s6_addr, 16);
		// reverse to Little Endian
		datarev((void *)res.addr.ipv6, 16);

		return true;
	}

	return false;
}

/*
Artem Boldarev (30.10.2015):
Domain name validator based on Recursive Descent Parsing technique.

It uses grammar based on recommended in https://tools.ietf.org/html/rfc1035.
It was converted to EBNF notation, to simplify parser creation.

Here is domain name grammar (converted to EBNF):

domain       = subdomain { subdomain } end.
subdomain    = label [ "." ] .
label        = letter { ldh_str } .
ldh_str      = let_dig_hyp { let_dig_hyp } .
let_dig_hyph = let_dig | "-" .
let_dig      = letter | digit .
letter       = "a" | "b" | <any upper case or lower case character> .
digit        = "0" | "1" | <any digit character> .
end          = "\0" | <any space character> .
*/

bool netutils::IsValidDomainName(const char *name, const bool non_strict, const bool allow_trailing_whitespace)
{
	int pos = 0;

	// utils
	auto match = [&](const int c) -> bool {
		return name[pos] == c;
	};

	auto advance = [&]() -> void {
		pos++;
	};


	// parsing rules
	auto end = [&]() -> bool {
		if (match('\0') || (allow_trailing_whitespace ? isspace(name[pos]) : 0))
			return true;

		return false;
	};

	auto letter = [&]() -> bool {

		if (isalpha(name[pos]))
		{
			advance();
			return true;
		}

		return false;
	};

	auto digit = [&]() -> bool {

		if (isdigit(name[pos]))
		{
			advance();
			return true;
		}

		return false;
	};

	auto let_dig = [&]() -> bool {
		return (letter() || digit());
	};

	auto let_dig_hyp = [&]() -> bool {
		if (let_dig())
		{
			return true;
		}
		else if (match('-') || (non_strict ? match('_') : 0))
		{
			advance();
			return true;
		}

		return false;
	};

	auto ldh_str = [&]() -> bool {

		if (!let_dig_hyp())
			return false;

		while (let_dig_hyp())
			;

		return true;
	};

	auto label = [&]() -> bool {

		if (non_strict)
		{
			if (!let_dig())
				return false;
		}
		else
		{
			if (!letter())
				return false;
		}

		ldh_str();

		return true;
	};

	auto subdomain = [&]() -> bool {
		if (label())
		{
			if (match('.'))
			{
				advance();
			}
		}
		else
		{
			return false;
		}

		return true;
	};

	auto domain = [&]() -> bool {
		if (!subdomain())
		{
			return false;
		}

		while (subdomain())
			;

		return end();
	};

	return domain();
}
