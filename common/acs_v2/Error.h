#pragma once

#include <boost/system/error_code.hpp>

namespace acs {

const boost::system::error_category& acs_category();

enum class errc
{
	handler_initialization_failed = 1,
	handler_not_found,
	listener_already_exists,
	listener_not_found,
};

inline boost::system::error_code make_error_code(errc e)
{
  return boost::system::error_code(static_cast<int>(e), acs_category());
}

inline boost::system::error_condition make_error_condition(errc e)
{
  return boost::system::error_condition(static_cast<int>(e), acs_category());
}

}

namespace boost { namespace system {
template <> struct is_error_code_enum<acs::errc> { static const bool value = true; };
}}

