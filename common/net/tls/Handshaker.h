#pragma once

#include "SecureLib/OpenSSLCompat/tc_ssl.h"

#include <vector>
#include <cstdint>

namespace ssl {
	enum class HandshakeStatus
	{
		hs_success,
		hs_ongoing,
		hs_failure,
	};

	class Handshaker {
		BIO *m_in, *m_out;
		SSL *m_ssl;
		bool m_isServer;
		HandshakeStatus m_hsStatus;

		HandshakeStatus TryHandshake();
	public:
		Handshaker(BIO *in, BIO *out, SSL *ssl, bool isServer);
		HandshakeStatus DoHandshake(const void *data, size_t size);
		bool GetDataToSend(std::vector<uint8_t> &data);
		size_t PendingBytes() const;
		bool WantData() const;
		bool IsServer() const;
	};
}