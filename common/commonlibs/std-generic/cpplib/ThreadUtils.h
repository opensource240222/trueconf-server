#pragma once

#include "std-generic/cpplib/ptr_arg.h"

#include <chrono>
#include <thread>

namespace vs {

namespace detail { void SetThreadName(const char* name); } // Don't use this function directly.

// Sets current thread name.
// Name must 0-15 characters long (not including terminating null).
// On Windows<10 name will be displayed in the debugger only if it was attached before the call to this function.
inline void SetThreadName(vs::ptr_arg<const char> name)
{
	detail::SetThreadName(name);
}
template <unsigned N>
void SetThreadName(const char (&name)[N])
{
	static_assert(N <= 15 + 1, "Thread name is too long");
	detail::SetThreadName(name);
}

// On Windows sets language for messages produced by FormatMessage to english.
// On Linux this is a no-op.
void FixThreadSystemMessages();

void SleepFor(std::chrono::nanoseconds nano);

namespace thread
{
	enum priority
	{
		normal,
		high,
		highest,
		critical
	};

	// native_handle() is non-const, so have to pass non-const link,
	//return true, if result SUCCESS
	bool SetPriority(std::thread& thread, vs::thread::priority priority);

}//namespace thread

} //namespace vs
