#pragma once

#include <cstdint>

#include <string>
#include "std-generic/cpplib/string_view.h"

#pragma pack(push, 1)

struct VS_UUID {
	uint32_t time_low;
	uint16_t time_mid;
	uint16_t time_hi_and_version;
	uint8_t  clock_seq_hi_and_reserved;
	uint8_t  clock_seq_low;
	uint8_t  node[6];

	explicit VS_UUID(const char *str);
	explicit VS_UUID(const unsigned char hash[20]);

	operator std::string();
	VS_UUID& operator=(const VS_UUID&);
	bool operator!=(const VS_UUID&)const;
	VS_UUID(const VS_UUID&);

	static std::string GenEpid(string_view self_sip_uri, string_view hostname, string_view ip_address);
	static VS_UUID GenUUID(string_view epid);
};

static_assert(sizeof(VS_UUID) == 16, "!");

#pragma pack(pop)