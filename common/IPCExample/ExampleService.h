#pragma once

#include <atomic>
#include <memory>
#include <future>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

// Service that stores vector<int> and performs operations on it
class ExampleService : public std::enable_shared_from_this<ExampleService>
{
public:
	// typedefs for conveniance
	typedef std::vector<int> array_type;
	struct UsageStatistics
	{
		unsigned Append;
		unsigned Sort;
		unsigned GetAverage;
		unsigned Minus;
	};

	ExampleService(boost::asio::io_service& ios);

	// Synchronous request without a return value
	void Append(const array_type& v);
	void Sort();

	// Synchronous request with a return value
	float GetAverage();

	// Synchronous request that modifies argument passed by non-const reference
	void Minus(array_type& v);

	// Synchronous request that doesn't use unprotected data, and therefore don't require dispathing
	UsageStatistics GetStatistics();

	// Asynchronous versions of requests above
	// They can't pass arguments as efficently as synchronous request, but allow caller to do something else while they are executed
	// In this example:
	//   * Append should be preferred over Append_async because Append_async have to copy supplied vector.
	//   * both Sort and Sort_async are efficient because they don't have arguments, same for GetAverage/GetAverage_async.
	//   * Minus_async demostrates how synchronous call that takes non-const reference can be modified to make it asynchronous.
	//   * GetStatistics should be preferred over GetStatistics_async, because it's very fast.

	// Asynchronous request without a return value
	std::future<void> Append_async(const array_type& v);
	std::future<void> Sort_async();

	// Asynchronous request with a return value
	std::future<float> GetAverage_async();

	// Can't do asynchronous requestwith by non-const reference arguments
	// So Minus_async should have dirrerent API
	std::future<array_type> Minus_async(array_type&& v);

	// Asynchronous request that doesn't use unprotected data, and therefore can be run outside of strand
	// Only useful when it's a long operation
	std::future<UsageStatistics> GetStatistics_async();

	// Asynchronous notification
	void Stop();

private:
	// Implementations
	void Append_impl(std::promise<void>&& p, const array_type& v);
	void Sort_impl(std::promise<void>&& p);
	void GetAverage_impl(std::promise<float>&& p);
	void Minus_impl(std::promise<void>&& p, array_type& v);
	void Minus_impl_async(std::promise<array_type>&& p, array_type&& v);
	void GetStatistics_impl(std::promise<UsageStatistics>&& p);
	void Stop_impl();

private:
	// Reference to supplied io_service
	// Following asio convention reference is used instead of pointers (raw or smart)
	boost::asio::io_service& m_ios;
	// Object used to inform io_service that we are actively using it
	std::unique_ptr<boost::asio::io_service::work> m_work;
	// Object used to ensure implementation calls dispatched through it are run sequentially (not in parallel)
	boost::asio::io_service::strand m_strand;
	// Atomic variable used to inform all function that service is stopping and should stop using io_service
	std::atomic<bool> m_running;

	// Service-specific data
	array_type m_data;
	std::atomic<unsigned> m_calls_Append;
	std::atomic<unsigned> m_calls_Sort;
	std::atomic<unsigned> m_calls_GetAverage;
	std::atomic<unsigned> m_calls_Minus;
};
