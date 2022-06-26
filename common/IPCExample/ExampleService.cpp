#include "ExampleService.h"
#include "../std/cpplib/bind_tools.h"

#include "std-generic/compat/memory.h"
#include <algorithm>

ExampleService::ExampleService(boost::asio::io_service& ios)
	: m_ios(ios)
	, m_work(vs::make_unique<boost::asio::io_service::work>(m_ios))
	, m_strand(m_ios)
	, m_running(true)
	, m_calls_Append(0)
	, m_calls_Sort(0)
	, m_calls_GetAverage(0)
	, m_calls_Minus(0)
{
}

void ExampleService::Append(const array_type& v)
{
	// Check if we are stopped, and therefore shouldn't use io_service
	// if so return immediately (or maybe throw exception, or something else, it's up to each service to decide what is appropriate)
	if (!m_running)
		return;

	// Create future/promise pair that we will use to wait for handler completion (and retrieve result in requests that have one).
	std::promise<void> p;
	std::future<void> f(p.get_future());
	// Wrap call to _impl function using vs::forward_sync_call and dispatch it to strand.
	//
	// Using raw this pointer instead of shared_from_this(), because we are not leaving function until operation is completed
	// and our caller is responsible to keeping us alive.
	// All other arguments (other than this) will be forwarded to implementation in the most efficient manner:
	// In this case v will not be copied, and reference will be passed instead.
	//
	// If implentation functions can potentially call public methods of service (possibly through other services)
	// either they should be written to be reenterable or calls to dispatch() should be replaced with post() everywhere in the service.
	m_strand.dispatch(vs::forward_sync_call(&ExampleService::Append_impl, shared_from_this(), p, v));
	// Wait for handler to complete
	f.wait();
}

void ExampleService::Sort()
{
	if (!m_running)
		return;

	std::promise<void> p;
	std::future<void> f(p.get_future());
	m_strand.dispatch(vs::forward_sync_call(&ExampleService::Sort_impl, shared_from_this(), p));
	f.wait();
}

float ExampleService::GetAverage()
{
	if (!m_running)
		return 0.0f;

	std::promise<float> p;
	std::future<float> f(p.get_future());
	m_strand.dispatch(vs::forward_sync_call(&ExampleService::GetAverage_impl, shared_from_this(), p));
	// Wait for handler to complete and return value to caller.
	return f.get();
}

void ExampleService::Minus(array_type& v)
{
	if (!m_running)
		return;

	std::promise<void> p;
	std::future<void> f(p.get_future());
	m_strand.dispatch(vs::forward_sync_call(&ExampleService::Minus_impl, shared_from_this(), p, v));
	f.wait();
}

ExampleService::UsageStatistics ExampleService::GetStatistics()
{
	return {m_calls_Append, m_calls_Sort, m_calls_GetAverage, m_calls_Minus};
}

std::future<void> ExampleService::Append_async(const array_type& v)
{
	if (!m_running)
		return std::future<void>();

	std::promise<void> p;
	std::future<void> f(p.get_future());
	// Wrap call to _impl function using vs::forward_async_call and dispatch it to strand.
	//
	// Using shared_from_this(), because we are not waiting for operation to complete
	// and we must ensure that service won't be destroyed until operation is completed.
	// All other arguments (other than this) will be forwarded to implementation in the most efficient manner:
	// In this case v will be copied.
	//
	// dispatch() can be replaced with post() to make call even more asynchronous
	// (to guarantee that implementation wouldn't be executed right now in this thread).
	m_strand.dispatch(vs::forward_async_call(&ExampleService::Append_impl, shared_from_this(), p, v));
	// return future object to caller, so it can wait (or poll) on it when he wants.
	return std::move(f);
}

std::future<void> ExampleService::Sort_async()
{
	if (!m_running)
		return std::future<void>();

	std::promise<void> p;
	std::future<void> f(p.get_future());
	m_strand.dispatch(vs::forward_async_call(&ExampleService::Sort_impl, shared_from_this(), p));
	return std::move(f);
}

std::future<float> ExampleService::GetAverage_async()
{
	if (!m_running)
		return std::future<float>();

	std::promise<float> p;
	std::future<float> f(p.get_future());
	m_strand.dispatch(vs::forward_async_call(&ExampleService::GetAverage_impl, shared_from_this(), p));
	return std::move(f);
}

std::future<ExampleService::array_type> ExampleService::Minus_async(array_type&& v)
{
	if (!m_running)
		return std::future<array_type>();

	std::promise<array_type> p;
	std::future<array_type> f(p.get_future());
	m_strand.dispatch(vs::forward_async_call(&ExampleService::Minus_impl_async, shared_from_this(), p, v));
	return std::move(f);
}

std::future<ExampleService::UsageStatistics> ExampleService::GetStatistics_async()
{
	if (!m_running)
		return std::future<UsageStatistics>();

	std::promise<UsageStatistics> p;
	std::future<UsageStatistics> f(p.get_future());
	// Because this is a call that can be run outside of strand we are dispathing it directly on m_ios.
	// Note:
	// On Windows post/dispatch on io_service::strand if far more efficient than post/dispatch on io_service itself.
	// Also because implementation must not use unprotected data it can be called imme
	m_ios.dispatch(vs::forward_async_call(&ExampleService::GetStatistics_impl, shared_from_this(), p));
	return std::move(f);
}

void ExampleService::Stop()
{
	if (!m_running)
		return;

	// Because it's a notification and don't need to wait for completion and future/promise aren't needed
	m_strand.dispatch(vs::forward_async_call(&ExampleService::Stop_impl, shared_from_this()));
}

void ExampleService::Append_impl(std::promise<void>&& p, const array_type& v)
{
	// This function and all other functions that use m_data are executed is serialized manner,
	// and don't need to use mutex to access m_data.
	m_data.insert(m_data.end(), v.begin(), v.end());

	// Communicate with parts that are not running on strand using safe mechanism (like atomic variable)
	++m_calls_Append;

	// Set promise when we are done
	// If we don't do this then promise will be destroyed without ever receiving a value (or exception)
	// and wait operations of corresponding future will throw std::future_error with code std::future_errc::broken_promise.
	p.set_value();
}

void ExampleService::Sort_impl(std::promise<void>&& p)
{
	std::sort(m_data.begin(), m_data.end());
	++m_calls_Sort;
	p.set_value();
}

void ExampleService::GetAverage_impl(std::promise<float>&& p)
{
	float avg = 0;
	for (auto n : m_data)
		avg += n;
	avg /= m_data.size();
	++m_calls_GetAverage;

	// Store result in promise when we are done
	p.set_value(avg);
}

void ExampleService::Minus_impl(std::promise<void>&& p, array_type& v)
{
	for (size_t i = 0; i < m_data.size() && i < v.size(); ++i)
		v[i] -= m_data[i];
	++m_calls_Minus;
	p.set_value();
}

void ExampleService::Minus_impl_async(std::promise<array_type>&& p, array_type&& v)
{
	for (size_t i = 0; i < m_data.size() && i < v.size(); ++i)
		v[i] -= m_data[i];
	++m_calls_Minus;
	p.set_value(std::move(v));
}

void ExampleService::GetStatistics_impl(std::promise<UsageStatistics>&& p)
{
	// This function is executed outside of strand and mist not  use unprotected data (m_data in this case)
	p.set_value(GetStatistics());
}

void ExampleService::Stop_impl()
{
	// Inform public API that we are stopping
	m_running = false;
	// Destroy work, to release io_service
	m_work.reset();
}
