#pragma once

#include <vector>
#include <cstdint>
#include <xmmintrin.h> // SSE2 extensions
#include "std/clib/int128utils.h"
#include "net/Address.h"
#include "std-generic/attributes.h"

/*
Artem Boldarev (09.10.2015):

SIP and H323 connection Access Control List (ACL) class.
*/

class VS_NetworkConnectionACL
{
public:
	typedef enum _ConnectionType {
		CONN_SIP, // Load ACL data from "<server_root>\SIP Peers"
		CONN_H323 // Load ACL data from "<server_root>\H323 Peers"
	} ConnectionType;

	typedef enum _AddrType {
		IP_V4,
		IP_V6
	} AddressType;

	typedef enum _ACLMode {
		ACL_NONE,     // do not check access
		ACL_WHITELIST,
		ACL_BLACKLIST
	} ACLMode;

private:

	typedef struct _ACLEntryIPv4 {
		uint32_t subnet; // host order
		uint32_t mask;   // mask value
	} ACLEntryIPv4;

	typedef struct _ACLEntryIPv6 {
		__m128i subnet;
		__m128i mask;
	} ACLEntryIPv6;

	typedef struct _ACLEntry {
		AddressType type; // IPv4/IPv6
		union {
			ACLEntryIPv4 ipv4;
			ACLEntryIPv6 ipv6;
		} data;
	} ACLEntry;

	typedef std::vector<ACLEntry> ACL;
public:
	VS_NetworkConnectionACL();
	virtual ~VS_NetworkConnectionACL();

	// add new Access Control List entry
	bool AddEntry(const uint32_t subnet, const uint32_t mask);     // IPv4
	// subnet and mask are arrays of four unsigned 32 bit inntegers, which shall unite into one 128 bit host order (Little Endian) integer
	//  high bits -> | [0] [1] [2] [3] | <- low bits
	bool AddEntry(const uint32_t subnet[], const uint32_t mask[]); // IPv6
	bool AddEntry(const char *subnet_cidr); // IPv4 and IPv6
	// set ACL mode
	bool SetMode(const ACLMode acl_type);
	ACLMode GetMode(void) const;
	bool Clear(void);
	void Finalize(void);

	// load Access Control List data from registry
	void LoadACL(const ConnectionType type);

	// check if address can connect to the server
	bool IsAllowed(const uint32_t ipv4addr_hostorder)   const; // IPv4
	// See AddEntry() for IPv6
	bool IsAllowed(const uint32_t ipv6addr_hostorder[]) const; // IPv6
	bool IsAllowed(const char *addr) const; // IPv4 and IPv6
	bool IsAllowed(const net::address &addr) const; // IPv4 and IPv6
private:
	static bool ParseEntry(ACLEntry &e, const char *entry);
	static bool EntriesAreEqual(const ACLEntry &e1, const ACLEntry &e2);
	// check if value with this entry is already in ACL
	bool CheckEntry(const ACLEntry &e) const;
	bool AddEntry(const ACLEntry &e);
	bool AddEntry(const __m128i subnet, const __m128i mask); // IPv6
	bool IsAllowed(const __m128i ipv6addr_hostorder) const;  // IPv6
	// parse ACL data, read from registry
	void ParseACLData(const std::vector<uint8_t> &data);
	bool HandleFoundResult(const bool res) const;
private:

	ACLMode m_mode;
	ACL m_acl;
	bool m_finalized;
};
