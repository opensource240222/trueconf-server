#include "Globals.h"

#include <atomic>
#include <cassert>

std::string g_tr_endpoint_name;
static std::string g_log_directory = "./";

const std::string& vs::GetLogDirectory()
{
	return g_log_directory;
}

void vs::SetLogDirectory(std::string value)
{
#if !defined(NDEBUG)
	static std::atomic<bool> was_set { false };
	if (was_set.exchange(true))
		assert(false && "Attempting to change log directory a second time");
#endif

#if defined(_WIN32)
	if (value.back() != '/' && value.back() != '\\')
		value += '\\';
#else
	if (value.back() != '/')
		value += '/';
#endif
	g_log_directory = std::move(value);
}
