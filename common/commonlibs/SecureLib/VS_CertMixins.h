#pragma once
// Don't include this file in header

#include "SecureLib/SecureTypes.h"
#include "std-generic/cpplib/string_view.h"
#include <openssl/x509v3.h>

X509_EXTENSION *MakeExtension(SubjectAltNameType type, string_view value);
void FetchExtensions(const STACK_OF(X509_EXTENSION) *exts, SubjAltNameExtensionsSet &out);
