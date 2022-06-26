#pragma once

#include <boost/version.hpp>

namespace boost { namespace asio {
#if BOOST_VERSION < 106600
	class io_service;
#else
	class io_context;
	typedef io_context io_service;
#endif
}}
