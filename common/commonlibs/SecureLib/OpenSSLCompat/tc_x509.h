#pragma once
#include <openssl/x509.h>
#if OPENSSL_VERSION_NUMBER < 0x10100000L

inline ASN1_TIME *X509_getm_notBefore(const X509 *x)
{
    return x->cert_info->validity->notBefore;
}
inline ASN1_TIME *X509_getm_notAfter(const X509 *x)
{
    return x->cert_info->validity->notAfter;
}
inline const STACK_OF(X509_EXTENSION) *X509_get0_extensions(const X509 *x)
{
    return x->cert_info->extensions;
}

inline X509 *X509_STORE_CTX_get0_cert(X509_STORE_CTX *ctx)
{
    return ctx->cert;
}

#endif
