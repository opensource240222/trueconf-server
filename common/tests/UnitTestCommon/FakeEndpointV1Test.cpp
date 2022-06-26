#if defined(_WIN32) // Not ported yet

#include "FakeClient/VS_FakeEndpointV1.h"
#include "../../transport/Router/VS_TransportHandler.h"
#include "newtransport/Handshake.h"
#include "std-generic/cpplib/scope_exit.h"

#include <gmock/gmock.h>

class VS_TransportRouter_SetConnectionFake : public VS_TransportRouter_SetConnection {
	void SetConnection(const char* /*cid*/,
		const unsigned long /*version*/,
		const char* /*sid*/,
		class VS_Connection* conn, const bool /*isAccept*/,
		const unsigned short /*maxConnSilenceMs*/,
		const unsigned char /*fatalSilenceCoef*/,
		const unsigned char /*hop*/,
		const unsigned long /*rnd_data_ln*/,
		const unsigned char* /*rnd_data*/,
		const unsigned long /*sign_ln*/,
		const unsigned char* /*sign*/,
		const bool /*hs_error*/ = false,
		const bool /*tcpKeepAliveSupport*/ = false)
	{
		auto hs_reply = transport::CreateHandshakeReply("SID", "CID", transport::HandshakeResult::ok, 0, 0, 0);
		conn->Write(hs_reply.get(), sizeof(net::HandshakeHeader) + hs_reply->body_length + 1);
	}
};

#endif
