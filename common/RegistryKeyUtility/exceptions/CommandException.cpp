#include "CommandException.h"

CommandException::CommandException(const std::size_t line, const char* file, const std::string& message) : Exception(
	line, file, message)
{
}

CommandException::CommandException(std::size_t line, const char* file, const std::string& message,
	const std::exception& ex) : Exception(line, file, message, ex)
{
}
