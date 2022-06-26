#pragma once

#include "../common/std/cpplib/VS_Protocol.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>

#include <atomic>
#include <chrono>
#include <mutex>

class VS_RoamingSettings
	: public std::enable_shared_from_this<VS_RoamingSettings>
{
	boost::asio::io_service& m_ios;
	boost::asio::steady_timer m_timer;
	std::atomic<bool> m_should_run;

	std::vector<boost::regex> m_e;
	eRoamingMode_t m_roaming_mode;
	std::map<std::string, std::string>	m_sid_with_domain;		// key=server_host, value=domain
	std::mutex m_lock;

	void ScheduleTimer(std::chrono::steady_clock::duration delay);
	bool IsFound(const char *for_server_name);

public:
	void Start();
	void Stop();
	void SetRoamingSettings(const eRoamingMode_t mode, const std::string& params);

	bool IsRoamingAllowed(const char *for_server_name);
	eRoamingMode_t RoamingMode();

protected:
	VS_RoamingSettings(boost::asio::io_service &ios);
};
