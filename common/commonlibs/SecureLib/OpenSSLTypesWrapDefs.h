#pragma once
#include "std-generic/cpplib/Box.h"

namespace vs
{
class EvpPkeyDeleter{public: void operator()(void* p) const;};
class StackOfX509ExtensionDeleter{public: void operator()(void* p) const;};
class X509Deleter{public: void operator()(void* p) const;};
class X509NameDeleter{public: void operator()(void* p) const;};
class X509ReqDeleter{public: void operator()(void* p) const;};
class X509StoreCtxDeleter{public: void operator()(void* p) const;};
class X509StoreDeleter{public: void operator()(void* p) const;};

struct evp_cipher_ctx_tag;
struct evp_pkey_ptr_tag;
struct rc4_key_ptr_tag;
struct stack_of_x509_ext_ptr_tag;
struct x509_name_ptr_tag;
struct x509_ptr_tag;
struct x509_req_ptr_tag;
struct x509_store_ptr_tag;
struct x509v3_ctx_ptr_tag;

using EvpCipherCtxRawPtr = Box<evp_cipher_ctx_tag, sizeof(void*)>;
using EvpPKeyPtr = Box<evp_pkey_ptr_tag, sizeof(void*)>;
using RC4KeyPtr = vs::Box<rc4_key_ptr_tag, sizeof(void*)>;
using StackOfX509ExtPtr = Box<stack_of_x509_ext_ptr_tag, sizeof(void*)>;
using X509NamePtr = Box<x509_name_ptr_tag, sizeof(void*)>;
using X509Ptr = Box<x509_ptr_tag, sizeof(void*)>;
using X509ReqPtr = Box<x509_req_ptr_tag, sizeof(void*)>;
using X509StorePtr = Box<x509_store_ptr_tag, sizeof(void*)>;
using X509v3CtxPtr = Box<x509v3_ctx_ptr_tag, sizeof(void*)>;
}