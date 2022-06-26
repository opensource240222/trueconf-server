#include "ExtractorException.h"


ExtractorException::ExtractorException(const std::size_t line, const char* file, const std::string& message): Exception(
	line, file, message)
{
}

ExtractorException::ExtractorException(const std::size_t line, const char* file, const std::string& message,
                                       const std::exception& ex): Exception(line, file, message, ex)
{
}
