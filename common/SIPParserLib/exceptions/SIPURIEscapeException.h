#pragma once
#include <iostream>

namespace vs {
	class SIPURIEscapeException : public std::exception
	{
	public:
		explicit SIPURIEscapeException(const std::size_t line, const char *file, std::string message);
		explicit SIPURIEscapeException(const std::size_t line, const char *file, std::string message, const std::exception& ex);
		char const* what() const noexcept override;
	private:
		std::string message_;
	};
}
