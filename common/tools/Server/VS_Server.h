/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 01.08.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_Server.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef VS_SERVER_H
#define VS_SERVER_H

#include "tools/Server/RegistrationParams.h"
#include "../../std/cpplib/VS_SimpleStr.h"

/////////////////////////////////////////////////////////////////////////////////////////
class VS_ServerComponentsInterface;
class VS_RoutersWatchdog;

class VS_Server
{
private:
	static VS_RoutersWatchdog* GetWatchDog();
public:
	// ��� ����� ��������� ����������, ��������������� � ������� EXE ������,
	// ���������� ����������� ����������� ����� ������.

	// ��� ������ ������ ���������� ��������� �� ����������� ������-�����
	// (����������� �����������).
	static const char* ShortName();      // Short Name
	static const char* LongName();       // Long Name
	static const char* RegistryKey();    // Registry Key
	static const char* ServiceName();    // Windows Service Name
	static const char* ProductVersion(); // Product Version

	// � ���� ������� ���������� ����������� ����� � ���������� ��������� �������.
	static bool		Start( void );
	static void		Stop( void );

	// � ������� ���������� ������� Start(),Stop() ��� ����������
	// ����� ���������������� ����������� VS_Server.
	static char   *cl;							// Command Line cl = GetCommandLine()
	static int   argc;	static char   **argv;	// From main( int argc, char *argv[] )		static boost::program_options::options_description  options_desc;
	static char   sdir[];			// Endpoint Name, Start Path
	static wchar_t wdir[];						// Working Directory
	static VS_SimpleStr g_root_key;				// Registry Root Key (with Instance suffix)

	static vs::RegistrationParams reg_params;

	static bool QuickDestroy();
	static bool PauseDestroy ( unsigned long msec );

	static const char endpoint_for_config[];

	static VS_ServerComponentsInterface	*srv_components;

	static int main(int argc, char *argv[]);
};
// end VS_Server class

/////////////////////////////////////////////////////////////////////////////////////////

#define   VS_SERVER_ENDPOINT_ARG_PREF   "Endpoint"

#define   VS_SERVER_CONFIG_W3SRV_NAME   "W3SVC"
#define   VS_SERVER_CONFIG_WINMGMT_NAME   "WinMgmt"


/////////////////////////////////////////////////////////////////////////////////////////

#endif
