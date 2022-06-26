#pragma once

#include <boost/system/error_code.hpp>

namespace transport {

const boost::system::error_category& transport_category();

enum class errc
{
	invalid_message_header = 1,
	invalid_message_head_cksum,
	invalid_message_body_cksum,
	message_encryption_error,
	handshake_error,
	secure_handshake_error,
};

inline boost::system::error_code make_error_code(errc e)
{
  return boost::system::error_code(static_cast<int>(e), transport_category());
}

inline boost::system::error_condition make_error_condition(errc e)
{
  return boost::system::error_condition(static_cast<int>(e), transport_category());
}

}

namespace boost { namespace system {
template <> struct is_error_code_enum<transport::errc> { static const bool value = true; };
}}

