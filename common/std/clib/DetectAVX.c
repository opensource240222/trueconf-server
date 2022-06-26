#if defined(_WIN32) && defined(_M_IX86) // Not ported yet

#include <Windows.h>
#include <VersionHelpers.h>

int VS_DetectAVXSupport()
{
	int result = 0;
	__asm {
		xor eax, eax
			cpuid
			cmp eax, 1;
			jb not_supported
			mov eax, 1
			cpuid
			and ecx, 018000000h; 27 bit(OS XSAVE / XRSTOR) and 28 bit(AVX in processor)
			cmp ecx, 018000000h;
			jne not_supported
			xor ecx, ecx;
			xgetbv;  XFEATURE_ENABLED_MASK in edx : eax
			and eax, 110b
			cmp eax, 110b; OS save AVX on CONTEXT switching
			jne not_supported
			mov eax, 1
			jmp done
		not_supported :
		xor eax, eax
		done :
		mov result, eax
	}
	// code above sometimes produce fictive AVX detection on WinXP run on Sandy Bridge
	// correct it
	if (!IsWindows7SP1OrGreater()) // AVX support added in Win 7 Sp1
		result = 0;
	return result;
}

#endif