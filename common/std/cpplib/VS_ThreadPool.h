#pragma once

#include "fast_mutex.h"
#include "function.h"
#include "MakeShared.h"

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "std-generic/compat/condition_variable.h"

class VS_ThreadPool
{
public:
	typedef vs::function<void()> task_type;

	explicit VS_ThreadPool(unsigned int n_threads = 0);
	~VS_ThreadPool();

	void Start(unsigned int n_threads = 0);
	void Stop(bool graceful = true);

	void Post(task_type&& task);
	template <class Callable>
	void Post(Callable&& task)
	{
		Post(task_type(std::forward<Callable>(task)));
	}

	void Post(task_type&& task, std::chrono::steady_clock::duration delay)
	{
		Post(std::move(task), std::chrono::steady_clock::now() + delay);
	}
	template <class Callable>
	void Post(Callable&& task, std::chrono::steady_clock::duration delay)
	{
		Post(task_type(std::forward<Callable>(task)), delay);
	}

	void Post(task_type&& task, std::chrono::steady_clock::time_point call_time);
	template <class Callable>
	void Post(Callable&& task, std::chrono::steady_clock::time_point call_time)
	{
		Post(task_type(std::forward<Callable>(task)), call_time);
	}

	class Strand
	{
	public:
		explicit Strand(VS_ThreadPool& thread_pool)
			: m_impl(vs::MakeShared<Impl>(thread_pool))
		{
		}
		Strand(const VS_ThreadPool&) = delete;
		Strand(VS_ThreadPool&&) = delete;
		Strand& operator=(const VS_ThreadPool&) = delete;
		Strand& operator=(VS_ThreadPool&&) = delete;

		void Post(task_type&& task)
		{
			m_impl->Post(std::move(task));
		}
		template <class Callable>
		void Post(Callable&& task)
		{
			Post(task_type(std::forward<Callable>(task)));
		}

	private:
		class Impl : public std::enable_shared_from_this<Impl>
		{
		public:
			~Impl();

			void Post(task_type&& task);

		protected:
			explicit Impl(VS_ThreadPool& thread_pool);

		private:
			void Invoke();

			VS_ThreadPool& m_thread_pool;
			task_type m_task;
			vs::fast_mutex m_mutex;
			bool m_running;
			std::queue<task_type> m_waiting_tasks;
		};
		std::shared_ptr<Impl> m_impl;
	};

private:
	void ThreadFunc();

	struct delayed_task
	{
		std::chrono::steady_clock::time_point time;
		task_type task;

		delayed_task(std::chrono::steady_clock::time_point time_, task_type&& task_)
			: time(time_)
			, task(std::move(task_))
		{
		}
		delayed_task(const delayed_task&) = delete;
		delayed_task(delayed_task&& x)
			: time(std::move(x.time))
			, task(std::move(x.task))
		{
		}
		delayed_task& operator=(const delayed_task&) = delete;
		delayed_task& operator=(delayed_task&& x)
		{
			if (&x == this)
				return *this;
			time = std::move(x.time);
			task = std::move(x.task);
			return *this;
		}

		struct priority_less
		{
			bool operator()(const delayed_task& lhs, const delayed_task& rhs) const;
		};
	};

	std::vector<std::thread> m_threads;
	std::mutex m_mutex;
	vs::condition_variable m_cv;
	bool m_running;
	unsigned int m_waiting_threads;
	std::queue<task_type> m_tasks;
	std::priority_queue<delayed_task, std::vector<delayed_task>, delayed_task::priority_less> m_delayed_tasks;
};

