#pragma once

#include "std-generic/cpplib/ThreadUtils.h" // Parts of this module are being moved to std-generic, we include them here to avoid breaking user code.

namespace vs {

// Returns ID of the current thread, suitable for output:
// On Linux it returns PID of the LWP corresponding to the thread, instead of
// the result of pthread_self() as std::this_thread::id() does.
int GetThreadID();

}
