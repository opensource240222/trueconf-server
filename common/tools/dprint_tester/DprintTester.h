#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <utility>
#include <chrono>

#include <cstdint>
#include <cstddef>

class DprintTester
{
public:
	struct Report {
		Report(void);

		uint_least64_t m_call_count;
		std::chrono::microseconds m_real_execution_time;
		std::chrono::microseconds m_total_execution_time;

		std::chrono::microseconds m_max_dprint_execution_time;
		char *m_max_dprint_execution_time_fmt;

		std::chrono::microseconds m_min_dprint_execution_time;
		char *m_min_dprint_execution_time_fmt;
	};
public:
	DprintTester();
	virtual ~DprintTester();

	bool Init(size_t n_threads);
	void Start(void);
	void Stop(void);
	std::chrono::microseconds GetTotalExecutionTime();
	void GatherReport(Report &ret);
private:
	inline void WorkerBody(const unsigned int number, Report &report);
private:
	unsigned int m_n_threads;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_stop_time;

	bool m_initialized;
	std::vector<std::tuple<std::thread, Report>> m_tasks;

	std::mutex m_lock; // for signal handling
	std::atomic_bool m_stop_flag;
};
