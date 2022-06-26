#include "VS_ThreadPool.h"
#include "std/cpplib/ThreadUtils.h"

bool VS_ThreadPool::delayed_task::priority_less::operator()(const delayed_task& lhs, const delayed_task& rhs) const
{
	return lhs.time > rhs.time;
}

VS_ThreadPool::VS_ThreadPool(unsigned int n_threads)
	: m_running(false)
	, m_waiting_threads(0)
{
	Start(n_threads);
}

VS_ThreadPool::~VS_ThreadPool()
{
	Stop(false);
}

void VS_ThreadPool::Start(unsigned int n_threads)
{
	if (n_threads == 0)
		n_threads = std::thread::hardware_concurrency();
	if (n_threads == 0)
		n_threads = 4;

	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		if (m_running)
			return;
		m_running = true;
	}

	assert(m_threads.empty());
	for (unsigned i = 0; i < n_threads; ++i)
		m_threads.emplace_back(std::bind(&VS_ThreadPool::ThreadFunc, this));
}

void VS_ThreadPool::Stop(bool graceful)
{
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		if (!m_running)
			return;
		m_running = false;
	}
	m_cv.notify_all();

	for (std::thread& thread: m_threads)
		thread.join();
	m_threads.clear();

	if (graceful)
	{
		std::queue<task_type> tasks;
		while (true)
		{
			{
				std::lock_guard<decltype(m_mutex)> lock(m_mutex);
				if (m_tasks.empty())
					break;
				tasks.swap(m_tasks);
			}
			while (!tasks.empty())
			{
				assert(tasks.front());
				tasks.front()();
				tasks.pop();
			}
			// Executed tasks might have created new tasks, so we have to repeat
		}
	}
}

void VS_ThreadPool::Post(task_type&& task)
{
	if (!task)
		return;

	bool need_notify;
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		need_notify = m_waiting_threads > 0;
		m_tasks.push(std::move(task));
	}
	if (need_notify)
		m_cv.notify_one();
}

void VS_ThreadPool::Post(task_type&& task, std::chrono::steady_clock::time_point call_time)
{
	if (!task)
		return;

	bool need_notify;
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		const bool new_task_is_first = m_delayed_tasks.empty() || m_delayed_tasks.top().time > call_time;;
		need_notify = m_waiting_threads > 0 && new_task_is_first;
		m_delayed_tasks.emplace(call_time, std::move(task));
	}
	if (need_notify)
		m_cv.notify_one();
}

void VS_ThreadPool::ThreadFunc()
{
	vs::SetThreadName("ThreadPool");

	task_type task;
	while (true)
	{
		std::unique_lock<decltype(m_mutex)> lock(m_mutex);
		if (!m_running)
			return;
		while (m_tasks.empty() && (m_delayed_tasks.empty() || m_delayed_tasks.top().time > std::chrono::steady_clock::now()))
		{
			++m_waiting_threads;
			if (m_delayed_tasks.empty())
				m_cv.wait(lock);
			else
				m_cv.wait_until(lock, m_delayed_tasks.top().time);
			--m_waiting_threads;
			if (!m_running)
				return;
		}
		if (!m_tasks.empty())
		{
			task = std::move(m_tasks.front());
			m_tasks.pop();
		}
		else
		{
			task = std::move(m_delayed_tasks.top().task);
			m_delayed_tasks.pop();
		}
		lock.unlock();
		assert(task);
		task();
		task = nullptr;
	}
}

VS_ThreadPool::Strand::Impl::Impl(VS_ThreadPool& thread_pool)
	: m_thread_pool(thread_pool)
	, m_running(false)
{
}

VS_ThreadPool::Strand::Impl::~Impl()
{
	assert(m_waiting_tasks.empty());
	assert(!m_task);
}

void VS_ThreadPool::Strand::Impl::Post(task_type&& task)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	if (m_running)
		m_waiting_tasks.push(std::move(task));
	else
	{
		assert(m_waiting_tasks.empty());
		assert(!m_task);
		m_task = std::move(task);
		m_running = true;
		m_thread_pool.Post(std::bind(&Impl::Invoke, shared_from_this()));
	}
}

void VS_ThreadPool::Strand::Impl::Invoke()
{
	assert(m_task);
	m_task();
	m_task = nullptr;
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	if (!m_waiting_tasks.empty())
	{
		m_task = std::move(m_waiting_tasks.front());
		m_waiting_tasks.pop();
		m_thread_pool.Post(std::bind(&Impl::Invoke, shared_from_this()));
	}
	else
		m_running = false;
}