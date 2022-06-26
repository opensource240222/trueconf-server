#pragma once
/*
	Fot including in source files only.
	Don't include this file in headers, otherwise you get dependencies from openssl types.
*/
#include "OpenSSLTypesWrapDefs.h"
#include "SecureLib/OpenSSLCompat/tc_x509.h"

#include <openssl/evp.h>
#include <openssl/rc4.h>

#include <memory>

namespace vs
{
inline void EvpPkeyDeleter::operator()(void* p) const { EVP_PKEY_free(static_cast<EVP_PKEY*>(p)); };
inline void StackOfX509ExtensionDeleter::operator()(void* p) const
{
	//Don't even ask me about this
	sk_X509_EXTENSION_pop_free(static_cast<STACK_OF(X509_EXTENSION)*>(p),
		reinterpret_cast<void(*)(X509_EXTENSION *)>(::X509_EXTENSION_free));
}
inline void X509Deleter::operator()(void* p) const{ X509_free(static_cast<X509*>(p)); }
inline void X509NameDeleter::operator()(void* p) const { X509_NAME_free(static_cast<X509_NAME*>(p)); }
inline void X509ReqDeleter::operator()(void* p) const { X509_REQ_free(static_cast<X509_REQ*>(p)); }
inline void X509StoreCtxDeleter::operator()(void* p) const{ X509_STORE_CTX_free(static_cast<X509_STORE_CTX*>(p)); }
inline void X509StoreDeleter::operator()(void* p) const { X509_STORE_free(static_cast<X509_STORE*>(p)); }


struct evp_cipher_ctx_tag : vs::BoxTag<EVP_CIPHER_CTX*> {};
struct evp_pkey_ptr_tag : BoxTag<std::unique_ptr<EVP_PKEY, EvpPkeyDeleter>> {};
struct rc4_key_ptr_tag : vs::BoxTag<std::unique_ptr<RC4_KEY>>{};
struct stack_of_x509_ext_ptr_tag : vs::BoxTag<
	std::unique_ptr<STACK_OF(X509_EXTENSION), StackOfX509ExtensionDeleter>> {};
struct x509_name_ptr_tag : BoxTag<std::unique_ptr<X509_NAME, X509NameDeleter>> {};
struct x509_ptr_tag : BoxTag<std::unique_ptr<X509, X509Deleter>> {};
struct x509_req_ptr_tag : BoxTag<std::unique_ptr<X509_REQ, X509ReqDeleter>> {};
struct x509_store_ptr_tag : BoxTag<std::unique_ptr<X509_STORE, X509StoreDeleter>> {};
struct x509v3_ctx_ptr_tag : BoxTag<std::unique_ptr<X509V3_CTX>> {};
}