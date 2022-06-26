#pragma once

#if !defined(_WIN32)
#	error Windows only functions
#endif

#include "std/cpplib/VS_SimpleStr.h"
#include "std/cpplib/VS_WideStr.h"

#include <comutil.h>

namespace vs {

inline VS_SimpleStr StrFromVariantT(const variant_t& x)
{
	if (x.vt != VT_NULL && x.vt != VT_EMPTY)
		return { (char*)(_bstr_t)x };
	else
		return {};
}

inline VS_WideStr WStrFromVariantT(const variant_t& x)
{
	if (x.vt != VT_NULL && x.vt != VT_EMPTY && !(x.vt == VT_BSTR && x.bstrVal == 0))
		return { (wchar_t*)(_bstr_t)x };
	else
		return {};
}

}
