#pragma once

#include <cstdint>
#include <time.h>


#if defined(_WIN32)
#	define timezone _timezone
#endif

#pragma pack(push, 1)

	// See https://wiki.wireshark.org/Development/LibpcapFileFormat
	struct PcapGlobalHeader
	{
		uint32_t magic_number = 0xa1b2c3d4; // magic number; Always this value in application byte order.
		uint16_t version_major = 2; // major version number
		uint16_t version_minor = 4; // minor version number
		int32_t thiszone = timezone; // GMT to local correction
		uint32_t sigfigs = 0; // accuracy of timestamps
		uint32_t snaplen = 0xffff; // max length of captured packets, in octets
		uint32_t network = 101; // data link type; We write fake IPv4 headers, see http://www.tcpdump.org/linktypes.html
	};
	struct PcapPacketHeader
	{
		uint32_t ts_sec; // timestamp seconds
		uint32_t ts_usec; // timestamp microseconds
		uint32_t incl_len; // number of octets of packet saved in file
		uint32_t orig_len; // actual length of packet
	};


	// See https://tools.ietf.org/html/rfc791#section-3.1
	struct IPv4Header
	{
		uint8_t version_and_ihl = 0x45; // version=4, ihl=5 (no options or padding)
		uint8_t tos = 0; // fake value //TODO: find good value; 0 is bad value (wireshark doesnt like it) 
		uint16_t total_length;
		uint16_t identification = 0; // unneeded, no fragmentation
		uint16_t flags_and_fragment_offset = 0; // no fragmentation
		uint8_t ttl = 42; // fake value
		uint8_t protocol; // TCP - 6, UDP - 17
		uint16_t head_checksum = 0; // technically we should calculate the correct value for each packet, but Wireshark treats 0 as special case and disables checksum validation
		uint32_t source_address = 0x0100007f; // fake value (127.0.0.1)
		uint32_t destination_address = 0x0200007f; // fake value (127.0.0.2)

	};

	struct IPv6Header
	{
		uint32_t version_traffic_class_and_flow_label = 0x60; // version 6
		uint16_t total_length;
		uint8_t  protocol;  // TCP - 6, UDP - 17
		uint8_t  hop_limit = 42; // fake value
		struct
		{
			uint64_t first_part;
			uint64_t second_part;
		} source_address;
		struct 
		{
			uint64_t first_part;
			uint64_t second_part;
		} destination_address;
	};

	// See https://tools.ietf.org/html/rfc768
	struct UDPHeader
	{
		static constexpr uint8_t protocol_number = 17;
		uint16_t source_port = 0x7217; // fake value (6002), matches our default port for RTP
		uint16_t destination_port = 0x7217; // fake value (6002), matches our default port for RTP
		uint16_t length;
		uint16_t checksum = 0; // 0 means value is unused
	};

	struct TCPHeader
	{
		static constexpr  uint8_t protocol_number = 6;
		uint16_t source_port = 0xC413; // 5060
		uint16_t destination_port = 0xC413;
		uint32_t sequence_number = 0;
		uint32_t ack = 0;
		uint8_t  data_offset = 0b01010000;  // first 4 bits - size of TCPHeader in 32-bit words
		uint8_t  flags = 0;//= 0b00011000;
		uint16_t window_size = 0x0001; // 256
		uint16_t checksum = 0;
		uint16_t urgent_p = 0;
	};

#pragma pack(pop)
