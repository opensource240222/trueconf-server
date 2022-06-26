#ifndef VS_IPPORTSTORAGE_H
#define VS_IPPORTSTORAGE_H

#include "VS_IPPortAddress.h"

class VS_IPPortStorage
{
public:
	// Constructor.
	VS_IPPortStorage()
		:m_ipv4_pointer(0), m_ipv6_pointer(0), m_port_pointer(0), m_address_family(VS_IPPortAddress::ADDR_UNDEF) {}

	// Set an address family (VS_IPPortAddress::ADDR_IPV4 or VS_IPPortAddress::ADDR_IPV6).
	void set_address_family(unsigned af)
	{
		m_address_family = af;
	}

	// Returns a pointer to the socket address storage.
	char* storage_ptr()
	{
		return m_sockaddr_storage;
	}

	// Returns a pointer to ipv4 address place in sockaddr_in structure that will be saved in m_sockaddr_storage.
	unsigned long*&	ipv4_ptr()
	{
		return m_ipv4_pointer;
	}

	// Returns a pointer to ipv6 address place in sockaddr_in6 structure that will be saved in m_sockaddr_storage.
	in6_addr*&	ipv6_ptr()
	{
		return m_ipv6_pointer;
	}

	// Returns a pointer to port place in sockaddr_* struecture that will be saved in m_sockaddr_storage.
	unsigned short*& port_ptr()
	{
		return m_port_pointer;
	}

	// Returns an address in VS_IPPortAddress format.
	VS_IPPortAddress	make_address();
private:
	// A space for a sockaddr_storage structure, that will keep an ip address and port.
	char			m_sockaddr_storage[128];
	unsigned long*	m_ipv4_pointer;
	in6_addr*		m_ipv6_pointer;
	unsigned short*	m_port_pointer;
	// A type of ip address (VS_IPPortAddress::ADDR_IPV4 or VS_IPPortAddress::ADDR_IPV6).
	unsigned		m_address_family;
};

#endif
