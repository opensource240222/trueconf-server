#include "Handshaker.h"
#include <cassert>
#include "Utils.h"
#include <string.h>

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_OTHER

namespace ssl
{
Handshaker::Handshaker(BIO * in, BIO * out, SSL * ssl, bool isServer)
	: m_in(in)
	, m_out(out)
	, m_ssl(ssl)
	, m_isServer(isServer)
	, m_hsStatus(HandshakeStatus::hs_failure)
{
	assert(m_in != nullptr);
	assert(m_out != nullptr);
	assert(m_ssl != nullptr);

	if (!m_isServer) // client
	{
		//		If we are the ones initiating the connection, we can't possibly have incoming data
		SSL_set_connect_state(m_ssl);
	}
	else // server
	{
		SSL_set_accept_state(m_ssl);
	}
}

HandshakeStatus Handshaker::DoHandshake(const void * data, size_t size)
{
	assert(m_ssl != nullptr);
	assert(m_in != nullptr);

	if (data != nullptr && size > 0) {
		if (BIO_write(m_in, data, static_cast<int>(size)) != static_cast<int>(size))
		{
			dstream2 << "tls::Connection error: couldn't feed data for handshake!\n" << utils::GetOpenSSLErrorStack();
			return HandshakeStatus::hs_failure;
		}
	}

	return TryHandshake();
}

bool Handshaker::GetDataToSend(std::vector<uint8_t>& data)
{
	size_t pending = PendingBytes();
	data.resize(pending);
	int toSend = BIO_read(m_out, data.data(), data.size());
	if (toSend > 0)
		data.resize(toSend);

	assert(static_cast<int>(pending) == toSend);
	return toSend > 0;
}

size_t Handshaker::PendingBytes() const
{
	assert(m_out != nullptr);
	return BIO_ctrl_pending(m_out);
}

bool Handshaker::WantData() const
{
	return 0 != BIO_ctrl(reinterpret_cast<BIO *>(m_in), BIO_CTRL_EOF, 0, NULL);
}

bool Handshaker::IsServer() const
{
	return m_isServer;
}

HandshakeStatus Handshaker::TryHandshake() {
	assert(m_ssl != nullptr);

	int result = SSL_do_handshake(m_ssl);
	if (result == 1 && !strcmp("SSLOK ", SSL_state_string(m_ssl))) {
		return m_hsStatus = HandshakeStatus::hs_success;
	}

	if (result == -1) {
		int error = SSL_get_error(m_ssl, result);
		if (error == SSL_ERROR_WANT_READ)
			return m_hsStatus = HandshakeStatus::hs_ongoing;
		if (error == SSL_ERROR_WANT_WRITE)
			return  m_hsStatus = HandshakeStatus::hs_ongoing;
	}

	return  m_hsStatus = HandshakeStatus::hs_failure;
}
}	// end of namespace tls