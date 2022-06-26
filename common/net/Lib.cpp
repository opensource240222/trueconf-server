#include "Lib.h"

#include <cstring>
#include <type_traits>

#if defined(_WIN32)
#	include <WinSock2.h>
#	include <ws2ipdef.h>
#	include <Mstcpip.h>
#else
#	include <arpa/inet.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <sys/socket.h>
#endif

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/io_service.hpp>

namespace net {

#if defined(_WIN32)
static_assert(std::is_same<socket_handle_type, SOCKET>::value, "Unexpected socket handle type");
#endif

const char* inet_ntop(int af, const void* src, char* dst, size_t size)
{
#if defined(_WIN32)
	sockaddr_storage srcaddr;
	DWORD srcaddr_sz;

	if (af == AF_INET)
	{
		srcaddr_sz = sizeof(sockaddr_in);
		auto addr4 = reinterpret_cast<sockaddr_in*>(&srcaddr);
		addr4->sin_family = af;
		addr4->sin_port = 0;
		::memcpy(&addr4->sin_addr, src, sizeof(addr4->sin_addr));
	}
	else if (af == AF_INET6)
	{
		srcaddr_sz = sizeof(sockaddr_in6);
		auto addr6 = reinterpret_cast<sockaddr_in6*>(&srcaddr);
		addr6->sin6_family = af;
		addr6->sin6_port = 0;
		addr6->sin6_flowinfo = 0;
		::memcpy(&addr6->sin6_addr, src, sizeof(addr6->sin6_addr));
		addr6->sin6_scope_id = 0;
	}
	else
	{
		WSASetLastError(WSAEAFNOSUPPORT);
		return nullptr;
	}

	DWORD dst_sz = size;
	if (WSAAddressToStringA(reinterpret_cast<sockaddr*>(&srcaddr), srcaddr_sz, nullptr, dst, &dst_sz) != 0)
	{
		WSASetLastError(ERROR_INVALID_PARAMETER);
		return nullptr;
	}

	return dst;
#else
	return ::inet_ntop(af, src, dst, size);
#endif
}

int inet_pton(int af, const char* src, void* dst)
{
#if defined(_WIN32)
	if (af != AF_INET && af != AF_INET6)
	{
		WSASetLastError(WSAEAFNOSUPPORT);
		return -1;
	}

	// WSAStringToAddress wants non-const string
	char src_copy[INET6_ADDRSTRLEN + 1];
	::strncpy(src_copy, src, sizeof(src_copy));
	src_copy[sizeof(src_copy) - 1] = '\0';

	sockaddr_storage dstaddr;
	int dstaddr_sz = sizeof(dstaddr);
	if (WSAStringToAddress(src_copy, af, nullptr, reinterpret_cast<sockaddr*>(&dstaddr), &dstaddr_sz) != 0)
		return 0;

	if (af == AF_INET)
	{
		auto addr4 = reinterpret_cast<sockaddr_in*>(&dstaddr);
		::memcpy(dst, &addr4->sin_addr, sizeof(addr4->sin_addr));
		return 1;
	}
	else if (af == AF_INET6)
	{
		auto addr6 = reinterpret_cast<sockaddr_in6*>(&dstaddr);
		::memcpy(dst, &addr6->sin6_addr, sizeof(addr6->sin6_addr));
		return 1;
	}
	else
		return 0;
#else
	return ::inet_pton(af, src, dst);
#endif
}

boost::system::error_code EnableTCPKeepAlive(socket_handle_type socket, unsigned idle_s, unsigned interval_s)
{
#if defined(_WIN32)
	tcp_keepalive alive;
	alive.onoff = true;
	alive.keepalivetime = idle_s * 1000;
	alive.keepaliveinterval = interval_s * 1000;
	DWORD unused;
	if (0 != ::WSAIoctl(socket, SIO_KEEPALIVE_VALS, &alive, sizeof(alive), NULL, 0, &unused, NULL, NULL))
		return boost::system::error_code(::WSAGetLastError(), boost::system::system_category());
	return {};
#else
	int optval;
#if defined(TCP_KEEPIDLE)
	optval = idle_s;
	if (0 != ::setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, &optval, sizeof(optval)))
		return boost::system::error_code(errno, boost::system::system_category());
#endif
#if defined(TCP_KEEPINTVL)
	optval = interval_s;
	if (0 != ::setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, &optval, sizeof(optval)))
		return boost::system::error_code(errno, boost::system::system_category());
#endif
	optval = 1;
	if (0 != ::setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)))
		return boost::system::error_code(errno, boost::system::system_category());
	return {};
#endif
}

net::address MakeA_lookup(const std::string &host, boost::asio::io_service & ios, bool ipv6) {
	using udp = boost::asio::ip::udp;
	udp::resolver resolver(ios);
	udp::resolver::query query(ipv6 ? udp::v6() : udp::v4(), host, "");

	boost::system::error_code ec;
	auto it = resolver.resolve(query, ec);
	if (boost::system::errc::success == ec && it != decltype(it)())	return static_cast<udp::endpoint>(*it).address();

	return net::address();
}

bool IsRecoverableUDPReadError(boost::system::error_code ec)
{
	// This list is based on the similar list in udp_socket.cpp from libtorrent 1.1.11.
	// For some of these errors we ourselves observed read operations fail with them.
	// (E.g. connection_refused on Microsoft Windows Server 2012 R2 Server Standard, 64-bit (build 9600) in Rosgvardiya).
	//
	// Note: Because we are checking for a lot of errors, we do it in a more manual way to avoid comparing the category multiple times.
	return ec.category() == boost::system::system_category() && (
	       ec.value() == boost::asio::error::host_unreachable
	    || ec.value() == boost::asio::error::connection_aborted
	    || ec.value() == boost::asio::error::connection_refused
	    || ec.value() == boost::asio::error::connection_reset
	    || ec.value() == boost::asio::error::message_size
	    || ec.value() == boost::asio::error::network_reset
	    || ec.value() == boost::asio::error::network_unreachable
#if defined(_WIN32)
	    || ec.value() == ERROR_CONNECTION_ABORTED
	    || ec.value() == ERROR_CONNECTION_REFUSED
	    || ec.value() == ERROR_HOST_UNREACHABLE
	    || ec.value() == ERROR_NETWORK_UNREACHABLE
	    || ec.value() == ERROR_PORT_UNREACHABLE
	    || ec.value() == ERROR_RETRY
#endif
	);
}

}
