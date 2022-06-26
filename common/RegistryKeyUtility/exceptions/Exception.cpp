#include "Exception.h"
#include <assert.h>

Exception::Exception(const std::size_t line, const char *file, std::string message)
	: message_(std::move(message))
{
	assert(file != nullptr);

#ifndef NDEBUG
	message_ += " <" + std::string(file) + ":" + std::to_string(line) + ">";
#endif

}

Exception::Exception(const std::size_t line, const char *file, std::string message,
	const std::exception& ex) : Exception(line, file, std::move(message))
{
	const auto what_msg = ex.what();
	const std::size_t len_msg = std::char_traits<char>::length(what_msg);
	if (len_msg > 0)
	{
		message_ += "\n at ";
		message_ += std::string(what_msg, len_msg);
	}
}

char const* Exception::what() const noexcept
{
	return message_.c_str();
}