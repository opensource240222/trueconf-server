#pragma once

#include "Exception.h"

class CommandException : public Exception
{
public:
	explicit CommandException(const std::size_t line, const char* file, const std::string& message);
	explicit CommandException(const std::size_t line, const char* file, const std::string& message, const std::exception& ex);
};