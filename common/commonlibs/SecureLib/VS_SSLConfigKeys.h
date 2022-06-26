#pragma once

/* Registry Key Constants for SSL */

/* !!! Server Certificate and Chain Keys !!! */
// server private key
#define SRV_PRIVATE_KEY_NAME "PrivateKey"
// server end certificate
#define SRV_CERT_KEY_NAME "Certificate"
// server cert chain
#define SRV_CERT_CHAIN_KEY_NAME "Certificate Chain"
extern const char *SRV_PRIVATE_KEY;
// server end certificate
extern const char *SRV_CERT_KEY;
// server cert chain
extern const char *SRV_CERT_CHAIN_KEY;

/* !!! TLS Configuration Keys !!! */
// TLS private server key
#define TLS_PRIVATE_KEY_NAME "TLSPrivateKey"
// TLS certificate (or chain) key
#define TLS_CERT_KEY_NAME "TLSCert"
// Use server key for TLS
#define TLS_USE_SERVER_CERT_KEY_NAME "UseSrvCertForTLS"

extern const char *TLS_PRIVATE_KEY;
extern const char *TLS_CERT_KEY;
extern const char *TLS_ROOT_CA_CERT_KEY;
extern const char *TLS_USE_SERVER_CERT_KEY;

