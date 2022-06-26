#pragma once

#include "Exception.h"

class StorageException : public Exception
{
public:
	explicit StorageException(const std::size_t line, const char* file, const std::string& message);
	explicit StorageException(const std::size_t line, const char* file, const std::string& message, const std::exception& ex);
};