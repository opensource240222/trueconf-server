#pragma once

#include <exception>
#include <string>

class Exception : public std::exception
{
public:
	explicit Exception(const std::size_t line, const char *file, std::string message);
	explicit Exception(const std::size_t line, const char *file, std::string message, const std::exception& ex);
	char const* what() const noexcept override;
private:
	std::string message_;
};
