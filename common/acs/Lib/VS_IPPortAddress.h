#pragma once

#include <cstddef>
#include <string>

// Winsock2.h and Windows.h conflict
// Cannot incliude any of this libraries
#define AF_UNSPEC_FAKE	0
#define AF_INET_FAKE	2
#define AF_INET6_FAKE	23
// A fake structure, that have the same size as the in6_addr structure.
// Need to declare VS_IPPortAddress class (see member m_ip.v6).
struct in6_addr_fake { unsigned char addr[16]; };
// =================================

struct sockaddr;
struct in6_addr;
enum eConnectionType : int;
class VS_SimpleStr;

// Comparations between ipv6 adresses.
// Need for VS_IPPortAddress::operator<
bool operator<(const in6_addr& addr1, const in6_addr& addr2);
bool operator==(const in6_addr& addr1, const in6_addr& addr2);

// Ip endpoint (ip address + port). Supports ipv4 and ipv6.
class VS_IPPortAddress
{
public:
	// Address types that we can use.
	// Values are equal to winsocket address family constants.
	enum AddressFamily {
		ADDR_UNDEF	= AF_UNSPEC_FAKE,
		ADDR_IPV4	= AF_INET_FAKE,
		ADDR_IPV6	= AF_INET6_FAKE
	};

	// Constructor. Creates an empty address with ADDR_UNDEF address type.
	VS_IPPortAddress();

	// Constructor. Creates an address from string address representation and specified port.
	explicit VS_IPPortAddress(const char* address, unsigned short port = 0);

	// Constructor. Creates an address from sockaddr structure.
	explicit VS_IPPortAddress(const sockaddr& sa);

	// Makes string in format: "<protocol>:<address>:<port>", for example "TCP:127.0.0.1:5060"
	bool GetAddrString(VS_SimpleStr &str) const;

	// Make string, that contains ip address and write it into buff variable.
	//	 buff - pointer to a char buffer,
	//   length - max length of the buffer.
	// Returns true, if an operation was succesfully completed and buff contains an address string.
	// False - otherwise.
	bool GetHostByIp(char* buff, size_t length) const;
	std::string GetHostByIp() const;

	// Uses if we need to set ip address, but have only host name (string), that can be:
	// 1) correct ipv4 address
	// 2) correct ipv6 address
	// 3) existing domain name
	// 4) something bad...
	// Returns: true in cases 1-3, false in case 4.
	bool SetIPFromHostName(const char* hostname);

	// Uses if we need to set ip address, but have only string address representation, that can be:
	// 1) correct ipv4 address
	// 2) correct ipv6 address
	// 4) something else...
	// Returns: true in cases 1-2, false in case 3.
	bool SetIPFromAddressString(const char* address);

	// Sets address and port from sockaddr structure.
	bool SetFrom(const sockaddr& sa);

	// Copy all data from this object to addr.
	// Maybe worth to make operator= instead.
	// We cannot use default operator= because addr can points to subclass.
	void CopyTo(VS_IPPortAddress* addr);

	// Copy only ip address (and address type variable) from addr to this.
	void CopyIPFrom(const VS_IPPortAddress& addr);

	// Set ipv4 value that represented in network byte order. Now class describes ipv4 address.
	void ipv4_netorder(unsigned long addr);

	// Set ipv4 value. Now class describes ipv4 address.
	void ipv4(unsigned long addr);

	// Set ipv6 value. Now class describes ipv6 address.
	void ipv6(in6_addr addr);

	// Returns ipv4 address (do not use if getAddressType() != ADDR_IPV4).
	unsigned long ipv4() const;

	// Returns ipv4 address that represented in network byte order (do not use if getAddressType() != ADDR_IPV4).
	unsigned long ipv4_netorder() const;

	// Returns ipv6 address (do not use if getAddressType() != ADDR_IPV6).
	in6_addr ipv6() const;

	// Writes a little-endian representation (not network-byte-order) of ipv6_address
	// into buff array. The object must contain an ipv6 address, the size of buff must be at least 16 bytes.
	// For example:
	// ff80::1
	// will be represented as
	// ff - 80 - 00 - 00 - 00 - 00 - 00 - 00 - 00 - 00 - 00 - 00 - 00 - 00 - 00 - 01
	void make_ipv6_little_endian(unsigned char buff[16]) const;

	// methods to work with port (in common or network byte order).
	unsigned short port(unsigned short p);
	void port_netorder(unsigned short p);
	unsigned short port() const;
	unsigned short port_netorder() const;

	// Comparations between address+port objects.
	// Do not compare Connection Types (variable "type").
	bool operator==(const VS_IPPortAddress& other) const;
	bool operator!=(const VS_IPPortAddress& other) const { return !(*this == other); }
	bool operator< (const VS_IPPortAddress& addr) const;

	// Tests if ip is empty ("0.0.0.0" or "::").
	bool isZero() const;

	bool IsLocal() const;

	// Returns ip address type of current object (ipv4, ipv6 or undefined).
	unsigned getAddressType() const;

	// Connection type (eConnectionType).
	eConnectionType type;

private:
	// IP address.
	union
	{
		unsigned long	v4;
		in6_addr_fake	v6;
	} m_ip;
	// Type of ip address (ADDR_IPV4, ADDR_IPV6, ADDR_UNDEF).
	unsigned m_addressType;
	// Port.
	unsigned short	m_port;
};
