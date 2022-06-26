#include "ProtectionLib/RSA.h"
#include "ProtectionLib/Lib.h"
#include "ProtectionLib/Protection.h"
#include "std-generic/cpplib/scope_exit.h"

namespace protection {

int RSA_public_decrypt_PKCS1_type_1(int flen, const unsigned char *from, unsigned char *to, BIGNUM *n, BIGNUM *e) noexcept;

int RSA_padding_check_PKCS1_type_1(unsigned char *to, int tlen, const unsigned char *from, int flen, int num) noexcept;

// Based on int_rsa_verify()
SECURE_FUNC_INTERNAL
int RSA_verify_SHA1_PKCS1_type_1(const unsigned char *m, unsigned int m_len, const unsigned char *sigbuf, unsigned int siglen, BIGNUM *n, BIGNUM *e) noexcept
{
    if (siglen != (size_t)BN_num_bytes(n)) {
        return -RSA_WRONG_SIGNATURE_LENGTH;
    }

    unsigned char decrypt_buf[16384/*OPENSSL_RSA_MAX_MODULUS_BITS*/ / 8];
    int decrypt_len = RSA_public_decrypt_PKCS1_type_1((int)siglen, sigbuf, decrypt_buf, n, e);
    if (decrypt_len <= 0)
        return decrypt_len;

    // int_rsa_verify() from OpenSSL uses encode_pkcs1() function to format
    // the digest according to PKCS#1 type 1 pading scheme.
    // We only use SHA-1 digest and in that case encode_pkcs1() always produces
    // the same result: 15 bytes of fixed prefix followed by 20 bytes of the
    // digest value.
    // To avoid importing encode_pkcs1() with all its dependencies (which are
    // many) we hardcode the value of the fixed prefix.
    const unsigned char pkcs1_digest_info_sha1[] = {
        0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14,
    };

    if ((size_t)decrypt_len != sizeof(pkcs1_digest_info_sha1) + m_len
        || memcmp(decrypt_buf, pkcs1_digest_info_sha1, sizeof(pkcs1_digest_info_sha1)) != 0
        || memcmp(decrypt_buf + sizeof(pkcs1_digest_info_sha1), m, m_len) != 0) {
        return -RSA_BAD_SIGNATURE;
    }

    return 1;
}

// Based on rsa_ossl_public_decrypt()
SECURE_FUNC_INTERNAL
int RSA_public_decrypt_PKCS1_type_1(int flen, const unsigned char *from, unsigned char *to, BIGNUM *n, BIGNUM *e) noexcept
{
    BIGNUM *f, *ret;
    int i, num = 0;
    BN_CTX *ctx = NULL;

    if (BN_num_bits(n) > 16384/*OPENSSL_RSA_MAX_MODULUS_BITS*/) {
        return -RSA_MODULUS_TOO_LARGE;
    }

    if (BN_ucmp(n, e) <= 0) {
        return -RSA_BAD_E_VALUE;
    }

    /* for large moduli, enforce exponent limit */
    if (BN_num_bits(n) > 3072/*OPENSSL_RSA_SMALL_MODULUS_BITS*/) {
        if (BN_num_bits(e) > 64/*OPENSSL_RSA_MAX_PUBEXP_BITS*/) {
            return -RSA_BAD_E_VALUE;
        }
    }

    if ((ctx = BN_CTX_new()) == NULL)
        return -RSA_MALLOC_FAILURE;
    BN_CTX_start(ctx);
    VS_SCOPE_EXIT {
        // This code doesn't influence the result of the function, so it can be left unprotected.
        BN_CTX_end(ctx);
        BN_CTX_free(ctx);;
    };
    f = BN_CTX_get(ctx);
    ret = BN_CTX_get(ctx);
    num = BN_num_bytes(n);

    /*
     * This check was for equality but PGP does evil things and chops off the
     * top '0' bytes
     */
    if (flen > num) {
        return -RSA_DATA_GREATER_THAN_MOD_LEN;
    }

    unsigned char f_buffer[32 + 512];
    if (flen <= 512)
        f = BN_bin2bn_static(from, flen, f_buffer);
    else
    if (BN_bin2bn(from, flen, f) == NULL)
        return -RSA_MALLOC_FAILURE;

    if (BN_ucmp(f, n) >= 0) {
        return -RSA_DATA_TOO_LARGE_FOR_MODULUS;
    }

    if (!BN_mod_exp_mont(ret, f, e, n, ctx, NULL))
        return -RSA_MODEXP_ERROR;

    i = BN_bn2binpad(ret, to, num);

    return RSA_padding_check_PKCS1_type_1(to, num, to, i, num);
}

// Based on RSA_padding_check_PKCS1_type_1()
SECURE_FUNC_INTERNAL
int RSA_padding_check_PKCS1_type_1(unsigned char *to, int tlen, const unsigned char *from, int flen, int num) noexcept
{
    int i, j;
    const unsigned char *p;

    p = from;

    /*
     * The format is
     * 00 || 01 || PS || 00 || D
     * PS - padding string, at least 8 bytes of FF
     * D  - data.
     */

    if (num < 11)
        return -RSA_OTHER_ERROR;

    /* Accept inputs with and without the leading 0-byte. */
    if (num == flen) {
        if ((*p++) != 0x00) {
            return -RSA_INVALID_PADDING;
        }
        flen--;
    }

    if ((num != (flen + 1)) || (*(p++) != 0x01)) {
        return -RSA_BLOCK_TYPE_IS_NOT_01;
    }

    /* scan over padding data */
    j = flen - 1;               /* one for type. */
    for (i = 0; i < j; i++) {
        if (*p != 0xff) {       /* should decrypt to 0xff */
            if (*p == 0) {
                p++;
                break;
            } else {
                return -RSA_BAD_FIXED_HEADER_DECRYPT;
            }
        }
        p++;
    }

    if (i == j) {
        return -RSA_NULL_BEFORE_BLOCK_MISSING;
    }

    if (i < 8) {
        return -RSA_BAD_PAD_BYTE_COUNT;
    }
    i++;                        /* Skip over the '\0' */
    j -= i;
    if (j > tlen) {
        return -RSA_DATA_TOO_LARGE;
    }
    memmove(to, p, (unsigned int)j);

    return j;
}

}
