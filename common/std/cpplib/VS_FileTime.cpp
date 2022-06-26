/**
**************************************************************************
* \file VS_FileTime.cpp
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
* \brief FILETIME operating class
*
* \b Project Standart Libraries
* \author StasS
* \author SMirnovK
* \date 05.09.03
*
* $Revision: 1 $
*
* $History: VS_FileTime.cpp $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 5.02.07    Time: 18:43
 * Updated in $/VS2005/std/cpplib
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
****************************************************************************/

#if defined(_WIN32) // Not ported yet

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_FileTime.h"

LONGLONG operator-(const VS_FileTime& a,const FILETIME* b)
{
	LARGE_INTEGER aa;
  aa.LowPart  = a.m_filetime.dwLowDateTime;
  aa.HighPart = a.m_filetime.dwHighDateTime;

	if(b!=NULL)
 	{
		LARGE_INTEGER bb;
    bb.LowPart  = b->dwLowDateTime;
    bb.HighPart = b->dwHighDateTime;
    aa.QuadPart -= bb.QuadPart;
	};

  return aa.QuadPart;	///< result in 100 ns units

}
const char* VS_FileTime::MONTH[] = {
	"Inv",	"Jan", 	"Feb", 	"Mar", 	"Apr", 	"May", 	"Jun",
	"Jul", 	"Aug", 	"Sep", 	"Oct", 	"Nov", 	"Dec",	"Inv"};

#endif
