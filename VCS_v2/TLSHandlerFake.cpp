#include "TLSHandlerFake.h"

#include "acs/AccessConnectionSystem/TLS_Check.h"
#include "std-generic/cpplib/ignore.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

acs::Response vs::TLSHandlerFake::Protocol(const stream_buffer& buffer, unsigned /*channel_token*/)
{
	if (buffer.size() < TLS_HANDSHAKE_RECORD_HEADER_SIZE)
		return acs::Response::next_step;

	return VS_IsTLSClientHello(buffer.data(), buffer.size()) ? acs::Response::accept_connection : acs::Response::not_my_connection;
}

void vs::TLSHandlerFake::Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& /*buffer*/)
{
	dstream3 << "FIXME: Rejecting TLS connection from " << socket.remote_endpoint(vs::ignore<boost::system::error_code>());
	socket.close(vs::ignore<boost::system::error_code>());
}
