#pragma once

#include <cstdint>

namespace mdns
{

//DNS MESSAGE HEADER
//------------------------------------------------------------------------------

//Identifier
typedef uint16_t ID;

//Query/Response Flag
enum class QR
{
	NONE = -1,
	QUERY = 0,
	RESPONSE = 1,
};

//Operation code
enum class OPCODE
{
	NONE = -1,
	QUERY = 0,
	IQUERY = 1,
	STATUS = 2,
	RESERVED_UNUSED = 3,
	NOTIFY = 4,
	UPDATE = 5
};

//Authoritative Answer Flag
enum class AA
{
	NONE = -1,
	NO = 0,
	YES = 1
};

//Truncation Flag
enum class TC
{
	NONE = -1,
	NO = 0,
	YES = 1
};

//Recursion Desired
enum class RD
{
	NONE = -1,
	NO = 0,
	YES = 1
};

//Recursion Available
enum class RA
{
	NONE = -1,
	NO = 0,
	YES = 1
};

//Zero
typedef uint8_t ZERO;

//Response Code
enum class RCODE
{
	NONE = -1,
	ERROR_NONE = 0,
	FORMAT_ERROR = 1,
	SERVER_FAILURE = 2,
	NAME_ERROR = 3,
	NOT_IMPLEMENTED = 4,
	REFUSED = 5,
	YX_DOMAIN = 6,
	YX_RR_SET = 7,
	NX_RR_SET = 8,
	NOT_AUTH = 9,
	NOT_ZONE = 10
};

typedef uint16_t QD;
typedef uint16_t AN;
typedef uint16_t NS;
typedef uint16_t AR;

//DNS Resource Records Section
//------------------------------------------------------------------------------

//RR Type
enum class TYPE
{
	NONE = -1,
	A = 1,
	NS = 2,
	CNAME = 5,
	SOA = 6,
	PTR = 12,
	MX = 15,
	TXT = 16,
	RP = 17,
	AFSDB = 18,
	SIG = 24,
	KEY = 25,
	AAAA = 28,
	LOC = 29,
	SRV = 33,
	NAPTR = 35,
	KX = 36,
	CERT = 37,
	DNAME = 39,
	OPT = 41,
	APL = 42,
	DS = 43,
	SSHFP = 44,
	IPSECKEY = 45,
	RRSIG = 46,
	NSEC = 47,
	DNSKEY = 48,
	DHCID = 49,
	NSEC3 = 50,
	NSEC3PARAM = 51,
	TLSA = 52,
	HIP = 55,
	CDS = 59,
	CDNSKEY = 60,
	OPENPGPKEY = 61,
	TKEY = 249,
	TSIG = 250,
	IXFR = 251,
	AXFR = 252,
	ANY = 255,
	URI = 256,
	CAA = 257
};

enum class CACHEFLUSH
{
	NONE = -1,
	NO = 0,
	YES = 1
};

enum class CLASS
{
	NONE = -1,
	INTERNET = 1
};

typedef uint32_t TTL;
typedef uint16_t RDLENGTH;

//DNS Question Section
//------------------------------------------------------------------------------

//Unicast/Multicast flag
enum class QU
{
	NONE = -1,
	NO = 0,
	YES = 1
};

}
