#include "ExampleService.h"

#include <boost/asio/io_service.hpp>

#include "std-generic/compat/memory.h"
#include <thread>
#include <vector>

#include <algorithm>
#include <iostream>
#include <random>
#include <iterator>

void do_random_stuff(ExampleService* svc)
{
	std::random_device rd;
	std::mt19937 rnd(rd());
	std::uniform_int_distribution<ExampleService::array_type::value_type> dist(-1000, 1000);

	ExampleService::array_type v1{ 1, 2, 3, 5, 7, 11, 13, 17 };
	std::cout << "v1: ";
	std::copy(v1.begin(), v1.end(), std::ostream_iterator<ExampleService::array_type::value_type>(std::cout, ", "));
	std::cout << '\n';
	svc->Append(v1);
	auto avg = svc->GetAverage();
	std::cout << "average: " << avg << '\n';

	v1.resize(100);
	std::generate(v1.begin(), v1.end(), [&rnd, &dist]() { return dist(rnd); });
	std::cout << "v1: ";
	std::copy(v1.begin(), v1.end(), std::ostream_iterator<ExampleService::array_type::value_type>(std::cout, ", "));
	std::cout << '\n';
	auto f1 = svc->Append_async(v1);
	auto f2 = svc->Sort_async();
	// We can use v1 because async call made a copy

	// Do someting else while async calls are being handeled
	std::generate(v1.begin(), v1.end(), [&rnd, &dist]() { return dist(rnd); });
	std::cout << "v1: ";
	std::copy(v1.begin(), v1.end(), std::ostream_iterator<ExampleService::array_type::value_type>(std::cout, ", "));
	std::cout << '\n';

	// Wait for them to complete
	f1.wait();
	f2.wait();
	avg = svc->GetAverage();
	std::cout << "average: " << avg << '\n';

	svc->Minus(v1);
	std::cout << "v1: ";
	std::copy(v1.begin(), v1.end(), std::ostream_iterator<ExampleService::array_type::value_type>(std::cout, ", "));
	std::cout << '\n';

	auto stats = svc->GetStatistics();
	std::cout << "stats: {Append: " << stats.Append << ", Sort: " << stats.Sort << ", GetAverage: " << stats.GetAverage << ", Minus: " << stats.Minus << "}\n";

	svc->Stop();
}

int main()
{
	// Create common io_service, it will be used to handle all I/O and execute all IPC calls for all services
	boost::asio::io_service ios(std::thread::hardware_concurrency());

	// Create temporary io_service::work to prevent threads from terminating while we are setting up our services
	auto work = vs::make_unique<boost::asio::io_service::work>(ios);

	// Start thread pool that will run io_service handlers
	std::vector<std::thread> threads;
	for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i)
		threads.emplace_back(static_cast<size_t(boost::asio::io_service::*)()>(&boost::asio::io_service::run), std::ref(ios));

	// Start services
	auto example_service = std::make_shared<ExampleService>(ios);

	// Destroy work object after we started all services, they should have their own work objects while they are running
	work.reset();

	// In example we use service right from main(), in real server they will act on network requests
	do_random_stuff(example_service.get());

	// Wait for threads to terminate
	for (auto& thread : threads)
		thread.join();
}
