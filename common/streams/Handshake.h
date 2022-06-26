#pragma once

#include "fwd.h"
#include "../net/Handshake.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/deleters.h"
#include "std-generic/attributes.h"

#include <cstdint>
#include <memory>

namespace stream {

extern const char PrimaryField[net::HandshakeHeader::primary_field_size];

enum class ClientType : unsigned
{
	uninstalled = 0,
	sender,
	receiver,
	transmitter,
	rtp,
	direct_udp,
};

bool GetHandshakeFields(
	const net::HandshakeHeader* hs,
	ClientType& type,
	const char*& conference_name,
	const char*& participant_name,
	const char*& connected_participant_name,
	const char*& connected_endpoint_name,
	const uint8_t*& mtracks
);
bool GetHandshakeResponseFields(const net::HandshakeHeader* hs, const uint8_t*& mtracks);

VS_NODISCARD std::unique_ptr<net::HandshakeHeader, array_deleter<char>>
CreateHandshake(ClientType type, string_view conference_name, string_view participant_name, string_view connected_participant_name, string_view connected_endpoint_name, const uint8_t mtracks[32]);
VS_NODISCARD std::unique_ptr<net::HandshakeHeader, array_deleter<char>>
CreateHandshakeResponse(const uint8_t mtracks[32]);

//  tracks - compact list of track IDs
// ftracks - sparse byteset of track IDs, track with ID ==N is present in the set if N-th byte !=0
// mtracks - sparse bitset of track IDs, track with ID ==N is present in the set if N-th bit is set
// Because track ID 0 is invalid first byte in ftracks and first bit in mtracks are ignored.
bool TracksToFTracks(uint8_t ftracks[256], const Track* tracks, unsigned n_tracks);
unsigned FTracksToTracks(Track* tracks, const uint8_t ftracks[256]);
void FTracksToMTracks(uint8_t mtracks[32], const uint8_t ftracks[256]);
void MTracksToFTracks(uint8_t ftracks[256], const uint8_t mtracks[32]);
void FTracksIntersection(uint8_t ftracks1[256], const uint8_t ftracks2[256]);
void MTracksIntersection(uint8_t mtracks1[32], const uint8_t mtracks2[32]);
VS_NODISCARD unsigned CountFTracks(const uint8_t ftracks[256]);
VS_NODISCARD unsigned CountMTracks(const uint8_t mtracks[32]);
VS_NODISCARD bool IsInMTracks(const uint8_t mtracks[32], Track track);

}
