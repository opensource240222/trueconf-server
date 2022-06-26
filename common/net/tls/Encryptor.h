#pragma once

#include "acs_v2/Handler.h"
#include "SecureLib/OpenSSLCompat/tc_ssl.h"

namespace ssl {
class Encryptor{
public:
	Encryptor(BIO *out, SSL *ssl);
	bool Encrypt(const void* in, size_t size, acs::Handler::stream_buffer &out);
private:
	BIO *m_out;
	SSL *m_ssl;
};

class Decryptor {
public:
	Decryptor(BIO *in, SSL *ssl);
	int Decrypt(const void* in, size_t size, acs::Handler::stream_buffer &out);
	int Pending() const;
private:
	BIO *m_in;
	SSL *m_ssl;
};
}