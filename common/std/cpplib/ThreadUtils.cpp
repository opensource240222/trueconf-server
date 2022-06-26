#include "std/cpplib/ThreadUtils.h"

#if defined(_WIN32)
#	include <Windows.h>
#elif defined(__linux__)
#	include <sys/syscall.h>
#	include <unistd.h>
#endif

namespace vs {

int GetThreadID()
{
#if defined(_WIN32)
	return ::GetCurrentThreadId();
#elif defined(__linux__)
	return ::syscall(SYS_gettid);
#endif
}

}//vs
