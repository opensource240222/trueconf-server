/**
 **************************************************************************
 * \file VS_DModule.h
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief Contain classes working with audio: capturing, rendering, compressing
 *
 * \b Project Client
 * \author SMirnovK
 * \date 11.08.2005
 *
 * $Revision: 5 $
 *
 * $History: VS_Dmodule.h $
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 28.09.11   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - beta nhp revision
 * - fix fps on low bitrates
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 5.03.11    Time: 13:16
 * Updated in $/VSNA/VSClient
 * - improve camera initialization
 * - auto modes for camera init
 * - hardware test
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 18.08.10   Time: 14:09
 * Updated in $/VSNA/VSClient
 * - vad corrected
 * - alfa Native freq
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 27.12.07   Time: 16:23
 * Updated in $/VS2005/VSClient
 * - prefiltering in software echocansellation in case of render absence
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 25.12.06   Time: 13:46
 * Updated in $/VS/VSClient
 * - added internal interface for AviWrite module
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 20.04.06   Time: 13:46
 * Updated in $/VS/VSClient
 * - new audio hardware test
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 16.11.05   Time: 19:03
 * Updated in $/VS/VSClient
 * - added ThinkClient module
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 7.09.05    Time: 17:57
 * Updated in $/VS/VSClient
 * - added proportional scaling (only square pixel)
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 11.08.05   Time: 18:21
 * Created in $/VS/VSClient
 * - added debug module
 *
 ****************************************************************************/
#ifndef _VS_DMODULE_H_
#define _VS_DMODULE_H_


/****************************************************************************
 * Includes
 ****************************************************************************/
#include <stdio.h>


/****************************************************************************
 * Defines
 ****************************************************************************/
#ifdef _DEBUG
#define DTRACE	VS_TraseOut
#else
#define DTRACE
#endif


/****************************************************************************
 * Declarations
 ****************************************************************************/
/// init it as soon as possible
extern class VS_DebugOut* g_pDtrase;


/// put to debug out text, use printf() syntax
void VS_TraseOut(int module, const char* a,...);

/**
 **************************************************************************
 * \brief module param in VS_TraseOut() func,
 ****************************************************************************/
enum VS_Trase_Modules
{
	VSTM_NONE		= 0x0000,
	VSTM_AUDI0		= 0x0001,
	VSTM_PRTCL		= 0x0002,
	VSTM_VRND		= 0x0004,
	VSTM_THCL		= 0x0008,
	VSTM_PROC		= 0x0010,
	VSTM_BTRC		= 0x0020,
	VSTM_AVIW		= 0x0040,
	VSTM_ECHO		= 0x0080,
	VSTM_AGCIN		= 0x0100,
	VSTM_VIDEO		= 0x0200,
	VSTM_NHP_AUDIO	= 0x0400,
	VSTM_NHP_VIDEO	= 0x0800,
	VSTM_NHP_STAT	= 0x1000,
	VSTM_NHP_OTHER	= 0x2000,
	VSTM_NHP_SND	= 0x4000,
	VSTM_VCAPTURE	= 0x8000,
	VSTM_MIXER      = 0x10000,
	VSTM_VIDEO_CODEC = 0x20000,
	VSTM_NETWORK = 0x40000,
	// add new there, new_en = prev_en * 2
	VSTM_ALL		= ~0
};

/**
 **************************************************************************
 * \brief Class contained some debug methods, when created read registry
 ****************************************************************************/
class VS_DebugOut
{
	enum Print_Type {
		PT_NONE,
		PT_DEBUGOUT,
		PT_CONSOLE,
		PT_FILE
	};
	FILE*		m_file;
	Print_Type	m_Ptype;
public:
	int			m_Modules;
	VS_DebugOut();
	~VS_DebugOut();
	int PutOut(int module, char* s);
};

#endif /*_VS_DMODULE_H_*/