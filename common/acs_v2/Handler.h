#pragma once

#include "net/UDPRouter.h"
#include "std-generic/cpplib/string_view.h"
#include "std/cpplib/BlockQueue.h"
#include "Responses.h"

#include <boost/asio/ip/tcp.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace acs{

class ConnectionTCP;
class ConnectionUDP;
class Service;

class Handler
{
public:
	using stream_buffer = std::vector<uint8_t>;
	using packet_buffer = vs::BlockQueue;

	virtual ~Handler();
	string_view Name() const { return m_name; };

	virtual bool Init(string_view name);
	virtual Response Protocol(const stream_buffer& buffer, unsigned channel_token = 0);
	virtual Response Protocol(const packet_buffer& buffer, unsigned channel_token = 0);
	virtual void Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer);
	virtual void Accept(net::UDPConnection&& connection, packet_buffer&& buffer);

private:
	std::string m_name;

	// These fields are used internally by Service, classes derived from Handler shouldn't modify them.
	unsigned m_processed_connections = 0;
	unsigned m_accepted_connections = 0;
	friend class ConnectionTCP;
	friend class ConnectionUDP;
	friend class Service;
};

}
