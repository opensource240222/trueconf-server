#pragma once

#include <cstdint>

#include "std-generic/cpplib/hton.h"
#include "SecureLib/OpenSSLCompat/tc_ssl.h"

/* SSLv3/TLS hanshshake record format (unpacked structure!) */
static const int TLS_VERSION_MAJOR = 3;
static const int TLS_VERSION_MIN_MINOR = 1;
/* packet packet handshake record size */
static const size_t TLS_HANDSHAKE_RECORD_HEADER_SIZE = 8;

inline bool VS_IsTLSClientHello(const uint8_t *data, const size_t size)
{
	if (size < 9)
		return false;
	// check record type
	if (data[0] != SSL3_RT_HANDSHAKE)
		return false;
	// check protocol version
	if ((data[1] != TLS_VERSION_MAJOR) || (data[2] < TLS_VERSION_MIN_MINOR))
		return false;
	// check handshake record type
	if (data[5] != SSL3_MT_CLIENT_HELLO)
		return false;
	// additional checks
	uint16_t record_len = vs_ntohs(*reinterpret_cast<const uint16_t*>(data + 3));
//	As a result, we basically take three bytes: data[6], data[7] and data[8],
//	and then convert them from big-endian to host endianness (little-endian on Intel)
	uint32_t data_len = vs_ntohl(*reinterpret_cast<const uint32_t*>(data + 5) & vs_ntohl(0x00ffffff));
//	First comparison: data_len is placed later than record_len so it should be smaller
//	Second comparison guarantees that we received a full ClientHello (size covers record_len fully)
	if (record_len < data_len || record_len > size - 5)
		return false;
	return true;
}

inline const SSL_METHOD* VS_TLSClientHelloCheck(const uint8_t* data, size_t size)
{
	if (size < 9)
		return nullptr;
	uint16_t record_len = vs_ntohs(*reinterpret_cast<const uint16_t*>(data + 3));
//	As a result, we basically take three bytes: data[6], data[7] and data[8],
//	and then convert them from big-endian to host endianness (little-endian on Intel)
	uint32_t data_len = vs_ntohl(*reinterpret_cast<const uint32_t*>(data + 5) & vs_ntohl(0x00ffffff));
//	First comparison: data_len is placed later than record_len so it should be smaller
//	Second comparison guarantees that we received a full ClientHello (size covers record_len fully)
	if (record_len < data_len || record_len > size - 5)
		return nullptr;

	const uint8_t* currentPointer = data + 43;
	if (static_cast<size_t>(currentPointer - data) >= size)
		return nullptr;
//	Legacy session ID
	currentPointer += *currentPointer + 1;
	if (static_cast<size_t>(currentPointer + 1 - data) >= size)
		return nullptr;

//	CipherSuites
	currentPointer += vs_ntohs(*reinterpret_cast<const uint16_t*>(currentPointer)) + 2;
	if (static_cast<size_t>(currentPointer - data) >= size)
		return nullptr;

//	Legacy compression methods
	currentPointer += *currentPointer + 1;
	if (static_cast<size_t>(currentPointer + 1 - data) >= size)
		return nullptr;

//	Get extensions' length
	size_t remainingLength = vs_ntohs(*reinterpret_cast<const uint16_t*>(currentPointer));
	currentPointer += 2;
	if (static_cast<size_t>(currentPointer + remainingLength - 1 - data) >= size)
		return nullptr;

	while (remainingLength >= 4)
	{
		uint16_t extensionType = vs_ntohs(*reinterpret_cast<const uint16_t*>(currentPointer));
		if (extensionType == 43) // Supported versions, defined only in TLS 1.3
			return TLS_method();
		size_t shift =
			/*extensionType*/ 2 +
			/*data length length*/ 2 +
			/*data length*/vs_ntohs(*reinterpret_cast<const uint16_t*>(currentPointer + 2));
		if (remainingLength >= shift)
			remainingLength -= shift;
		else
			remainingLength = 0;
		currentPointer += shift;
	}
//	If extension isn't found, we can fall back to TLS v1.2
	return TLSv1_2_method();
}