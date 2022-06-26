#pragma once

#include "asio_resolver_mock.h"
#include "asio_socket_mock.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

namespace test { namespace asio {

template <class RealProtocol>
class protocol_mock
{
public:
	// using acceptor = basic_acceptor_mock<RealProtocol>;
	using endpoint = typename RealProtocol::endpoint;
	using resolver = basic_resolver_mock<RealProtocol>;
	using socket = basic_socket_mock<RealProtocol>;

	protocol_mock(int family = 2/*AF_INET*/, int protocol = 0x12341234, int type = 0x56785678)
		: family_(family)
		, protocol_(protocol)
		, type_(type)
	{
	}
	int family() const
	{
		return family_;
	}
	int protocol() const
	{
		return protocol_;
	}
	int type() const
	{
		return type_;
	}
	static protocol_mock v4()
	{
		protocol_mock(2/*AF_INET*/);
	}
	static protocol_mock v6()
	{
		protocol_mock(10/*AF_INET6*/);
	}
	bool operator==(const protocol_mock& x) const
	{
		return family_ == x.family_ && protocol_ == x.protocol_ && type_ == x.type_;
	}
	bool operator!=(const protocol_mock& x) const
	{
		return !(*this == x);
	}

private:
	int family_;
	int protocol_;
	int type_;
};

using tcp_mock = protocol_mock<boost::asio::ip::tcp>;
using udp_mock = protocol_mock<boost::asio::ip::udp>;

#if !(defined(_MSC_VER) && _MSC_VER < 1900)
static_assert(std::is_copy_constructible<tcp_mock>::value, "!");
static_assert(std::is_copy_assignable   <tcp_mock>::value, "!");
static_assert(std::is_move_constructible<tcp_mock>::value, "!");
static_assert(std::is_move_assignable   <tcp_mock>::value, "!");

static_assert(std::is_copy_constructible<udp_mock>::value, "!");
static_assert(std::is_copy_assignable   <udp_mock>::value, "!");
static_assert(std::is_move_constructible<udp_mock>::value, "!");
static_assert(std::is_move_assignable   <udp_mock>::value, "!");
#endif

}}
