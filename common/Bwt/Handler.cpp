
#include "Handler.h"
#include "ContentHandler.h"
#include "std/cpplib/MakeShared.h"

namespace bwt
{

	acs::Response Handler::Protocol(const acs::Handler::stream_buffer& buffer, unsigned /*channel_token*/)
	{
		if (buffer.size() < sizeof(net::HandshakeHeader))
		{
			return acs::Response::next_step;
		}
		const auto& hs = *reinterpret_cast<const net::HandshakeHeader*>(buffer.data());
		if (hs.head_cksum != net::GetHandshakeHeaderChecksum(hs)
			|| strncmp(hs.primary_field, VS_Bwt_PrimaryField, sizeof(hs.primary_field)))
		{
			return acs::Response::not_my_connection;
		}
		const uint32_t body_length = hs.body_length + 1,
			handshake_length = sizeof(net::HandshakeHeader) + body_length;

		if (buffer.size() > handshake_length)
		{
			return acs::Response::not_my_connection;
		}

		if (buffer.size() < handshake_length)
		{
			return acs::Response::my_connection;
		}
		if (hs.body_cksum != net::GetHandshakeBodyChecksum(hs))
		{
			return acs::Response::not_my_connection;
		}
		return acs::Response::accept_connection;
	}

	void Handler::Accept(boost::asio::ip::tcp::socket&& sock, acs::Handler::stream_buffer&& buffer)
	{
		auto ptr = vs::MakeShared<ContentHandler>(std::move(sock), *reinterpret_cast<Handshake*>(buffer.data()), std::make_shared<Intermediate>());
		ptr->send_reply();
	}

	Handler::~Handler()
	{
	}

}

