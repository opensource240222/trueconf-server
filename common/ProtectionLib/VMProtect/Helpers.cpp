#include "ProtectionLib/Protection.h"

#if defined(ENABLE_VMPROTECT_BUILD)

uint32_t VS_ArmReadHardwareKey(bool /*debug*/)
{
	// Hardware keys are supported only in the Ultimate edition of VMProtect.
	return 0xffffffff;
}

uint32_t VS_ArmReadHardwareKeyEx()
{
	// This function is not used anywhere.
	return 0xffffffff;
}

bool VS_ArmDetectedVM()
{
	return VMProtectIsVirtualMachinePresent();
}

bool VS_ArmFirstRun()
{
	// This function is not used anywhere.
	return false;
}

bool VS_ArmKeyExpired()
{
	// This function is not used anywhere.
	return false;
}

bool VS_ArmDetectedClockFF()
{
	// This function is not used anywhere.
	return false;
}

bool VS_ArmDetectedClockREW()
{
	// This function is not used anywhere.
	return false;
}

bool VS_ArmInstallKey(const char* /*key*/)
{
	// This operation is not supported.
	return true;
}

bool VS_ArmSetDefaultKey()
{
	// This operation is not supported.
	return true;
}

bool VS_ArmUpdateEnvironment()
{
	// This function is not used anywhere.
	return true;
}

bool VS_ArmIsKeyInstalled()
{
	// This function is not used anywhere.
	return true;
}

bool VS_ArmInstalledKeyCompare(const char* /*keyCompareWith*/)
{
	// This function is not used anywhere.
	return true;
}

bool VS_ArmGetInstalledKey(char* /*keyBuffer*/, long& /*keyBufferSize*/)
{
	// This function is not used anywhere.
	return false;
}

bool VS_ArmExpireInstalledKey()
{
	// This operation is not supported.
	return true;
}

bool VS_ArmCheckFeatures(unsigned long /*features*/)
{
	// This operation is not supported.
	return true;
}

#endif