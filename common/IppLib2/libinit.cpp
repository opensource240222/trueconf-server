/**
 **************************************************************************
 * \file libinit.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief IPP Library initialization
 *
 * \b Project Statndart Libraries
 * \author SmirnovK
 * \date 11.01.05
 *
 * $Revision: 3 $
 *
 * $History: libinit.cpp $
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 9.09.11    Time: 14:22
 * Updated in $/VSNA/IppLib2
 * - new ipp lib
 * - new audio codecs
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 15.06.09   Time: 18:49
 * Updated in $/VSNA/IppLib2
 * - static link h.264
 * - remove x264 define
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:51
 * Created in $/VS2005/IppLib2
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 27.05.05   Time: 16:08
 * Updated in $/VS/IppLib2
 * aded new IPP ver 4.1
 * added g711, g728, g729 from IPP
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 11.01.05   Time: 19:39
 * Created in $/VS/IppLib2
 * added amd mmx suppotr for ipp library
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
//#include "..\std\cpplib\VS_Cpu.h"
//#include "ippmerged.h"
#include "ipp.h"

/// flag to do not reinit library
static int ippStaticLibInited = 0;

void IppLibInit()
{
	if (ippStaticLibInited)
		return;

	IppStatus err = ippInit();

	if (err == ippStsNoErr)
		ippStaticLibInited++;
}
