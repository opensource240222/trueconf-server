#include "Handler.h"

namespace acs {

Handler::~Handler()
{
}

bool Handler::Init(string_view name)
{
	m_name.assign(name.data(), name.size());
	return true;
}

auto Handler::Protocol(const stream_buffer& /*buffer*/, unsigned /*channel_token*/) -> Response
{
	return Response::not_my_connection;
}

auto Handler::Protocol(const packet_buffer& /*buffer*/, unsigned /*channel_token*/) -> Response
{
	return Response::not_my_connection;
}

void Handler::Accept(boost::asio::ip::tcp::socket&& /*socket*/, stream_buffer&& /*buffer*/)
{
}

void Handler::Accept(net::UDPConnection&& /*connection*/, packet_buffer&& /*buffer*/)
{
}

}
