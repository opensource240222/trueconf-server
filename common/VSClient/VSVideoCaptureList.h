/**
 **************************************************************************
 * \file VSVideoCaptureList.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief VideoCaptureList class declaration
 * \b Project Client
 * \author Melechko Ivan
 * \date 27.12.2004
 *
 * $Revision: 9 $
 *
 * $History: VSVideoCaptureList.h $
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 24.07.12   Time: 15:21
 * Updated in $/VSNA/VSClient
 * - ench direct show : were add hw h.264 description mode
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 21.10.11   Time: 19:01
 * Updated in $/VSNA/VSClient
 * - support correct SD/HQ capture on HD PTZ cameras
 * - in video proc were add ResampleCropI420_IPP based on IPP
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 23.06.11   Time: 14:23
 * Updated in $/VSNA/VSClient
 * - fix enumerate bug in directshow
 * - fix bug: av null name in capture list
 * - self view fps to 60
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 5.03.11    Time: 13:16
 * Updated in $/VSNA/VSClient
 * - improve camera initialization
 * - auto modes for camera init
 * - hardware test
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 11.01.10   Time: 14:45
 * Updated in $/VSNA/VSClient
 * - solution merge vzo7 to add unicode devices support
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 10.12.09   Time: 19:04
 * Updated in $/VSNA/VSClient
 * - unicode capability for hardware lists
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 13.09.07   Time: 15:51
 * Updated in $/VS2005/VSClient
 * - bug with device ststuses when started without devices
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 1  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:00
 * Created in $/VS/VSClient
 * some changes :)
 *
*/
#ifndef _VSVIDCAP_LIST_H
#define _VSVIDCAP_LIST_H
/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSInterface.h"
#include "VSThinkClient.h"
#include "../std/cpplib/VS_VideoLevelCaps.h"
#include <map>
#include <vector>
#include <deque>
/****************************************************************************
 * Declarations
 ****************************************************************************/
/**
 **************************************************************************
 * \brief CVideoCaptureList  retrive avaliable
 * devices
 ****************************************************************************/

struct tc_LevelModeState {
public:
	int nVideoMode;
	int nTypeHWEncoder;
	int nMBps;
	int nMBFrame;
	int nWidth;
	int nHeight;
public:
	tc_LevelModeState()
	{
		nTypeHWEncoder = ENCODER_SOFTWARE;
		nVideoMode = nMBps = nMBFrame = nWidth = nHeight = -1;
	}
	tc_LevelModeState(int typeHW)
	{
		nTypeHWEncoder = typeHW;
		nVideoMode = nMBps = nMBFrame = nWidth = nHeight = -1;
	}
	~tc_LevelModeState() {};
};

struct DeviceModeState {
public :
	CModeList							*pModeList;
	tc_LevelModeState					*pCaptureLevelList[MAX_NUM_HW_ENCODERS][VIDEO_NUMLEVELS_MAX];
	int									iStereoMode;
	int									iHWEncoderMode;
	int									iHDVideoSourceMode;
	std::wstring						pathDevice;
	int									typeCapturer;
	int									typeDevice;
	DeviceModeState() {
		iStereoMode = 0;
		iHWEncoderMode = 0;
		iHDVideoSourceMode = 0;
		typeCapturer = 0;
		pathDevice.clear();
		memset(pCaptureLevelList, 0, sizeof(pCaptureLevelList));
	}
};

typedef std::pair<std::wstring, DeviceModeState>			devlist_pair;
typedef std::map <std::wstring, DeviceModeState>::iterator	devlist_iter;

class CVideoCaptureList: public CDeviceList,public CVSInterface, public VS_Lock {
public:
	enum eCapturerType
	{
		CAPTURE_DS = 0,
		CAPTURE_MF = 1,
		CAPTURE_SCREEN,
		CAPTURE_MAX
	};
	enum eDeviceType
	{
		DEVICE_CARD	= 0,
		DEVICE_WEB	= 1,
		DEVICE_SCREEN_MON,
		DEVICE_SCREEN_APP,
		DEVICE_MAX
	};
private:
	/// Video Level Caps
	tc_VideoLevelCaps *m_pLevelCaps;
	tc_AutoLevelCaps *m_pLevelAutoCaps;
	/// optimal level in system benchmark
	unsigned char m_uOptLevel;
	/// optimal aspect ratio
	eAspectRatio m_eAspectRatio;
	/// hardware encoder type
	int m_bUseHWEncoder;
	/// sender level
	unsigned char m_sndLevel;
	/// dump data
	void DumpCaptureLevelList(DeviceModeState *pDevMode, const wchar_t *szName);
protected:
	const static char _funcVCList[];
	const static char _funcVCList2[];
	/// \todo Hack !
	bool m_bVFW[256];
	/// Monikers for DS
	std::vector<IMoniker*> m_DevMonikerList;
	/// avaliable modes for device
	std::map<std::wstring, DeviceModeState> m_DeviceModeList;
	/// second init graph
	std::wstring m_sDeviceTrouble;
	/// reject MJPEG for win 8/8.1
	std::wstring m_sDeviceFixMjpeg;
	/// reinit trouble with avermedia ec u3
	std::wstring m_sDeviceECU3;
	/// media foundation devices
	std::deque <std::wstring> m_qDeviceMF;
	///
	std::map <std::wstring, uint32_t> m_mDeviceLimit;
	/// prefix to device
	std::map <std::wstring, eDeviceType> m_Prefix2Device;
	/// device to prefix
	std::map <eDeviceType, std::wstring> m_Device2Prefix;
	/// type capturer
	eCapturerType m_typeCapturer;
	/// reset avaliable modes for devices
	void ResetModeList();
	/// generate video modes list
	void SetCaptureLevelMode(DeviceModeState *pDevMode, const wchar_t *szName);
	/// retrive avaliable Screen Capture
	int iGetScreenCaptureList();
	/// retrive avaliable devices
	int iGetDeviceList();
	/// retrive avaliable modes for device
	virtual int iGetDeviceModeList(const wchar_t *szName, const wchar_t *szDevPath) = 0;
	/// see CVSInterface
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
	/// reset Monikers
	void ResetMonikerList();
	///
	void FilterModeList(unsigned char baseLevel, unsigned char maxLevel, DeviceModeState *pDevMode, const wchar_t *szName,
					    std::multimap <int, int> & filterModes, std::multimap <int, int> & filterExtendModes,
					    std::vector <std::multimap <int, int>> & hardwareModes, std::multimap <int, int> & validModes);
	///
	virtual int iInternalGetDeviceList() = 0;
public:
	static CVideoCaptureList* Create(CVideoCaptureList::eCapturerType type, CVSInterface* pParentInterface, unsigned char sndLevel);
	static bool m_bTryUpdateDevice;
	/// init variables
	CVideoCaptureList(CVSInterface* pParentInterface, unsigned char sndLevel);
	virtual ~CVideoCaptureList();
	CModeList*			GetModeListByName(const wchar_t* szName);
	const wchar_t*		GetSymbolicLinkByName(const wchar_t* szName);
	int					GetAutoModeHD(const wchar_t* szName);
	bool				IsHWEncoderSupport(const wchar_t* szName);
	void				SetAutoLevel(unsigned char lvl, eAspectRatio aspectRatio);
	void				SetUseHardwareEncoder(bool bUseHWEnc);
	int					GetStereoMode(const wchar_t* szName);
	tc_LevelModeState	GetLevelState(const wchar_t* szName);
	bool				UpdateDeviceModeList(const wchar_t *szName);
	bool				IsAverMediaDevice(const wchar_t *szName);
	bool				UpdateSenderLevel(unsigned char lvl, const bool encoderHardware);
	CVideoCaptureList::eDeviceType		ParseDeviceName(wchar_t *szName);
	CVideoCaptureList::eCapturerType	GetTypeCapturer(const wchar_t *szName);
	CVideoCaptureList::eDeviceType		GetTypeDevice(const wchar_t *szName);
};

class VS_VideoCaptureDirectShowList : public CVideoCaptureList
{
protected:
	virtual int iGetDeviceModeList(const wchar_t *szName, const wchar_t *szDevPath);
	virtual int iInternalGetDeviceList();
	/// connect to filter
	IBaseFilter* _iConnect(int device_number);
public:
	IBaseFilter* GetCaptureByName(const wchar_t* szName);
	VS_VideoCaptureDirectShowList(CVSInterface* pParentInterface, unsigned char sndLevel);
	virtual ~VS_VideoCaptureDirectShowList();
};

struct IMFMediaSource;

class VS_VideoCaptureMediaFoundationList : public VS_VideoCaptureDirectShowList
{
private:
	HRESULT EnumerateCaptureFormats(IMFMediaSource *pSource, CModeListMediaFoundation *pModeListMediaFoundation, bool fixMjpeg, unsigned int limitMB);
protected:
	int iGetDeviceModeList(const wchar_t *szName, const wchar_t *szDevPath);
	int iInternalGetDeviceList();
public:
	VS_VideoCaptureMediaFoundationList(CVSInterface* pParentInterface, unsigned char sndLevel);
	virtual ~VS_VideoCaptureMediaFoundationList();
};

#endif