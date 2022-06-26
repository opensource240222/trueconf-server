#pragma once

#include <string>
#include "Exception.h"


class ExtractorException : public Exception
{
public:
	explicit ExtractorException(std::size_t line, const char* file, const std::string& message);
	explicit ExtractorException(std::size_t line, const char* file, const std::string& message, const std::exception& ex);
};