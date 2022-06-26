#pragma once

#include <openssl/hmac.h>
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#define HMAC_CTX_new new HMAC_CTX
#define HMAC_CTX_free(c) HMAC_CTX_cleanup(c); delete c;
#endif