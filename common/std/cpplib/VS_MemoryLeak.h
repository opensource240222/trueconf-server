/**
**************************************************************************
* \file VS_MemoryLeak.h
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
* \brief show memory leaks in Visual studio debugger output window
*
* \b Project Standart Libraries
* \author SlavetskyA
* \date 30.12.02
*
* $Revision: 1 $
*
* $History: VS_MemoryLeak.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 16.08.06   Time: 15:43
 * Updated in $/VS/std/cpplib
 * - AddVAlue speeding up
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 14.08.06   Time: 15:53
 * Updated in $/VS/std/cpplib
 * - memory leaks in transport
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
****************************************************************************/
#if 0

#ifndef VS_STD_CPP_MEMORY_LEAK
#define VS_STD_CPP_MEMORY_LEAK


/****************************************************************************
 * Includes
 ****************************************************************************/
#if defined(_WIN32) && defined(_DEBUG)

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

inline void* __cdecl operator new(size_t nSize, const char* lpszFileName, int nLine) {
	return ::operator new(nSize, _NORMAL_BLOCK, lpszFileName, nLine);
}
inline void __cdecl operator delete(void* p, const char* lpszFileName, int nLine) {
  ::operator delete(p);
}

#ifndef new
#define new new(__FILE__, __LINE__)
#endif

#endif

/**
 **************************************************************************
 * \brief Set debugger options in constructor, output leaks in destructor
 ****************************************************************************/
class VS_MemoryLeak
{
public:
	VS_MemoryLeak( void )
	{
#if defined(_WIN32) && defined(_DEBUG)
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
		_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
		_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
		int tmp = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		tmp |=_CRTDBG_LEAK_CHECK_DF;
		_CrtSetDbgFlag(tmp);
#endif
	}
};

#endif	// VS_STD_CPP_MEMORY_LEAK
#endif
