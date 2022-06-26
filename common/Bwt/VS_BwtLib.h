
#ifndef VS_ACS_BWT_LIB_H
#define VS_ACS_BWT_LIB_H

#include <cstddef>
#include "../net/Handshake.h"

#include <stdint.h>

#define   VS_BWT_PRIMARY_FIELD   "BANDWIDTH_TEST"
extern const char VS_Bwt_PrimaryField[net::HandshakeHeader::primary_field_size];

#pragma pack( 1 )

struct VS_BwtHandshake
{
	net::HandshakeHeader hs;
	unsigned char   type;		unsigned long   tm, size, period;
};
// end VS_BwtHandshake struct
struct VS_BwtHandshakeReply
{
	net::HandshakeHeader hs;
	unsigned char   resultCode;
};
// end VS_BwtHandshakeReply struct

namespace bwt
{
	struct Handshake
	{
		net::HandshakeHeader hs;
		unsigned char type; // VS_ACS_LIB_SENDER, VS_ACS_LIB_RECEIVER
		uint32_t send_time_ms;
		uint32_t content_size;
		uint32_t send_period_ms;
	};
	// end VS_BwtHandshake struct
	struct HandshakeReply
	{
		net::HandshakeHeader hs;
		unsigned char   resultCode;
	};

}

#pragma pack(   )

#endif  // VS_ACS_BWT_LIB_H
