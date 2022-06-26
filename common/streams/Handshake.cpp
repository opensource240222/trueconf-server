#include "Handshake.h"
#include "Protocol.h"
#include "../acs/VS_AcsDefinitions.h"

#include <cassert>
#include <cstring>

namespace stream {

const char PrimaryField[net::HandshakeHeader::primary_field_size] = { '_','V','S','_','S','T','R','E','A','M','S','_' };

bool GetHandshakeFields(
	const net::HandshakeHeader* hs,
	ClientType& type,
	const char*& conference_name,
	const char*& participant_name,
	const char*& connected_participant_name,
	const char*& connected_endpoint_name,
	const uint8_t*& mtracks
)
{
	if (!hs || hs->version < 1 || hs->body_length + 1 < 10) /* { type, 0, '\0', 0, '\0', 0, '\0', 0, '\0', n_tracks=255 } */
		return false;
	auto p = reinterpret_cast<const uint8_t*>(hs) + sizeof(net::HandshakeHeader);
	const auto p_end = p + hs->body_length + 1;

	type = (*p++ & 0x01) ? ClientType::sender : ClientType::receiver;

	auto get_str = [&](const char*& str) {
		if (p >= p_end)
			return false;
		auto sz = *p++;
		str = reinterpret_cast<const char*>(p);
		p += sz;
		if (p >= p_end)
			return false;
		if (*p++ != 0)
			return false;
		return true;
	};
	if (!get_str(conference_name))
		return false;
	if (!get_str(participant_name))
		return false;
	if (!get_str(connected_participant_name))
		return false;
	if (!get_str(connected_endpoint_name))
		return false;

	if (p >= p_end)
		return false;
	auto n_tracks = *p++;
	if (n_tracks == 255)
		mtracks = nullptr;
	else
	{
		mtracks = reinterpret_cast<const uint8_t*>(p);
		p += 32;
	}

	return p <= p_end;
}

bool GetHandshakeResponseFields(const net::HandshakeHeader* hs, const uint8_t*& mtracks)
{
	if (!hs || hs->version < 1 || hs->body_length + 1 < 2) /* { 255 } */
		return false;
	auto p = reinterpret_cast<const uint8_t*>(hs) + sizeof(net::HandshakeHeader);
	const auto p_end = p + hs->body_length + 1;

	bool flag = *p++ & 0x01;
	if (!flag)
		return false;

	auto n_tracks = *p++;
	if (n_tracks == 255)
		mtracks = nullptr;
	else
	{
		mtracks = reinterpret_cast<const uint8_t*>(p);
		p += 32;
	}

	return p <= p_end;
}

std::unique_ptr<net::HandshakeHeader, array_deleter<char>>
CreateHandshake(ClientType type, string_view conference_name, string_view participant_name, string_view connected_participant_name, string_view connected_endpoint_name, const uint8_t mtracks[32])
{
	std::unique_ptr<net::HandshakeHeader, array_deleter<char>> hs;

	if (conference_name.size() > 255)
		return hs;
	if (participant_name.size() > 255)
		return hs;
	if (connected_participant_name.size() > 255)
		return hs;
	if (connected_endpoint_name.empty() || connected_endpoint_name.size() > VS_ACS_MAX_SIZE_ENDPOINT_NAME)
		return hs;
	if (type != ClientType::sender && type != ClientType::receiver)
		return hs;

	const auto n_tracks = mtracks ? CountMTracks(mtracks) : 255;
	const size_t body_size = 1/*type*/
		+ 1/*size*/ + conference_name.size() + 1/*'\0'*/
		+ 1/*size*/ + participant_name.size() + 1/*'\0'*/
		+ 1/*size*/ + connected_participant_name.size() + 1/*'\0'*/
		+ 1/*size*/ + connected_endpoint_name.size() + 1/*'\0'*/
		+ 1/*n_tracks*/ + (n_tracks == 255 ? 0 : 32)
		;
	hs.reset(reinterpret_cast<net::HandshakeHeader*>(new char[sizeof(net::HandshakeHeader) + body_size]));
	auto p = reinterpret_cast<uint8_t*>(hs.get()) + sizeof(net::HandshakeHeader);

	*p++ = type == ClientType::sender ? 0 : 1;

	auto put_str = [&](string_view str) {
		*p++ = str.size();
		std::memcpy(p, str.data(), str.size());
		p += str.size();
		*p++ = '\0';
	};
	put_str(conference_name);
	put_str(participant_name);
	put_str(connected_participant_name);
	put_str(connected_endpoint_name);

	*p++ = n_tracks;
	if (n_tracks != 255)
	{
		std::memcpy(p, mtracks, 32);
		p += 32;
	}
	assert(p == reinterpret_cast<uint8_t*>(hs.get()) + sizeof(net::HandshakeHeader) + body_size);

	std::memcpy(hs->primary_field, PrimaryField, sizeof(PrimaryField));
	hs->version = 1;
	hs->body_length = body_size - 1;
	net::UpdateHandshakeChecksums(*hs);
	return hs;
}

std::unique_ptr<net::HandshakeHeader, array_deleter<char>>
CreateHandshakeResponse(const uint8_t mtracks[32])
{
	std::unique_ptr<net::HandshakeHeader, array_deleter<char>> hs;

	const auto n_tracks = mtracks ? CountMTracks(mtracks) : 255;
	const size_t body_size = 1/*flag*/
		+ 1/*n_tracks*/ + (n_tracks == 255 ? 0 : 32)
		;
	hs.reset(reinterpret_cast<net::HandshakeHeader*>(new char[sizeof(net::HandshakeHeader) + body_size]));
	auto p = reinterpret_cast<uint8_t*>(hs.get()) + sizeof(net::HandshakeHeader);

	*p++ = 1; // flag

	*p++ = n_tracks;
	if (n_tracks != 255)
	{
		std::memcpy(p, mtracks, 32);
		p += 32;
	}
	assert(p == reinterpret_cast<uint8_t*>(hs.get()) + sizeof(net::HandshakeHeader) + body_size);

	std::memcpy(hs->primary_field, PrimaryField, sizeof(PrimaryField));
	hs->version = 1;
	hs->body_length = body_size - 1;
	net::UpdateHandshakeChecksums(*hs);
	return hs;
}

bool TracksToFTracks(uint8_t ftracks[256], const Track* tracks, unsigned n_tracks)
{
	if (n_tracks == 0 || n_tracks > 255)
		return false;
	if (!tracks)
	{
		if (n_tracks != 255)
			return false;
		ftracks[0] = 0;
		std::memset(ftracks + 1, 1, 255);
	}
	else
	{
		std::memset(ftracks, 0, 256);
		for (unsigned i = 0; i < n_tracks; ++i)
		{
			if (id(tracks[i]) == 0 || ftracks[id(tracks[i])] != 0)
				return false;
			ftracks[id(tracks[i])] = 1;
		}
	}
	return true;
}

unsigned FTracksToTracks(Track* tracks, const uint8_t ftracks[256])
{
	unsigned n_tracks = 0;
	for (unsigned i = 1; i < 256; ++i)
		if (ftracks[i])
			tracks[n_tracks++] = static_cast<Track>(i);
	return n_tracks;
}

void FTracksToMTracks(uint8_t mtracks[32], const uint8_t ftracks[256])
{
	for (unsigned i = 0; i < 32; ++i)
	{
		mtracks[i] = 0;
		if (ftracks[i*8+0]) mtracks[i] |= 0x01;
		if (ftracks[i*8+1]) mtracks[i] |= 0x02;
		if (ftracks[i*8+2]) mtracks[i] |= 0x04;
		if (ftracks[i*8+3]) mtracks[i] |= 0x08;
		if (ftracks[i*8+4]) mtracks[i] |= 0x10;
		if (ftracks[i*8+5]) mtracks[i] |= 0x20;
		if (ftracks[i*8+6]) mtracks[i] |= 0x40;
		if (ftracks[i*8+7]) mtracks[i] |= 0x80;
	}
}

void MTracksToFTracks(uint8_t ftracks[256], const uint8_t mtracks[32])
{
	for (unsigned i = 0; i < 32; ++i)
	{
		ftracks[i*8+0] = (mtracks[i] & 0x01) ? 1 : 0;
		ftracks[i*8+1] = (mtracks[i] & 0x02) ? 1 : 0;
		ftracks[i*8+2] = (mtracks[i] & 0x04) ? 1 : 0;
		ftracks[i*8+3] = (mtracks[i] & 0x08) ? 1 : 0;
		ftracks[i*8+4] = (mtracks[i] & 0x10) ? 1 : 0;
		ftracks[i*8+5] = (mtracks[i] & 0x20) ? 1 : 0;
		ftracks[i*8+6] = (mtracks[i] & 0x40) ? 1 : 0;
		ftracks[i*8+7] = (mtracks[i] & 0x80) ? 1 : 0;
	}
}

void FTracksIntersection(uint8_t ftracks1[256], const uint8_t ftracks2[256])
{
	for (unsigned i = 0; i < 256; ++i)
		ftracks1[i] = ftracks1[i] && ftracks2[i] ? 1 : 0;
}

void MTracksIntersection(uint8_t mtracks1[32], const uint8_t mtracks2[32])
{
	for (unsigned i = 0; i < 32; ++i)
		mtracks1[i] &= mtracks2[i];
}

unsigned CountFTracks(const uint8_t ftracks[256])
{
	unsigned n_tracks = 0;
	for (unsigned i = 1; i < 256; ++i)
		n_tracks += ftracks[i] ? 1 : 0;
	return n_tracks;
}

unsigned CountMTracks(const uint8_t mtracks[32])
{
	static const uint8_t popcnt_4bit[16] = { 0,1,1,2,1,2,2,3, 1,2,2,3,2,3,3,4, };
	unsigned n_tracks = 0;
	for (unsigned i = 0; i < 32; ++i)
		n_tracks += popcnt_4bit[mtracks[i] & 0x0f] + popcnt_4bit[mtracks[i] >> 4];
	if (mtracks[0] & 0x01)
		--n_tracks;
	return n_tracks;
}

bool IsInMTracks(const uint8_t mtracks[32], Track track)
{
	return mtracks[id(track) / 8] & (1 << (id(track) % 8));
}

}
