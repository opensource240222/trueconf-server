#pragma once

#include "std-generic/cpplib/string_view.h"

#include <boost/asio/io_service.hpp>
#include <boost/optional/optional.hpp>

#include <string>
#include <thread>
#include <vector>

namespace vs {

class ASIOThreadPool
{
public:
	ASIOThreadPool(unsigned n_threads = 0, string_view thread_name_prefix = {});
	virtual ~ASIOThreadPool();

	boost::asio::io_service& get_io_service()
	{
		return m_ios;
	}

	void Start();
	void Stop();

private:
	virtual void OnThreadStart();
	virtual void OnThreadExit();

private:
	const unsigned m_n_threads;
	const std::string m_thread_name_prefix;
	boost::asio::io_service m_ios;
	boost::optional<boost::asio::io_service::work> m_work;
	std::vector<std::thread> m_threads;
};

}
