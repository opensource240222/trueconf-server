#pragma once
#include <openssl/dh.h>
#if OPENSSL_VERSION_NUMBER < 0x10101000L

#if OPENSSL_VERSION_NUMBER < 0x10100000L
inline int DH_set0_pqg(DH *dh, BIGNUM *p, BIGNUM *q, BIGNUM *g)
{
    /* If the fields p and g in d are NULL, the corresponding input
     * parameters MUST be non-NULL.  q may remain NULL.
     */
    if ((dh->p == NULL && p == NULL)
        || (dh->g == NULL && g == NULL))
        return 0;

    if (p != NULL) {
        BN_free(dh->p);
        dh->p = p;
    }
    if (q != NULL) {
        BN_free(dh->q);
        dh->q = q;
    }
    if (g != NULL) {
        BN_free(dh->g);
        dh->g = g;
    }

    if (q != NULL) {
        dh->length = BN_num_bits(q);
    }

    return 1;
}

inline void DH_get0_pqg(const DH *dh,
                 const BIGNUM **p, const BIGNUM **q, const BIGNUM **g)
{
    if (p != NULL)
        *p = dh->p;
    if (q != NULL)
        *q = dh->q;
    if (g != NULL)
        *g = dh->g;
}

inline void DH_get0_key(const DH *dh, const BIGNUM **pub_key, const BIGNUM **priv_key)
{
    if (pub_key != NULL)
        *pub_key = dh->pub_key;
    if (priv_key != NULL)
        *priv_key = dh->priv_key;
}

inline int DH_set0_key(DH *dh, BIGNUM *pub_key, BIGNUM *priv_key)
{
    if (pub_key != NULL) {
        BN_free(dh->pub_key);
        dh->pub_key = pub_key;
    }
    if (priv_key != NULL) {
        BN_free(dh->priv_key);
        dh->priv_key = priv_key;
    }

    return 1;
}

#endif// if <1.1.0

inline const BIGNUM *DH_get0_p(const DH *dh)
{
	const BIGNUM* result;
	DH_get0_pqg(dh, &result, NULL, NULL);
	return result;
}

inline const BIGNUM *DH_get0_q(const DH *dh)
{
	const BIGNUM* result;
	DH_get0_pqg(dh, NULL, &result, NULL);
	return result;
}

inline const BIGNUM *DH_get0_g(const DH *dh)
{
	const BIGNUM* result;
	DH_get0_pqg(dh, NULL, NULL, &result);
	return result;
}

inline const BIGNUM *DH_get0_priv_key(const DH *dh)
{
	const BIGNUM* result;
	DH_get0_key(dh, NULL, &result);
    return result;
}

inline const BIGNUM *DH_get0_pub_key(const DH *dh)
{
	const BIGNUM* result;
	DH_get0_key(dh, &result, NULL);
    return result;
}
#endif// if <1.1.1
