#pragma once

#include <string>
#include "net/Address.h"
#include "std-generic/asio_fwd.h"

namespace net {
	std::string GetRTPInterface();
	address GetRTPAddress(boost::asio::io_service& ios);
}