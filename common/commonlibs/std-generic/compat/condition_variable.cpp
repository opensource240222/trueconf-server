#ifdef _WIN32

#include "condition_variable.h"

#include <Windows.h>
#include <cassert>

static_assert(std::is_same<vs::condition_variable::native_handle_type, HANDLE>::value, "!");
static_assert(std::is_same<DWORD, unsigned long>::value, "!");

namespace vs
{
	condition_variable::condition_variable()
		: m_numWaiters(0)
	{
#if _WIN32_WINNT <= 0x0501
		m_sem = ::CreateSemaphore(0, 0, INT_MAX, 0);
#else
		m_sem = ::CreateSemaphoreEx(0, 0, INT_MAX, 0, 0, SEMAPHORE_ALL_ACCESS);
#endif

		static_assert(INFINITE == INFINITE_TIMEOUT, "!");
	}

	condition_variable::~condition_variable()
	{
		(void)::CloseHandle(m_sem);
	}

	void condition_variable::notify_all()
	{
		(void)::ReleaseSemaphore(m_sem, m_numWaiters.load(std::memory_order_acquire), 0);
	}

	void condition_variable::notify_one()
	{
		(void)::ReleaseSemaphore(m_sem, (m_numWaiters.load(std::memory_order_acquire) < 1 ? 0 : 1), 0);
	}

	std::cv_status condition_variable::wait_for_impl(std::unique_lock<std::mutex> &lock, unsigned long dwMilliseconds)
	{
		assert(lock.owns_lock());

		m_numWaiters.fetch_add(1, std::memory_order_release);
		lock.unlock();
		//For WaitForSingleObject(), there are no spurious wakeups, so you can eliminate the loop.
		//If you use WaitForMultipleObjectsEx() with bAlertable = TRUE, MsgWaitForMultipleObjects() with a wake mask,
		//or MsgWaitForMultipleObjectsEx() with bAlertable = TRUE or a wake mask, then the wait can end on other conditions before the event is actually signaled.
		const auto res = ::WaitForSingleObjectEx(m_sem, dwMilliseconds, TRUE);
		lock.lock();
		m_numWaiters.fetch_sub(1, std::memory_order_release);

		if (res == WAIT_FAILED)
		{
			throw std::system_error(::GetLastError(), std::system_category(), "WaitForSingleObjectEx");
		}

		return res == WAIT_TIMEOUT ? std::cv_status::timeout : std::cv_status::no_timeout;
	}
} //namespace vs

#endif //_WIN32

