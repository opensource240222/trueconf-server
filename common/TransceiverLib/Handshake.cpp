#include "Handshake.h"
#include "net/Handshake.h"
#include "TransceiverLib/VS_ProtocolConst.h"

#include <cstring>

VS_ACS_Response ts::ProtocolHandshake(const void * in_buffer, unsigned long * in_len)
{
	if (*in_len < sizeof(net::HandshakeHeader))
	{
		*in_len = sizeof(net::HandshakeHeader);
		return vs_acs_next_step;
	}
	const auto& hs = *static_cast<const net::HandshakeHeader*>(in_buffer);
	if (hs.head_cksum != net::GetHandshakeHeaderChecksum(hs) || hs.version < 1)
		return vs_acs_connection_is_not_my;
	if ((!strncmp(hs.primary_field, VS_Circuit_PrimaryField, sizeof(hs.primary_field))) ||
		(!strncmp(hs.primary_field, VS_FrameTransmit_PrimaryField, sizeof(hs.primary_field))))
	{
		const unsigned long body_length = hs.body_length + 1; // because we have to store body_length-1 in header due to unfixable bug in net::GetHandshakeHeaderChecksum
		const unsigned long handshake_length = sizeof(net::HandshakeHeader) + body_length;

		if (*in_len > handshake_length)
			return vs_acs_connection_is_not_my;
		if (*in_len < handshake_length)
		{
			*in_len = handshake_length;
			return vs_acs_my_connections;
		}
		if (hs.body_cksum != net::GetHandshakeBodyChecksum(hs))
			return vs_acs_connection_is_not_my;
		else
			return vs_acs_accept_connections;
	}
	return vs_acs_connection_is_not_my;
}
