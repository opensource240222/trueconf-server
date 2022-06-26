#pragma once
#include <openssl/crypto.h>
#if OPENSSL_VERSION_NUMBER < 0x10100000L

inline void CRYPTO_free(void *str, const char* /*unused*/, int /*unused*/)
{
	CRYPTO_free(str);
};
#endif
