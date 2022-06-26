#include "std-generic/cpplib/ThreadUtils.h"

#if defined(_WIN32)
#	include "std-generic/cpplib/scope_exit.h"
#	include "std-generic/cpplib/utf8.h"

#	include <Windows.h>
#elif defined(__unix__)
#	include <pthread.h>
#	if defined (__FreeBSD__)
#		include <pthread_np.h>
#	endif
#	include <sched.h>
#	include <unistd.h>
#elif defined(IOS_BUILD)
#    include <pthread.h>
#endif

#include "std-generic/compat/chrono.h"
#include <cassert>
#include <cstring>

namespace vs {

#if defined(_WIN32)

static bool SetThreadName_Win10(const char* name)
{
	HINSTANCE lib = ::LoadLibrary("Kernel32.dll");
	if (!lib)
		return false;;
	VS_SCOPE_EXIT { ::FreeLibrary(lib); };

	using SetThreadDescription_t = HRESULT (WINAPI *)(HANDLE hThread, PCWSTR lpThreadDescription);
	auto pSetThreadDescription = reinterpret_cast<SetThreadDescription_t>(::GetProcAddress(lib, "SetThreadDescription"));
	if (!pSetThreadDescription)
		return false;

	const auto wname = vs::UTF8ToWideCharConvert(name);
	if (name[0] && wname.empty())
		return false;

	return SUCCEEDED(pSetThreadDescription(::GetCurrentThread(), wname.c_str()));
}

#pragma pack(push, 8)
struct THREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
};
#pragma pack(pop)

static void SetThreadName_Debugger(const char* name)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = -1;
    info.dwFlags = 0;

#pragma warning(push)
#pragma warning(disable: 6320 6322)
	static const DWORD MS_VC_EXCEPTION = 0x406D1388;
    __try { RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
#pragma warning(pop)
}

#endif

namespace detail {

void SetThreadName(const char* name)
{
	assert(name);
	if (!name)
		return;
	assert(std::strlen(name) < 16);

#if defined(_WIN32)
	if (SetThreadName_Win10(name))
		return;
	if (::IsDebuggerPresent() != FALSE)
		SetThreadName_Debugger(name);
#elif defined(__FreeBSD__)
    pthread_set_name_np(pthread_self(), name);
#elif defined(__unix__) && !defined(IOS_BUILD) && !defined(__APPLE__)
	pthread_setname_np(pthread_self(), name);
#endif
}

}

void FixThreadSystemMessages()
{
#if defined(_WIN32)
	SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
#endif
}

void SleepFor(std::chrono::nanoseconds nano)
{
#ifdef _WIN32
	::Sleep(vs::chrono::ceil<std::chrono::milliseconds>(nano).count());
#else
	std::this_thread::sleep_for(nano);
#endif
}

namespace thread
{
#ifdef _WIN32

	namespace
	{
		int MapPriority(vs::thread::priority priority)
		{
			switch (priority) {
			case vs::thread::high:
				return THREAD_PRIORITY_ABOVE_NORMAL;
			case vs::thread::highest:
				return THREAD_PRIORITY_HIGHEST;
			case vs::thread::critical:
				return THREAD_PRIORITY_TIME_CRITICAL;
			case vs::thread::normal:
			default:
				break;
			}
			return THREAD_PRIORITY_NORMAL;
		}
	}//namespace empty

	bool SetPriority(std::thread& thread, vs::thread::priority priority)
	{
		return 0 != SetThreadPriority(thread.native_handle(), MapPriority(priority));
	}

#elif defined (__unix__)

	namespace
	{
		/*must be SCHED_FIFO | SCHED_OTHER | SCHED_RR*/
		constexpr int c_sched_policy = SCHED_RR;

		int MapPriority(vs::thread::priority priority)
		{
			int priority_min = sched_get_priority_min(c_sched_policy),
				priority_max = sched_get_priority_max(c_sched_policy);

			if (priority_min == -1 || priority_max == -1)
				return -1;

			switch (priority) {
			case vs::thread::high:
				return priority_min + 2 * (priority_max - priority_min) / 3;
			case vs::thread::highest:
				return priority_min + 3 * (priority_max - priority_min) / 4;
			case vs::thread::critical:
				return priority_max;
			case vs::thread::normal:
				return 0;
			default:
				return -1;
			}
		}
	}//namespace empty

	bool SetPriority(std::thread& thread, vs::thread::priority priority)
	{
		if (priority == vs::thread::normal)
			return true;

		struct sched_param par = sched_param();
		par.sched_priority = MapPriority(priority);

		if (par.sched_priority == -1)
			return false;

		return 0 != pthread_setschedparam(thread.native_handle(), c_sched_policy, &par);
	}

#else

	bool SetPriority(std::thread& thread, vs::thread::priority priority)
	{
		return false;
	}

#endif
}//namespace thread

}
