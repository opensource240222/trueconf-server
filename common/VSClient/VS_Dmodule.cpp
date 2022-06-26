/**
 **************************************************************************
 * \file VS_DModule.cpp
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief Contain classes working with audio: capturing, rendering, compressing
 *
 * \b Project Client
 * \author SMirnovK
 * \date 11.08.2005
 *
 * $Revision: 4 $
 *
 * $History: VS_Dmodule.cpp $
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 18.08.10   Time: 14:09
 * Updated in $/VSNA/VSClient
 * - vad corrected
 * - alfa Native freq
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 31.03.08   Time: 17:54
 * Updated in $/VSNA/VSClient
 * - super discovery
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 27.12.07   Time: 16:23
 * Updated in $/VS2005/VSClient
 * - prefiltering in software echocansellation in case of render absence
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 6.12.07    Time: 16:56
 * Updated in $/VS2005/VSClient
 * - tuning
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 25.12.06   Time: 13:46
 * Updated in $/VS/VSClient
 * - added internal interface for AviWrite module
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 13.09.06   Time: 19:59
 * Updated in $/VS/VSClient
 * - formating in debug
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 20.04.06   Time: 13:46
 * Updated in $/VS/VSClient
 * - new audio hardware test
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 2.12.05    Time: 19:33
 * Updated in $/VS/VSClient
 * - new debug file name
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 16.11.05   Time: 19:03
 * Updated in $/VS/VSClient
 * - added ThinkClient module
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 7.09.05    Time: 17:57
 * Updated in $/VS/VSClient
 * - added proportional scaling (only square pixel)
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 19.08.05   Time: 17:37
 * Updated in $/VS/VSClient
 * - debug trase log not created now in release configration
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 11.08.05   Time: 18:21
 * Created in $/VS/VSClient
 * - added debug module
 *
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include <time.h>
#include <windows.h>
#include "VS_Dmodule.h"
#include "../std/cpplib/VS_RegistryKey.h"


/****************************************************************************
 * Globals
 ****************************************************************************/
/// global pointer VS_DebugOut calss
VS_DebugOut* g_pDtrase = 0;

#ifdef _DEBUG

VS_DebugOut::VS_DebugOut()
{
	m_Modules = VSTM_NONE;
	m_Ptype = PT_NONE;
	m_file = 0;

	VS_RegistryKey key(true, "Current Configuration");
	int val = 0;
	if (key.GetValue(&val, 4, VS_REG_INTEGER_VT, "Debug Modules")>0)
		m_Modules = val;
	if (m_Modules != VSTM_NONE) {
		if (key.GetValue(&val, 4, VS_REG_INTEGER_VT, "Debug OutType")>0) {
			val = val > PT_FILE ? PT_FILE : val < PT_NONE ? PT_NONE : val;
			m_Ptype = (Print_Type)val;
		}
	}
	if (m_Ptype==PT_FILE) {
		char nm[5][256];
		const std::string reg_root(VS_RegistryKey::GetDefaultRoot());
		_splitpath(reg_root.c_str(), nm[1], nm[2], nm[3], nm[4]);
		sprintf(nm[0], "%s_dbg.log", nm[3]);
		m_file = fopen(nm[0], "at");
		if (m_file) {
			time_t timeNow;
			time(&timeNow);
			fprintf(m_file, "VISICRON DLL LOG STARTED AT %s\n", ctime(&timeNow));
		}
	}
}

VS_DebugOut::~VS_DebugOut()
{
	if (m_file) {
		FILE* file = m_file; m_file = 0;
		time_t timeNow;
		time(&timeNow);
		fprintf(file, "VISICRON DLL LOG FINISHED AT %s\n", ctime(&timeNow));
		fclose(file);
	}
}

int VS_DebugOut::PutOut(int module, char* s)
{
	if (m_Ptype==PT_NONE) return 0;
	const char* name = nullptr;
	switch(module)
	{
	case VSTM_AUDI0: name = "Audio"; break;
	case VSTM_PRTCL: name = "Prtcl"; break;
	case VSTM_VRND:  name = "Vrndr"; break;
	case VSTM_THCL:  name = "ThClt"; break;
	case VSTM_PROC:  name = "ClInt"; break;
	case VSTM_BTRC:  name = "BCntr"; break;
	case VSTM_AVIW:  name = "AviWr"; break;
	case VSTM_ECHO:  name = "EchoC"; break;
	case VSTM_AGCIN: name = "AgcIn"; break;
	case VSTM_MIXER: name = "ConfMix"; break;
	case VSTM_NETWORK: name = "Ntwrk"; break;
	default: name = "unkn"; break;
	}
	char tt[1200] = {0};
	SYSTEMTIME st;
	GetSystemTime(&st);
	_snprintf(tt, sizeof(tt), "%s |%02d/%02d/%d %02d:%02d:%02d.%03d| %s\n", name, st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, s);

	switch(m_Ptype)
	{
	case PT_DEBUGOUT:
		OutputDebugString(tt); return strlen(tt);
	case PT_CONSOLE:
		return printf("%s", tt);
	case PT_FILE:
		if (m_file)
			return fputs(tt, m_file);
		else
			return 0;
	default:
		return 0;
	}
}

void VS_TraseOut(int module, const char* a,...)
{
	if (g_pDtrase && g_pDtrase->m_Modules&module) {
		char t[1024];
		va_list lst;
		va_start(lst, a);
		vsprintf(t, a, lst);
		va_end(lst);
		g_pDtrase->PutOut(module, t);
	}
}

#else

VS_DebugOut::VS_DebugOut(){}
VS_DebugOut::~VS_DebugOut(){}
int VS_DebugOut::PutOut(int module, char* s){return 0;}
void VS_TraseOut(int module, char* a,...){}

#endif /*_DEBUG*/
