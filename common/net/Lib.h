#pragma once

#include "Address.h"
#include "std-generic/asio_fwd.h"

#include <boost/system/error_code.hpp>

#include <cstddef>

namespace net {

#if defined(_WIN64)
	using socket_handle_type = unsigned __int64;
#elif defined(_WIN32)
	using socket_handle_type = unsigned int;
#else
	using socket_handle_type = int;
#endif

// Windows XP doesn't have these functions, so we provide custom
// implementations of them in that case.
const char* inet_ntop(int af, const void* src, char* dst, size_t size);
int inet_pton(int af, const char* src, void* dst);

// Enables TCP keep-alive with parameters:
//   idle_s - delay in seconds before the first keep-alive packet is sent.
//   interval_s - interval in seconds between sending keep-alive packets.
boost::system::error_code EnableTCPKeepAlive(socket_handle_type socket, unsigned idle_s, unsigned interval_s);

net::address MakeA_lookup(const std::string &host, boost::asio::io_service & ios, bool ipv6 = false);

// Returns true if the error is not related to a UDP read operation
// (for example, "ICMP Port Unreachable" received after some write operation performed on the same socket).
// In that case UDP read can (and should be) restarted.
bool IsRecoverableUDPReadError(boost::system::error_code ec);

}
