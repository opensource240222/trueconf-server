#pragma once

#include "std-generic/cpplib/macro_utils.h"

namespace net {

// Scoped bitmask for network protocols
enum class protocol : unsigned
{
	none = 0x0,
	TCP  = 0x1,
	UDP  = 0x2,
	TLS  = 0x4,
//	DTLS = 0x8,
	any = 0x7,
};
VS_ENUM_BITOPS(protocol, unsigned)

}
