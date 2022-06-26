
#include <stdio.h>
#include <conio.h>
#include <Windows.h>

#include "../../std/cpplib/VS_MemoryLeak.h"
#include "Restarter.h"
#include "../Server/VS_Server.h"
#include "../../acs/Lib/VS_AcsLibDefinitions.h"
#include <algorithm>

int main( int argc, char *argv[] )
{
	if (argc < 2)	return 0;
	DWORD   prId = (DWORD)_atoi64( argv[2] );
	HANDLE   prHn = OpenProcess( PROCESS_TERMINATE | SYNCHRONIZE, FALSE, prId );
	if (argc == 2)
	{	WaitForSingleObject( prHn, 30000 );		TerminateProcess( prHn, 0 );	}
	else if (argc > 2)
	{
		char   *rcl = GetCommandLine();
		if (!rcl)
		{
			CloseHandle(prHn);
			return 0;
		}
		size_t   sz = strlen( rcl ) + 1024;
		char   *appName = (char *)malloc( sz ), *cmdLine = (char *)malloc( sz ),
					*appArgs = strstr( rcl, VS_RESTARTER_CL_MARKER );
		if (!appName || !cmdLine || !appArgs)
		{
			CloseHandle(prHn);
			return 0;
		}
		appArgs += sizeof(VS_RESTARTER_CL_MARKER) - 1;
		memset( (void *)appName, 0, sz );	memset( (void *)cmdLine, 0, sz );
		std::transform(appArgs, appArgs+strlen(appArgs), appArgs, ::tolower);
		const bool   isCmd = (strstr(appArgs, "service") == 0);
		const DWORD   mills = !strcmp( "Soft", argv[1] ) ? 60000 : 10000;
		if (isCmd)
		{
			if (WaitForSingleObject( prHn, mills ) != WAIT_OBJECT_0)
			{	TerminateProcess( prHn, 0 );	Sleep( 3000 );	}
			else	Sleep( 1000 );
			strcpy( cmdLine, appArgs );		strcpy( appName, argv[3] );
			STARTUPINFO   si;	memset( (void *)&si, 0, sizeof(si) );	si.cb = sizeof(si);
			PROCESS_INFORMATION   pi;	memset( (void *)&pi, 0, sizeof(pi) );
			if (!CreateProcess( appName, cmdLine, 0, 0, FALSE, 0, 0, 0, &si, &pi ))
				printf( "%s: It could not create process ( Name: %s, Arguments: %s ), error: %lu\n", argv[0], appName, cmdLine, GetLastError() );
		}
		else
		{
			SC_HANDLE   hsm = 0, hsr = 0;
			SERVICE_STATUS   serviceStatus;
			memset( (void *)&serviceStatus, 0, sizeof(serviceStatus) );
			hsm = OpenSCManager( 0, 0, SC_MANAGER_ALL_ACCESS );
			if (!hsm)	goto go_return;
			hsr = OpenService( hsm, argv[3], SERVICE_ALL_ACCESS );
			if (!hsr)	goto go_return;
			ControlService( hsr, SERVICE_CONTROL_STOP, &serviceStatus );
			if (WaitForSingleObject( prHn, mills ) != WAIT_OBJECT_0)
			{	TerminateProcess( prHn, 0 );	Sleep( 3000 );	}
			else	Sleep( 1000 );
			StartService( hsr, 0, 0 );
go_return:
			if (hsr)	CloseServiceHandle( hsr );
			if (hsm)	CloseServiceHandle( hsm );
		}
		free( appName );	free( cmdLine );
	}
	CloseHandle( prHn );	return 0;
}
// end main
