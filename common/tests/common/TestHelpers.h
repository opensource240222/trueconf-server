#pragma once

#include "std/cpplib/event.h"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <utility>

namespace test {

inline ::testing::AssertionResult WaitFor(const char* operation, const vs::event& ev, const unsigned wait_timeout_ms = 1000)
{
	if (ev.wait_for(std::chrono::milliseconds(wait_timeout_ms)))
		return ::testing::AssertionSuccess();
	else
		return ::testing::AssertionFailure() << operation << " wasn't completed in time.";
}

template <class F, class = decltype(std::declval<F>()())>
inline ::testing::AssertionResult WaitFor(const char* operation, F&& f,
	const unsigned poll_interval_ms = 10, const unsigned wait_timeout_ms = 1000)
{
	for (auto remaining = wait_timeout_ms; remaining > 0; remaining -= poll_interval_ms)
	{
		if (std::forward<F>(f)())
			return ::testing::AssertionSuccess();
		std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
	}
	return ::testing::AssertionFailure() << operation << " wasn't completed in time.";
}

}
