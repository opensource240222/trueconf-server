#pragma once

#include <string>
#include "std-generic/cpplib/string_view.h"

namespace auth {
	std::string MakeTempPassword(const string_view secret, const string_view login);
	bool CheckTempPassword(const  string_view password, const string_view secret);
}