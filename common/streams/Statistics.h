#pragma once

#include "fwd.h"

#include <cstdint>

namespace stream {

#pragma pack(push, 1)

struct TrackStatistics
{
	Track track;
	uint16_t nFramesBuffer;
	uint32_t writeBytesBand;
	uint32_t writeFramesBand;
};
static_assert(sizeof(TrackStatistics) == 11, "!");

struct StreamStatistics
{
	uint16_t allFramesBuffer;
	uint32_t allWriteBytesBand;
	uint32_t allWriteFramesBand;
	uint8_t ntracks;
	struct TrackStatistics tracks[1];
};
static_assert(sizeof(StreamStatistics) == 11 + sizeof(TrackStatistics), "!");

struct ParticipantStatistics
{
	uint64_t allReceiverBytes;
	uint64_t allReceiverFrames;
	uint64_t allSenderBytes;
	uint64_t allSenderFrames;
	uint32_t receiverBytesBand;
	uint32_t receiverFramesBand;
	uint32_t senderBytesBand;
	uint32_t senderFramesBand;
	uint32_t receiverNConns;
	uint32_t senderNConns;
	char receiverIp[16];
	char senderIp[16];
};
static_assert(sizeof(ParticipantStatistics) == 88, "!");

#pragma pack(pop)

struct ConferenceStatistics
{
	unsigned char unused;
};

struct RouterStatistics
{
	unsigned streams;
	double out_bytes;
	double in_bytes;
	float out_byterate;
	float in_byterate;
};

}
