#pragma once
//	OpenSSL v1.0.2 (ssl.h in particular) tries to include winsock.h v1 unless winsock2 is already included
#if defined (_WIN32) && OPENSSL_VERSION_NUMBER < 0x10100000L
	#include <WinSock2.h>
#endif
#include <openssl/ssl.h>
#if OPENSSL_VERSION_NUMBER < 0x10100000L

// Actually just uses TLSv1.2 unlike its OpenSSL 1.1.0+ implementation
inline const SSL_METHOD* TLS_method()
{
	return TLSv1_2_method();
}
#endif
