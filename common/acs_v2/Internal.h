#pragma once

#include "Handler.h"

#include <ostream>

namespace acs {

template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s, Response& x)
{
	switch (x)
	{
	case Response::next_step:         return s << "next_step";
	case Response::not_my_connection: return s << "not_my_connection";
	case Response::my_connection:     return s << "my_connection";
	case Response::accept_connection: return s << "accept_connection";
	default:                          return s << "INVALID";
	}
}

}
