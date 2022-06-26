/**
 **************************************************************************
 * \file VS_Cpu.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief functions for CPU capabilities detection
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \date 13.02.2004
 *
 * $Revision: 5 $
 *
 * $History: VS_Cpu.h $
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 20.07.10   Time: 19:10
 * Updated in $/VSNA/std/cpplib
 * - were added detect number of cpu cores
 * - SystemBenchmark now detetct number of cpu cores
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 28.10.09   Time: 15:34
 * Updated in $/VSNA/std/cpplib
 * VS_GetCPUType fixed unresolved symbol export
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 28.10.09   Time: 15:18
 * Updated in $/VSNA/std/cpplib
 * VS_GetCPUInternalName fixed unresolved symbol export
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 28.10.09   Time: 14:07
 * Updated in $/VSNA/std/cpplib
 * - VS_GetCPUInternalName return type changed
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 ****************************************************************************/
#pragma once

/**
 **************************************************************************
 * \brief CPU features, can be combained
 ****************************************************************************/
enum VS_CPU_Type
{
	VS_CPU_PURE	= 0,
	VS_CPU_MMX	= 1,
	VS_CPU_SSE	= 2,
	VS_CPU_ISSE	= 4,
	VS_CPU_SSE2	= 8,
};

#ifdef __cplusplus
extern "C" {
#endif

	/// return CPU fetures
	int  VS_GetCPUType();

	/// return internal cpu name (if present)
	int VS_GetCPUInternalName(char* name, int size);

	/// return number of cpu cores
	void VS_GetNumCPUCores(unsigned int *phcores, unsigned int *lcores);

#ifdef __cplusplus
}
#endif


