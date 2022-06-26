#pragma once

#include "net/Address.h"
#include "net/Port.h"
#include "net/Protocol.h"

namespace net{
struct Endpoint {
	Endpoint() {}
	Endpoint(const net::address& a, net::port p, net::protocol proto) :addr(a), port(p), protocol(proto) {}
	net::address	addr = {};
	net::port		port = 0;
	net::protocol	protocol = protocol::none;

	friend bool operator<(const Endpoint& ep1, const Endpoint& ep2) {
		return std::tie(ep1.addr, ep1.port, ep1.protocol) < std::tie(ep2.addr, ep2.port, ep2.protocol);
	}
	friend bool operator==(const Endpoint& ep1, const Endpoint& ep2) {
		return std::tie(ep1.addr, ep1.port, ep1.protocol) == std::tie(ep2.addr, ep2.port, ep2.protocol);
	}
	friend bool operator!=(const Endpoint& ep1, const Endpoint& ep2) {
		return !operator==(ep1, ep2);
	}

	template <typename Elem, typename Traits>
	friend std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>&out, const net::Endpoint& ep) {
		switch (ep.protocol)
		{
		case net::protocol::TCP:
			out << "TCP:";
			break;
		case net::protocol::UDP:
			out << "UDP:";
			break;
		case net::protocol::TLS:
			out << "TLS:";
			break;
		case net::protocol::none:
		default:
			out << "NONE:";
			break;
		}
		out << ep.addr << ":" << ep.port;
		return out;
	}
};


}