#pragma once

#include "SecureLib/OpenSSLCompat/tc_ssl.h"

#include <string>

namespace ssl {
namespace utils{
	bool AddCertToCTX(SSL_CTX *ctx, const void *cert, const unsigned int size);
	bool AddCertChainToCTX(SSL_CTX *ctx, const void *cert, const unsigned int size);
	bool AddPrivateKeyToCTX(SSL_CTX *ctx, const void *key, const unsigned int size, const char* pass);
	std::string GetOpenSSLErrorStack();

	struct SSL_CTX_deleter { void operator() (SSL_CTX* ctx) { SSL_CTX_free(ctx); } };
	struct SSL_deleter { void operator() (SSL* ssl) { SSL_free(ssl); } };
}// end of namespace utils
}// end of namespace ssl