#pragma once
#include <openssl/asn1.h>
#if OPENSSL_VERSION_NUMBER < 0x10100000L

inline const unsigned char *ASN1_STRING_get0_data(const ASN1_STRING *x)
{
    return x->data;
}
#endif
