#pragma once

#include "std-generic/attributes.h"

#include <cstddef>
#include <cstdint>

namespace stream {

enum class Track : uint8_t
{
	undef = 0,
	audio = 1,
	video = 2,
	data = 5,
	garbage = 128,
	command = 254,
	old_command = 255,
};
inline uint8_t id(Track x) { return static_cast<uint8_t>(x); }
inline Track track(uint8_t x) { return static_cast<Track>(x); }

enum class TrackType : uint8_t
{
	undef = 0,
	audio = 1,
	video = 2,
	data = 3,
	garbage = 4,
	other = 5,
	slide = 6,
};

#pragma pack(push, 1)

struct FrameHeader
{
	uint16_t length;
	uint32_t tick_count; // Sender relative tickcount (milliseconds).
	Track track;
	uint8_t cksum; // Checksum of the body
};
static_assert(sizeof(FrameHeader) == 8, "!");

struct UDPFrameHeader
{
	uint16_t length;
	uint32_t tick_count; // Sender relative tickcount (milliseconds).
	Track track;
	uint8_t cksum; // Checksum of the body
	uint8_t head_length; // Lenght of this header
	uint8_t version;
	uint32_t UID; // field for identification
};
static_assert(sizeof(UDPFrameHeader) == 14, "!");

VS_NODISCARD inline uint8_t GetFrameBodyChecksum(const void* data, size_t size)
{
	uint_fast8_t cksum = 0xac;
	const uint8_t* p = static_cast<const uint8_t*>(data);
	for (size_t i = 0; i < size; i += 0xea)
		cksum += p[i];
	return cksum;
}

struct SliceHeader
{
	uint8_t id; // ID of this slice, first slice of the frame has the largest id, last slide has id == 0
	uint8_t first_id; // ID of the first slice of this frame
	uint8_t frame_counter; // Cyclic frame counter, used to detect (completely) missing frames
};
static_assert(sizeof(SliceHeader) == 3, "!");

struct SVCHeader
{
	uint8_t maxspatial:2;
	uint8_t temporal:2;
	uint8_t spatial:2;
	uint8_t quality:2;
};
static_assert(sizeof(SVCHeader) == 1, "!");

#pragma pack(pop)

}
