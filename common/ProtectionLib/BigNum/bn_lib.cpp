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
#include <limits.h>

namespace protection {

SECURE_FUNC_INTERNAL
const BIGNUM *BN_value_one(void)
{
    static const BN_ULONG data_one = 1L;
    static const BIGNUM const_one =
        { (BN_ULONG *)&data_one, 1, 1, 0, BN_FLG_STATIC_DATA };

    return &const_one;
}

SECURE_FUNC_INTERNAL
int BN_num_bits_word(BN_ULONG l)
{
    BN_ULONG x, mask;
    int bits = (l != 0);

#if BN_BITS2 > 32
    x = l >> 32;
    mask = (0 - x) & BN_MASK2;
    mask = (0 - (mask >> (BN_BITS2 - 1)));
    bits += 32 & mask;
    l ^= (x ^ l) & mask;
#endif

    x = l >> 16;
    mask = (0 - x) & BN_MASK2;
    mask = (0 - (mask >> (BN_BITS2 - 1)));
    bits += 16 & mask;
    l ^= (x ^ l) & mask;

    x = l >> 8;
    mask = (0 - x) & BN_MASK2;
    mask = (0 - (mask >> (BN_BITS2 - 1)));
    bits += 8 & mask;
    l ^= (x ^ l) & mask;

    x = l >> 4;
    mask = (0 - x) & BN_MASK2;
    mask = (0 - (mask >> (BN_BITS2 - 1)));
    bits += 4 & mask;
    l ^= (x ^ l) & mask;

    x = l >> 2;
    mask = (0 - x) & BN_MASK2;
    mask = (0 - (mask >> (BN_BITS2 - 1)));
    bits += 2 & mask;
    l ^= (x ^ l) & mask;

    x = l >> 1;
    mask = (0 - x) & BN_MASK2;
    mask = (0 - (mask >> (BN_BITS2 - 1)));
    bits += 1 & mask;

    return bits;
}

SECURE_FUNC_INTERNAL
int BN_num_bits(const BIGNUM *a)
{
    int i = a->top - 1;
    bn_check_top(a);

    if (BN_is_zero(a))
        return 0;
    return ((i * BN_BITS2) + BN_num_bits_word(a->d[i]));
}

SECURE_FUNC_INTERNAL
static void bn_free_d(BIGNUM *a)
{
        OPENSSL_free(a->d);
}


SECURE_FUNC_INTERNAL
void BN_clear_free(BIGNUM *a)
{
    if (a == NULL)
        return;
    if (a->d != NULL && !BN_get_flags(a, BN_FLG_STATIC_DATA)) {
        OPENSSL_cleanse(a->d, a->dmax * sizeof(a->d[0]));
        bn_free_d(a);
    }
    if (BN_get_flags(a, BN_FLG_MALLOCED)) {
        OPENSSL_cleanse(a, sizeof(*a));
        OPENSSL_free(a);
    }
}

SECURE_FUNC_INTERNAL
void BN_free(BIGNUM *a)
{
    if (a == NULL)
        return;
    if (!BN_get_flags(a, BN_FLG_STATIC_DATA))
        bn_free_d(a);
    if (a->flags & BN_FLG_MALLOCED)
        OPENSSL_free(a);
}

SECURE_FUNC_INTERNAL
void bn_init(BIGNUM *a)
{
    // This variable was made const because otherwise GCC fails to see that its value never changes.
    // As a result it produces strange assembly which breaks VMProtect (3.3.1 build 1096).
    static const BIGNUM nilbn {};

    *a = nilbn;
    bn_check_top(a);
}

SECURE_FUNC_INTERNAL
BIGNUM *BN_new(void)
{
    BIGNUM *ret;

    if ((ret = static_cast<BIGNUM*>(OPENSSL_zalloc(sizeof(*ret)))) == NULL) {
        BNerr(BN_F_BN_NEW, ERR_R_MALLOC_FAILURE);
        return NULL;
    }
    ret->flags = BN_FLG_MALLOCED;
    bn_check_top(ret);
    return ret;
}

/* This is used by bn_expand2() */
/* The caller MUST check that words > b->dmax before calling this */
SECURE_FUNC_INTERNAL
static BN_ULONG *bn_expand_internal(const BIGNUM *b, int words)
{
    BN_ULONG *a = NULL;

    if (words > (INT_MAX / (4 * BN_BITS2))) {
        BNerr(BN_F_BN_EXPAND_INTERNAL, BN_R_BIGNUM_TOO_LONG);
        return NULL;
    }
    if (BN_get_flags(b, BN_FLG_STATIC_DATA)) {
        BNerr(BN_F_BN_EXPAND_INTERNAL, BN_R_EXPAND_ON_STATIC_BIGNUM_DATA);
        return NULL;
    }
        a = static_cast<BN_ULONG*>(OPENSSL_zalloc(words * sizeof(*a)));
    if (a == NULL) {
        BNerr(BN_F_BN_EXPAND_INTERNAL, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    assert(b->top <= words);
    if (b->top > 0)
        memcpy(a, b->d, sizeof(*a) * b->top);

    return a;
}

/*
 * This is an internal function that should not be used in applications. It
 * ensures that 'b' has enough room for a 'words' word number and initialises
 * any unused part of b->d with leading zeros. It is mostly used by the
 * various BIGNUM routines. If there is an error, NULL is returned. If not,
 * 'b' is returned.
 */

SECURE_FUNC_INTERNAL
BIGNUM *bn_expand2(BIGNUM *b, int words)
{
    if (words > b->dmax) {
        BN_ULONG *a = bn_expand_internal(b, words);
        if (!a)
            return NULL;
        if (b->d) {
            OPENSSL_cleanse(b->d, b->dmax * sizeof(b->d[0]));
            bn_free_d(b);
        }
        b->d = a;
        b->dmax = words;
    }

    return b;
}

#ifdef BN_BUILD_ALL
SECURE_FUNC_INTERNAL
BIGNUM *BN_dup(const BIGNUM *a)
{
    BIGNUM *t;

    if (a == NULL)
        return NULL;
    bn_check_top(a);

    t = BN_new();
    if (t == NULL)
        return NULL;
    if (!BN_copy(t, a)) {
        BN_free(t);
        return NULL;
    }
    bn_check_top(t);
    return t;
}
#endif

SECURE_FUNC_INTERNAL
BIGNUM *BN_copy(BIGNUM *a, const BIGNUM *b)
{
    bn_check_top(b);

    if (a == b)
        return a;
    if (bn_wexpand(a, b->top) == NULL)
        return NULL;

    if (b->top > 0)
        memcpy(a->d, b->d, sizeof(b->d[0]) * b->top);

    a->neg = b->neg;
    a->top = b->top;
    a->flags |= b->flags & BN_FLG_FIXED_TOP;
    bn_check_top(a);
    return a;
}

#ifdef BN_BUILD_ALL
#define FLAGS_DATA(flags) ((flags) & (BN_FLG_STATIC_DATA \
                                    | BN_FLG_FIXED_TOP))
#define FLAGS_STRUCT(flags) ((flags) & (BN_FLG_MALLOCED))

SECURE_FUNC_INTERNAL
void BN_swap(BIGNUM *a, BIGNUM *b)
{
    int flags_old_a, flags_old_b;
    BN_ULONG *tmp_d;
    int tmp_top, tmp_dmax, tmp_neg;

    bn_check_top(a);
    bn_check_top(b);

    flags_old_a = a->flags;
    flags_old_b = b->flags;

    tmp_d = a->d;
    tmp_top = a->top;
    tmp_dmax = a->dmax;
    tmp_neg = a->neg;

    a->d = b->d;
    a->top = b->top;
    a->dmax = b->dmax;
    a->neg = b->neg;

    b->d = tmp_d;
    b->top = tmp_top;
    b->dmax = tmp_dmax;
    b->neg = tmp_neg;

    a->flags = FLAGS_STRUCT(flags_old_a) | FLAGS_DATA(flags_old_b);
    b->flags = FLAGS_STRUCT(flags_old_b) | FLAGS_DATA(flags_old_a);
    bn_check_top(a);
    bn_check_top(b);
}

SECURE_FUNC_INTERNAL
void BN_clear(BIGNUM *a)
{
    if (a == NULL)
        return;
    bn_check_top(a);
    if (a->d != NULL)
        OPENSSL_cleanse(a->d, sizeof(*a->d) * a->dmax);
    a->neg = 0;
    a->top = 0;
    a->flags &= ~BN_FLG_FIXED_TOP;
}

SECURE_FUNC_INTERNAL
BN_ULONG BN_get_word(const BIGNUM *a)
{
    if (a->top > 1)
        return BN_MASK2;
    else if (a->top == 1)
        return a->d[0];
    /* a->top == 0 */
    return 0;
}
#endif

SECURE_FUNC_INTERNAL
int BN_set_word(BIGNUM *a, BN_ULONG w)
{
    bn_check_top(a);
    if (bn_expand(a, (int)sizeof(BN_ULONG) * 8) == NULL)
        return 0;
    a->neg = 0;
    a->d[0] = w;
    a->top = (w ? 1 : 0);
    a->flags &= ~BN_FLG_FIXED_TOP;
    bn_check_top(a);
    return 1;
}

SECURE_FUNC_INTERNAL
BIGNUM *BN_bin2bn(const unsigned char *s, int len, BIGNUM *ret)
{
    unsigned int i, m;
    unsigned int n;
    BN_ULONG l;
    BIGNUM *bn = NULL;

    if (ret == NULL)
        ret = bn = BN_new();
    if (ret == NULL)
        return NULL;
    bn_check_top(ret);
    /* Skip leading zero's. */
    for ( ; len > 0 && *s == 0; s++, len--)
        continue;
    n = len;
    if (n == 0) {
        ret->top = 0;
        return ret;
    }
    i = ((n - 1) / BN_BYTES) + 1;
    m = ((n - 1) % (BN_BYTES));
    if (bn_wexpand(ret, (int)i) == NULL) {
        BN_free(bn);
        return NULL;
    }
    ret->top = i;
    ret->neg = 0;
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

/* ignore negative */
SECURE_FUNC_INTERNAL
static int bn2binpad(const BIGNUM *a, unsigned char *to, int tolen)
{
    int n;
    size_t i, lasti, j, atop, mask;
    BN_ULONG l;

    /*
     * In case |a| is fixed-top, BN_num_bytes can return bogus length,
     * but it's assumed that fixed-top inputs ought to be "nominated"
     * even for padded output, so it works out...
     */
    n = BN_num_bytes(a);
    if (tolen == -1) {
        tolen = n;
    } else if (tolen < n) {     /* uncommon/unlike case */
        BIGNUM temp = *a;

        bn_correct_top(&temp);
        n = BN_num_bytes(&temp);
        if (tolen < n)
            return -1;
    }

    /* Swipe through whole available data and don't give away padded zero. */
    atop = a->dmax * BN_BYTES;
    if (atop == 0) {
        OPENSSL_cleanse(to, tolen);
        return tolen;
    }

    lasti = atop - 1;
    atop = a->top * BN_BYTES;
    for (i = 0, j = 0, to += tolen; j < (size_t)tolen; j++) {
        l = a->d[i / BN_BYTES];
        mask = 0 - ((j - atop) >> (8 * sizeof(i) - 1));
        *--to = (unsigned char)(l >> (8 * (i % BN_BYTES)) & mask);
        i += (i - lasti) >> (8 * sizeof(i) - 1); /* stay on last limb */
    }

    return tolen;
}

SECURE_FUNC_INTERNAL
int BN_bn2binpad(const BIGNUM *a, unsigned char *to, int tolen)
{
    if (tolen < 0)
        return -1;
    return bn2binpad(a, to, tolen);
}

#ifdef BN_BUILD_ALL
SECURE_FUNC_INTERNAL
int BN_bn2bin(const BIGNUM *a, unsigned char *to)
{
    return bn2binpad(a, to, -1);
}

SECURE_FUNC_INTERNAL
BIGNUM *BN_lebin2bn(const unsigned char *s, int len, BIGNUM *ret)
{
    unsigned int i, m;
    unsigned int n;
    BN_ULONG l;
    BIGNUM *bn = NULL;

    if (ret == NULL)
        ret = bn = BN_new();
    if (ret == NULL)
        return NULL;
    bn_check_top(ret);
    s += len;
    /* Skip trailing zeroes. */
    for ( ; len > 0 && s[-1] == 0; s--, len--)
        continue;
    n = len;
    if (n == 0) {
        ret->top = 0;
        return ret;
    }
    i = ((n - 1) / BN_BYTES) + 1;
    m = ((n - 1) % (BN_BYTES));
    if (bn_wexpand(ret, (int)i) == NULL) {
        BN_free(bn);
        return NULL;
    }
    ret->top = i;
    ret->neg = 0;
    l = 0;
    while (n--) {
        s--;
        l = (l << 8L) | *s;
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
int BN_bn2lebinpad(const BIGNUM *a, unsigned char *to, int tolen)
{
    int i;
    BN_ULONG l;
    bn_check_top(a);
    i = BN_num_bytes(a);
    if (tolen < i)
        return -1;
    /* Add trailing zeroes if necessary */
    if (tolen > i)
        memset(to + i, 0, tolen - i);
    to += i;
    while (i--) {
        l = a->d[i / BN_BYTES];
        to--;
        *to = (unsigned char)(l >> (8 * (i % BN_BYTES))) & 0xff;
    }
    return tolen;
}
#endif

SECURE_FUNC_INTERNAL
int BN_ucmp(const BIGNUM *a, const BIGNUM *b)
{
    int i;
    BN_ULONG t1, t2, *ap, *bp;

    bn_check_top(a);
    bn_check_top(b);

    i = a->top - b->top;
    if (i != 0)
        return i;
    ap = a->d;
    bp = b->d;
    for (i = a->top - 1; i >= 0; i--) {
        t1 = ap[i];
        t2 = bp[i];
        if (t1 != t2)
            return ((t1 > t2) ? 1 : -1);
    }
    return 0;
}

#ifdef BN_BUILD_ALL
SECURE_FUNC_INTERNAL
int BN_cmp(const BIGNUM *a, const BIGNUM *b)
{
    int i;
    int gt, lt;
    BN_ULONG t1, t2;

    if ((a == NULL) || (b == NULL)) {
        if (a != NULL)
            return -1;
        else if (b != NULL)
            return 1;
        else
            return 0;
    }

    bn_check_top(a);
    bn_check_top(b);

    if (a->neg != b->neg) {
        if (a->neg)
            return -1;
        else
            return 1;
    }
    if (a->neg == 0) {
        gt = 1;
        lt = -1;
    } else {
        gt = -1;
        lt = 1;
    }

    if (a->top > b->top)
        return gt;
    if (a->top < b->top)
        return lt;
    for (i = a->top - 1; i >= 0; i--) {
        t1 = a->d[i];
        t2 = b->d[i];
        if (t1 > t2)
            return gt;
        if (t1 < t2)
            return lt;
    }
    return 0;
}
#endif

SECURE_FUNC_INTERNAL
int BN_set_bit(BIGNUM *a, int n)
{
    int i, j, k;

    if (n < 0)
        return 0;

    i = n / BN_BITS2;
    j = n % BN_BITS2;
    if (a->top <= i) {
        if (bn_wexpand(a, i + 1) == NULL)
            return 0;
        for (k = a->top; k < i + 1; k++)
            a->d[k] = 0;
        a->top = i + 1;
        a->flags &= ~BN_FLG_FIXED_TOP;
    }

    a->d[i] |= (((BN_ULONG)1) << j);
    bn_check_top(a);
    return 1;
}

#ifdef BN_BUILD_ALL
SECURE_FUNC_INTERNAL
int BN_clear_bit(BIGNUM *a, int n)
{
    int i, j;

    bn_check_top(a);
    if (n < 0)
        return 0;

    i = n / BN_BITS2;
    j = n % BN_BITS2;
    if (a->top <= i)
        return 0;

    a->d[i] &= (~(((BN_ULONG)1) << j));
    bn_correct_top(a);
    return 1;
}
#endif

SECURE_FUNC_INTERNAL
int BN_is_bit_set(const BIGNUM *a, int n)
{
    int i, j;

    bn_check_top(a);
    if (n < 0)
        return 0;
    i = n / BN_BITS2;
    j = n % BN_BITS2;
    if (a->top <= i)
        return 0;
    return (int)(((a->d[i]) >> j) & ((BN_ULONG)1));
}

#ifdef BN_BUILD_ALL
SECURE_FUNC_INTERNAL
int BN_mask_bits(BIGNUM *a, int n)
{
    int b, w;

    bn_check_top(a);
    if (n < 0)
        return 0;

    w = n / BN_BITS2;
    b = n % BN_BITS2;
    if (w >= a->top)
        return 0;
    if (b == 0)
        a->top = w;
    else {
        a->top = w + 1;
        a->d[w] &= ~(BN_MASK2 << b);
    }
    bn_correct_top(a);
    return 1;
}
#endif

SECURE_FUNC_INTERNAL
void BN_set_negative(BIGNUM *a, int b)
{
    if (b && !BN_is_zero(a))
        a->neg = 1;
    else
        a->neg = 0;
}

SECURE_FUNC_INTERNAL
int bn_cmp_words(const BN_ULONG *a, const BN_ULONG *b, int n)
{
    int i;
    BN_ULONG aa, bb;

    if (n == 0)
        return 0;

    aa = a[n - 1];
    bb = b[n - 1];
    if (aa != bb)
        return ((aa > bb) ? 1 : -1);
    for (i = n - 2; i >= 0; i--) {
        aa = a[i];
        bb = b[i];
        if (aa != bb)
            return ((aa > bb) ? 1 : -1);
    }
    return 0;
}

/*
 * Here follows a specialised variants of bn_cmp_words().  It has the
 * capability of performing the operation on arrays of different sizes. The
 * sizes of those arrays is expressed through cl, which is the common length
 * ( basically, min(len(a),len(b)) ), and dl, which is the delta between the
 * two lengths, calculated as len(a)-len(b). All lengths are the number of
 * BN_ULONGs...
 */

SECURE_FUNC_INTERNAL
int bn_cmp_part_words(const BN_ULONG *a, const BN_ULONG *b, int cl, int dl)
{
    int n, i;
    n = cl - 1;

    if (dl < 0) {
        for (i = dl; i < 0; i++) {
            if (b[n - i] != 0)
                return -1;      /* a < b */
        }
    }
    if (dl > 0) {
        for (i = dl; i > 0; i--) {
            if (a[n + i] != 0)
                return 1;       /* a > b */
        }
    }
    return bn_cmp_words(a, b, cl);
}

#ifdef BN_BUILD_ALL
/*-
 * Constant-time conditional swap of a and b.
 * a and b are swapped if condition is not 0.
 * nwords is the number of words to swap.
 * Assumes that at least nwords are allocated in both a and b.
 * Assumes that no more than nwords are used by either a or b.
 */
SECURE_FUNC_INTERNAL
void BN_consttime_swap(BN_ULONG condition, BIGNUM *a, BIGNUM *b, int nwords)
{
    BN_ULONG t;
    int i;

    if (a == b)
        return;

    bn_wcheck_size(a, nwords);
    bn_wcheck_size(b, nwords);

    condition = ((~condition & ((condition - 1))) >> (BN_BITS2 - 1)) - 1;

    t = (a->top ^ b->top) & condition;
    a->top ^= t;
    b->top ^= t;

    t = (a->neg ^ b->neg) & condition;
    a->neg ^= t;
    b->neg ^= t;

    /*-
     * BN_FLG_STATIC_DATA: indicates that data may not be written to. Intention
     * is actually to treat it as it's read-only data, and some (if not most)
     * of it does reside in read-only segment. In other words observation of
     * BN_FLG_STATIC_DATA in BN_consttime_swap should be treated as fatal
     * condition. It would either cause SEGV or effectively cause data
     * corruption.
     *
     * BN_FLG_MALLOCED: refers to BN structure itself, and hence must be
     * preserved.
     *
     * BN_FLG_FIXED_TOP: indicates that we haven't called bn_correct_top() on
     * the data, so the d array may be padded with additional 0 values (i.e.
     * top could be greater than the minimal value that it could be). We should
     * be swapping it
     */

#define BN_CONSTTIME_SWAP_FLAGS (BN_FLG_FIXED_TOP)

    t = ((a->flags ^ b->flags) & BN_CONSTTIME_SWAP_FLAGS) & condition;
    a->flags ^= t;
    b->flags ^= t;

    /* conditionally swap the data */
    for (i = 0; i < nwords; i++) {
        t = (a->d[i] ^ b->d[i]) & condition;
        a->d[i] ^= t;
        b->d[i] ^= t;
    }
}

#undef BN_CONSTTIME_SWAP_FLAGS

/* Bits of security, see SP800-57 */

SECURE_FUNC_INTERNAL
int BN_security_bits(int L, int N)
{
    int secbits, bits;
    if (L >= 15360)
        secbits = 256;
    else if (L >= 7680)
        secbits = 192;
    else if (L >= 3072)
        secbits = 128;
    else if (L >= 2048)
        secbits = 112;
    else if (L >= 1024)
        secbits = 80;
    else
        return 0;
    if (N == -1)
        return secbits;
    bits = N / 2;
    if (bits < 80)
        return 0;
    return bits >= secbits ? secbits : bits;
}
#endif

SECURE_FUNC_INTERNAL
void BN_zero_ex(BIGNUM *a)
{
    a->neg = 0;
    a->top = 0;
    a->flags &= ~BN_FLG_FIXED_TOP;
}

SECURE_FUNC_INTERNAL
int BN_abs_is_word(const BIGNUM *a, const BN_ULONG w)
{
    return ((a->top == 1) && (a->d[0] == w)) || ((w == 0) && (a->top == 0));
}

SECURE_FUNC_INTERNAL
int BN_is_zero(const BIGNUM *a)
{
    return a->top == 0;
}

SECURE_FUNC_INTERNAL
int BN_is_one(const BIGNUM *a)
{
    return BN_abs_is_word(a, 1) && !a->neg;
}

SECURE_FUNC_INTERNAL
int BN_is_word(const BIGNUM *a, const BN_ULONG w)
{
    return BN_abs_is_word(a, w) && (!w || !a->neg);
}

SECURE_FUNC_INTERNAL
int BN_is_odd(const BIGNUM *a)
{
    return (a->top > 0) && (a->d[0] & 1);
}

#ifdef BN_BUILD_ALL
SECURE_FUNC_INTERNAL
int BN_is_negative(const BIGNUM *a)
{
    return (a->neg != 0);
}

SECURE_FUNC_INTERNAL
int BN_to_montgomery(BIGNUM *r, const BIGNUM *a, BN_MONT_CTX *mont,
                     BN_CTX *ctx)
{
    return BN_mod_mul_montgomery(r, a, &(mont->RR), mont, ctx);
}

SECURE_FUNC_INTERNAL
void BN_with_flags(BIGNUM *dest, const BIGNUM *b, int flags)
{
    dest->d = b->d;
    dest->top = b->top;
    dest->dmax = b->dmax;
    dest->neg = b->neg;
    dest->flags = ((dest->flags & BN_FLG_MALLOCED)
                   | (b->flags & ~BN_FLG_MALLOCED)
                   | BN_FLG_STATIC_DATA | flags);
}

SECURE_FUNC_INTERNAL
void BN_set_flags(BIGNUM *b, int n)
{
    b->flags |= n;
}
#endif

SECURE_FUNC_INTERNAL
int BN_get_flags(const BIGNUM *b, int n)
{
    return b->flags & n;
}

SECURE_FUNC_INTERNAL
BIGNUM *bn_wexpand(BIGNUM *a, int words)
{
    return (words <= a->dmax) ? a : bn_expand2(a, words);
}

SECURE_FUNC_INTERNAL
void bn_correct_top(BIGNUM *a)
{
    BN_ULONG *ftl;
    int tmp_top = a->top;

    if (tmp_top > 0) {
        for (ftl = &(a->d[tmp_top]); tmp_top > 0; tmp_top--) {
            ftl--;
            if (*ftl != 0)
                break;
        }
        a->top = tmp_top;
    }
    if (a->top == 0)
        a->neg = 0;
    a->flags &= ~BN_FLG_FIXED_TOP;
    bn_pollute(a);
}

}
