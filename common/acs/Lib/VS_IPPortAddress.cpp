#include "VS_IPPortAddress.h"

#include <stdio.h>

#ifdef _WIN32
#include <Winsock2.h>
#include <ws2ipdef.h>
#else
#include <netinet/in.h>
#endif

#ifndef _WIN32
#define ZeroMemory(p, sz) memset((p), 0, (sz))
#endif

#include "VS_AcsLib.h"
#include "SIPParserBase/VS_Const.h"
#include "std/cpplib/VS_SimpleStr.h"
#include "std-generic/cpplib/hton.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

bool operator<(const in6_addr& addr1, const in6_addr& addr2)
{
	for(int i = 0; i < 8; i++)
	{
		if(addr1.u.Word[i] < addr2.u.Word[i]) return true;
		if(addr1.u.Word[i] > addr2.u.Word[i]) return false;
	}
	return false;
}

bool operator==(const in6_addr& addr1, const in6_addr& addr2)
{
	for(int i = 0; i < 8; i++)
	{
		if(addr1.u.Word[i] != addr2.u.Word[i]) return false;
	}
	return true;
}

bool operator<(const in6_addr_fake& a1, const in6_addr_fake& a2)
{
	const in6_addr& addr1 = (const in6_addr&) a1;
	const in6_addr& addr2 = (const in6_addr&) a2;
	return addr1 < addr2;
}

bool operator==(const in6_addr_fake& a1, const in6_addr_fake& a2)
{
	const in6_addr& addr1 = (const in6_addr&) a1;
	const in6_addr& addr2 = (const in6_addr&) a2;
	return addr1 == addr2;
}

VS_IPPortAddress::VS_IPPortAddress()
	:type(CONNECTIONTYPE_INVALID),m_addressType(ADDR_UNDEF),m_port(0)
{
	ZeroMemory(&m_ip, sizeof(m_ip));
}

VS_IPPortAddress::VS_IPPortAddress(const char* address, unsigned short port)
	:type(CONNECTIONTYPE_INVALID),m_addressType(ADDR_UNDEF),m_port(port)
{
	if (!SetIPFromAddressString(address))
		ZeroMemory(&m_ip, sizeof(m_ip));
}

VS_IPPortAddress::VS_IPPortAddress(const sockaddr& sa)
	:type(CONNECTIONTYPE_INVALID),m_addressType(ADDR_UNDEF),m_port(0)
{
	if (!SetFrom(sa))
		ZeroMemory(&m_ip, sizeof(m_ip));
}

bool VS_IPPortAddress::GetAddrString(VS_SimpleStr &str) const
{
	char host[256];
	if((type != CONNECTIONTYPE_TCP && type != CONNECTIONTYPE_UDP && type != CONNECTIONTYPE_TLS && type != CONNECTIONTYPE_BOTH) || !m_port)
		return false;

	if(m_addressType == ADDR_IPV4)
	{
		if (!VS_GetHostByIp(m_ip.v4, host, 256)) return false;
	}
	else if(m_addressType == ADDR_IPV6)
	{
		in6_addr tmp;
		memcpy_s(&tmp, sizeof(tmp), &m_ip.v6, sizeof(m_ip.v6));
		if (!VS_GetHostByIpv6(tmp, host, 256)) return false;
	}

	str = type == CONNECTIONTYPE_TCP ? "TCP" :
		  type == CONNECTIONTYPE_UDP ? "UDP" :
		  type == CONNECTIONTYPE_TLS ? "TLS" : "BOTH";
	str+=":";
	str += host;
	str+=":";
	char ch_port[256];
	_itoa(m_port,ch_port,10);
	str+=ch_port;

	return true;
}

bool VS_IPPortAddress::GetHostByIp(char* buff, size_t length) const
{
	if(m_addressType == ADDR_IPV4)
		return VS_GetHostByIp(ipv4(), buff, length);
	else if(m_addressType == ADDR_IPV6)
		return VS_GetHostByIpv6(ipv6(), buff, length);
	// m_addressType == ADDR_UNDEF
	return false;
}

std::string VS_IPPortAddress::GetHostByIp() const
{
	if (m_addressType == ADDR_IPV4)
		return VS_GetHostByIp_str(ipv4());
	else if (m_addressType == ADDR_IPV6)
		return VS_GetHostByIpv6_str(ipv6());
	// m_addressType == ADDR_UNDEF
	return "";
}

bool VS_IPPortAddress::SetIPFromHostName(const char* hostname)
{
	unsigned long	ip = 0;
	in6_addr		ip6;

	bool ipv4flag;
	if(VS_GetIPAddressByHostName(hostname, AF_INET, &ip, &ip6, &ipv4flag))
	{
		if(ipv4flag) // IPv4
			ipv4_netorder(ip);
		else // IPv6
			ipv6(ip6);
		return true;
	}
	return false;
}

bool VS_IPPortAddress::SetIPFromAddressString(const char* address)
{
	unsigned long	ip = 0;
	in6_addr		ip6;

	if (!address)
		return false;

	if(VS_GetIpByHost(address, &ip))
		ipv4(ip);
	else if(VS_GetIpv6ByHost(address, &ip6))
		ipv6(ip6);
	else return false;

	return true;
}

bool VS_IPPortAddress::SetFrom(const sockaddr& sa)
{
	switch (sa.sa_family)
	{
	case AF_INET:
	{
		const auto& sa_4 = reinterpret_cast<const sockaddr_in&>(sa);
		ipv4_netorder(reinterpret_cast<const unsigned long&>(sa_4.sin_addr));
		port_netorder(sa_4.sin_port);
		return true;
	}
	case AF_INET6:
	{
		const auto& sa_6 = reinterpret_cast<const sockaddr_in6&>(sa);
		ipv6(sa_6.sin6_addr);
		port_netorder(sa_6.sin6_port);
		return true;
	}
	}
	return false;
}

void VS_IPPortAddress::CopyTo(VS_IPPortAddress* addr)
{
	addr->type = type;
	addr->m_ip = m_ip;
	addr->m_addressType = m_addressType;
	addr->m_port = m_port;
}

void VS_IPPortAddress::CopyIPFrom(const VS_IPPortAddress& addr)
{
	m_ip			= addr.m_ip;
	m_addressType	= addr.m_addressType;
}

void VS_IPPortAddress::ipv4_netorder(unsigned long addr)
{
	m_addressType = ADDR_IPV4;
	ZeroMemory(&m_ip, sizeof(m_ip));
	m_ip.v4 = vs_ntohl(addr);
}

void VS_IPPortAddress::ipv4(unsigned long addr)
{
	m_addressType = ADDR_IPV4;
	ZeroMemory(&m_ip, sizeof(m_ip));
	m_ip.v4 = addr;
}

void VS_IPPortAddress::ipv6(in6_addr addr)
{
	m_addressType = ADDR_IPV6;
	ZeroMemory(&m_ip, sizeof(m_ip));
	memcpy_s(&m_ip.v6, sizeof(m_ip.v6), &addr, sizeof(addr));
}

unsigned long VS_IPPortAddress::ipv4() const
{
	if(m_addressType != ADDR_IPV4)
		dprint1("VS_IPPortAddress: trying to use non-ipv4 address as ipv4\n");
	return m_ip.v4;
}

unsigned long VS_IPPortAddress::ipv4_netorder() const
{
	if(m_addressType != ADDR_IPV4)
		dprint1("VS_IPPortAddress: trying to use non-ipv4 address as ipv4\n");
	return vs_htonl(m_ip.v4);
}

in6_addr VS_IPPortAddress::ipv6() const
{
	if(m_addressType != ADDR_IPV6)
		dprint1("VS_IPPortAddress: trying to use non-ipv6 address as ipv6\n");
	return *((in6_addr*)(&m_ip.v6));
}

void VS_IPPortAddress::make_ipv6_little_endian(unsigned char buff[16]) const
{
	in6_addr ipv6 = this->ipv6();
	// a last byte of ipv6 variable.
	const unsigned char* orig = ((const unsigned char*)&ipv6) + 15;
	for(int i = 0; i < 16; i++)
	{
		*buff = *orig;
		++buff;
		--orig;
	}
}

unsigned short VS_IPPortAddress::port(unsigned short p)
{
	return m_port = p;
}

void VS_IPPortAddress::port_netorder(unsigned short p)
{
	m_port = vs_ntohs(p);
}

unsigned short VS_IPPortAddress::port() const
{
	return m_port;
}

unsigned short VS_IPPortAddress::port_netorder() const
{
	return vs_htons(m_port);
}

bool VS_IPPortAddress::operator==(const VS_IPPortAddress& other) const
{
	// if types of address are different, then addresses are different too
	if(m_addressType != other.m_addressType) return false;
	// comparing ipv4
	if(m_addressType == ADDR_IPV4) return m_ip.v4 == other.m_ip.v4 && m_port == other.m_port;
	// comparing ipv6
	if(m_addressType == ADDR_IPV6) return m_ip.v6 == other.m_ip.v6 && m_port == other.m_port;
	// if both addrtype are undefined, then they are equal
	return true;
}

bool VS_IPPortAddress::operator < (const VS_IPPortAddress& addr) const
{
	if(addr.type != type) return addr.type < type;
	if(addr.m_addressType != m_addressType) return addr.m_addressType < m_addressType;
	bool addr_compare_res = false;

	// arbv writes (30.09.2016):
	// This operator is really important for storing addreesses as keys in map (as in VS_NetAcceptor at the time of writing).
	// One should consider comparing *both* IP and port values or BadThings (tm) could happen (like treating two VS_IPPortAddress objects
	// as equal while ports are really different).
	//
	// For the time of being:
	//  - If address are different - compare them and return result of comparison.
	//  - If addressed are equal - compare ports and return the result of comparison.
	// Makes sense for me.
	if (m_addressType == ADDR_IPV4)
	{
		if (addr.m_ip.v4 == m_ip.v4)
			return addr.m_port < m_port;
		else
			return addr.m_ip.v4 < m_ip.v4;

	}
	else if (m_addressType == ADDR_IPV6)
	{
		if (addr.m_ip.v6 == m_ip.v6)
			return addr.m_port < m_port;
		else
			return addr.m_ip.v6 < m_ip.v6;
	}

	return false; // undefined address is not lesser than another one
}

bool VS_IPPortAddress::isZero() const
{
	switch(m_addressType)
	{
		case ADDR_IPV4: return m_ip.v4 == 0;
		case ADDR_IPV6: return VS_TestIPv6_IsZero((const in6_addr*)&m_ip.v6);
		default: /* ADDR_UNDEF */ return true;
	}
}

bool VS_IPPortAddress::IsLocal() const
{
	if (m_addressType == VS_IPPortAddress::ADDR_IPV4)
	{
		if (ipv4() && (
			((ipv4() & VS_SUBNET_MASK_1) == VS_ADDRESS_SPACE_1) ||
			((ipv4() & VS_SUBNET_MASK_2) == VS_ADDRESS_SPACE_2) ||
			((ipv4() & VS_SUBNET_MASK_3) == VS_ADDRESS_SPACE_3)
			))
		{
			return true;
		}
	}
	else if (m_addressType == VS_IPPortAddress::ADDR_IPV6)
	{
		if (!isZero() && (
			(ipv6().u.Word[0] & VS_SUBNET_MASK_IPV6) == VS_ADDRESS_SPACE_IPV6
			))
		{
			return true;
		}
	}
	return false;
}

unsigned VS_IPPortAddress::getAddressType() const
{
	return m_addressType;
}