/**
 **************************************************************************
 * \file VSVideoCaptureSlot.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief VideoCaptureSlot class declaration
 * \b Project Client
 * \author Melechko Ivan
 * \date 22.12.2004
 *
 * $Revision: 6 $
 *
 * $History: VSVideoCaptureSlot.h $
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 29.03.11   Time: 13:08
 * Updated in $/VSNA/VSClient
 * - update Capture module (STA implementation)
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 5.03.11    Time: 13:16
 * Updated in $/VSNA/VSClient
 * - improve camera initialization
 * - auto modes for camera init
 * - hardware test
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 2.12.10    Time: 16:14
 * Updated in $/VSNA/VSClient
 * - increase accuracy video capture timestamp
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 20.03.08   Time: 17:03
 * Updated in $/VSNA/VSClient
 * - Video for Linux I420 support added via memory mapped files.
 * Compilation controls via #define VS_LINUX_DEVICE in VSCapture.h
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 11.05.06   Time: 13:16
 * Updated in $/VS/VSClient
 * - added stream command (alfa version)
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 2.05.06    Time: 18:41
 * Updated in $/VS/VSClient
 * - sender reinitialisation
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 25.04.06   Time: 14:05
 * Updated in $/VS/VSClient
 * - sender and it devices classes documented, code cleared
 *
 * *****************  Version 4  *****************
 * User: Melechko     Date: 25.02.05   Time: 15:00
 * Updated in $/VS/VSClient
 * Add DV support
 *
 * *****************  Version 3  *****************
 * User: Melechko     Date: 24.02.05   Time: 12:39
 * Updated in $/VS/VSClient
 * Add CaptureSlotExt
 *
 * *****************  Version 2  *****************
 * User: Melechko     Date: 25.01.05   Time: 13:38
 * Updated in $/VS/VSClient
 * split input video buffer
 *
 * *****************  Version 1  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:00
 * Created in $/VS/VSClient
 * some changes :)
 *
*/
#ifndef _VSVIDCAP_SLOT_H
#define _VSVIDCAP_SLOT_H
/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSInterface.h"
#include "VSThinkClient.h"
#include "VSVideoCaptureList.h"
/****************************************************************************
 * Declarations
 ****************************************************************************/
class CVideoCaptureList;

class CVideoCaptureSlotBase: public CVSInterface
{

protected:

	const static char	_funcResetWindow[];

	bool				m_bValid;			///< validity flag
	HWND				m_hwnd;				///< current window handle
	CVideoRenderBase	*m_pRender;			///< videorender
	DWORD				*m_ppRender;
	unsigned char *		m_pRenderBuffer;	///< pointer to half of m_pInputBuffer buffer
	VS_MediaFormat		m_sndFormat, m_rcvFormat;
	int					m_inputSize;
	wchar_t				m_cDeviceName[MAX_PATH];
	wchar_t				m_cDeviceRegName[MAX_PATH];

	/// set new window handle
	void ResetWindow(HWND hwnd) { m_hwnd = hwnd; }
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;

public:

	/// create video render
	CVideoCaptureSlotBase(const char *szSlotName, CVSInterface* pParentInterface, TVideoWindowParams *pVParams);
	virtual ~CVideoCaptureSlotBase();
	/// init by medaiformat
	virtual void _Init(VS_MediaFormat &mf, eDeviceAction action) = 0;
	/// release all
	virtual void _Release() = 0;
	/// return "new video frame" event
	virtual HANDLE GetVideoEvent() = 0;
	/// return "change device state" event
	virtual HANDLE GetVideoEventConnect() = 0;
	/// return "change device state" event
	virtual HANDLE GetVideoEventDisconnect() = 0;
	/// get pointer to new videoframe
	virtual unsigned char *GetVideoFrame(int *pFPS, unsigned int *pTimestamp, unsigned char *&pRenderFrame) = 0;
	/// get type capturer
	virtual CVideoCaptureList::eCapturerType GetTypeCapturer() = 0;
	/// draw last captured frame
	void DrawFrame();
	/// return validation flag
	bool IsValid() { return m_bValid; }

};

/**
 **************************************************************************
 * \brief Contain video device and correspondent video render
 ****************************************************************************/
class CVideoCaptureSlot: public CVideoCaptureSlotBase, public VS_Lock, public VS_VideoCaptureSlotObserver
{

	const static char _RegDeviceName[];
	const static char _RegDeviceChannel[];
	const static char _RegDeviceMode[];
	const static char _RegDeviceDeinterlace[];
	const static char _RegDeviceHDSource[];
	const static char _RegDeviceFixNTSC[];

	/// Video Device
	VS_CaptureDevice			*m_pCaptureDevice;
	VS_CaptureDevice			*m_pCaptureDeviceList[CVideoCaptureList::CAPTURE_MAX];
	CVideoCaptureList			*m_pVideoCaptureList;
	VS_MediaFormatManager		*m_pMediaFormatManager;
	CCaptureCConv				*m_pColorConversion;
	unsigned char				*m_pInputBuffer;
	/// Handles
	HANDLE						m_hConnectDevice;
	HANDLE						m_hDisconnectDevice;
	HANDLE						m_hGetFrame;
	HANDLE						m_hCaptureMutex;
	HANDLE						m_hProcessFrameEnd;
	eDeviceAction				m_eState;
	int							m_numConnectRequest;
	bool						m_bScreenCapture;
	int							m_realFramerate;
	unsigned int				m_timestamp;
	int							m_frameSize;
	/// Settings capture
	VS_CaptureDeviceSettings	m_devSettings;

private :

	bool    UpdateSndFormat(VS_MediaFormat *mf, bool bScreenCapture);
	bool    UpdateRcvFormat(VS_MediaFormat *mf, int size);
	void	ReleaseRcvBuffers();
	void	ReleaseSndBuffers();
	int		ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
	bool	IsInterfaceSupport(VS_INTERFACE_TYPE TypeChecked) { return TypeChecked == IT_VIDEOCAPTURE; }

public :
	/// create DirectShow video device
	CVideoCaptureSlot(const char *szSlotName, CVSInterface* pParentInterface, TVideoWindowParams *pVParams,
					  CVideoCaptureList *pVideoCaptureList, VS_MediaFormatManager *pMediaFormatManager);
	~CVideoCaptureSlot();
	/// return "new video frame" event
	HANDLE GetVideoEvent();
	/// return "change device state" event
	HANDLE GetVideoEventConnect();
	/// return "change device state" event
	HANDLE GetVideoEventDisconnect();
	/// get pointer to new videoframe
	unsigned char *GetVideoFrame(int *pFPS, unsigned int *pTimestamp, unsigned char *&pRenderFrame);
	/// push frame from capture device
	void PushFrame(unsigned char *pBuffer, int size, int realFramerate, unsigned int timestamp, int width, int height, unsigned int color, bool hardware);
	/// init video render
	void _Init(VS_MediaFormat &mf, eDeviceAction action);
	/// close video device and video render
	void _Release();
	/// get type capturer
	CVideoCaptureList::eCapturerType GetTypeCapturer();
};

/**
 **************************************************************************
 * \brief Contain video render, can receive video from GUI, convert
 * to appropriate size and I420 videoframe, can draw it
 ****************************************************************************/
class CVideoCaptureSlotExt: public CVideoCaptureSlotBase
{
	const static char _funcSetFormat[];
	const static char _funcPushData[];

	CCaptureCConv		m_ColConv;		///< image converter class
	unsigned char *		m_pTmpBuffer;	///< converted to appropriate size I420 Image
	unsigned char *		m_pInputBuffer;	///< pointer to buffer sizeof I420 Image
	HANDLE				m_hEvent;		///< set when conversion occure (new data arrived)
	int					m_iSampleSize;	///< size of I420 Image
	int					iRealFps;		///< ??? fps
	int					iFps;			///< ??? fps
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
	/// check type of interface
	bool IsInterfaceSupport(VS_INTERFACE_TYPE TypeChecked) { return TypeChecked == IT_VIDEOCAPTURE; }
public:
	/// set to zero members
	CVideoCaptureSlotExt(const char *szSlotName, CVSInterface* pParentInterface, TVideoWindowParams *pVParams);
	~CVideoCaptureSlotExt();
	/// init video render
	void _Init(VS_MediaFormat &mf, eDeviceAction action);
	/// release all recources
	void _Release();
	/// return "new video frame" event
	HANDLE GetVideoEvent();
	/// return "change device state" event
	HANDLE GetVideoEventConnect();
	/// return "change device state" event
	HANDLE GetVideoEventDisconnect();
	/// get pointer to new videoframe
	unsigned char *GetVideoFrame(int *pFPS, unsigned int *pTimestamp, unsigned char *&pRenderFrame);
	/// get type capturer
	CVideoCaptureList::eCapturerType GetTypeCapturer();
};

#ifdef VS_LINUX_DEVICE
class CVideocaptureV4L: public CVideoCaptureSlotBase
{
private:
	HANDLE m_Event;
	HANDLE m_EventTimer;
	unsigned char *					m_pInputBuffer;			///< pointer to buffer sizeof I420 Image
	CCaptureCConv m_ColConv;
	HANDLE m_Thread;
	HANDLE m_file;
	HANDLE m_ofile;
	size_t m_size;
	DWORD m_sleep;
	void *m_map, *m_copy;
	bool destroy;

public:
	/// create V4L video device
	CVideocaptureV4L(const char *szSlotName, CVSInterface* pParentInterface, TVideoWindowParams *pVParams, CVideoCaptureList *pVideoCaptureList);
	~CVideocaptureV4L();
	/// return "new video frame" event
	HANDLE GetVideoEvent();
	/// get pointer to new videoframe
	unsigned char *GetVideoFrame(int *pFPS);
	/// init video render
	void _Init(VS_MediaFormat &mf);
	/// close video device and video render
	void _Release();
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
};
#endif //VS_LINUX_DEVICE
#endif
