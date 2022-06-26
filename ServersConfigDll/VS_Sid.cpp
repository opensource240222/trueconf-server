#include <windows.h>
#include <lmcons.h>
#include <Sddl.h>
#include "VS_ServersConfigDll.h"


#define BUFFER_SIZE (DNLEN + UNLEN)
char result[BUFFER_SIZE];
wchar_t buffer[BUFFER_SIZE];
wchar_t buffer_2[BUFFER_SIZE];

char  * GetUserLoginBySID(char *sSID)
{
	result[0] = 0;
	buffer[0] = 0;
	buffer_2[0] = 0;

	PSID pSID = NULL;
	MultiByteToWideChar(CP_UTF8, 0, sSID, -1, buffer, BUFFER_SIZE);

	if ( !ConvertStringSidToSidW(buffer, &pSID) )
	{
		int x = GetLastError();
		result[0] = '\0';
		return result;
	}

	DWORD size = BUFFER_SIZE;
	DWORD size2 = BUFFER_SIZE;
	SID_NAME_USE peUse = SidTypeUnknown ;

	int res = LookupAccountSidW(NULL, pSID, buffer, &size, buffer_2, &size2, &peUse);
	if (!res)
	{
		int x = GetLastError();
		result[0] = '\0';
		return result;
	}

	//WideCharToMultiByte(CP_UTF8, 0, buffer, -1, result, BUFFER_SIZE, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, buffer, -1, result, BUFFER_SIZE, NULL, NULL);

	return result;
}
