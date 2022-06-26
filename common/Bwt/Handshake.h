#pragma once

#include "../net/Handshake.h"

#include <cstdint>

#define   VS_ACS_LIB_SENDER     0
#define   VS_ACS_LIB_RECEIVER   1

#define	  VS_ACS_LIB_BOTH		2

#define   _MD_BOOL_(val)	(char *)(val ? "true" : "false")
#define   _MD_CTYPE_(val)	(char *)(val ? "VS_ACS_LIB_RECEIVER" : "VS_ACS_LIB_SENDER")
#define   _MD_POINT_(val)	(unsigned long)((unsigned __int64)val)
#define   _MD_STR_(val)		(char *)(val ? (*val ? val : "\0") : "NULL")

//ip&255	== 10*256^3;				10.0.0.0/8
//ip&4095	== 172*256^3 + 16*256^2;	172.16.0.0/12
//ip&65535	== 192*256^3 + 168*256^2	192.168.0.0/16
#define VS_SUBNET_MASK_1	0xFF000000
#define VS_SUBNET_MASK_2	0xFFF00000
#define VS_SUBNET_MASK_3	0xFFFF0000

#define VS_ADDRESS_SPACE_1	0x0A000000
#define VS_ADDRESS_SPACE_2	0xAC100000
#define VS_ADDRESS_SPACE_3	0xC0A80000

// for ipv6: FC00::/7
#define VS_SUBNET_MASK_IPV6		0xFFFE
#define VS_ADDRESS_SPACE_IPV6	0xFC00

namespace bwt
{
#pragma pack(push, 1)
	struct Handshake
	{
		Handshake(unsigned char type, uint32_t send_time_ms, uint32_t content_size, uint32_t send_period_ms) :
			type(type),
			send_time_ms(send_time_ms),
			content_size(content_size),
			send_period_ms(send_period_ms){}
		net::HandshakeHeader hs;
		unsigned char type;
		uint32_t send_time_ms;
		uint32_t content_size;
		uint32_t send_period_ms;
	};

	struct HandshakeReply
	{
		net::HandshakeHeader hs;
		unsigned char   resultCode;
	};
#pragma pack(pop)

	extern const char VS_Bwt_PrimaryField[net::HandshakeHeader::primary_field_size];
}
