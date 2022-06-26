#pragma once

#include "VS_IPPortAddress.h"
#include "VS_AcsLib.h"
#include "../../net/Lib.h"

#include <ostream>

#include <in6addr.h>

template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s, const VS_IPPortAddress& x)
{
	auto flags(s.flags());
	s.width(0);
	switch (x.getAddressType())
	{
	case VS_IPPortAddress::ADDR_IPV4:
	{
		char buffer[16]; // "123.123.123.123\0"
		auto ip(x.ipv4_netorder());
		const char* result = net::inet_ntop(VS_IPPortAddress::ADDR_IPV4, &ip, buffer, sizeof(buffer));
		if (result)
			s << result;
		else
			s << '?';
	}
		break;
	case VS_IPPortAddress::ADDR_IPV6:
	{
		char buffer[40]; // "abcd:abcd:abcd:abcd:abcd:abcd:abcd:abcd\0"
		auto ip(x.ipv6());
		const char* result = net::inet_ntop(VS_IPPortAddress::ADDR_IPV6, &ip, buffer, sizeof(buffer));
		if (result)
			s << '[' << result << ']';
		else
			s << '?';
	}
		break;
	default:
		s << '?';
	}
	s << std::dec;
	s << ':' << x.port();
	s.flags(flags);
	return s;
}
