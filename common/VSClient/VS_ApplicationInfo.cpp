/**
 **************************************************************************
 * \file VS_ApplicationInfo.cpp
 * (c) 2002-2008 Visicron Inc.  http://www.visicron.net/
 * \brief Contain implemenation of current application registry constatnts
 *
 * \b Project Visicron
 * \author SMirnovK
 * \date 22.02.08
 *
 * $Revision: 3 $
 *
 * $History: VS_ApplicationInfo.cpp $
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 21.08.08   Time: 21:16
 * Updated in $/VSNA/VSClient
 * - h261
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.03.08    Time: 22:10
 * Updated in $/VSNA/VSClient
 * - bugs with app name fixed
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 26.02.08   Time: 16:54
 * Created in $/VSNA/VSClient
 * - new servers coonect shceme
 *
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_ApplicationInfo.h"
#include "../std/cpplib/VS_RegistryKey.h"

#include <cstring>

/****************************************************************************
 * Globals
 ****************************************************************************/
char GUI_version[260];
char APP_version[260];

/**
 **************************************************************************
 ****************************************************************************/
void VS_GetAppPorts(unsigned long* &ports, unsigned long &num)
{
	static unsigned long iports[] = {0, 443};
	ports = iports; num = sizeof(iports)/sizeof(unsigned long);
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_ReadAS(char * server, bool Default)
{
	if (!server)
		return false;
	*server = 0;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	return key.GetValue(server, 255, VS_REG_STRING_VT, Default ? REG_DefaultServer : REG_Server) > 0;
}
/**
 **************************************************************************
 ****************************************************************************/
bool VS_WriteAS(const char * server, bool Default)
{
	if (!server)
		return false;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
	return key.SetString(server, Default ? REG_DefaultServer : REG_Server);
}

void VS_SetAppVer(const char * app, const char * ver)
{
	if (app)
		strcpy(GUI_version, app);
	if (ver)
		strcpy(APP_version, ver);
}
