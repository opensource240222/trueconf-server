#pragma once

#include "../../streams/fwd.h"
#include "std-generic/cpplib/string_view.h"

#include <boost/asio/ip/tcp.hpp>

#include <chrono>
#include <memory>
#include <string>

namespace stream {

template <class Socket>
class RouterConference;

class RouterInternalInterface
{
public:
	using conference_ptr = std::shared_ptr<RouterConference<boost::asio::ip::tcp::socket>>;

	virtual ~RouterInternalInterface() {};

	virtual ConferencesConditions* GetCCS() = 0;
	virtual const std::string& GetEndpointName() const = 0;
	virtual const std::string& GetLogDirectory() const = 0;

	virtual conference_ptr GetConference(string_view conference_name) = 0;
	virtual void DeregisterConference(string_view conference_name) = 0;
	virtual void NotifyRead(size_t bytes) = 0;
	virtual void NotifyWrite(size_t bytes) = 0;
	virtual void Timer(std::chrono::steady_clock::time_point now) = 0;
};

}
