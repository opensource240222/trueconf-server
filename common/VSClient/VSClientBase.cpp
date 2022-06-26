/**
 **************************************************************************
 * \file VSClientbase.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Contain base classes used in many classes of Client project
 *
 * \b Project Client
 * \author Melechko Ivan
 * \author SMirnovK
 * \date 07.10.2004
 *
 * $Revision: 19 $
 *
 * $History: VSClientBase.cpp $
 *
 * *****************  Version 19  *****************
 * User: Sanufriev    Date: 24.07.12   Time: 15:21
 * Updated in $/VSNA/VSClient
 * - ench direct show : were add hw h.264 description mode
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 5.07.12    Time: 13:53
 * Updated in $/VSNA/VSClient
 * - projects depens repaired
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 2.11.11    Time: 14:12
 * Updated in $/VSNA/VSClient
 * - were added capability for capture devices : framerate list &
 * interlaced flag
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 1.11.11    Time: 16:15
 * Updated in $/VSNA/VSClient
 * - were add set thread priority
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 25.07.11   Time: 12:47
 * Updated in $/VSNA/VSClient
 * - were adde support HDYC color format
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 27.06.11   Time: 14:58
 * Updated in $/VSNA/VSClient
 * - set cpu cores info int VSGetSystemInfo_Processor
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 5.03.11    Time: 13:16
 * Updated in $/VSNA/VSClient
 * - improve camera initialization
 * - auto modes for camera init
 * - hardware test
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 13.01.11   Time: 18:55
 * Updated in $/VSNA/VSClient
 * - ench #8342 (refactoring camera states)
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 15.03.10   Time: 14:24
 * Updated in $/VSNA/VSClient
 * - system info for WIN7
 *
 * *****************  Version 9  *****************
 * User: Dront78      Date: 11.01.10   Time: 14:45
 * Updated in $/VSNA/VSClient
 * - solution merge vzo7 to add unicode devices support
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 23.12.09   Time: 20:58
 * Updated in $/VSNA/VSClient
 * - change set caps for video in the capture
 *
 * *****************  Version 7  *****************
 * User: Dront78      Date: 24.03.09   Time: 16:53
 * Updated in $/VSNA/VSClient
 * - VZOCHAT7 VSClient support added
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 12.03.09   Time: 18:11
 * Updated in $/VSNA/VSClient
 * - OS name updated
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 5.03.09    Time: 14:21
 * Updated in $/VSNA/VSClient
 * - activatethread changed
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 17.02.08   Time: 10:58
 * Updated in $/VSNA/VSClient
 * - HardwareTest: fix output cause of funcs have other names
 * - Stored Procedure: call SetAllEpProperties() for known params
 * - split Capabilities into: is_audio, is_mic, is_camera
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 16.02.08   Time: 11:29
 * Updated in $/VSNA/VSClient
 * - ChatSend(): Compose method depends on To-param
 * - EpProperties added
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 14.02.08   Time: 13:30
 * Updated in $/VSNA/VSClient
 * - endpoInt properties
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 4.07.07    Time: 19:25
 * Updated in $/VS2005/VSClient
 * - device statuses added
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 2.05.07    Time: 19:48
 * Updated in $/VS2005/VSClient
 * - retriving OS version info renewed
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:42
 * Updated in $/VS2005/VSClient
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 34  *****************
 * User: Smirnov      Date: 29.05.06   Time: 17:58
 * Updated in $/VS/VSClient
 * - new system info
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 2.05.06    Time: 18:41
 * Updated in $/VS/VSClient
 * - sender reinitialisation
 *
 * *****************  Version 32  *****************
 * User: Smirnov      Date: 25.04.06   Time: 18:27
 * Updated in $/VS/VSClient
 * - removed answer-to-die event in Loop() functions
 *
 * *****************  Version 31  *****************
 * User: Smirnov      Date: 16.02.06   Time: 12:15
 * Updated in $/VS/VSClient
 * - new TheadBase class
 * - receiver now can be inited while in conference
 *
 * *****************  Version 30  *****************
 * User: Smirnov      Date: 15.11.05   Time: 12:32
 * Updated in $/VS/VSClient
 * - multi video codecs support
 *
 * *****************  Version 29  *****************
 * User: Smirnov      Date: 12.09.05   Time: 14:41
 * Updated in $/VS/VSClient
 * - added new codecs support in client: g728, g729a, g722.1
 *
 * *****************  Version 28  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSClientBase.h"
#include "VS_ApplicationInfo.h"
#include <dvdmedia.h>
#include <atomic>
#include <memory>


/****************************************************************************
* Classes
****************************************************************************/
/****************************************************************************
 * CVSThread
 ****************************************************************************/
CVSThread::CVSThread()
{
}

CVSThread::~CVSThread()
{
	DesactivateThread();
}

bool CVSThread::ActivateThread(LPVOID lpVal, LPDWORD retThreadId)
{
	auto old_ctx = std::atomic_load(&m_ctx);
	if (!!old_ctx)
		DesactivateThread(old_ctx->m_dThreadID);
	auto ctx = std::make_shared<ThreadContext>();
	ctx->m_hEvDie = CreateEvent(NULL, TRUE, FALSE, NULL);
	auto start_param = new std::pair<CVSThread*, HANDLE>(this, ctx->m_hEvDie);
	// On some systems with 1 CPU (at least on Windows 10 Professional build 10586 32-bit)
	// CreateThread() immediately starts executing the new thread, even before setting thread ID to variable pointed by lpThreadId.
	// This leads to m_dThreadID being uninitialized from the point of view of new thread and breaks logic in it.
	// To workaround this we create thread with CREATE_SUSPENDED and resume it after thread ID is saved where it should be (m_dThreadID and *retThreadId).
	ctx->m_pThread = CreateThread(NULL, 0, ThreadProc, start_param, CREATE_SUSPENDED, &ctx->m_dThreadID);
	if (!ctx->m_pThread)
	{
		CloseHandle(ctx->m_hEvDie);
		return false;
	}
	if (std::atomic_compare_exchange_strong(&m_ctx, &old_ctx, ctx) == false)
	{
		CloseHandle(ctx->m_hEvDie);
		CloseHandle(ctx->m_pThread);
		return false;
	}
	if (retThreadId) *retThreadId = ctx->m_dThreadID;
	ResumeThread(ctx->m_pThread);
	return true;
}

void CVSThread::DesactivateThread(const DWORD ThreadId)
{
	auto old_ctx = std::atomic_load(&m_ctx);
	if (!old_ctx)
		return;
	if (ThreadId && old_ctx->m_dThreadID != ThreadId)
		return;

	auto old_handle = old_ctx->m_pThread.exchange(nullptr);
	if (old_handle)
	{
		SetEvent(old_ctx->m_hEvDie);
		if (GetCurrentThreadId() != old_ctx->m_dThreadID)
			WaitForSingleObject(old_handle, INFINITE);
		CloseHandle(old_handle);
		CloseHandle(old_ctx->m_hEvDie);
	}
}

bool CVSThread::IsThreadActiv()
{
	auto old_ctx = std::atomic_load(&m_ctx);
	if (!old_ctx || old_ctx->m_pThread == nullptr)
		return false;
	return WaitForSingleObject(old_ctx->m_pThread, 0) != WAIT_OBJECT_0;		// kt: when thread exits, it signals its handle
}

void CVSThread::SetThreadPriority(int priority)
{
	auto ctx = std::atomic_load(&m_ctx);
	if (!!ctx)
		::SetThreadPriority(ctx->m_pThread, priority);
}

/****************************************************************************
 * CDeviceList
 ****************************************************************************/
CDeviceList::CDeviceList(){
		m_pSourceList=new CStringList();
	return;
};

CDeviceList::~CDeviceList(){
		delete m_pSourceList;
    m_pSourceList=NULL;
	return;
};


/****************************************************************************
 * CColorMode
 ****************************************************************************/
void CColorMode::ColorModeToBitmapInfoHeader(LPBITMAPINFOHEADER lpB)
{
	lpB->biClrImportant= 0;
	lpB->biClrUsed= 0;
	lpB->biPlanes= 1;
	lpB->biSize= sizeof(BITMAPINFOHEADER);
	lpB->biWidth= m_width;
	lpB->biHeight= m_height;
	lpB->biXPelsPerMeter	= 0;
	lpB->biYPelsPerMeter	= 0;
	switch(m_ColorSpaceTag)
	{
	case RGB24:
	   lpB->biBitCount= 24;
	   lpB->biCompression= BI_RGB;
	   break;
	case RGB32:
	   lpB->biBitCount= 32;
	   lpB->biCompression= BI_RGB;
	   break;
	case YUY2:
	   lpB->biBitCount= 16;
	   lpB->biCompression = FCC_YUY2;
	   break;
	case UYVY:
	case HDYC:
	   lpB->biBitCount= 16;
	   lpB->biCompression = FCC_UYVY;
	   break;
	case I420:							//<! planar
	   lpB->biBitCount= 12;
	   lpB->biCompression = FCC_I420;
	   break;
	case IYUV:							//<! planar
	   lpB->biBitCount= 12;
	   lpB->biCompression = FCC_IYUV;
	   break;
	case YV12:							//<! planar
	   lpB->biBitCount= 12;
	   lpB->biCompression = FCC_YV12;
	   break;
	case YVU9:							//<! planar
	   lpB->biBitCount= 9;
	   lpB->biCompression = FCC_YVU9;
	   break;
	case IF09:							//<! planar
	   lpB->biBitCount= 9;				//<! realy 9.5 bit; it the same as YVU9 plus additional m_width/4*m_height/4 plane
	   lpB->biCompression = FCC_IF09;
	   break;
	case I420_STR0:						//<! planar + stereo
	case I420_STR1:
	   lpB->biBitCount= 12;
	   lpB->biCompression = FCC_I42S;
	   lpB->biHeight *= 2;
	   break;
	case H264:						//<! h.264 & h.264 (not alligned)
	case H264_ES:
	   lpB->biBitCount= 12;
	   lpB->biCompression = FCC_H264;
	   break;
	case NV12:
		lpB->biBitCount = 12;
		lpB->biCompression = FCC_NV12;
		break;
	default:
	   lpB->biBitCount= 16;
	   lpB->biCompression = -1;
	   break;
	}
   lpB->biSizeImage= (lpB->biWidth*lpB->biHeight*lpB->biBitCount)/8;
}

/****************************************************************************
 * CColorModeList
 ****************************************************************************/
int CColorModeList::iGetModeDescription(int mode_number,void *pModeDesc){
	CColorModeDescription * pMode=reinterpret_cast<CColorModeDescription*>(pModeDesc);
	if (mode_number >= 0 && mode_number < (int)m_ModeList.size()) {
	   pMode->Height=m_ModeList[mode_number].m_height;
       pMode->Width=m_ModeList[mode_number].m_width;
	   pMode->HeightBase=m_ModeList[mode_number].m_height_orig;
       pMode->WidthBase=m_ModeList[mode_number].m_width_orig;
	   pMode->Color=m_ModeList[mode_number].m_ColorSpaceTag;
	   pMode->WidthMin=m_ModeList[mode_number].m_width_min;
	   pMode->WidthMax=m_ModeList[mode_number].m_width_max;
	   pMode->HeightMin=m_ModeList[mode_number].m_height_min;
	   pMode->HeightMax=m_ModeList[mode_number].m_height_max;
	   pMode->dW=m_ModeList[mode_number].m_dw;
	   pMode->dH=m_ModeList[mode_number].m_dh;
	   pMode->FrameIntMin=m_ModeList[mode_number].m_frame_int_min;
	   pMode->FrameIntMax=m_ModeList[mode_number].m_frame_int_max;
	   pMode->FrameIntList=m_ModeList[mode_number].m_FrameIntList;
	   pMode->bInterlace=m_ModeList[mode_number].m_bInterlace;
	   pMode->bIncorrect =m_ModeList[mode_number].m_bIncorrect;
	   return 0;
	}
	return -1;
};

int CColorModeList::iUpdateModeDescription(int mode_number,void *pModeDesc){
	CColorModeDescription * pMode=reinterpret_cast<CColorModeDescription*>(pModeDesc);
	if (mode_number >= 0 && mode_number < (int)m_ModeList.size()) {
	   m_ModeList[mode_number].m_height = pMode->Height;
       m_ModeList[mode_number].m_width = pMode->Width;
	   m_ModeList[mode_number].m_height_orig = pMode->HeightBase;
       m_ModeList[mode_number].m_width_orig = pMode->WidthBase;
	   m_ModeList[mode_number].m_ColorSpaceTag = pMode->Color;
	   m_ModeList[mode_number].m_width_min = pMode->WidthMin;
	   m_ModeList[mode_number].m_width_max = pMode->WidthMax;
	   m_ModeList[mode_number].m_height_min = pMode->HeightMin;
	   m_ModeList[mode_number].m_height_max = pMode->HeightMax;
	   m_ModeList[mode_number].m_dw = pMode->dW;
	   m_ModeList[mode_number].m_dh = pMode->dH;
	   m_ModeList[mode_number].m_frame_int_min = pMode->FrameIntMin;
	   m_ModeList[mode_number].m_frame_int_max = pMode->FrameIntMax;
	   m_ModeList[mode_number].m_FrameIntList = pMode->FrameIntList;
	   m_ModeList[mode_number].m_bInterlace = pMode->bInterlace;
	   m_ModeList[mode_number].m_bIncorrect = pMode->bIncorrect;
	   return 0;
	}
	return -1;
};

int CColorModeList::iGetPinNumber(int mode_number){
	if (mode_number >= 0 && mode_number < (int)m_ModeList.size()) {
		return m_ModeList[mode_number].m_iPinNumber;
	}
	return 0;
};

void* CColorModeList::GetModeID(int mode_number){
	if (mode_number >= 0 && mode_number < (int)m_ModeList.size()) {
		return m_ModeList[mode_number].m_ColorModeID;
	}
	return NULL;
};

CColorModeList::~CColorModeList(){
	ResetList();
}

/****************************************************************************
 * CColorMode
 ****************************************************************************/
void CColorMode::SetColorMode(void* ColorModeID,int ColorSpaceTag,int height,int width){
	m_ColorModeID=ColorModeID;
	m_ColorSpaceTag=ColorSpaceTag;
	m_height=height;
	m_width=width;
	m_height_orig=height;
	m_width_orig=width;
	return;
}

void CColorMode::SetVideoCaps(int height_min, int height_max, int width_min, int width_max, int dx, int dy, __int64 frame_int_min, __int64 frame_int_max,
							  __int64 *pFrameIntList, int sizeList, bool bInterlace)
{
	m_height_min = height_min;
	m_height_max = height_max;
	m_width_min = width_min;
	m_width_max = width_max;
	m_dw = dx;
	m_dh = dy;
	m_frame_int_min = frame_int_min;
	m_frame_int_max = frame_int_max;
	if (pFrameIntList != 0) {
		for (int i = 0; i < sizeList; i++) m_FrameIntList.push_back(pFrameIntList[i]);
	}
	m_bInterlace = bInterlace;
}

/****************************************************************************
 * CStringList
 ****************************************************************************/
CStringList::CStringList(){
	int i;
	for(i=0;i<m_iMAX_STRINGS;i++)
		m_szSourceList[i]=NULL;
	m_iMaxList=0;

};

CStringList::~CStringList(){
	int i;
	for(i=0;i<m_iMAX_STRINGS;i++)
		if (m_szSourceList[i])
			free(m_szSourceList[i]);
}

int CStringList::iGetMaxString(void){
	return(m_iMaxList);
};

wchar_t * CStringList::szGetStringByNumber(int string_number){
	if((string_number>=0)&&(string_number<m_iMaxList))
		return m_szSourceList[string_number];
	return NULL;
};

int CStringList::iAddString(wchar_t* szWstr){
	if(szWstr && m_iMaxList<m_iMAX_STRINGS){
		if (m_szSourceList[m_iMaxList])
			free(m_szSourceList[m_iMaxList]);
		m_szSourceList[m_iMaxList] = NULL;
		m_szSourceList[m_iMaxList] = _wcsdup(szWstr);
		m_iMaxList++;
		return 0;
	}
	return -1;
};

int CStringList::iAddString(char* szStr){
	if (!szStr) return -1;
	//wchar_t szWstr[m_iMAX_STRING_LENGTH];
	//mbstowcs( szWstr, szStr, m_iMAX_STRING_LENGTH );
  _bstr_t bs=szStr;
	return iAddString((wchar_t*)bs);
};

int CStringList::iFindString(const wchar_t *szWstr){
	int i=0;
    for(i=0;i<m_iMaxList;i++){
		if(wcscmp(m_szSourceList[i], szWstr)==0){
			return i;
		}
    }
	return -1;
};

int CStringList::iFindString(char *szStr) {
	if (!szStr) return -1;
	//wchar_t szWstr[m_iMAX_STRING_LENGTH];
	//mbstowcs( szWstr, szStr, m_iMAX_STRING_LENGTH );
  _bstr_t bs=szStr;
	return iFindString((wchar_t*)bs);
};

VS_DeviceStatus g_DevStatus;