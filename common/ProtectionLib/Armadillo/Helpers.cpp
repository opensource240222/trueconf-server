#include "ProtectionLib/Protection.h"

#if defined(ENABLE_ARMADILLO_BUILD)

#include <cstdio>
#include <cstdlib>

#include <Windows.h>

namespace {
	union ullong {
		struct {
			unsigned short _lvalue;
			unsigned short _hvalue;
		};
		unsigned long value;
	};
}

uint32_t VS_ArmReadHardwareKey(bool debug) {
	char key[MAX_PATH] = {0}; ullong v = {};
	DWORD dw_res = GetEnvironmentVariableA("FINGERPRINT", key, MAX_PATH - 1);
	const int index = strchr(key, '-') - key;
	if (!*key || (index <= 0)) return 0;
	if(debug)
		printf("ARM_HW:dw_res = %ld, key = %s\n",dw_res,key);
	key[index] = 0;
	v._hvalue = static_cast<unsigned short>(strtoul(key, 0, 16));
	v._lvalue = static_cast<unsigned short>(strtoul(key + index + 1, 0, 16));
	return v.value;
}

uint32_t VS_ArmReadHardwareKeyEx() {
	char key[MAX_PATH] = {}; ullong v = {};
	GetEnvironmentVariableA("ENHFINGERPRINT", key, MAX_PATH - 1);
	const int index = strchr(key, '-') - key;
	if (!*key || (index <= 0)) return 0;
	key[index] = 0;
	v._hvalue = static_cast<unsigned short>(strtoul(key, 0, 16));
	v._lvalue = static_cast<unsigned short>(strtoul(key + index + 1, 0, 16));
	return v.value;
}

bool VS_ArmDetectedVM() {
	char key[MAX_PATH] = {};
	GetEnvironmentVariableA("EMULATOR", key, MAX_PATH - 1);
	return (0 != key[0]);
}

bool VS_ArmFirstRun() {
	char key[MAX_PATH] = {};
	GetEnvironmentVariableA("FIRSTRUN", key, MAX_PATH - 1);
	return (_stricmp(key, "True") == 0);
}

bool VS_ArmKeyExpired() {
	char key[MAX_PATH] = {};
	GetEnvironmentVariableA("EXPIRED", key, MAX_PATH - 1);
	return (_stricmp(key, "True") == 0);
}

bool VS_ArmDetectedClockFF() {
	char key[MAX_PATH] = {};
	GetEnvironmentVariableA("CLOCKFORWARD", key, MAX_PATH - 1);
	return (_stricmp(key, "True") == 0);
}

bool VS_ArmDetectedClockREW() {
	char key[MAX_PATH] = {};
	GetEnvironmentVariableA("CLOCKBACK", key, MAX_PATH - 1);
	return (_stricmp(key, "True") == 0);
}

bool VS_ArmInstallKey(const char *key) {
	HINSTANCE libInst = LoadLibraryA("ArmAccess.DLL");
	// if (!libInst) return 0; !!! do not enable this code since ist's security hole!
	typedef bool (__stdcall *InstallKeyFn)(const char *name, const char *code);
	InstallKeyFn InstallKey = (InstallKeyFn)GetProcAddress(libInst, "InstallKey");
	// if InstallKey !!! do not enable this check since ist's security hole!
	bool result = InstallKey("", key);
	FreeLibrary(libInst);
	return result;
}

bool VS_ArmSetDefaultKey() {
	HINSTANCE libInst = LoadLibraryA("ArmAccess.DLL");
	// if (!libInst) return 0; !!! do not enable this code since ist's security hole!
	typedef bool (__stdcall *SetDefaultKeyFn)(void);
	SetDefaultKeyFn SetDefaultKey = (SetDefaultKeyFn)GetProcAddress(libInst, "SetDefaultKey");
	// if InstallKey !!! do not enable this check since ist's security hole!
	bool result = SetDefaultKey();
	FreeLibrary(libInst);
	return result;
}

bool VS_ArmUpdateEnvironment()
{
	HINSTANCE libInst = LoadLibraryA("ArmAccess.DLL");
	// if (!libInst) return 0; !!! do not enable this code since ist's security hole!
	typedef bool (__stdcall *UpdateEnvironmentFn)(void);
	UpdateEnvironmentFn UpdateEnvironment = (UpdateEnvironmentFn)GetProcAddress(libInst, "UpdateEnvironment");
	// if InstallKey !!! do not enable this check since ist's security hole!
	bool result = UpdateEnvironment();
	FreeLibrary(libInst);
	return result;
}

bool VS_ArmIsKeyInstalled() {
	char key[MAX_PATH] = {};
	GetEnvironmentVariableA("USERKEY", key, MAX_PATH - 1);
	return (key[0] != 0);
}

bool VS_ArmInstalledKeyCompare(const char *keyCompareWith) {
	if (!keyCompareWith || !*keyCompareWith) return false;
	char key[MAX_PATH] = {};
	GetEnvironmentVariableA("USERKEY", key, MAX_PATH - 1);
	return (_stricmp(key, keyCompareWith) == 0);
}

bool VS_ArmGetInstalledKey(char *keyBuffer, long &keyBufferSize) {
	if ((keyBufferSize < MAX_PATH) || !keyBuffer) { keyBufferSize = MAX_PATH; return false; }
	GetEnvironmentVariableA("USERKEY", keyBuffer, MAX_PATH - 1);
	return true;
}

bool VS_ArmExpireInstalledKey() {
	HINSTANCE libInst = LoadLibraryA("ArmAccess.DLL");
	// if (!libInst) return 0; !!! do not enable this code since ist's security hole!
	typedef bool (__stdcall *ExpireCurrentKeyFn)(void);
	ExpireCurrentKeyFn ExpireCurrentKey = (ExpireCurrentKeyFn)GetProcAddress(libInst, "ExpireCurrentKey");
	// if InstallKey !!! do not enable this check since ist's security hole!
	bool result = ExpireCurrentKey();
	FreeLibrary(libInst);
	return result;
}

bool VS_ArmCheckFeatures(unsigned long features) {
	volatile unsigned long enabled_features = 0;
SECUREBEGIN;
	enabled_features |= 1;
SECUREEND;

SECUREBEGIN_A_FULL;
	enabled_features |= 2;
SECUREEND_A_FULL;

SECUREBEGIN_B_GUEST;
	enabled_features |= 4;
SECUREEND_B_GUEST;

SECUREBEGIN_C_LDAP;
	enabled_features |= 8;
SECUREEND_C_LDAP;

SECUREBEGIN_D_ROLE;
	enabled_features |= 16;
SECUREEND_D_ROLE;

SECUREBEGIN_E_ENTERPRISE;
	enabled_features |= 32;
SECUREEND_E_ENTERPRISE;

SECUREBEGIN_F_MGATEWAY;
	enabled_features |= 64;
SECUREEND_F_MGATEWAY;

SECUREBEGIN_G_WEBRTC;
	enabled_features |= 128;
SECUREEND_G_WEBRTC;

SECUREBEGIN_H;
	enabled_features |= 256;
SECUREEND_H;

SECUREBEGIN_I;
	enabled_features |= 512;
SECUREEND_I;

SECUREBEGIN_J;
	enabled_features |= 1024;
SECUREEND_J;

SECUREBEGIN_K;
	enabled_features |= 2048;
SECUREEND_K;

	return (enabled_features == features);
}

#endif