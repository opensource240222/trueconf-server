#include "ProtectionLib/Protection.h"

#if !defined(ENABLE_ARMADILLO_BUILD) && !defined(ENABLE_VMPROTECT_BUILD) && !defined(ENABLE_TRIAL_BUILD)

uint32_t VS_ArmReadHardwareKey(bool /*debug*/)
{
	return 0xffffffff;
}

uint32_t VS_ArmReadHardwareKeyEx()
{
	return 0xffffffff;
}

bool VS_ArmDetectedVM()
{
	return false;
}

bool VS_ArmFirstRun()
{
	return false;
}

bool VS_ArmKeyExpired()
{
	return false;
}

bool VS_ArmDetectedClockFF()
{
	return false;
}

bool VS_ArmDetectedClockREW()
{
	return false;
}

bool VS_ArmInstallKey(const char* /*key*/)
{
	return true;
}

bool VS_ArmSetDefaultKey()
{
	return true;
}

bool VS_ArmUpdateEnvironment()
{
	return true;
}

bool VS_ArmIsKeyInstalled()
{
	return true;
}

bool VS_ArmInstalledKeyCompare(const char* /*keyCompareWith*/)
{
	return true;
}

bool VS_ArmGetInstalledKey(char* /*keyBuffer*/, long& /*keyBufferSize*/)
{
	return false;
}

bool VS_ArmExpireInstalledKey()
{
	return true;
}

bool VS_ArmCheckFeatures(unsigned long /*features*/)
{
	return true;
}

#endif