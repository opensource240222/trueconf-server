/*
 * Copyright 1995-2018 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#pragma once

#if defined(OPENSSL_VERSION_NUMBER) || defined(OPENSSL_VERSION_TEXT)
#   error This file conflicts with OpenSSL
#endif

#include <limits.h>
#include <stdlib.h>

// Platform specific configuration.
#if defined(_WIN32) && defined(_M_X64)
#   define SIXTY_FOUR_BIT
#elif defined(__linux__) && defined(__amd64)
#   define SIXTY_FOUR_BIT_LONG
#else
    static_assert(sizeof(void*) == 4, "It looks like this is not a 32-bit architecture, but that wasn't detected by #if conditions.");
    static_assert(sizeof(long long) == 8, "This platform has a very strange 'long long' type, additional #ifs are required to not define BN_LLONG on it.");
#   define THIRTY_TWO_BIT
#   define BN_LLONG
#endif

#include "bn_int.h"
#include "ProtectionLib/Lib.h"
#include "ProtectionLib/Protection.h"

// Uncomment to build all currently unused functions.
//#define BN_BUILD_ALL

namespace protection {

//
// BEGIN: bn.h
//

/*
 * 64-bit processor with LP64 ABI
 */
# ifdef SIXTY_FOUR_BIT_LONG
#  define BN_ULONG        unsigned long
#  define BN_BYTES        8
# endif

/*
 * 64-bit processor other than LP64 ABI
 */
# ifdef SIXTY_FOUR_BIT
#  define BN_ULONG        unsigned long long
#  define BN_BYTES        8
# endif

# ifdef THIRTY_TWO_BIT
#  define BN_ULONG        unsigned int
#  define BN_BYTES        4
# endif

# define BN_BITS2       (BN_BYTES * 8)
# define BN_BITS        (BN_BITS2 * 2)
# define BN_TBIT        ((BN_ULONG)1 << (BN_BITS2 - 1))

# define BN_FLG_MALLOCED         0x01
# define BN_FLG_STATIC_DATA      0x02

//
// END: bn.h
//

/*
 * These preprocessor symbols control various aspects of the bignum headers
 * and library code. They're not defined by any "normal" configuration, as
 * they are intended for development and testing purposes. NB: defining all
 * three can be useful for debugging application code as well as openssl
 * itself. BN_DEBUG - turn on various debugging alterations to the bignum
 * code BN_DEBUG_RAND - uses random poisoning of unused words to trip up
 * mismanagement of bignum internals. You must also define BN_DEBUG.
 */
/* #define BN_DEBUG */
/* #define BN_DEBUG_RAND */

#  define BN_MUL_COMBA
#  define BN_SQR_COMBA
#  define BN_RECURSION

/*
 * This next option uses the C libraries (2 word)/(1 word) function. If it is
 * not defined, I use my C version (which is slower). The reason for this
 * flag is that when the particular C compiler library routine is used, and
 * the library is linked with a different compiler, the library is missing.
 * This mostly happens when the library is built with gcc and then linked
 * using normal cc.  This would be a common occurrence because gcc normally
 * produces code that is 2 times faster than system compilers for the big
 * number stuff. For machines with only one compiler (or shared libraries),
 * this should be on.  Again this in only really a problem on machines using
 * "long long's", are 32bit, and are not using my assembler code.
 */
#  define BN_DIV2W

/*
 * 64-bit processor with LP64 ABI
 */
# ifdef SIXTY_FOUR_BIT_LONG
#  define BN_ULLONG       unsigned long long
#  define BN_BITS4        32
#  define BN_MASK2        (0xffffffffffffffffL)
#  define BN_MASK2l       (0xffffffffL)
#  define BN_MASK2h       (0xffffffff00000000L)
#  define BN_MASK2h1      (0xffffffff80000000L)
#  define BN_DEC_CONV     (10000000000000000000UL)
#  define BN_DEC_NUM      19
#  define BN_DEC_FMT1     "%lu"
#  define BN_DEC_FMT2     "%019lu"
# endif

/*
 * 64-bit processor other than LP64 ABI
 */
# ifdef SIXTY_FOUR_BIT
#  undef BN_LLONG
#  undef BN_ULLONG
#  define BN_BITS4        32
#  define BN_MASK2        (0xffffffffffffffffLL)
#  define BN_MASK2l       (0xffffffffL)
#  define BN_MASK2h       (0xffffffff00000000LL)
#  define BN_MASK2h1      (0xffffffff80000000LL)
#  define BN_DEC_CONV     (10000000000000000000ULL)
#  define BN_DEC_NUM      19
#  define BN_DEC_FMT1     "%llu"
#  define BN_DEC_FMT2     "%019llu"
# endif

# ifdef THIRTY_TWO_BIT
#  ifdef BN_LLONG
#   if defined(_WIN32) && !defined(__GNUC__)
#    define BN_ULLONG     unsigned __int64
#   else
#    define BN_ULLONG     unsigned long long
#   endif
#  endif
#  define BN_BITS4        16
#  define BN_MASK2        (0xffffffffL)
#  define BN_MASK2l       (0xffff)
#  define BN_MASK2h1      (0xffff8000L)
#  define BN_MASK2h       (0xffff0000L)
#  define BN_DEC_CONV     (1000000000L)
#  define BN_DEC_NUM      9
#  define BN_DEC_FMT1     "%u"
#  define BN_DEC_FMT2     "%09u"
# endif


/*-
 * Bignum consistency macros
 * There is one "API" macro, bn_fix_top(), for stripping leading zeroes from
 * bignum data after direct manipulations on the data. There is also an
 * "internal" macro, bn_check_top(), for verifying that there are no leading
 * zeroes. Unfortunately, some auditing is required due to the fact that
 * bn_fix_top() has become an overabused duct-tape because bignum data is
 * occasionally passed around in an inconsistent state. So the following
 * changes have been made to sort this out;
 * - bn_fix_top()s implementation has been moved to bn_correct_top()
 * - if BN_DEBUG isn't defined, bn_fix_top() maps to bn_correct_top(), and
 *   bn_check_top() is as before.
 * - if BN_DEBUG *is* defined;
 *   - bn_check_top() tries to pollute unused words even if the bignum 'top' is
 *     consistent. (ed: only if BN_DEBUG_RAND is defined)
 *   - bn_fix_top() maps to bn_check_top() rather than "fixing" anything.
 * The idea is to have debug builds flag up inconsistent bignums when they
 * occur. If that occurs in a bn_fix_top(), we examine the code in question; if
 * the use of bn_fix_top() was appropriate (ie. it follows directly after code
 * that manipulates the bignum) it is converted to bn_correct_top(), and if it
 * was not appropriate, we convert it permanently to bn_check_top() and track
 * down the cause of the bug. Eventually, no internal code should be using the
 * bn_fix_top() macro. External applications and libraries should try this with
 * their own code too, both in terms of building against the openssl headers
 * with BN_DEBUG defined *and* linking with a version of OpenSSL built with it
 * defined. This not only improves external code, it provides more test
 * coverage for openssl's own code.
 */

# ifdef BN_DEBUG
/*
 * The new BN_FLG_FIXED_TOP flag marks vectors that were not treated with
 * bn_correct_top, in other words such vectors are permitted to have zeros
 * in most significant limbs. Such vectors are used internally to achieve
 * execution time invariance for critical operations with private keys.
 * It's BN_DEBUG-only flag, because user application is not supposed to
 * observe it anyway. Moreover, optimizing compiler would actually remove
 * all operations manipulating the bit in question in non-BN_DEBUG build.
 */
#  define BN_FLG_FIXED_TOP 0x10000
#   define bn_pollute(a)
#  define bn_check_top(a) \
        do { \
                const BIGNUM *_bnum2 = (a); \
                if (_bnum2 != NULL) { \
                        int _top = _bnum2->top; \
                        assert((_top == 0 && !_bnum2->neg) || \
                                  (_top && ((_bnum2->flags & BN_FLG_FIXED_TOP) \
                                            || _bnum2->d[_top - 1] != 0))); \
                        bn_pollute(_bnum2); \
                } \
        } while(0)

#  define bn_fix_top(a)           bn_check_top(a)

#  define bn_check_size(bn, bits) bn_wcheck_size(bn, ((bits+BN_BITS2-1))/BN_BITS2)
#  define bn_wcheck_size(bn, words) \
        do { \
                const BIGNUM *_bnum2 = (bn); \
                assert((words) <= (_bnum2)->dmax && \
                       (words) >= (_bnum2)->top); \
                /* avoid unused variable warning with NDEBUG */ \
                (void)(_bnum2); \
        } while(0)

# else                          /* !BN_DEBUG */

#  define BN_FLG_FIXED_TOP 0
#  define bn_pollute(a)
#  define bn_check_top(a)
#  define bn_fix_top(a)           bn_correct_top(a)
#  define bn_check_size(bn, bits)
#  define bn_wcheck_size(bn, words)

# endif

BN_ULONG bn_mul_add_words(BN_ULONG *rp, const BN_ULONG *ap, int num,
                          BN_ULONG w);
BN_ULONG bn_mul_words(BN_ULONG *rp, const BN_ULONG *ap, int num, BN_ULONG w);
void bn_sqr_words(BN_ULONG *rp, const BN_ULONG *ap, int num);
BN_ULONG bn_div_words(BN_ULONG h, BN_ULONG l, BN_ULONG d);
BN_ULONG bn_add_words(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp,
                      int num);
BN_ULONG bn_sub_words(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp,
                      int num);

struct bignum_st {
    BN_ULONG *d;                /* Pointer to an array of 'BN_BITS2' bit
                                 * chunks. */
    int top;                    /* Index of last used d +1. */
    /* The next are internal book keeping for bn_expand. */
    int dmax;                   /* Size of the d array. */
    int neg;                    /* one if the number is negative */
    int flags;
};

/* Used for montgomery multiplication */
struct bn_mont_ctx_st {
    int ri;                     /* number of bits in R */
    BIGNUM RR;                  /* used to convert to montgomery form,
                                   possibly zero-padded */
    BIGNUM N;                   /* The modulus */
    BIGNUM Ni;                  /* R*(1/R mod N) - N*Ni = 1 (Ni is only
                                 * stored for bignum algorithm) */
    BN_ULONG n0[2];             /* least significant word(s) of Ni; (type
                                 * changed with 0.9.9, was "BN_ULONG n0;"
                                 * before) */
    int flags;
};

/*-
 * BN_window_bits_for_exponent_size -- macro for sliding window mod_exp functions
 *
 *
 * For window size 'w' (w >= 2) and a random 'b' bits exponent,
 * the number of multiplications is a constant plus on average
 *
 *    2^(w-1) + (b-w)/(w+1);
 *
 * here  2^(w-1)  is for precomputing the table (we actually need
 * entries only for windows that have the lowest bit set), and
 * (b-w)/(w+1)  is an approximation for the expected number of
 * w-bit windows, not counting the first one.
 *
 * Thus we should use
 *
 *    w >= 6  if        b > 671
 *     w = 5  if  671 > b > 239
 *     w = 4  if  239 > b >  79
 *     w = 3  if   79 > b >  23
 *    w <= 2  if   23 > b
 *
 * (with draws in between).  Very small exponents are often selected
 * with low Hamming weight, so we use  w = 1  for b <= 23.
 */
# define BN_window_bits_for_exponent_size(b) \
                ((b) > 671 ? 6 : \
                 (b) > 239 ? 5 : \
                 (b) >  79 ? 4 : \
                 (b) >  23 ? 3 : 1)

/* Pentium pro 16,16,16,32,64 */
/* Alpha       16,16,16,16.64 */
# define BN_MULL_SIZE_NORMAL                     (16)/* 32 */
# define BN_MUL_RECURSIVE_SIZE_NORMAL            (16)/* 32 less than */
# define BN_SQR_RECURSIVE_SIZE_NORMAL            (16)/* 32 */

# if !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM) && !defined(PEDANTIC)
/*
 * BN_UMULT_HIGH section.
 * If the compiler doesn't support 2*N integer type, then you have to
 * replace every N*N multiplication with 4 (N/2)*(N/2) accompanied by some
 * shifts and additions which unavoidably results in severe performance
 * penalties. Of course provided that the hardware is capable of producing
 * 2*N result... That's when you normally start considering assembler
 * implementation. However! It should be pointed out that some CPUs (e.g.,
 * PowerPC, Alpha, and IA-64) provide *separate* instruction calculating
 * the upper half of the product placing the result into a general
 * purpose register. Now *if* the compiler supports inline assembler,
 * then it's not impossible to implement the "bignum" routines (and have
 * the compiler optimize 'em) exhibiting "native" performance in C. That's
 * what BN_UMULT_HIGH macro is about:-) Note that more recent compilers do
 * support 2*64 integer type, which is also used here.
 */
#  if defined(__SIZEOF_INT128__) && __SIZEOF_INT128__==16 && \
      (defined(SIXTY_FOUR_BIT) || defined(SIXTY_FOUR_BIT_LONG))
#   define BN_UMULT_HIGH(a,b)          (((__uint128_t)(a)*(b))>>64)
#   define BN_UMULT_LOHI(low,high,a,b) ({       \
        __uint128_t ret=(__uint128_t)(a)*(b);   \
        (high)=ret>>64; (low)=ret;      })
#  elif defined(__alpha) && (defined(SIXTY_FOUR_BIT_LONG) || defined(SIXTY_FOUR_BIT))
#   if defined(__DECC)
#    include <c_asm.h>
#    define BN_UMULT_HIGH(a,b)   (BN_ULONG)asm("umulh %a0,%a1,%v0",(a),(b))
#   elif defined(__GNUC__) && __GNUC__>=2
#    define BN_UMULT_HIGH(a,b)   ({     \
        register BN_ULONG ret;          \
        asm ("umulh     %1,%2,%0"       \
             : "=r"(ret)                \
             : "r"(a), "r"(b));         \
        ret;                      })
#   endif                       /* compiler */
#  elif defined(_ARCH_PPC64) && defined(SIXTY_FOUR_BIT_LONG)
#   if defined(__GNUC__) && __GNUC__>=2
#    define BN_UMULT_HIGH(a,b)   ({     \
        register BN_ULONG ret;          \
        asm ("mulhdu    %0,%1,%2"       \
             : "=r"(ret)                \
             : "r"(a), "r"(b));         \
        ret;                      })
#   endif                       /* compiler */
#  elif (defined(__x86_64) || defined(__x86_64__)) && \
       (defined(SIXTY_FOUR_BIT_LONG) || defined(SIXTY_FOUR_BIT))
#   if defined(__GNUC__) && __GNUC__>=2
#    define BN_UMULT_HIGH(a,b)   ({     \
        register BN_ULONG ret,discard;  \
        asm ("mulq      %3"             \
             : "=a"(discard),"=d"(ret)  \
             : "a"(a), "g"(b)           \
             : "cc");                   \
        ret;                      })
#    define BN_UMULT_LOHI(low,high,a,b) \
        asm ("mulq      %3"             \
                : "=a"(low),"=d"(high)  \
                : "a"(a),"g"(b)         \
                : "cc");
#   endif
#  elif (defined(_M_AMD64) || defined(_M_X64)) && defined(SIXTY_FOUR_BIT)
#   if defined(_MSC_VER) && _MSC_VER>=1400
extern "C"
unsigned __int64 __umulh(unsigned __int64 a, unsigned __int64 b);
extern "C"
unsigned __int64 _umul128(unsigned __int64 a, unsigned __int64 b,
                          unsigned __int64 *h);
#    pragma intrinsic(__umulh,_umul128)
#    define BN_UMULT_HIGH(a,b)           __umulh((a),(b))
#    define BN_UMULT_LOHI(low,high,a,b)  ((low)=_umul128((a),(b),&(high)))
#   endif
#  elif defined(__mips) && (defined(SIXTY_FOUR_BIT) || defined(SIXTY_FOUR_BIT_LONG))
#   if defined(__GNUC__) && __GNUC__>=2
#    define BN_UMULT_HIGH(a,b) ({       \
        register BN_ULONG ret;          \
        asm ("dmultu    %1,%2"          \
             : "=h"(ret)                \
             : "r"(a), "r"(b) : "l");   \
        ret;                    })
#    define BN_UMULT_LOHI(low,high,a,b) \
        asm ("dmultu    %2,%3"          \
             : "=l"(low),"=h"(high)     \
             : "r"(a), "r"(b));
#   endif
#  elif defined(__aarch64__) && defined(SIXTY_FOUR_BIT_LONG)
#   if defined(__GNUC__) && __GNUC__>=2
#    define BN_UMULT_HIGH(a,b)   ({     \
        register BN_ULONG ret;          \
        asm ("umulh     %0,%1,%2"       \
             : "=r"(ret)                \
             : "r"(a), "r"(b));         \
        ret;                      })
#   endif
#  endif                        /* cpu */
# endif                         /* OPENSSL_NO_ASM */

#  define bn_clear_top2max(a)

# ifdef BN_LLONG
/*******************************************************************
 * Using the long long type, has to be twice as wide as BN_ULONG...
 */
#  define Lw(t)    (((BN_ULONG)(t))&BN_MASK2)
#  define Hw(t)    (((BN_ULONG)((t)>>BN_BITS2))&BN_MASK2)

#  define mul_add(r,a,w,c) { \
        BN_ULLONG t; \
        t=(BN_ULLONG)w * (a) + (r) + (c); \
        (r)= Lw(t); \
        (c)= Hw(t); \
        }

#  define mul(r,a,w,c) { \
        BN_ULLONG t; \
        t=(BN_ULLONG)w * (a) + (c); \
        (r)= Lw(t); \
        (c)= Hw(t); \
        }

#  define sqr(r0,r1,a) { \
        BN_ULLONG t; \
        t=(BN_ULLONG)(a)*(a); \
        (r0)=Lw(t); \
        (r1)=Hw(t); \
        }

# elif defined(BN_UMULT_LOHI)
#  define mul_add(r,a,w,c) {              \
        BN_ULONG high,low,ret,tmp=(a);  \
        ret =  (r);                     \
        BN_UMULT_LOHI(low,high,w,tmp);  \
        ret += (c);                     \
        (c) =  (ret<(c))?1:0;           \
        (c) += high;                    \
        ret += low;                     \
        (c) += (ret<low)?1:0;           \
        (r) =  ret;                     \
        }

#  define mul(r,a,w,c)    {               \
        BN_ULONG high,low,ret,ta=(a);   \
        BN_UMULT_LOHI(low,high,w,ta);   \
        ret =  low + (c);               \
        (c) =  high;                    \
        (c) += (ret<low)?1:0;           \
        (r) =  ret;                     \
        }

#  define sqr(r0,r1,a)    {               \
        BN_ULONG tmp=(a);               \
        BN_UMULT_LOHI(r0,r1,tmp,tmp);   \
        }

# elif defined(BN_UMULT_HIGH)
#  define mul_add(r,a,w,c) {              \
        BN_ULONG high,low,ret,tmp=(a);  \
        ret =  (r);                     \
        high=  BN_UMULT_HIGH(w,tmp);    \
        ret += (c);                     \
        low =  (w) * tmp;               \
        (c) =  (ret<(c))?1:0;           \
        (c) += high;                    \
        ret += low;                     \
        (c) += (ret<low)?1:0;           \
        (r) =  ret;                     \
        }

#  define mul(r,a,w,c)    {               \
        BN_ULONG high,low,ret,ta=(a);   \
        low =  (w) * ta;                \
        high=  BN_UMULT_HIGH(w,ta);     \
        ret =  low + (c);               \
        (c) =  high;                    \
        (c) += (ret<low)?1:0;           \
        (r) =  ret;                     \
        }

#  define sqr(r0,r1,a)    {               \
        BN_ULONG tmp=(a);               \
        (r0) = tmp * tmp;               \
        (r1) = BN_UMULT_HIGH(tmp,tmp);  \
        }

# else
/*************************************************************
 * No long long type
 */

#  define LBITS(a)        ((a)&BN_MASK2l)
#  define HBITS(a)        (((a)>>BN_BITS4)&BN_MASK2l)
#  define L2HBITS(a)      (((a)<<BN_BITS4)&BN_MASK2)

#  define LLBITS(a)       ((a)&BN_MASKl)
#  define LHBITS(a)       (((a)>>BN_BITS2)&BN_MASKl)
#  define LL2HBITS(a)     ((BN_ULLONG)((a)&BN_MASKl)<<BN_BITS2)

#  define mul64(l,h,bl,bh) \
        { \
        BN_ULONG m,m1,lt,ht; \
 \
        lt=l; \
        ht=h; \
        m =(bh)*(lt); \
        lt=(bl)*(lt); \
        m1=(bl)*(ht); \
        ht =(bh)*(ht); \
        m=(m+m1)&BN_MASK2; if (m < m1) ht+=L2HBITS((BN_ULONG)1); \
        ht+=HBITS(m); \
        m1=L2HBITS(m); \
        lt=(lt+m1)&BN_MASK2; if (lt < m1) ht++; \
        (l)=lt; \
        (h)=ht; \
        }

#  define sqr64(lo,ho,in) \
        { \
        BN_ULONG l,h,m; \
 \
        h=(in); \
        l=LBITS(h); \
        h=HBITS(h); \
        m =(l)*(h); \
        l*=l; \
        h*=h; \
        h+=(m&BN_MASK2h1)>>(BN_BITS4-1); \
        m =(m&BN_MASK2l)<<(BN_BITS4+1); \
        l=(l+m)&BN_MASK2; if (l < m) h++; \
        (lo)=l; \
        (ho)=h; \
        }

#  define mul_add(r,a,bl,bh,c) { \
        BN_ULONG l,h; \
 \
        h= (a); \
        l=LBITS(h); \
        h=HBITS(h); \
        mul64(l,h,(bl),(bh)); \
 \
        /* non-multiply part */ \
        l=(l+(c))&BN_MASK2; if (l < (c)) h++; \
        (c)=(r); \
        l=(l+(c))&BN_MASK2; if (l < (c)) h++; \
        (c)=h&BN_MASK2; \
        (r)=l; \
        }

#  define mul(r,a,bl,bh,c) { \
        BN_ULONG l,h; \
 \
        h= (a); \
        l=LBITS(h); \
        h=HBITS(h); \
        mul64(l,h,(bl),(bh)); \
 \
        /* non-multiply part */ \
        l+=(c); if ((l&BN_MASK2) < (c)) h++; \
        (c)=h&BN_MASK2; \
        (r)=l&BN_MASK2; \
        }
# endif                         /* !BN_LLONG */

void BN_MONT_CTX_init(BN_MONT_CTX *ctx);

void bn_init(BIGNUM *a);
void bn_mul_normal(BN_ULONG *r, BN_ULONG *a, int na, BN_ULONG *b, int nb);
void bn_mul_comba8(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b);
void bn_mul_comba4(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b);
void bn_sqr_normal(BN_ULONG *r, const BN_ULONG *a, int n, BN_ULONG *tmp);
void bn_sqr_comba8(BN_ULONG *r, const BN_ULONG *a);
void bn_sqr_comba4(BN_ULONG *r, const BN_ULONG *a);
int bn_cmp_words(const BN_ULONG *a, const BN_ULONG *b, int n);
int bn_cmp_part_words(const BN_ULONG *a, const BN_ULONG *b, int cl, int dl);
void bn_mul_recursive(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b, int n2,
                      int dna, int dnb, BN_ULONG *t);
void bn_mul_part_recursive(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b,
                           int n, int tna, int tnb, BN_ULONG *t);
void bn_sqr_recursive(BN_ULONG *r, const BN_ULONG *a, int n2, BN_ULONG *t);
void bn_mul_low_normal(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b, int n);
void bn_mul_low_recursive(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b, int n2,
                          BN_ULONG *t);
BN_ULONG bn_sub_part_words(BN_ULONG *r, const BN_ULONG *a, const BN_ULONG *b,
                           int cl, int dl);
int bn_mul_mont(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp,
                const BN_ULONG *np, const BN_ULONG *n0, int num);

BIGNUM *int_bn_mod_inverse(BIGNUM *in,
                           const BIGNUM *a, const BIGNUM *n, BN_CTX *ctx,
                           int *noinv);

SECURE_FUNC_INTERNAL
static inline BIGNUM *bn_expand(BIGNUM *a, int bits)
{
    if (bits > (INT_MAX - BN_BITS2 + 1))
        return NULL;

    if (((bits+BN_BITS2-1)/BN_BITS2) <= (a)->dmax)
        return a;

    return bn_expand2((a),(bits+BN_BITS2-1)/BN_BITS2);
}

#define OPENSSL_cleanse explicit_bzero
#define OPENSSL_free free
#define OPENSSL_malloc malloc // Original CRYPTO_malloc returns NULL when size==0, but it is not called that way in the extracted parts.
SECURE_FUNC_INTERNAL
static void* OPENSSL_zalloc(size_t n)
{
    auto p = malloc(n);
    if (p)
        memset(p, 0, n);
    return p;
}

// FIXME: Report errors?
#define BNerr(a,b) ((void)0)

}
