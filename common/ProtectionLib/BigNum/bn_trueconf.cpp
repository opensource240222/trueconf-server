/*
 * Copyright 1995-2019 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "bn_lcl.h"

#include <assert.h>

namespace protection {

SECURE_FUNC_INTERNAL
BIGNUM* BN_bin2bn_static(const unsigned char *s, int len, void* buffer)
{
    unsigned int i, m;
    unsigned int n;
    BN_ULONG l;
    BIGNUM *ret;

    assert(buffer);
    static_assert(sizeof(BIGNUM) <= 24, "BIGNUM is too big");
    ret = static_cast<BIGNUM*>(buffer);
    ret->d = static_cast<BN_ULONG*>(static_cast<void*>(static_cast<char*>(buffer) + 24));
    ret->neg = 0;
    ret->flags = BN_FLG_STATIC_DATA;

    /* Skip leading zero's. */
    for ( ; len > 0 && *s == 0; s++, len--)
        continue;
    n = len;
    if (n == 0) {
        ret->top = 0;
        ret->dmax = 0;
        return ret;
    }
    i = ((n - 1) / BN_BYTES) + 1;
    m = ((n - 1) % (BN_BYTES));
    ret->top = i;
    ret->dmax = i;
    l = 0;
    while (n--) {
        l = (l << 8L) | *(s++);
        if (m-- == 0) {
            ret->d[--i] = l;
            l = 0;
            m = BN_BYTES - 1;
        }
    }
    /*
     * need to call this due to clear byte at top if avoiding having the top
     * bit set (-ve number)
     */
    bn_correct_top(ret);
    return ret;
}

SECURE_FUNC_INTERNAL
BIGNUM* BN_word2bn_static(unsigned w, void* buffer)
{
    BIGNUM *ret;

    assert(buffer);
    static_assert(sizeof(BIGNUM) <= 24, "BIGNUM is too big");
    ret = static_cast<BIGNUM*>(buffer);
    ret->d = static_cast<BN_ULONG*>(static_cast<void*>(static_cast<char*>(buffer) + 24));
    ret->d[0] = w;
    ret->top = (w ? 1 : 0);
    ret->dmax = 1;
    ret->neg = 0;
    ret->flags = BN_FLG_STATIC_DATA;
    return ret;
}

}
