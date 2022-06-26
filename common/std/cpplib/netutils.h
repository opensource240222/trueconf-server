#pragma once

#include <cstdlib>
#include <cstdint>
#include <emmintrin.h>

namespace netutils {

	// !!! IPv4 Utilities !!!
	// returns netmask (host byte order)
	extern uint32_t GetMaskValue_IPv4(size_t nbits);
	extern bool IsPrivateAddress_IPv4(const char *str, bool ignore_zeroconf = false);
	extern bool IsPrivateAddress_IPv4(const uint32_t ip_hostorder, bool ignore_zeroconf = false);

	// check if address belongs to the subnet
	inline bool IsAddressInRange_IPv4(const uint32_t addr, const uint32_t subnet, const uint32_t mask, bool align_subnet = true)
	{
		return (addr & mask) == (align_subnet == true ? (subnet & mask) : subnet);
	}

	// make IPv4 address from 4 octets. Returns address in host order.
	inline uint32_t MakeAddress_IPv4(const uint32_t o1, const uint32_t o2, const uint32_t o3, const uint32_t o4)
	{
		return ((o1) & 0x000000FFLU) << 24 |
			   ((o2) & 0x000000FFLU) << 16 |
			   ((o3) & 0x000000FFLU) <<  8 |
			   ((o4) & 0x000000FFLU);
	}

	// !!! IPv6 Utilities !!!

	// IPv6 address (and mask as well) represented as four 32 bit unsigned untegers.
	// They are together represent a 128 bit integer number with following format in memory:
	//  high bits -> | [0] [1] [2] [3] | <- low bits
	extern void GetMaskValue_IPv6(uint32_t mask[], size_t nbits);
	// check if address belongs to the subnet
	extern bool IsAddressInRange_IPv6(const uint32_t addr[], const uint32_t subnet[], const uint32_t mask[], bool align_subnet = true);

	// for VS_NetworkConnectionACL mostly. Consider using previous functions.
	// IPv6 address is prepresented as 128 bit SSE2 integer
	extern void GetMaskValue_IPv6(__m128i &mask, size_t nbits);
	extern bool IsAddressInRange_IPv6(const __m128i addr, const __m128i subnet, const __m128i mask, bool align_subnet = true);

	// !!! Common Utilities !!!

	typedef enum _IPAddrType {
		IP_ADDR_V4,
		IP_ADDR_V6
	} IPAddrType;

	typedef struct _IPAddress
	{
		IPAddrType type;
		union _addr { // adresses are stored in host order
			uint32_t ipv4;
			uint32_t ipv6[4]; // 128 nit integer in host order
		} addr;
	} IPAddress;

	extern bool StringToIPAddress(const char *addr, IPAddress &res);

	// check if given name is part of subdomain
	extern bool IsSubdomain(const char *a_name, const char *a_domain);
	// domain name validator
	// if non_strict equals true - allow some widely although non-standart extensions:
	// - allow underscore '_' characters in domain names;
	// - allow begining of the subdomain with number.
	//
	// Otherwise (if non_strict equals false) it will validate domain name strictly to the grammar found in RFC 1035.
	extern bool IsValidDomainName(const char *p, const bool non_strict = false, const bool allow_trailing_whitespace = false);
}
