#pragma once

#include "net/Endpoint.h"
#include "acs/Lib/VS_IPPortAddress.h"
#include "SIPParserBase/VS_Const.h"

#include <cstring>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

namespace net {

	// Conversion functions between old (VS_IPPortAddress) and new (net::address) IP address classes.
	// They shouldn't be used in ported code.
	// TODO: Remove after porting is completed.
#if defined(_WIN32) && !defined(_TRY_PORTED_)

static_assert(std::tuple_size<net::address_v6::bytes_type>::value == sizeof(in6_addr), "net::address_v6::bytes_type has incorrect size");

inline net::address ConvertAddress(const VS_IPPortAddress& x)
{
	switch (x.getAddressType())
	{
	case VS_IPPortAddress::ADDR_IPV4:
		return net::address_v4(x.ipv4());
	case VS_IPPortAddress::ADDR_IPV6:
	{
		const auto addr6 = x.ipv6();
		net::address_v6::bytes_type bytes;
		std::memcpy(bytes.data(), &addr6, sizeof(addr6));
		return net::address_v6(bytes);
	}
	default:
		return {};
	}
}

inline VS_IPPortAddress ConvertAddress(const net::address_v4& x)
{
	VS_IPPortAddress result;
	result.ipv4(x.to_ulong());
	return result;
}

inline VS_IPPortAddress ConvertAddress(const net::address_v6& x)
{
	VS_IPPortAddress result;
	const auto bytes = x.to_bytes();
	in6_addr addr6;
	std::memcpy(&addr6, bytes.data(), bytes.size());
	result.ipv6(addr6);
	return result;
}

inline VS_IPPortAddress ConvertAddress(const net::address& x)
{
	if (x.is_v4())
		return ConvertAddress(x.to_v4());
	else if (x.is_v6())
		return ConvertAddress(x.to_v6());
	else
		return {};
}

inline net::protocol ConvertProtocol(eConnectionType val) {
	switch (val)
	{

	case CONNECTIONTYPE_TCP:return net::protocol::TCP;
	case CONNECTIONTYPE_UDP:return net::protocol::UDP;
	case CONNECTIONTYPE_BOTH:return net::protocol::any;
	case CONNECTIONTYPE_TLS:return net::protocol::TLS;
	case CONNECTIONTYPE_INVALID:
	default:
		return net::protocol::none;
	}

	return net::protocol::none;
}

inline eConnectionType ConvertProtocol(net::protocol p) {
	switch (p)
	{

	case net::protocol::TCP: return CONNECTIONTYPE_TCP;
	case net::protocol::UDP: return CONNECTIONTYPE_UDP;
	case net::protocol::TLS: return CONNECTIONTYPE_TLS;
	case net::protocol::any: return CONNECTIONTYPE_BOTH;
	case net::protocol::none:
	default:
		return CONNECTIONTYPE_INVALID;
	}
	return CONNECTIONTYPE_INVALID;
}

inline net::Endpoint ConvertEndpoint(const VS_IPPortAddress& x) {
	net::Endpoint res;
	res.addr = ConvertAddress(x);
	res.port = x.port();
	res.protocol = ConvertProtocol(x.type);
	return res;
}

inline VS_IPPortAddress ConvertEndpoint(const net::Endpoint& ep) {
	auto res = ConvertAddress(ep.addr);
	res.port(ep.port);
	res.type = ConvertProtocol(ep.protocol);
	return res;
}

#endif

namespace detail {

		template<typename Protocol>
		struct ConvertProtocol { /*stub*/ };

		template<>
		struct ConvertProtocol<boost::asio::ip::udp> : std::integral_constant<eConnectionType, CONNECTIONTYPE_UDP> {};

		template<>
		struct ConvertProtocol<boost::asio::ip::tcp> : std::integral_constant<eConnectionType, CONNECTIONTYPE_TCP> {};
	}

template<typename Protocol>
using ConnectionType = detail::ConvertProtocol<typename std::decay<Protocol>::type::protocol_type>;

}
