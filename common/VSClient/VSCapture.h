/**
 **************************************************************************
 * \file VSCapture.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Classes used to capture videodata from videocapture device
 *
 * \b Project Client
 * \author Melechko Ivan
 * \author SMirnovK
 * \date 07.10.2004
 *
 * $Revision: 19 $
 *
 * $History: VSCapture.h $
 *
 * *****************  Version 19  *****************
 * User: Sanufriev    Date: 24.10.11   Time: 11:15
 * Updated in $/VSNA/VSClient
 * - ench csc module for directshow
 *
 * *****************  Version 18  *****************
 * User: Sanufriev    Date: 21.10.11   Time: 19:01
 * Updated in $/VSNA/VSClient
 * - support correct SD/HQ capture on HD PTZ cameras
 * - in video proc were add ResampleCropI420_IPP based on IPP
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 27.04.11   Time: 19:14
 * Updated in $/VSNA/VSClient
 * - were added auto change media format
 * - were added info media format command
 * - wait time reduced to 1000 ms in EventManager
 * - were added new capability : dynamic change media format
 * - capture : unblock SetFps and GetFrame
 * - receivers can dynamic change media format
 * - were added auto check media format from bitstream in receivers
 * - change scheme BtrVsFPS for vpx
 * - change AviWriter for auto change media format
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 29.03.11   Time: 13:08
 * Updated in $/VSNA/VSClient
 * - update Capture module (STA implementation)
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 14.03.11   Time: 14:27
 * Updated in $/VSNA/VSClient
 * - change VS_MediaFormat - were added dwFps
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 11.03.11   Time: 12:52
 * Updated in $/VSNA/VSClient
 * - change method retrive camera propety, pins pages
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 5.03.11    Time: 16:56
 * Updated in $/VSNA/VSClient
 * - directshow fix : camera propety, pins pages
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 5.03.11    Time: 13:16
 * Updated in $/VSNA/VSClient
 * - improve camera initialization
 * - auto modes for camera init
 * - hardware test
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 13.01.11   Time: 18:55
 * Updated in $/VSNA/VSClient
 * - ench #8342 (refactoring camera states)
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 20.12.10   Time: 14:41
 * Updated in $/VSNA/VSClient
 * - were add Blend deinterlace init
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 2.12.10    Time: 16:14
 * Updated in $/VSNA/VSClient
 * - increase accuracy video capture timestamp
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
 * User: Smirnov      Date: 16.06.09   Time: 14:02
 * Updated in $/VSNA/VSClient
 * - add lock in device status query
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 12.03.09   Time: 15:28
 * Updated in $/VSNA/VSClient
 * - camera swtched of in tray
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
 * *****************  Version 35  *****************
 * User: Smirnov      Date: 24.07.06   Time: 16:56
 * Updated in $/VS/VSClient
 * - added deintrlacing algorithm
 *
 * *****************  Version 34  *****************
 * User: Smirnov      Date: 4.05.06    Time: 12:40
 * Updated in $/VS/VSClient
 * - hardware test repaired
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 3.05.06    Time: 18:09
 * Updated in $/VS/VSClient
 * - removed 1 frame latency in video capture device
 * - removed null (first) frame rendering after capture deinit
 * - InvertUv registry param moved
 *
 * *****************  Version 32  *****************
 * User: Smirnov      Date: 2.05.06    Time: 18:41
 * Updated in $/VS/VSClient
 * - sender reinitialisation
 *
 * *****************  Version 31  *****************
 * User: Smirnov      Date: 21.02.06   Time: 17:02
 * Updated in $/VS/VSClient
 * - added preferred video capture dimension flag
 *
 * *****************  Version 30  *****************
 * User: Smirnov      Date: 28.11.05   Time: 18:17
 * Updated in $/VS/VSClient
 * - scale captured image instead clip it
 *
 * *****************  Version 29  *****************
 * User: Smirnov      Date: 25.11.05   Time: 18:25
 * Updated in $/VS/VSClient
 *
 * *****************  Version 28  *****************
 * User: Smirnov      Date: 13.10.05   Time: 13:43
 * Updated in $/VS/VSClient
 * - hardware test project repaired
 *
 * *****************  Version 27  *****************
 * User: Melechko     Date: 28.02.05   Time: 11:45
 * Updated in $/VS/VSClient
 * Add audio event in SlotEx
 *
 * *****************  Version 26  *****************
 * User: Melechko     Date: 16.02.05   Time: 14:21
 * Updated in $/VS/VSClient
 * Add MJPEG support
 *
 * *****************  Version 25  *****************
 * User: Melechko     Date: 25.01.05   Time: 13:38
 * Updated in $/VS/VSClient
 * split input video buffer
 *
 * *****************  Version 24  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:02
 * Updated in $/VS/VSClient
 * some changes :)
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/
#ifndef VS_CAPTURE_H
#define VS_CAPTURE_H


/****************************************************************************
 * Includes
 ****************************************************************************/
#include <atlbase.h>
#include "VSClientBase.h"
#include "VSGrabber.h"
#include "../std/cpplib/VS_Lock.h"
#include "../std/cpplib/VS_MediaFormat.h"
#include <queue>
#include <deque>
#include <vector>
#include "../Transcoder/VideoCodec.h"
#include <ks.h>
#include <ksproxy.h>
#include <Vidcap.h>
#include <Mfidl.h>
#include <Mfreadwrite.h>
#include "UvcH264.h"
#include "std/cpplib/VS_VideoLevelCaps.h"

typedef HRESULT (WINAPI * LPMFStartup)(ULONG, DWORD);
typedef HRESULT (WINAPI * LPMFShutdown)(void);
typedef HRESULT (WINAPI * LPMFEnumDeviceSources)(IMFAttributes *, IMFActivate ***, UINT32 *);
typedef HRESULT (WINAPI * LPMFCreateAttributes)(IMFAttributes **, UINT32);
typedef HRESULT (WINAPI * LPMFCreateDeviceSource)(IMFAttributes *, IMFMediaSource **);
typedef HRESULT (WINAPI * LPMFPutWorkItem)(DWORD, IMFAsyncCallback  *, IUnknown *);
typedef HRESULT (WINAPI * LPMFCreateAsyncResult)(IUnknown *, IMFAsyncCallback *, IUnknown *, IMFAsyncResult **);
typedef HRESULT (WINAPI * LPMFCreateSourceReaderFromMediaSource)(IMFMediaSource *, IMFAttributes *, IMFSourceReader **);
typedef HRESULT (WINAPI * LPMFCreateMemoryBuffer)(DWORD, IMFMediaBuffer **);
typedef HRESULT (WINAPI * LPMFCreateMediaType)(IMFMediaType **);
typedef HRESULT (WINAPI * LPMFInvokeCallback)(IMFAsyncResult *);
typedef HRESULT (WINAPI * LPMFAllocateSerialWorkQueue)(DWORD, DWORD  *);
typedef HRESULT (WINAPI * LPMFUnlockWorkQueue)(DWORD);

extern HINSTANCE								g_hMf;
extern HINSTANCE								g_hMfplat;
extern HINSTANCE								g_hMfreadwrite;
extern HINSTANCE								g_hMfuuid;

extern LPMFStartup								g_MFStartup;
extern LPMFShutdown								g_MFShutdown;
extern LPMFEnumDeviceSources					g_MFEnumDeviceSources;
extern LPMFCreateAttributes						g_MFCreateAttributes;
extern LPMFCreateDeviceSource					g_MFCreateDeviceSource;
extern LPMFPutWorkItem							g_MFPutWorkItem;
extern LPMFCreateAsyncResult					g_MFCreateAsyncResult;
extern LPMFCreateSourceReaderFromMediaSource	g_MFCreateSourceReaderFromMediaSource;
extern LPMFCreateMemoryBuffer					g_MFCreateMemoryBuffer;
extern LPMFCreateMediaType						g_MFCreateMediaType;
extern LPMFInvokeCallback						g_MFInvokeCallback;
extern LPMFAllocateSerialWorkQueue				g_MFAllocateSerialWorkQueue;
extern LPMFUnlockWorkQueue						g_MFUnlockWorkQueue;

#ifndef VS_LINUX_DEVICE
//undefine if compile for wine
//#define VS_LINUX_DEVICE
#endif

class CSharedMemorySlave;
class VS_MediaFormat;
class CVideoCaptureList;
class VS_VideoCaptureDirectShowList;
class VS_VideoProc;
class VS_Deinterlacing;
class ScreenCapturer;

enum eStateCaptureDevice
{
	DEVICE_SHUTDOWN	= 0,
	DEVICE_INIT		= 1,
	DEVICE_BUILD,
	DEVICE_START,
};

enum eDeviceAction
{
	DEVICE_UNDEF		= 0,
	DEVICE_DISCONNECT	= 1,
	DEVICE_CONNECT,
	DEVICE_CHANGEFORMAT,
	DEVICE_STARTUP,
	DEVICE_SHOOTDOWN,
	DEVICE_CHANGEFPS,
	DEVICE_SLEEP,
	DEVICE_DESTROY,
	DEVICE_PROPERTY,
	DEVICE_ACTMAX
};

enum eHardwareRequest
{
	HARDWARE_UNDEF		= 0x00000000,
	HARDWARE_SETBITRATE = 0x00000001,
	HARDWARE_FRAMERATE	= 0x00000004,
	HARDWARE_NEEDKEY	= 0x00000008,
	HARDWARE_RESET		= 0x00000010,
};

typedef struct {
	WORD      dlgVer;
	WORD      signature;
	DWORD     helpID;
	DWORD     exStyle;
	DWORD     style;
	WORD      cDlgItems;
	short     x;
	short     y;
	short     cx;
	short     cy;
} DLGTEMPLATEEX;

/****************************************************************************
 * Classes
 ****************************************************************************/

/**
 **************************************************************************
 * \brief Convrt clolor mode of video data recieved from camera to I420
 * \brief Deintarlace it
 * \brief Adjust size of image (pan/scan)
 ****************************************************************************/
class CCaptureCConv
{
	enum eCaptureConvMode {
		NOT_SRC_INPUT		= 0x00000001,
		CSC_INPUT			= 0x00000002,
		DEINTERLACE			= 0x00000004,
		CROP_OUTPUT			= 0x00000008,
		RESAMPLE_OUTPUT		= 0x00000010,
		AR_OUTPUT			= 0x00000020,
		IDLE_OUTPUT			= 0x00000040,
		STEREO_UD_INPUT		= 0x00000080,
		STEREO_UD_OUTPUT	= 0x00000100,
		COMPRESS_INPUT		= 0x00000200,
		AR_IN_OUTPUT		= 0x00000400,
	};
	enum eCaptureConvState {
		SET_INPUT			= 0x00000001,
		SET_OUTPUT			= 0x00000002,
		SET_CHECK			= 0x00000004,
		SET_INIT			= 0x00000007
	};
	VideoCodec*			m_pVideoDecoder;
	unsigned char*		m_pDeint;
	unsigned char*		m_pConv;
	unsigned char*		m_pClip;
	unsigned char*		m_pClipRsmpl;
	unsigned char*		m_pCaptureFrame;
	unsigned char*		m_pOut;
	unsigned char*		m_pOutCompress;
	unsigned char		*m_pSrcPlane[3];
	unsigned char		*m_pDstPlaneU[3], *m_pDstPlaneD[3];
	unsigned char		*m_pOutPlaneU[3], *m_pOutPlaneD[3];
	unsigned char		*m_pClipRsmplUD[2];
	BITMAPINFOHEADER	m_bihIn;
	BITMAPINFOHEADER	m_bihOut;
	VS_VideoProc*		m_proc;
	VS_Deinterlacing*	m_DeInt;
	unsigned int		m_Inited;
	int					m_CrpType;
	int					m_inHeight, m_outHeight;
	int					m_outWidthAR, m_outHeightAR;
	int					m_iOffsetW, m_iOffsetH;
	double				m_fFactorW, m_fFactorH;
	int					m_rMode;
	unsigned int		m_dCaptureFlags;
public:
	CCaptureCConv();
	~CCaptureCConv();
	bool SetInFormat(LPBITMAPINFOHEADER pbih, unsigned char* pCaptureFrame = NULL, int deint = 0);
	bool SetOutFormat(CColorMode &cm, int CrpType = 5);
	bool ResetInFormat(LPBITMAPINFOHEADER pbih, unsigned char* pCaptureFrame = NULL, int deint = 0);
	bool ResetOutFormat(CColorMode &cm, int CrpType = 5);
	bool CheckFormats();
	bool ConvertInput(unsigned char *src, int size);
	unsigned int GetOutput(unsigned char* &frame, unsigned char* &compress);
};

struct VS_CaptureDeviceState
{
	wchar_t			cDeviceName[MAX_PATH];
	eDeviceAction	eAction;
	VS_MediaFormat	mf;
	int				iOptimalMode;
	int				iSetFps;
	VS_CaptureDeviceState()
	{
		cDeviceName[0] = 0;
		eAction = DEVICE_UNDEF;
		mf.SetZero();
		iOptimalMode = -1;
		iSetFps = 15;
	}
	void FillState(wchar_t *cName, eDeviceAction act, VS_MediaFormat *fmt, int mode)
	{
		eAction = act;
		if (fmt) mf = *fmt;
		iOptimalMode = mode;
		if (cName) wcscpy(cDeviceName, cName);
	}
};

struct VS_HardwareDSEncoderState
{
	IKsControl							*pKsControl;
	GUID								extensionGuid;
	int									iNode;
	_uvcx_video_config_probe_commit_t	h264State;
	unsigned int						bitrate;
	bool								is_first_frame;
	void ResetState(int width, int height, unsigned int framerate)
	{
		h264State.wProfile = 0x4200;
		h264State.wWidth = width;
		h264State.wHeight = height;
		h264State.wSliceMode = SLICEMODE_BITSPERSLICE;
		h264State.wSliceUnits = 1200 * 8;
		h264State.dwFrameInterval = (int)(10000000.0 / (double)framerate);
		h264State.wIFramePeriod = 15 * 1000;
		h264State.bRateControlMode = RATECONTROL_CBR;
		h264State.bEntropyCABAC = ENTROPY_CAVLC;
		h264State.bUsageType = 1;
		is_first_frame = true;
	}
};

struct VS_HardwareMFEncoderState
{
	unsigned int bitrate;
	int			 framerate;
};

class VS_HardwareEncoderObserver
{
public :
	virtual void SetHWEncoderRequest(eHardwareRequest request, unsigned int iVal = 0) {};
};

class VS_VideoCaptureSlotObserver
{
public :
	virtual void PushFrame(unsigned char *pBuffer, int size, int realFramerate, unsigned int timestamp, int width, int height, unsigned int color, bool hardware) = 0;
};

struct VS_CaptureDeviceSettings
{
	long	iChannel;
	long	iVideoMode;
	int		iDeinterlace;
	int		iCheckHDInput;
	int		iFixNTSC;
	HWND	hwndProp;
};

class VS_CaptureDevice : public VS_Lock
{

protected :

	static VS_HardwareEncoderObserver	*m_pHardwareObserver;
	VS_VideoCaptureSlotObserver			*m_pVideoSlotObserver;

	wchar_t					m_CurrentDeviceName[MAX_PATH];
	CVideoCaptureList		*m_pCaptureList;
	VS_Lock					m_lockFramerate;
	VS_MediaFormat			m_captureFmt;
	VS_MediaFormat			m_renderFmt;
	VS_CaptureDeviceState	m_devState;
	HWND					m_hwndPropPage;
	HANDLE					m_hUpdateState;
	HANDLE					m_hGetPropertyPage;
	eStateCaptureDevice		m_eDeviceState;
	int						m_nOptimalVideoMode;
	int						m_realFramerate;
	int						m_startFramerate;
	int						m_setFramerate, m_last_setFramerate;
	std::queue<unsigned int>	m_qFrameTimestamp;
	__int64					m_lastTimestamp;
	__int64					m_accTimestamp;
	std::queue <VS_CaptureDeviceState> m_qDevState;

public :

	CModeList				*m_pModeList;

	enum eCaptureType
	{
		DIRECT_SHOW,
		MEDIA_FOUNDATION,
		SCREEN_CAPTURE
	};

	const static char _funcConnect[];
	const static char _funcDisconnect[];
	const static char _funcControl[];
	const static char _funcRealFramerate[];
	const static char _funcCaptureFramerate[];
	const static char _funcPropertyPage[];
	const static char _funcPins[];
	const static char _funcVideoMode[];
	const static char _funcCurrentName[];

protected :

	VS_CaptureDevice::eCaptureType m_typeDevice;

protected :

	void CleanFramerate();
	bool SnapFramerate(unsigned int ctime, __int64 timestamp, int baseFramerate);
	bool GetFramerate(unsigned int ctime, int &realFramerate);
	int FindOptimalVideoMode(VS_MediaFormat renderFmt, int iCheckHDInput);
	virtual bool GetPropertyPage() { return false; }

public :

	static void Open();
	static void Close();
	static bool IsMediaFoundationSupported();
	static VS_CaptureDevice* Create(VS_CaptureDevice::eCaptureType type, VS_VideoCaptureSlotObserver *observerSlot, CVideoCaptureList *pCaptureList);
	static VS_HardwareEncoderObserver* GetHardwareObserver();

	VS_CaptureDevice(VS_VideoCaptureSlotObserver *observerSlot, CVideoCaptureList *pCaptureList = NULL);
	virtual ~VS_CaptureDevice();
	virtual int Connect(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, VS_CaptureDeviceSettings *devSettings);
	virtual int Disconnect();
	virtual void Sleep(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, bool state) {};
	/// framerate
	virtual void StartCaptureFramerate(int framerate);
	virtual void SetCaptureFramerate(int framerate);
	int GetCaptureFramerate();
	int GetRealFramerate();
	/// property page
	virtual bool IsPropertyPage() { return false; }
	bool RunPropertyPage(HWND hwndPage);
	/// pins
	virtual long GetPins(_variant_t *var) { return -1; }
	virtual bool SetPin(long nPin) { return false; }
	/// video mode
	virtual __int64 GetVideoModes() { return -1; }
	virtual bool SetVideoMode(long mode) { return false; }
};

class VS_CaptureDeviceDirectShow : public VS_CaptureDevice, CVSThread, VS_HardwareEncoderObserver
{
	VS_Lock							m_lockCapture;
	VS_CaptureDeviceSettings		m_devSettings;
	std::deque<std::string>			m_qCrossbarPins;
	int								m_CrossbarPinsReal[128];
	int								m_CrossbarPinsGUI[128];
	///
	VS_HardwareDSEncoderState		m_stHardwareEncoder;
	/// DS interfaces
	IGraphBuilder*					m_pGraph;
	ICaptureGraphBuilder2*			m_pBuilder;
	IBaseFilter*					m_pCaptureFilter;
	IBaseFilter*					m_pGrabberFilter;
	IBaseFilter*					m_pMixerVideoPortFilter;
	IBaseFilter*					m_pVideoDecoderFilter;
	IAMAnalogVideoDecoder*			m_pAnalogVideoDecoder;
	IAMCrossbar*					m_pCrossbar;
	IAMTVTuner*						m_pTVTuner;
	ISpecifyPropertyPages*			m_pPropertyPages;
	IMediaControl*					m_pMediaControl;
	CVSGrabber*						m_pGrabber;
	///
	HANDLE							m_hMutexGetParams;
	HANDLE							m_hWasInitGraph;
	/// Hardware
	HANDLE							m_hSetBitrateHW;
	HANDLE							m_hNeedIFrameHW;
	HANDLE							m_hResetHW;

private :

	int			ConnectGraph();
	int			InitGraph();
	void		ShutdownGraph();
	int			BuildGraph();
	void		TearDownGraph();
	HRESULT		StartGraph();
	HRESULT		StopGraph();
	int			FindOutPin();
	void		CheckCrossbarPins();
	int			iGetCaptureFormat(VS_MediaFormat renderFmt, int iCheckHDInput);
	/// h.264 XU
	HRESULT FindExtensionNode(IKsTopologyInfo* pKsTopologyInfo, GUID extensionGuid, VS_HardwareDSEncoderState *pHWState);
	HRESULT GetHWEncoderState(VS_HardwareDSEncoderState *pHWState);
	HRESULT SetHWEncoderState(VS_HardwareDSEncoderState *pHWState);
	void UpdateHWEncoderState(eHardwareRequest request, unsigned int iVal = 0);
	/// device settings
	bool GetPropertyPage();

public :

	VS_CaptureDeviceDirectShow(VS_VideoCaptureSlotObserver *observerSlot, CVideoCaptureList *pCaptureList);
	virtual ~VS_CaptureDeviceDirectShow();
	DWORD Loop(LPVOID lpParameter);
	int Connect(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, VS_CaptureDeviceSettings *devSettings);
	int Disconnect();
	void Sleep(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, bool state);
	int	iGetDeviceModeList(const wchar_t *szName);
	/// device settings
	bool IsPropertyPage();
	long GetPins(_variant_t *var);
	bool SetPin(long nPin);
	__int64 GetVideoModes();
	bool SetVideoMode(long mode);
	/// hardware
	void SetHWEncoderRequest(eHardwareRequest request, unsigned int iVal = 0);
};

class VS_CaptureDeviceScreen : public VS_CaptureDevice, CVSThread
{
public :

	struct PropertyApplication
	{
		HWND hWnd;
		CHAR nameApp[256];
		CHAR nameClass[256];
	};

private :

	VS_Lock								m_lockProperty;
	VS_Lock								m_lockApp;
	ScreenCapturer						*m_pDesktopCapture;
	unsigned char						*m_pBuffer;
	int									m_iSampleSize;
	HANDLE								m_hTtreadProperty;
	HANDLE								m_hUpdateApp;
	std::vector <PropertyApplication>	m_listApp;

public :

	const static wchar_t _nameScreenCapture[];
	const static wchar_t _nameApplicationCapture[];

private :

	int InternalConnect(wchar_t *deviceName);
	int InternalDisconnect(wchar_t *deviceName);
	int ReinitApplication(int width, int height);
	int UpdateCaptureState();
	bool GetPropertyPage();
	int InternalPropertyPage();

public :

	VS_CaptureDeviceScreen(VS_VideoCaptureSlotObserver *observerSlot);
	virtual ~VS_CaptureDeviceScreen();
	bool IsPropertyPage() { return true; }
	HWND GetHwndPropertyPage() { return m_hwndPropPage; }
	int InternalClosePropertyPage();
	bool EnumerateApp();
	int GetNumActiveApp();
	bool SetAppProperty(PropertyApplication *prop);
	bool GetAppProperty(int index, PropertyApplication *prop);
	void SetCaptureApp(int index);
	void Sleep(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, bool state);
	DWORD Loop(LPVOID lpParameter);
};

class VS_HardwareEncoder
{
public :
	VS_HardwareEncoder() {};
	virtual ~VS_HardwareEncoder() {};
	virtual void SetControl(void *pControl, int set_val) = 0;
	virtual void SetBitrate(int val) = 0;
	virtual void SetFramerate(int val) = 0;
	virtual void SetKeyFrame() = 0;
};

class VS_HardwareEncoderDS : public VS_HardwareEncoder
{
	GUID m_gExtensionGuid;
	IKsControl *m_pKsControl;
	int m_iNode;
public :
	VS_HardwareEncoderDS() : m_pKsControl(0) {};
	~VS_HardwareEncoderDS() {};
	void SetControl(void *pControl, int set_val);
	void SetBitrate(int val);
	void SetFramerate(int val);
	void SetKeyFrame();
};

class VS_HardwareEncoderMF : public VS_HardwareEncoder
{
	ICodecAPI *m_pCodecApi;
public :
	VS_HardwareEncoderMF() : m_pCodecApi(0) {};
	~VS_HardwareEncoderMF() {};
	void SetControl(void *pControl, int set_val);
	void SetBitrate(int val);
	void SetFramerate(int val);
	void SetKeyFrame();
};

class VS_CaptureDeviceMediaFoundation : public VS_CaptureDevice,
										public IMFAsyncCallback,
										public IMFSourceReaderCallback,
										public VS_HardwareEncoderObserver
{
public :

	enum {
		BRIGHTNESS = 0,
		CONTRAST = 1,
		HUE,
		SATURATION,
		SHARPNESS,
		GAMMA,
		COLORENABLE,
		WHITEBALANCE,
		BACKLIGHTCOMP,
		GAIN,
		POWERLINEFREQ,
		FOCUS,
		EXPOSURE,
		LOWLIGHTCOMP,
		PAN,
		TILT,
		ZOOM,
		MAXTYPES
	} PropertyType;

	struct PropertyDevice
	{
		bool valid;
		long val;
		long flag;
		long min_val;
		long max_val;
		long step;
		long default_val;
		long flag_caps;
	public :
		PropertyDevice() : val(0), min_val(0), max_val(0), step(0), default_val(0)
		{
			valid = false;
			flag = VideoProcAmp_Flags_Manual;
			flag_caps = VideoProcAmp_Flags_Manual;
		}
		~PropertyDevice() {};
	};

private :

	VS_Lock						m_hardwareLock;
	VS_Lock						m_lockCapture;
	VS_Lock						m_lockProperty;
	IMFMediaSource				*m_pSource;
	IMFSourceReader				*m_pReader;
	IMFPresentationDescriptor	*m_pPresentation;
	IMFMediaBuffer				*m_pMediaBuffer;
	ICodecAPI					*m_pCodecApi;
	long						m_cRef;
	DWORD						m_dwWorkQueue;
	int							m_idActiveStream;
    bool						m_bFirstSample;
    __int64						m_llBaseTime;
	unsigned char				*m_pBuffer;
	unsigned int				m_lastKeyRequest;
	eHardwareEncoder			m_eTypeHardware;
	DWORD						m_hardwareRequest;
	VS_HardwareMFEncoderState	m_stHardwareEncoder;
	VS_HardwareEncoder			*m_pHardwareEncoder;
	HANDLE						m_hDestroy;
	HANDLE						m_hTtreadProperty;
	PropertyDevice				m_propertyDev[MAXTYPES];

private :

	int InternalConnect(wchar_t *deviceName, VS_MediaFormat *mf, int deviceMode);
	int InternalDisconnect(wchar_t *deviceName);
	int InternalPropertyPage();
	int InternalDestroy();
	HRESULT BuildSinkSource();
	HRESULT StartSinkSource();
	HRESULT TrySourceProperty(GUID guid, int val);
	HRESULT ConfigureHardwareSinkSource(GUID guid_subtype);
	HRESULT SetPropertySource(IUnknown *pUnknown, long devProperty);
	HRESULT GetPropertySource(IUnknown *pUnknown, long devProperty);
	HRESULT ConfigurePropertySource();
	HRESULT ConBuildSinkSource();
	HRESULT ConfigureCurrentType(IMFMediaType *pMediaType, CColorModeDescription *cmd, GUID guid_subtype);
	HRESULT OpenMediaSource(IMFMediaSource *pSource, GUID guid_subtype);
	HRESULT FindExtensionNode(IKsTopologyInfo* pKsTopologyInfo, GUID extensionGuid);
	int CreateAsyncResult(VS_CaptureDeviceState devState);
	bool GetPropertyPage();

public :

	VS_CaptureDeviceMediaFoundation(VS_VideoCaptureSlotObserver *observerSlot, CVideoCaptureList *pCaptureList);
	virtual ~VS_CaptureDeviceMediaFoundation();
	int Connect(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, VS_CaptureDeviceSettings *devSettings);
	int Disconnect();
	void Sleep(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, bool state);
	bool IsPropertyPage() { return true; }
	int InternalClosePropertyPage();
	HWND GetHwndPropertyPage() { return m_hwndPropPage; }
	void GetPropertyState(PropertyDevice devProperty[]);
	void SetPropertyState(PropertyDevice devProperty[]);
	/// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	/// IMFAsyncCallback
	STDMETHODIMP GetParameters(DWORD* pdwFlags, DWORD* pdwQueue) { return E_NOTIMPL; }
	STDMETHODIMP Invoke(IMFAsyncResult* pResult);
    /// IMFSourceReaderCallback methods
    STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample);
    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *) { return S_OK; }
    STDMETHODIMP OnFlush(DWORD) { return S_OK; }
	/// hardware
	void SetHWEncoderRequest(eHardwareRequest request, unsigned int iVal = 0);
};

#ifdef VS_LINUX_DEVICE
class CCaptureVideoSourceV4L : public CVSInterface, VS_Lock
{
private:
public:
	CCaptureVideoSourceV4L(CVSInterface *pParentInterface,char *szName,CVideoCaptureList *pCaptureList);
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
};
#endif //VS_LINUX_DEVICE

#endif