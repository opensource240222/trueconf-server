#pragma once

#include "../net/Handshake.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/deleters.h"

#include <cstdint>
#include <memory>

namespace transport {

extern const char PrimaryField[net::HandshakeHeader::primary_field_size];

enum class HandshakeResult : uint8_t
{
	ok                  = 0,
	antique_you         = 1,
	antique_me          = 3,
	alien_server        = 4, // i reply resultCode=alien_server when server connects to me, but set not my server.name#vcs at VisicronZeroHandshakeRequest
	busy                = 5,
	verification_failed = 6, // sign verification failed;
	ssl_required		= 7, // a client tried to connect without SSL encryption
	oldarch             = 0xff, // use only to answer to old arch.
};

std::unique_ptr<net::HandshakeHeader, array_deleter<char>> CreateHandshake(
	string_view cid,
	string_view sid,
	uint8_t hops,
	bool ssl_support = false,
	bool tcp_keep_alive_support = true,
	const void* rnd_data = nullptr,
	size_t rnd_data_size = 0);

bool ParseHandshake(
	const net::HandshakeHeader* hs,
	const char*& cid,
	const char*& sid,
	uint8_t& hops,
	const void*& rnd_data,
	size_t& rnd_data_size,
	const void*& sign,
	size_t& sign_size,
	bool& tcp_keep_alive_support);

std::unique_ptr<net::HandshakeHeader, array_deleter<char>> CreateHandshakeReply(
	string_view sid,
	string_view cid,
	HandshakeResult result,
	uint16_t max_conn_silence_ms,
	uint8_t fatal_silence_coef,
	uint8_t hops,
	bool ssl_support = false,
	bool tcp_keep_alive_support = true);

bool ParseHandshakeReply(
	const net::HandshakeHeader* hs,
	const char*& sid,
	const char*& cid,
	HandshakeResult& result,
	uint16_t& max_conn_silence_ms,
	uint8_t& fatal_silence_coef,
	uint8_t& hops,
	bool& tcp_keep_alive_support);

std::unique_ptr<net::HandshakeHeader, array_deleter<char>> CreateHandshakeReply_OLDARCH(); // use for answer to old architecture only

}
