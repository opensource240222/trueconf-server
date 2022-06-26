#pragma once

#include "std-generic/cpplib/macro_utils.h"

enum class VS_RTSPSourceType : unsigned
{
	Default = 1,
	Mix = 2,
	Speaker = 4,
};
VS_ENUM_BITOPS(VS_RTSPSourceType, unsigned)
