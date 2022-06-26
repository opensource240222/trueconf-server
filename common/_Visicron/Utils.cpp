#include "Utils.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/clib/detectAVX.h"

#include <Windows.h>

int VS_CopyRegKeys(char *s, char* d)
{
	if (!s || !*s || !d || !*d)
		return -1;
	return VS_RegistryKey::CopyKey(true, s, true, d);
}

int VS_CheckAVXOnInstall()
{
	int ret = VS_DetectAVXSupport();
	if (ret == 0) {
		const char* root = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
		HKEY	key;
		DWORD	err = RegOpenKeyExA(HKEY_LOCAL_MACHINE, root, 0, KEY_READ | KEY_WRITE, &key);
		if (err == ERROR_SUCCESS) {
			const char * value = "~0x1000000000000000";
			err = RegSetValueExA(key, "OPENSSL_ia32cap", 0, REG_SZ, (BYTE*) value, strlen(value) + 1);
			if (ERROR_SUCCESS == err) {
				DWORD_PTR dwReturnValue = 0;
				SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 5000, &dwReturnValue);
				ret = 2;
			}
			RegCloseKey(key);
		}
	}
	return ret;
}
