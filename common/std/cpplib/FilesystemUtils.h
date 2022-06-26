#pragma once
#include <cstdio>
#include "std-generic/cpplib/string_view.h"

namespace boost {
	namespace system {
		class error_code;
	}
}

namespace vs
{
	/*
	 * If function returned NULL, then the value of newFileName is not defined
	 */
	FILE *GetTempFile(string_view nameFile, const char *mode, std::string &newFileName, boost::system::error_code &ec) noexcept;

	void RenameFile(string_view oldFile, string_view newFile, boost::system::error_code &ec) noexcept;

	bool RemoveFile(string_view file, boost::system::error_code &ec) noexcept;
}
