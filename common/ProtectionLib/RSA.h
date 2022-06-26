#pragma once

#include "ProtectionLib/BigNum.h"

namespace protection {

enum RSAError
{
    RSA_BAD_E_VALUE = 1,
    RSA_BAD_FIXED_HEADER_DECRYPT,
    RSA_BAD_PAD_BYTE_COUNT,
    RSA_BAD_SIGNATURE,
    RSA_BLOCK_TYPE_IS_NOT_01,
    RSA_DATA_GREATER_THAN_MOD_LEN,
    RSA_DATA_TOO_LARGE,
    RSA_DATA_TOO_LARGE_FOR_MODULUS,
    RSA_INVALID_PADDING,
    RSA_MALLOC_FAILURE,
    RSA_MODULUS_TOO_LARGE,
    RSA_NULL_BEFORE_BLOCK_MISSING,
    RSA_WRONG_SIGNATURE_LENGTH,
    RSA_MODEXP_ERROR,
    RSA_OTHER_ERROR,
};

// Works like RSA_verify() from OpenSSL, with following exceptions:
// 1. Assumes SHA-1 digest (same as RSA_verify() with type==NID_sha1).
// 2. Instead of RSA structure accepts modulus (n) and public exponent (e) directly.
// 3. In case of error returns one of RSAError values with negative sign.
int RSA_verify_SHA1_PKCS1_type_1(const unsigned char *m, unsigned int m_len, const unsigned char *sigbuf, unsigned int siglen, BIGNUM *n, BIGNUM *e) noexcept;

}
