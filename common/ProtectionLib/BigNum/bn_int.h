/*
 * Copyright 2014-2018 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#pragma once

#include "bn.h"

namespace protection {

BIGNUM *bn_wexpand(BIGNUM *a, int words);
BIGNUM *bn_expand2(BIGNUM *a, int words);

void bn_correct_top(BIGNUM *a);

/*
 * Some BIGNUM functions assume most significant limb to be non-zero, which
 * is customarily arranged by bn_correct_top. Output from below functions
 * is not processed with bn_correct_top, and for this reason it may not be
 * returned out of public API. It may only be passed internally into other
 * functions known to support non-minimal or zero-padded BIGNUMs. Even
 * though the goal is to facilitate constant-time-ness, not each subroutine
 * is constant-time by itself. They all have pre-conditions, consult source
 * code...
 */
int bn_mul_mont_fixed_top(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                          BN_MONT_CTX *mont, BN_CTX *ctx);
int bn_to_mont_fixed_top(BIGNUM *r, const BIGNUM *a, BN_MONT_CTX *mont,
                         BN_CTX *ctx);
int bn_from_mont_fixed_top(BIGNUM *r, const BIGNUM *a, BN_MONT_CTX *mont,
                           BN_CTX *ctx);
int bn_mod_add_fixed_top(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                         const BIGNUM *m);
int bn_mod_sub_fixed_top(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                         const BIGNUM *m);
int bn_mul_fixed_top(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx);
int bn_sqr_fixed_top(BIGNUM *r, const BIGNUM *a, BN_CTX *ctx);
int bn_lshift_fixed_top(BIGNUM *r, const BIGNUM *a, int n);
int bn_rshift_fixed_top(BIGNUM *r, const BIGNUM *a, int n);
int bn_div_fixed_top(BIGNUM *dv, BIGNUM *rem, const BIGNUM *m,
                     const BIGNUM *d, BN_CTX *ctx);

}
