#include "VS_IPPortStorage.h"

#include "Winsock2.h"
#include "in6addr.h"

// Returns an address in VS_IPPortAddress format.
VS_IPPortAddress VS_IPPortStorage::make_address()
{
	VS_IPPortAddress result;
	switch(m_address_family)
	{
	case VS_IPPortAddress::ADDR_IPV4:
		result.ipv4_netorder(*m_ipv4_pointer);
		result.port_netorder(*m_port_pointer);
		break;
	case VS_IPPortAddress::ADDR_IPV6:
		result.ipv6(*m_ipv6_pointer);
		result.port_netorder(*m_port_pointer);
		break;
	}
	return result;
}