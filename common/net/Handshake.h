#pragma once

#include "std-generic/attributes.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace net {

#pragma pack(push, 1)
struct HandshakeHeader // Version 1
{
	static const size_t primary_field_size = 16;

	char primary_field[primary_field_size]; // Indicates type of the handshake
	uint32_t version : 5; // Version (currently == 1) of header and (it may be) body
	uint32_t head_cksum : 8; // Checksum of the fixed part
	uint32_t body_cksum : 6; // Checksum of the body (Eq.: body_cksum + 1)
	uint32_t body_length : 13; // Body length minus one
};
static_assert(sizeof(HandshakeHeader) == 20, "!");
#pragma pack(pop)

// Calculate checksum of the header
VS_NODISCARD inline uint8_t GetHandshakeHeaderChecksum(const HandshakeHeader& hs)
{
	uint_fast8_t cksum = 0xac + hs.version + hs.body_cksum + hs.body_length;
	for (size_t i = 0; i < HandshakeHeader::primary_field_size; ++i)
		cksum += hs.primary_field[i];
	return cksum & 0xff;
}

// Calculate checksum of the body
VS_NODISCARD inline uint8_t GetHandshakeBodyChecksum(const void* data, size_t size)
{
	uint_fast8_t cksum = 0xca;
	const uint8_t* p = static_cast<const uint8_t*>(data);
	for (size_t i = 0; i < size; i += 3)
		cksum += p[i] - static_cast<uint8_t>(i) - sizeof(HandshakeHeader);
	return cksum & 0x3f;
}

// Calculate checksum of the body, assume that body is located immediately after the header
VS_NODISCARD inline uint8_t GetHandshakeBodyChecksum(const HandshakeHeader& hs)
{
	return GetHandshakeBodyChecksum(&hs + 1, hs.body_length + 1);
}

// Calculate both ckecksums and store them in the header
inline void UpdateHandshakeChecksums(HandshakeHeader& hs)
{
	hs.body_cksum = GetHandshakeBodyChecksum(hs);
	hs.head_cksum = GetHandshakeHeaderChecksum(hs);
}

}
