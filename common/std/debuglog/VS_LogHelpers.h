#pragma once

#include <string>
#include "net/Endpoint.h"
#include "net/UDPRouter.h"
#include <boost/asio/ip/basic_endpoint.hpp>

// log helper
namespace logh {

	template<class Stream, class Endpoint>
	void PrintEndpoints(Stream &&out, const Endpoint &ep1, const Endpoint &ep2) {
		if (ep1.address().is_unspecified())
			out << "unknown";
		else
			out << ep1;

		out << " <-> ";

		if (ep2.address().is_unspecified())
			out << "unknown";
		else
			out << ep2;
	}

	template<class Stream, class Socket>
	void PrintSocketEndpoints(Stream &&out, Socket && s) {
		boost::system::error_code ec;
		PrintEndpoints(out, s.local_endpoint(ec), s.remote_endpoint(ec));
	}

	template<class Socket>
	std::string GetSocketEndpointsStr(Socket && s)
	{
		std::stringstream ss;
		PrintSocketEndpoints(ss, s);
		return ss.str();
	}
}