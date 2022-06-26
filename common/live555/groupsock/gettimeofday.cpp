#if defined(__WIN32__) || defined(_WIN32)

#ifndef _GROUPSOCK_HELPER_HH
#include "GroupsockHelper.hh"
#endif

#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")
#include <sys/timeb.h>

#include <atomic>
#include <memory>
#include <thread>

class gettimeofday_impl
{
public:
	gettimeofday_impl()
	{
		// Increace precission to maximum available
		timeBeginPeriod(1);

		// Wait until new tick begins
		unsigned int tgt = timeGetTime();
		while (tgt == (m_start_tgt = timeGetTime()))
			;

		// Get time since required base (1 Jan 1970)
		struct timeb tb;
		ftime(&tb);
		m_start_tv.tv_sec  = tb.time;
		m_start_tv.tv_usec = tb.millitm*1000;
	}

	~gettimeofday_impl()
	{
		// Restore old (?) precission
		timeEndPeriod(1);
	}

	int operator()(struct timeval* tp, int*) const
	{
		unsigned int tgt = timeGetTime();
		tp->tv_sec  = m_start_tv.tv_sec  +  (tgt-m_start_tgt)/1000;
		tp->tv_usec = m_start_tv.tv_usec + ((tgt-m_start_tgt)%1000)*1000;
		tp->tv_sec  += tp->tv_usec/1000000;
		tp->tv_usec %= 1000000;
		return 0;
	}

private:
	struct timeval m_start_tv;
	unsigned int m_start_tgt;
};

int gettimeofday(struct timeval* tp, int* tz)
{
	static std::unique_ptr<gettimeofday_impl> impl /*(new gettimeofday_impl)*/ ;

	// MSVC doesn't support (yet) thread safe local static initialization, so we do it manually
	static std::atomic<bool> init_done(false);
	static std::atomic_flag init_started = ATOMIC_FLAG_INIT;
	if (!init_done.load(std::memory_order_acquire))
	{
		if (!init_started.test_and_set())
		{
			impl.reset(new gettimeofday_impl);
			init_done.store(true, std::memory_order_release);
		}
		else
			while (!init_done.load(std::memory_order_acquire))
				std::this_thread::yield();
	}

	return (*impl)(tp, tz);
}

#endif
