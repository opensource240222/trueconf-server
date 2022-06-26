#pragma once

#if !defined(__linux__)
#	error This file should be used only on Linux
#endif

#include "ProtectionLib/Protection.h"

namespace protection {

#define DECLARE_SECURE_SYSCALL_VARS \
	long sys_ret; \
	unsigned sys_err

// Performs syscall 'num' with supplied arguments '...'.
// Saves result into local variable sys_ret, saves errno value into local variable sys_err.
#define SECURE_SYSCALL(num, ...) do { \
		sys_ret = VS_SYSCALL_N(__VA_ARGS__, VS_SYSCALL_6, VS_SYSCALL_5, VS_SYSCALL_4, VS_SYSCALL_3, VS_SYSCALL_2, VS_SYSCALL_1, VS_SYSCALL_0)(num, __VA_ARGS__); \
		sys_err = sys_ret < 0 && sys_ret >= -4095l ? static_cast<unsigned>(-sys_ret) : 0u; \
	} while (0)

// Helper macro that allows us to inspect number of variadic macro arguments.
#define VS_SYSCALL_N(a1,a2,a3,a4,a5,a6,impl,...) impl

#if __i386__

#define VS_SYSCALL_0(num)                         Syscall(num)
#define VS_SYSCALL_1(num, a1)                     Syscall(num, (long)(a1))
#define VS_SYSCALL_2(num, a1, a2)                 Syscall(num, (long)(a1), (long)(a2))
#define VS_SYSCALL_3(num, a1, a2, a3)             Syscall(num, (long)(a1), (long)(a2), (long)(a3))
#define VS_SYSCALL_4(num, a1, a2, a3, a4)         Syscall(num, (long)(a1), (long)(a2), (long)(a3), (long)(a4))
#define VS_SYSCALL_5(num, a1, a2, a3, a4, a5)     Syscall(num, (long)(a1), (long)(a2), (long)(a3), (long)(a4), (long)(a5))
#define VS_SYSCALL_6(num, a1, a2, a3, a4, a5, a6) Syscall(num, (long)(a1), (long)(a2), (long)(a3), (long)(a4), (long)(a5), (long)(a6))

SECURE_FUNC_INLINE
long Syscall(long num)
{
	long ret;
	asm volatile ( "int $128" : "=a" (ret) : "a" (num) : "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
long Syscall(long num, long a1)
{
	long ret;
	asm volatile ( "int $128" : "=a" (ret) : "a" (num), "b" (a1) : "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
long Syscall(long num, long a1, long a2)
{
	long ret;
	asm volatile ( "int $128" : "=a" (ret) : "a" (num), "b" (a1), "c" (a2) : "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
long Syscall(long num, long a1, long a2, long a3)
{
	long ret;
	asm volatile ( "int $128" : "=a" (ret) : "a" (num), "b" (a1), "c" (a2), "d" (a3) : "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
long Syscall(long num, long a1, long a2, long a3, long a4)
{
	long ret;
	asm volatile ( "int $128" : "=a" (ret) : "a" (num), "b" (a1), "c" (a2), "d" (a3), "S" (a4) : "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
long Syscall(long num, long a1, long a2, long a3, long a4, long a5)
{
	long ret;
	asm volatile ( "int $128" : "=a" (ret) : "a" (num), "b" (a1), "c" (a2), "d" (a3), "S" (a4), "D" (a5) : "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
__attribute__ ((optimize("-fomit-frame-pointer"))) // This is required to free up the ebp register.
long Syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6)
{
	long ret;
	register long ebp asm("ebp") = a6;
	asm volatile ( "int $128" : "=a" (ret) : "a" (num), "b" (a1), "c" (a2), "d" (a3), "S" (a4), "D" (a5), "r" (ebp) : "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

#elif __x86_64__

// On x86_64 we pass syscall number as the last argument.
// This greatly simplifies generated assembly - registers in which 5 out of 6
// parameters are passed to Syscall match registers which should be used to
// pass corresponding arguments to the kernel.
#define VS_SYSCALL_0(num)                         Syscall(num)
#define VS_SYSCALL_1(num, a1)                     Syscall((long)(a1), num)
#define VS_SYSCALL_2(num, a1, a2)                 Syscall((long)(a1), (long)(a2), num)
#define VS_SYSCALL_3(num, a1, a2, a3)             Syscall((long)(a1), (long)(a2), (long)(a3), num)
#define VS_SYSCALL_4(num, a1, a2, a3, a4)         Syscall((long)(a1), (long)(a2), (long)(a3), (long)(a4), num)
#define VS_SYSCALL_5(num, a1, a2, a3, a4, a5)     Syscall((long)(a1), (long)(a2), (long)(a3), (long)(a4), (long)(a5), num)
#define VS_SYSCALL_6(num, a1, a2, a3, a4, a5, a6) Syscall((long)(a1), (long)(a2), (long)(a3), (long)(a4), (long)(a5), (long)(a6), num)

SECURE_FUNC_INLINE
long Syscall(long num)
{
	long ret;
	asm volatile ( "syscall" : "=a" (ret) : "a" (num) : "rcx", "r11", "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
long Syscall(long a1, long num)
{
	long ret;
	asm volatile ( "syscall" : "=a" (ret) : "a" (num), "D" (a1) : "rcx", "r11", "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
long Syscall(long a1, long a2, long num)
{
	long ret;
	asm volatile ( "syscall" : "=a" (ret) : "a" (num), "D" (a1), "S" (a2) : "rcx", "r11", "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
long Syscall(long a1, long a2, long a3, long num)
{
	long ret;
	asm volatile ( "syscall" : "=a" (ret) : "a" (num), "D" (a1), "S" (a2), "d" (a3) : "rcx", "r11", "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
long Syscall(long a1, long a2, long a3, long a4, long num)
{
	long ret;
	register long r10 asm("r10") = a4;
	asm volatile ( "syscall" : "=a" (ret) : "a" (num), "D" (a1), "S" (a2), "d" (a3), "r" (r10) : "rcx", "r11", "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
long Syscall(long a1, long a2, long a3, long a4, long a5, long num)
{
	long ret;
	register long r10 asm("r10") = a4;
	register long r8 asm("r8") = a5;
	asm volatile ( "syscall" : "=a" (ret) : "a" (num), "D" (a1), "S" (a2), "d" (a3), "r" (r10), "r" (r8) : "rcx", "r11", "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

SECURE_FUNC_INLINE
long Syscall(long a1, long a2, long a3, long a4, long a5, long a6, long num)
{
	long ret;
	register long r10 asm("r10") = a4;
	register long r8 asm("r8") = a5;
	register long r9 asm("r9") = a6;
	asm volatile ( "syscall" : "=a" (ret) : "a" (num), "D" (a1), "S" (a2), "d" (a3), "r" (r10), "r" (r8), "r" (r9) : "rcx", "r11", "memory" );
	return ret;
}
SECURE_FUNC_INLINE_END

#else
#	error Unknown architecture
#endif

}
