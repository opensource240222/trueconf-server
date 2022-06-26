#pragma once

#include "std-generic/cpplib/string_view.h"

#include <boost/asio/ip/address.hpp>

namespace net {

using boost::asio::ip::address;
using boost::asio::ip::address_v4;
using boost::asio::ip::address_v6;

bool is_domain_name(string_view in) noexcept;
bool is_ipv4(string_view in) noexcept;
bool is_ipv6(string_view in) noexcept;

extern bool is_private_address(const net::address &addr);
}
