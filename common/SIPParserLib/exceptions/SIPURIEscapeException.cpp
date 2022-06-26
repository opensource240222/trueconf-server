#include "SIPURIEscapeException.h"
#include <string>
#include <limits>
#include <assert.h>

namespace vs {

	SIPURIEscapeException::SIPURIEscapeException(const std::size_t line, const char *file, std::string message)
		: message_(std::move(message))
	{

#ifndef NDEBUG
		assert(file != nullptr);
		char str_line[std::numeric_limits<std::size_t>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
		snprintf(str_line, sizeof str_line, "%zu", line);
		message_ += " <";
		message_ += file;
		message_ += ":";
		message_ += str_line;
		message_ += ">";
#endif

	}

	SIPURIEscapeException::SIPURIEscapeException(const std::size_t line, const char *file, std::string message,
		const std::exception& ex) : SIPURIEscapeException(line, file, std::move(message))
	{
		const auto what_msg = ex.what();
		const std::size_t len_msg = std::char_traits<char>::length(what_msg);
		if (len_msg > 0)
		{
			message_ += "\n at ";
			message_ += std::string(what_msg, len_msg);
		}
	}

	char const* SIPURIEscapeException::what() const noexcept
	{
		return message_.c_str();
	}
}
