#pragma once

#include "extlibs/VMProtectSDK/VMProtectSDK.h"

#define SECUREBEGIN VMProtectBeginUltra(0);
#define SECUREEND VMProtectEnd();
#define SECUREBEGIN_A_FULL VMProtectBeginUltra(0);
#define SECUREEND_A_FULL VMProtectEnd();
#define SECUREBEGIN_B_GUEST VMProtectBeginUltra(0);
#define SECUREEND_B_GUEST VMProtectEnd();
#define SECUREBEGIN_C_LDAP VMProtectBeginUltra(0);
#define SECUREEND_C_LDAP VMProtectEnd();
#define SECUREBEGIN_D_ROLE VMProtectBeginUltra(0);
#define SECUREEND_D_ROLE VMProtectEnd();
#define SECUREBEGIN_E_ENTERPRISE VMProtectBeginUltra(0);
#define SECUREEND_E_ENTERPRISE VMProtectEnd();
#define SECUREBEGIN_F_MGATEWAY VMProtectBeginUltra(0);
#define SECUREEND_F_MGATEWAY VMProtectEnd();
#define SECUREBEGIN_G_WEBRTC VMProtectBeginUltra(0);
#define SECUREEND_G_WEBRTC VMProtectEnd();
#define SECUREBEGIN_H VMProtectBeginUltra(0);
#define SECUREEND_H VMProtectEnd();
#define SECUREBEGIN_I VMProtectBeginUltra(0);
#define SECUREEND_I VMProtectEnd();
#define SECUREBEGIN_J VMProtectBeginUltra(0);
#define SECUREEND_J VMProtectEnd();
#define SECUREBEGIN_K VMProtectBeginUltra(0);
#define SECUREEND_K VMProtectEnd();
// Valid nanomites
#define NANOBEGIN2 VMProtectBeginUltra(0);
#define NANOEND2 VMProtectEnd();
// Invalid nanomites
#define NANOBEGIN ;
#define NANOEND ;

// Function annotations that ensure that code of the function can be properly protected:
// * SECURE_FUNC
//   Should be used on functions that are called from unprotected code.
//   Ensures that all function code is placed in one spot.
// * SECURE_FUNC_INTERNAL
//   Should be used on functions that are called only from functions marked as SECURE_FUNC, SECURE_FUNC_INTERNAL or SECURE_FUNC_INLINE.
//   Ensures that function code is placed in symbols which will be protected.
// * SECURE_FUNC_INLINE
//   Must be used on functions templates.
//   Also can be used instead of SECURE_FUNC_INTERNAL if the function is small and it is ok to inline it everywhere.
//   SECURE_FUNC_INTERNAL_END must be placed after the function body.
//   Functions marked as SECURE_FUNC_INLINE don't need to (and shouldn't) be listed in VMProtect config files.
//   Ensures that function code is placed inside the caller function.
// Explanation of used attributes:
// * noinline
//   Disallows copying the function logic inside another function.
//   Code duplicated in this way can not be protected.
// * used
//   Ensures that the function will be included in the binary even if it is not used.
//   This is required to prevent VMProtect from aborting (because of -we flag) in case the function was inlined at all call sites.
// * no_icf
//   Prevents compiler from merging other functions that has identical code with this function. Linker can still do that.
//   This is not critical for protection, it is desired to avoid random performance problems due to usage of protected code in places where it is not needed.
// * noclone
//   Disallows making copies of the function code (optimized for specific argument values).
//   These copies can be protected but we need to know the names of symbols that represent them and they are not predictable.
// * no-reorder-blocks-and-partition
//   Disallows splitting the function code into parts.
//   These parts can protected but we need to know the names of symbols that represent them and they are not predictable.
// * no-stack-protector, no_stack_protector
//   Disables runtime stack smashing protection to avoid calls to unprotected __stack_chk_fail() function.
// * always_inline
//   Instructs the compiler to inline function at all call sites.
//   If that is not possible GCC produces a compile error.
//   Clang doesn't produce neither error nor warning, this is not a problem as long GCC can't compile the same function.
// * __forceinline
//   Instructs the compiler to inline function at all call sites.
//   If that is not possible then MSVC produces warning C4714 which we convert into an error.
#if defined(__clang__)
#	define SECURE_FUNC            __attribute__ ((no_stack_protector, noinline))
#	define SECURE_FUNC_INTERNAL   __attribute__ ((no_stack_protector, used))
#	define SECURE_FUNC_INLINE     __attribute__ ((always_inline)) _Pragma("GCC warning \"function marked SECURE_FUNC_INLINE may be not inlined\"")
#	define SECURE_FUNC_INLINE_END
#elif defined(_MSC_VER)
#	define SECURE_FUNC            __declspec(noinline)
#	define SECURE_FUNC_INTERNAL
#	define SECURE_FUNC_INLINE     __pragma(warning(push)) __pragma(warning(error : 4714)) __forceinline
#	define SECURE_FUNC_INLINE_END __pragma(warning(pop))
#elif defined(__GNUC__)
#	define SECURE_FUNC            __attribute__ ((noclone, optimize("no-reorder-blocks-and-partition", "no-stack-protector"), noinline, no_icf))
#	define SECURE_FUNC_INTERNAL   __attribute__ ((noclone, optimize("no-reorder-blocks-and-partition", "no-stack-protector"), used))
#	define SECURE_FUNC_INLINE     __attribute__ ((always_inline)) inline
#	define SECURE_FUNC_INLINE_END
#else
#	error Unknown compiler
#endif

#define SECURE_STRING(s) ::VMProtectDecryptStringA(s)

// VMProtect doesn't support protection of the code that uses red-zone, its
// virtual machine uses red-zone for its own needs.
// We compile while ProtectionLib with -mno-red-zone, but there is no way to apply this flag to a specific function so other code has to use this macro to suppress red-zone usage.
// It tricks GCC into thinking that RSP is modified in the function and as a result GCC disables usage of red-zone for the whole function.
#if defined(__GNUC__) && !defined(__clang__) && defined(__amd64)
#	define DISABLE_RED_ZONE (*(volatile char*)__builtin_alloca(0))
#endif
