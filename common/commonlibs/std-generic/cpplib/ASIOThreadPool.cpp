#include "std-generic/cpplib/ASIOThreadPool.h"
#include "std-generic/cpplib/ThreadUtils.h"

#include <cassert>

namespace vs {

ASIOThreadPool::ASIOThreadPool(unsigned n_threads, string_view thread_name_prefix)
	: m_n_threads(n_threads != 0 ? n_threads : std::thread::hardware_concurrency())
	, m_thread_name_prefix(!thread_name_prefix.empty() ? thread_name_prefix : "ASIO")
	, m_ios(m_n_threads)
{
}

ASIOThreadPool::~ASIOThreadPool()
{
	Stop();
}

void ASIOThreadPool::Start()
{
	assert(!m_work);
	assert(m_threads.empty());

	const int max_id_len =
		m_n_threads < 10         ? 1 :
		m_n_threads < 100        ? 2 :
		m_n_threads < 1000       ? 3 :
		m_n_threads < 10000      ? 4 :
		m_n_threads < 100000     ? 5 :
		m_n_threads < 1000000    ? 6 :
		m_n_threads < 10000000   ? 7 :
		m_n_threads < 100000000  ? 8 :
		m_n_threads < 1000000000 ? 9 :
		10;
	const int max_prefix_len = 15 - 1/*space*/ - max_id_len;

	// Reset io_service in case we were stopped before
	m_ios.reset();

	m_work.emplace(m_ios);
	m_threads.reserve(m_n_threads);
	for (unsigned i = 0; i < m_n_threads; ++i)
		m_threads.emplace_back([&, i, max_prefix_len]
		{
			char name[16];
			snprintf(name, sizeof(name), "%.*s %u", max_prefix_len, m_thread_name_prefix.c_str(), i);
			vs::SetThreadName(name);
			vs::FixThreadSystemMessages();
			OnThreadStart();
			m_ios.run();
			OnThreadExit();
		});
}

void ASIOThreadPool::Stop()
{
	if (!m_work)
		return;

	// Allow io_service to stop when we are done
	m_work.reset();

	// Wait for threads to terminate
	for (auto& thread : m_threads)
		thread.join();
	m_threads.clear();
}

void ASIOThreadPool::OnThreadStart()
{
}

void ASIOThreadPool::OnThreadExit()
{
}

}
