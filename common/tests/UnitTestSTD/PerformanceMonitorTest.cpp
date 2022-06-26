#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_PerformanceMonitor.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/scope_exit.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

TEST(PerformanceMonitor, QuickStop) {
	VS_RegistryKey(false, CONFIGURATION_KEY).SetValueI32(30 * 1000, "PerformanceMonitorTimeout");
	VS_SCOPE_EXIT { VS_RegistryKey(false, CONFIGURATION_KEY).RemoveValue("PerformanceMonitorTimeout"); };
	const auto start_time = std::chrono::steady_clock::now();
	{
		VS_PerformanceMonitor pm;
		pm.Start();
		pm.Stop();
	}
	const auto allowed_for_waiting = std::chrono::seconds(3);
	const auto time_diff = std::chrono::steady_clock::now() - start_time;
	EXPECT_LE(time_diff, allowed_for_waiting);
}

TEST(PerformanceMonitor, FullLoad) {
	// Create separate VS_PerformanceMonitor for test, so we can create load before it starts measurements.
	VS_PerformanceMonitor pm;

	// Start load threads
	std::atomic<bool> should_run(true);
	std::vector<std::thread> load_threads;
	for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i)
		load_threads.emplace_back([&should_run]() {
			vs::SetThreadName("T:PerfMonitor");
			volatile double x = 0.;
			while (should_run)
				for (unsigned i = 0; i < 10000; ++i)
					x += sqrt(i);
		});

	// Poll for measurement
	pm.Start();
	double total_processor_time = 0.;
	while (total_processor_time == 0.)
	{
		total_processor_time = pm.GetTotalProcessorTime();
		vs::SleepFor(std::chrono::milliseconds(10));
	}

	// Stop load threads
	should_run = false;
	for (auto& x : load_threads)
		x.join();

	EXPECT_LE(total_processor_time, 100.);
	EXPECT_GE(total_processor_time, 98.);
}

