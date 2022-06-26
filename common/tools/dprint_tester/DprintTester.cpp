#include "DprintTester.h"
#include "std/cpplib/ThreadUtils.h"

#include <iostream>
#include <fstream>

#include <functional>
#include <random>

/*#include "../../std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_ALL_
*/

#include "dprint_samples.h"

DprintTester::Report::Report()
: m_call_count(0), m_max_dprint_execution_time_fmt(nullptr), m_min_dprint_execution_time_fmt(nullptr),
m_real_execution_time(-1), m_max_dprint_execution_time(-1), m_min_dprint_execution_time(-1)
{}

DprintTester::DprintTester()
: m_n_threads(0), m_initialized(false)
{
	m_stop_flag = false;
}

DprintTester::~DprintTester()
{
}

bool DprintTester::Init(size_t n_threads)
{
	std::lock_guard<std::mutex> lock(m_lock);
	if (m_initialized)
		return false;

	m_initialized = true;

	m_n_threads = n_threads;
	m_tasks.resize(n_threads);
	return true;
}

void DprintTester::Start(void)
{
	{
		std::lock_guard<std::mutex> lock(m_lock);
		if (m_stop_flag) // avoid calls to Start more than once
			return;

		// save start time
		m_start_time = std::chrono::high_resolution_clock::now();
		// start workers
		for (size_t n = 0; n < m_n_threads; n++)
		{
			std::get<0>(m_tasks[n]) = std::thread([this, n](void) ->void {
				vs::SetThreadName("Worker");
				auto &report = std::get<1>(m_tasks[n]);
				WorkerBody(n, report);
			});
		}
		m_stop_time = std::chrono::high_resolution_clock::now();
	}

	// wait for all worker threads ending
	for (auto &v : m_tasks)
	{
		auto &thread = std::get<0>(v);
		thread.join();
	}

	// save stop time
	auto stop_time = std::chrono::high_resolution_clock::now();
	{
		std::lock_guard<std::mutex> lock(m_lock);
		m_stop_time = stop_time;
    }

}

void DprintTester::Stop()
{
	std::lock_guard<std::mutex> lock(m_lock);
	m_stop_flag = true;
}

void DprintTester::GatherReport(Report &ret)
{
	std::lock_guard<std::mutex> lock(m_lock);
	ret.m_total_execution_time = GetTotalExecutionTime();

	for (auto &v : m_tasks)
	{
		auto &report = std::get<1>(v);

		ret.m_call_count += report.m_call_count;
		ret.m_real_execution_time += report.m_real_execution_time;

		if ((report.m_max_dprint_execution_time > ret.m_max_dprint_execution_time) || ret.m_max_dprint_execution_time.count() == -1)
		{
			ret.m_max_dprint_execution_time = report.m_max_dprint_execution_time;
			ret.m_max_dprint_execution_time_fmt = report.m_max_dprint_execution_time_fmt;
		}

		if ((report.m_min_dprint_execution_time < ret.m_min_dprint_execution_time) || ret.m_min_dprint_execution_time.count() == -1)
		{
			ret.m_min_dprint_execution_time = report.m_min_dprint_execution_time;
			ret.m_min_dprint_execution_time_fmt = report.m_min_dprint_execution_time_fmt;
		}
	}
}

std::chrono::microseconds DprintTester::GetTotalExecutionTime()
{
	auto total = (m_stop_time - m_start_time);
	return std::chrono::duration_cast<std::chrono::microseconds>(total);
}

void DprintTester::WorkerBody(const unsigned int number, Report &report)
{
	char *fmt = nullptr;
	int choice = 0;
	std::knuth_b rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, get_dprint_samples_count() - 1);

	uint_least64_t samples_count = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> sample_start;
	std::chrono::time_point<std::chrono::high_resolution_clock> sample_stop;

	std::chrono::microseconds real_execution_time(0);
	std::chrono::microseconds min_duration(-1);
	std::chrono::microseconds max_duration(-1);
	std::chrono::microseconds call_duration(-1);

	char *min_format = nullptr, *max_format = nullptr;

	while (!m_stop_flag)
	{
		choice = dist(mt);
		sample_start = std::chrono::high_resolution_clock::now();
		// begin sampling

		run_dprint_sample(choice, fmt);
		///vs::SleepFor(std::chrono::milliseconds(100)); // simulate some work
		//dprint0("Hello, World from %u thread!\n", number);

		// end sampling
		sample_stop = std::chrono::high_resolution_clock::now();

		call_duration = std::chrono::duration_cast<std::chrono::microseconds>(sample_stop - sample_start);
		real_execution_time += call_duration;

		if ((call_duration < min_duration) || min_duration.count() == -1)
		{
			min_format = fmt;
			min_duration = call_duration;
		}

		if ((call_duration > max_duration) || max_duration.count() == -1)
		{
			max_format = fmt;
			max_duration = call_duration;
		}
		samples_count++;
	}

	// save results
	report.m_call_count = samples_count;
	report.m_real_execution_time = real_execution_time;
	report.m_max_dprint_execution_time = max_duration;
	report.m_max_dprint_execution_time_fmt = max_format;
	report.m_min_dprint_execution_time = min_duration;
	report.m_min_dprint_execution_time_fmt = min_format;
}
