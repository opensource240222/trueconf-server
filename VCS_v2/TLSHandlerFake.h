#pragma once

#include "acs_v2/Handler.h"

namespace vs {

class TLSHandlerFake : public acs::Handler
{
public:
	// acs::Handler
	acs::Response Protocol(const stream_buffer& buffer, unsigned channel_token) override;
	void Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer) override;
};

}
