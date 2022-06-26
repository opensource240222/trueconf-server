#pragma once

#include "BigNum/bn.h"

namespace protection {

// Works same as BN_bin2bn() with following exceptions:
// 1. Stores BIGNUM and the underlying data in the supplied |buffer|.
//    Size of the |buffer| must be at least |32 + len| bytes.
// 2. Always succeeds and never returns NULL.
// 3. There is no need to call BN_free() on the returned pointer (but this is not prohibited).
BIGNUM *BN_bin2bn_static(const unsigned char *s, int len, void* buffer);

// Works same as BN_bin2bn_static(), but accepts value as a single number.
BIGNUM* BN_word2bn_static(unsigned w, void* buffer);

}
