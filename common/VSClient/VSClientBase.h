/**
 **************************************************************************
 * \file VSClientbase.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Contain base classes used in many classes of Client project
 *
 * \b Project Client
 * \author Melechko Ivan
 * \author SMirnovK
 * \date 07.10.2004
 *
 * $Revision: 15 $
 *
 * $History: VSClientBase.h $
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 24.07.12   Time: 15:21
 * Updated in $/VSNA/VSClient
 * - ench direct show : were add hw h.264 description mode
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 5.07.12    Time: 13:53
 * Updated in $/VSNA/VSClient
 * - projects depens repaired
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 2.11.11    Time: 14:12
 * Updated in $/VSNA/VSClient
 * - were added capability for capture devices : framerate list &
 * interlaced flag
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 1.11.11    Time: 16:15
 * Updated in $/VSNA/VSClient
 * - were add set thread priority
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 25.07.11   Time: 12:47
 * Updated in $/VSNA/VSClient
 * - were adde support HDYC color format
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 5.03.11    Time: 13:16
 * Updated in $/VSNA/VSClient
 * - improve camera initialization
 * - auto modes for camera init
 * - hardware test
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 13.01.11   Time: 18:55
 * Updated in $/VSNA/VSClient
 * - ench #8342 (refactoring camera states)
 *
 * *****************  Version 7  *****************
 * User: Dront78      Date: 11.01.10   Time: 14:45
 * Updated in $/VSNA/VSClient
 * - solution merge vzo7 to add unicode devices support
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 23.12.09   Time: 20:58
 * Updated in $/VSNA/VSClient
 * - change set caps for video in the capture
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 10.12.09   Time: 19:04
 * Updated in $/VSNA/VSClient
 * - unicode capability for hardware lists
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 24.03.09   Time: 16:53
 * Updated in $/VSNA/VSClient
 * - VZOCHAT7 VSClient support added
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
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 4.07.07    Time: 19:25
 * Updated in $/VS2005/VSClient
 * - device statuses added
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 6.02.07    Time: 20:29
 * Updated in $/VS2005/VSClient
 * - Added DirectShow project
 * - soluton configuration corrected
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 52  *****************
 * User: Melechko     Date: 16.05.06   Time: 12:00
 * Updated in $/VS/VSClient
 * Add support BJPG video format
 *
 * *****************  Version 51  *****************
 * User: Smirnov      Date: 4.05.06    Time: 12:40
 * Updated in $/VS/VSClient
 * - hardware test repaired
 *
 * *****************  Version 50  *****************
 * User: Smirnov      Date: 2.05.06    Time: 18:41
 * Updated in $/VS/vsclient
 * - sender reinitialisation
 *
 * *****************  Version 49  *****************
 * User: Smirnov      Date: 25.04.06   Time: 19:38
 * Updated in $/VS/VSClient
 * - removed old code, Thread class changed
 *
 * *****************  Version 48  *****************
 * User: Smirnov      Date: 25.04.06   Time: 18:27
 * Updated in $/VS/VSClient
 * - removed answer-to-die event in Loop() functions
 *
 * *****************  Version 47  *****************
 * User: Smirnov      Date: 16.02.06   Time: 12:15
 * Updated in $/VS/VSClient
 * - new TheadBase class
 * - receiver now can be inited while in conference
 *
 * *****************  Version 46  *****************
 * User: Smirnov      Date: 15.11.05   Time: 12:32
 * Updated in $/VS/VSClient
 * - multi video codecs support
 *
 * *****************  Version 45  *****************
 * User: Smirnov      Date: 12.09.05   Time: 14:41
 * Updated in $/VS/VSClient
 * - added new codecs support in client: g728, g729a, g722.1
 *
 * *****************  Version 44  *****************
 * User: Melechko     Date: 25.02.05   Time: 15:00
 * Updated in $/VS/VSClient
 * Add DV support
 *
 * *****************  Version 43  *****************
 * User: Melechko     Date: 16.02.05   Time: 14:21
 * Updated in $/VS/VSClient
 * Add MJPEG support
 *
 * *****************  Version 42  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:02
 * Updated in $/VS/VSClient
 * some changes :)
 *
 * *****************  Version 41  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/
#ifndef VS_CLIENT_BASE_H
#define VS_CLIENT_BASE_H

//#define _VIRTUAL_OUT
/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSInterface.h"
#include "extlibs/DirectShowBaseClasses/streams.h"
#include <vector>
#include <memory>
#include <atomic>

/****************************************************************************
 * Classes
 ****************************************************************************/
/**
 **************************************************************************
 * \brief Contain methods to mahage Loop() thread procedure
 ****************************************************************************/
class CVSThread
{
	struct ThreadContext {
		std::atomic<HANDLE> m_pThread;
		DWORD m_dThreadID;
		HANDLE m_hEvDie;
	};
	std::shared_ptr<ThreadContext> m_ctx;

protected:

public:

	CVSThread();
	virtual ~CVSThread();
	virtual DWORD Loop(LPVOID lpParameter)=0;
	static DWORD WINAPI ThreadProc(LPVOID lpParameter){
		auto start_param = reinterpret_cast<std::pair<CVSThread*, HANDLE>*>(lpParameter);
		auto me = start_param->first;
		auto event = start_param->second;
		delete start_param;
		return me->Loop(event);
	};
	bool ActivateThread(LPVOID lpVal, LPDWORD retThreadId = 0);
	void DesactivateThread(const DWORD ThreadId = 0);
	bool IsThreadActiv();
	void SetThreadPriority(int priority);
};

/**
 **************************************************************************
 * \brief String list class
 ****************************************************************************/
class CStringList{
public:
	CStringList();
	~CStringList();
	int iGetMaxString(void);
	int iFindString(const wchar_t *szWstr);
	int iFindString(char *szStr);
	wchar_t *szGetStringByNumber(int string_number);
	int iAddString(wchar_t *szWstr);
	int iAddString(char *szStr);
	void ResetList(void){m_iMaxList=0;};
	static const int m_iMAX_STRING_LENGTH=256;
private:
	static const int m_iMAX_STRINGS=128;
	wchar_t *m_szSourceList[m_iMAX_STRINGS];
	int m_iMaxList;
};

/**
 **************************************************************************
 * \brief Enum of internal color modes (FOURCC codes)
 ****************************************************************************/
class CColorSpace{
public:
	const static int YUYV=0;
	const static int CLPL=1;
	const static int IYUV=2;
	const static int MJPEG=3;
	const static int I420=4;
	const static int YVU9=5;
	const static int Y411=6;
	const static int Y41P=7;
	const static int YUY2=8;
	const static int YVYU=9;
	const static int UYVY=10;
	const static int HDYC=11;
	const static int Y211=12;
	const static int YV12=13;
	const static int CLJR=14;
	const static int IF09=15;
	const static int CPLA=16;
	const static int RGB24=17;
	const static int RGB32=18;
	const static int DVSD=19;
	const static int DVHD=20;
	const static int DVSL=21;
	const static int RGB1 = 22;
	const static int RGB4 = 23;
	const static int RGB8 = 24;
	const static int RGB565 = 25;
	const static int RGB555 = 26;
	const static int BJPG = 27;
	const static int STR0 = 28;
	const static int I420_STR0 = 29;
	const static int I420_STR1 = 30;
	const static int H264 = 31;
	const static int H264_ES = 32;
	const static int NV12 = 33;
	const static int UNKNOWN=-1;

	const static DWORD FCC_YUYV = mmioFOURCC('Y','U','Y','V');
	const static DWORD FCC_CLPL = mmioFOURCC('C','L','P','L');
	const static DWORD FCC_IYUV = mmioFOURCC('I','Y','U','V');
	const static DWORD FCC_I420 = mmioFOURCC('I','4','2','0');
	const static DWORD FCC_YVU9 = mmioFOURCC('Y','V','U','9');
	const static DWORD FCC_Y411 = mmioFOURCC('Y','4','1','1');
	const static DWORD FCC_Y41P = mmioFOURCC('Y','4','1','P');
	const static DWORD FCC_YUY2 = mmioFOURCC('Y','U','Y','2');
	const static DWORD FCC_YVYU = mmioFOURCC('Y','V','Y','U');
	const static DWORD FCC_UYVY = mmioFOURCC('U','Y','V','Y');
	const static DWORD FCC_Y211 = mmioFOURCC('Y','2','1','1');
	const static DWORD FCC_YV12 = mmioFOURCC('Y','V','1','2');
	const static DWORD FCC_NV12 = mmioFOURCC('N','V','1','2');
	const static DWORD FCC_CLJR = mmioFOURCC('C','L','J','R');
	const static DWORD FCC_IF09 = mmioFOURCC('I','F','0','9');
	const static DWORD FCC_CPLA = mmioFOURCC('C','P','L','A');
	const static DWORD FCC_MJPG = mmioFOURCC('M','J','P','G');
	const static DWORD FCC_BJPG = mmioFOURCC('B','J','P','G');
	const static DWORD FCC_STR0 = mmioFOURCC('S','T','R','0');
	const static DWORD FCC_I42S = mmioFOURCC('I','4','2','S');
	const static DWORD FCC_HDYC = mmioFOURCC('H','D','Y','C');
	const static DWORD FCC_H264 = mmioFOURCC('H','2','6','4');
};


/**
 **************************************************************************
 * \brief Contain mode ID and correspond color and dimensions of video image
 ****************************************************************************/
class CColorMode:public CColorSpace{
	friend  class CColorModeList;
public:
	CColorMode(){
		m_ColorSpaceTag = 0;
		m_height = 0;
		m_width = 0;
		m_height_orig = 0;
		m_width_orig = 0;
		m_width_min = 0;
		m_width_max = 0;
		m_height_min = 0;
		m_height_max = 0;
		m_dw = 0;
		m_dh = 0;
		m_frame_int_min = 0;
		m_frame_int_max = 0;
		m_ColorModeID = 0;
		m_bInterlace = false;
		m_bIncorrect = false;
		m_iPinNumber = 0;
		m_FrameIntList.clear();
	};
	bool operator ==(CColorMode Cm){return ((m_ColorSpaceTag==Cm.m_ColorSpaceTag)&&
											(m_height_orig==Cm.m_height_orig)&&(m_width_orig==Cm.m_width_orig)&&
											(m_height_min==Cm.m_height_min)&&(m_height_max==Cm.m_height_max)&&
											(m_width_min==Cm.m_width_min)&&(m_width_max==Cm.m_width_max)&&
											(m_dw==Cm.m_dw)&&(m_dh==Cm.m_dh)&&
											(m_frame_int_min==Cm.m_frame_int_min)&&(m_frame_int_max==Cm.m_frame_int_max)&&
											(m_FrameIntList==Cm.m_FrameIntList)&&
											(m_bInterlace==Cm.m_bInterlace))?TRUE:FALSE;};
	bool operator !=(CColorMode Cm){return ((m_ColorSpaceTag!=Cm.m_ColorSpaceTag)||(m_height_orig!=Cm.m_height_orig)||(m_width_orig!=Cm.m_width_orig))?TRUE:FALSE;};
	void SetColorMode(void* ColorModeID,int ColorSpaceTag,int height,int width);
	void SetVideoCaps(int height_min, int height_max, int width_min, int width_max, int dx, int dy, __int64 frame_int_min, __int64 frame_int_max,
					  __int64 *pFrameIntList, int sizeList, bool bInterlace);
	void SetPinNumber(int iNum) { m_iPinNumber = iNum; }
	void SetInrorrect(bool bIncorrect) { m_bIncorrect = bIncorrect; }
	void* GetColorMode(void){return m_ColorModeID;};
	void ColorModeToBitmapInfoHeader(LPBITMAPINFOHEADER lpB);
	int GetPinNumber() { return m_iPinNumber; }
	bool GetIncorrect() { return m_bIncorrect; }
	int GetColorSpaceTag(){return m_ColorSpaceTag;};
	void* m_ColorModeID;
protected:
	int m_ColorSpaceTag;
	int m_width, m_height;
	int m_width_orig, m_height_orig;
	int m_width_min, m_width_max;
	int m_height_min, m_height_max;
	int m_dw, m_dh;
	__int64 m_frame_int_min, m_frame_int_max;
	std::vector<__int64> m_FrameIntList;
	bool m_bInterlace;
	int m_iPinNumber;
	bool m_bIncorrect;
};

/**
 **************************************************************************
 * \brief Common virtual class to manage list of modes
 ****************************************************************************/
class CModeList
{
public:
	CModeList() { m_iMaxMode = 0; }
	virtual ~CModeList() { m_iMaxMode = 0; }
	virtual void* GetModeID(int mode_number) = 0;
	virtual void ResetList(void) = 0;
	virtual int iGetModeDescription(int mode_number,void *pModeDesc) = 0;
	virtual int iUpdateModeDescription(int mode_number,void *pModeDesc) = 0;
	virtual int iGetPinNumber(int mode_number) = 0;
	int iGetMaxMode(void) { return m_iMaxMode; }
protected:
	int m_iMaxMode;
};

/**
 **************************************************************************
 * \brief Description for color mode
 ****************************************************************************/
class CColorModeDescription
{
public:
	int Width, Height;
	int WidthBase, HeightBase;
	int Color;
	int WidthMin, WidthMax;
	int HeightMin, HeightMax;
	int dW, dH;
	__int64 FrameIntMin, FrameIntMax;
	std::vector<__int64> FrameIntList;
	bool bInterlace;
	bool bIncorrect;
public:
	int iGetdW() { return ((dW == 0 && WidthMin <= WidthMax) ? 4 : dW); }
	int iGetdH() { return ((dH == 0 && HeightMin <= HeightMax) ? 4 : dH); }
	bool operator == (CColorModeDescription& src) {
		bool ret = (Width == src.Width);
		ret = ret && (Height == src.Height);
		ret = ret && (bInterlace == src.bInterlace);
		return ret;
	}
};


/**
 **************************************************************************
 * \brief Color modes list
 ****************************************************************************/
class CColorModeList: public CModeList
{
public:
	virtual ~CColorModeList();
	void* GetModeID(int mode_number);
	int iGetModeDescription(int mode_number,void *pModeDesc);
	int iUpdateModeDescription(int mode_number,void *pModeDesc);
	int iGetPinNumber(int mode_number);
	virtual void ResetList(void) {};
protected:
	std::vector<CColorMode> m_ModeList;
};

/**
 **************************************************************************
 * \brief Retrive direct show capture device mode list
 ****************************************************************************/
class CModeListDirectShow: public CColorModeList
{
public:
	CModeListDirectShow() {};
	virtual ~CModeListDirectShow() { ResetList(); }
	void ResetList(void);
	int iAddMode(void* _pMode, void* _pVideoCaps, __int64 *pFrameIntList, int sizeList, int pinNumber, bool fixMjpeg, unsigned int limitMB);
};

/**
 **************************************************************************
 * \brief Retrive media foundation capture device mode list
 ****************************************************************************/
class CModeListMediaFoundation: public CColorModeList
{
public:
	CModeListMediaFoundation() {};
	virtual ~CModeListMediaFoundation() { ResetList(); }
	void ResetList(void);
	int iAddMode(void* _pMode, int pinNumber, bool fixMjpeg, unsigned int limitMB);
};

/**
 **************************************************************************
 * \brief Retrive screen capture device mode list
 ****************************************************************************/
class CModeListScreenCapture: public CColorModeList
{
public:
	CModeListScreenCapture() {};
	virtual ~CModeListScreenCapture() { ResetList(); }
	void ResetList(void);
	int iAddMode(void* _pMode);
};

/**
 **************************************************************************
 * \brief list of devices
 ****************************************************************************/

class CDeviceList{
public:
	CDeviceList();
	virtual ~CDeviceList();
	virtual int iGetDeviceList() = 0;
	int iGetDeviceNumberByName(wchar_t *pDeviceName){return m_pSourceList->iFindString(pDeviceName);};
	int iGetDeviceCounter(){return m_pSourceList->iGetMaxString();};
	CStringList *m_pSourceList;
	CModeList* m_pModeList;
};

class VS_DeviceStatus
{
	HANDLE	m_hChanged;
	int		m_audio;
	int		m_video;
public:
	VS_DeviceStatus() : m_audio(0), m_video(0) {
		m_hChanged = CreateEvent(0, 0, 0, 0);
	};
	~VS_DeviceStatus() {
		CloseHandle(m_hChanged);
	}
	void SetStatus(int status, bool isAudio, bool isSet) {
		int& iStatus = isAudio ? m_audio : m_video;
		int tStatus = iStatus;
		if (isSet)
			tStatus|=status;
		else
			tStatus&=~status;
		if (iStatus!= tStatus) {
			iStatus = tStatus;
			SetEvent(m_hChanged);
		}
	}
	HANDLE GetEvent() {return m_hChanged;}
	long GetStatus() {return m_audio | (m_video<<16);}
};

extern VS_DeviceStatus g_DevStatus;
#endif