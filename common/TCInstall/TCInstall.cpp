#include "TCInstall/TCInstall.h"

#include <Windows.h>

bool VSTryLoadLibrary(const char* name)
{
	HMODULE h = LoadLibrary(name);
	if (!h)
		return false;

	FreeLibrary(h);
	return true;
}
