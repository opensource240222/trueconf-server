#pragma once

#include <boost/system/error_code.hpp>

namespace net {

const boost::system::error_category& net_category();

enum class errc
{
	packet_truncated = 1,
};

inline boost::system::error_code make_error_code(errc e)
{
  return boost::system::error_code(static_cast<int>(e), net_category());
}

inline boost::system::error_condition make_error_condition(errc e)
{
  return boost::system::error_condition(static_cast<int>(e), net_category());
}

}

namespace boost { namespace system {
template <> struct is_error_code_enum<net::errc> { static const bool value = true; };
}}

