#pragma once

#include "../../acs_v2/Handler.h"

namespace stream {

class RouterInternalInterface;

class Handler : public acs::Handler
{

public:
	explicit Handler(RouterInternalInterface* router);
	~Handler();

	acs::Response Protocol(const acs::Handler::stream_buffer& buffer, unsigned channel_token = 0) override;
	void Accept(boost::asio::ip::tcp::socket&& socket, acs::Handler::stream_buffer&& buffer) override;

private:
	RouterInternalInterface* m_router;
};

}
