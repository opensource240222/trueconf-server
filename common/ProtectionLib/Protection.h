#pragma once

#if defined(ENABLE_TRIAL_BUILD) + defined(ENABLE_ARMADILLO_BUILD) + defined(ENABLE_VMPROTECT_BUILD) > 1
#	error Only one protection system can be used at the same time
#endif

#if defined(ENABLE_ARMADILLO_BUILD) && !defined(_WIN32)
#	error Armadillo protect works only on Windows
#endif

#if defined(ENABLE_ARMADILLO_BUILD)
#	include "Armadillo/SecuredSections.h"
#elif defined(ENABLE_VMPROTECT_BUILD)
#	include "VMProtect/SecuredSections.h"
#elif defined(ENABLE_TRIAL_BUILD)
#	include "Trial/SecuredSections.h"
#else
#	include "None/SecuredSections.h"
#endif

#if !defined(SECURE_FUNC)
#	define SECURE_FUNC
#endif
#if !defined(SECURE_FUNC_INTERNAL)
#	define SECURE_FUNC_INTERNAL
#endif
#if !defined(SECURE_FUNC_INLINE)
#	define SECURE_FUNC_INLINE inline
#endif
#if !defined(SECURE_FUNC_INLINE_END)
#	define SECURE_FUNC_INLINE_END
#endif
#if !defined(SECURE_STRING)
#	define SECURE_STRING(s) (s)
#endif
#if !defined(DISABLE_RED_ZONE)
#	define DISABLE_RED_ZONE ((void)0)
#endif

#include <cstdint>

uint32_t VS_ArmReadHardwareKey(bool debug = false);
uint32_t VS_ArmReadHardwareKeyEx();
bool VS_ArmDetectedVM();
bool VS_ArmFirstRun();
bool VS_ArmKeyExpired();
bool VS_ArmDetectedClockFF();
bool VS_ArmDetectedClockREW();
bool VS_ArmInstallKey(const char* key);
bool VS_ArmSetDefaultKey();
bool VS_ArmUpdateEnvironment();
bool VS_ArmIsKeyInstalled();
bool VS_ArmInstalledKeyCompare(const char* keyCompareWith);
bool VS_ArmGetInstalledKey(char* keyBuffer, long& keyBufferSize);
bool VS_ArmExpireInstalledKey();
bool VS_ArmCheckFeatures(unsigned long features);
