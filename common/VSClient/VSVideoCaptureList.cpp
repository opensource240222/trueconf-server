/**
 **************************************************************************
 * \file VSVideoCaptureList.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief VideoCaptureList class definition
 * \b Project Client
 * \author Melechko Ivan
 * \date 27.12.2004
 *
 * $Revision: 18 $
 *
 * $History: VSVideoCaptureList.cpp $
 *
 * *****************  Version 18  *****************
 * User: Sanufriev    Date: 24.07.12   Time: 15:21
 * Updated in $/VSNA/VSClient
 * - ench direct show : were add hw h.264 description mode
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 15.03.12   Time: 10:49
 * Updated in $/VSNA/VSClient
 * - define max restrict video mode
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 18.11.11   Time: 15:18
 * Updated in $/VSNA/VSClient
 * - disable detect FrameRate List for video devices
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 2.11.11    Time: 14:12
 * Updated in $/VSNA/VSClient
 * - were added capability for capture devices : framerate list &
 * interlaced flag
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 21.10.11   Time: 19:01
 * Updated in $/VSNA/VSClient
 * - support correct SD/HQ capture on HD PTZ cameras
 * - in video proc were add ResampleCropI420_IPP based on IPP
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 25.07.11   Time: 13:57
 * Updated in $/VSNA/VSClient
 * - fix auto mode
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 22.07.11   Time: 16:10
 * Updated in $/VSNA/VSClient
 * - fix capture list
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 23.06.11   Time: 14:23
 * Updated in $/VSNA/VSClient
 * - fix enumerate bug in directshow
 * - fix bug: av null name in capture list
 * - self view fps to 60
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 14.03.11   Time: 12:13
 * Updated in $/VSNA/VSClient
 * - fix crash on vp83 (test video modes on stereo camera)
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 5.03.11    Time: 13:16
 * Updated in $/VSNA/VSClient
 * - improve camera initialization
 * - auto modes for camera init
 * - hardware test
 *
 * *****************  Version 7  *****************
 * User: Dront78      Date: 11.01.10   Time: 14:45
 * Updated in $/VSNA/VSClient
 * - solution merge vzo7 to add unicode devices support
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 10.12.09   Time: 19:04
 * Updated in $/VSNA/VSClient
 * - unicode capability for hardware lists
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 28.10.09   Time: 13:57
 * Updated in $/VSNA/VSClient
 * - directx dshow headers changed
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 3.04.09    Time: 17:58
 * Updated in $/VSNA/VSClient
 * - mark as VFW not working filters (cameras)
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 20.03.08   Time: 17:03
 * Updated in $/VSNA/VSClient
 * - Video for Linux I420 support added via memory mapped files.
 * Compilation controls via #define VS_LINUX_DEVICE in VSCapture.h
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 13.09.07   Time: 15:51
 * Updated in $/VS2005/VSClient
 * - bug with device ststuses when started without devices
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
 * *****************  Version 2  *****************
 * User: Melechko     Date: 25.01.05   Time: 16:47
 * Updated in $/VS/VSClient
 * Device list array HighBound fix
 *
 * *****************  Version 1  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:00
 * Created in $/VS/VSClient
 * some changes :)
 *
*/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSVideoCaptureList.h"
#include "VSCapture.h"
#include "ScreenCapturerFactory.h"
#include "../std/cpplib/VS_Protocol.h"
#include "../std/cpplib/VS_RegistryKey.h"
/****************************************************************************
* Static
****************************************************************************/

bool CVideoCaptureList::m_bTryUpdateDevice = true;
const char CVideoCaptureList::_funcVCList[] = "VideoCaptureList";
const char CVideoCaptureList::_funcVCList2[] = "VideoCaptureList2";

/****************************************************************************
* CVideoCaptureList
****************************************************************************/

/**********************************/
bool IsWindows8OrGreater();

CVideoCaptureList* CVideoCaptureList::Create(CVideoCaptureList::eCapturerType type, CVSInterface* pParentInterface, unsigned char sndLevel)
{
	switch (type)
	{
	case CVideoCaptureList::CAPTURE_MF:
		{
			if (IsWindows8OrGreater() && VS_CaptureDevice::IsMediaFoundationSupported())
				return new VS_VideoCaptureMediaFoundationList(pParentInterface, sndLevel);
			else
				return new VS_VideoCaptureDirectShowList(pParentInterface, sndLevel);
		}
	case CVideoCaptureList::CAPTURE_DS:
		{
			return new VS_VideoCaptureDirectShowList(pParentInterface, sndLevel);
		}
	}
	return 0;
}

/**********************************/
CVideoCaptureList::CVideoCaptureList(CVSInterface* pParentInterface, unsigned char sndLevel) :
CVSInterface("VideoCaptureDevices",pParentInterface)
{
	m_uOptLevel = 0;
	m_eAspectRatio = VS_VIDEOAR_4_3;
	m_bUseHWEncoder = false;
	m_DeviceModeList.clear();
	VS_RegistryKey key(true, "Client\\VideoCaptureSlot");
	m_sDeviceTrouble = L"usb#vid_07ca&pid_c039"; /// avermedia c039
	//m_sDeviceFixMjpeg = L"usb#vid_046d&pid_082d"; /// logitech c920
	m_sDeviceFixMjpeg = L"usb#vid_046d&pid_"; /// logitech vendor
	m_sDeviceECU3 = L"usb#vid_07ca&pid_c877"; /// avermedia ec u3
	m_qDeviceMF.clear();
	m_mDeviceLimit.insert(std::pair<std::wstring, uint32_t>(L"usb#vid_046d&pid_0823", 3600)); /// max 720p for b910
	for (int i = 0; i < CVideoCaptureList::DEVICE_MAX; i++) {
		wchar_t prefix[32];
		wsprintfW(prefix, L"@%d ", i);
		m_Prefix2Device.insert(std::pair<std::wstring, eDeviceType>(prefix, (eDeviceType)i));
		m_Device2Prefix.insert(std::pair<eDeviceType, std::wstring>((eDeviceType)i, prefix));
	}
	m_typeCapturer = CVideoCaptureList::CAPTURE_DS;
	m_pLevelCaps = new tc_VideoLevelCaps();
	m_pLevelAutoCaps = new tc_AutoLevelCaps();
	m_sndLevel = sndLevel;
}

/**********************************/
CVideoCaptureList::~CVideoCaptureList()
{
	ResetMonikerList();
	ResetModeList();
	delete m_pLevelCaps;
	delete m_pLevelAutoCaps;
}

/**********************************/
void CVideoCaptureList::ResetMonikerList()
{
	unsigned int i;
	for (i = 0; i < m_DevMonikerList.size(); i++) {
		if (m_DevMonikerList[i]) {
			m_DevMonikerList[i]->Release();
			m_DevMonikerList[i] = NULL;
		}
	}
	m_DevMonikerList.clear();
}

/**********************************/
void CVideoCaptureList::ResetModeList() {
	for (devlist_iter i = m_DeviceModeList.begin(), e = m_DeviceModeList.end(); i != e;) {
		delete (i->second.pModeList);
		for (int j = 0; j < MAX_NUM_HW_ENCODERS; j++) {
			for (int k = 0; k < VIDEO_NUMLEVELS_MAX; k++) {
				delete [] i->second.pCaptureLevelList[j][k];
			}
		}
		i++;
	}
	m_DeviceModeList.clear();
}

/**********************************/
int CVideoCaptureList::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar){
	_variant_t* var=(_variant_t*)pVar;
	if (strncmp(pSection,_funcVCList,sizeof(_funcVCList)) == 0) {
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			{
				long i;
				long n=0;
				SAFEARRAYBOUND rgsabound[1];
				SAFEARRAY * psa;
				rgsabound[0].lLbound = 0;
				n=m_pSourceList->iGetMaxString();
				if(n<0)
					n=0;
				rgsabound[0].cElements = n;
				psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
				if(psa == NULL)
					return VS_INTERFACE_INTERNAL_ERROR;
				var->parray=psa;
				var->vt= VT_ARRAY | VT_VARIANT;
				n=0;
				for(i=0;i<m_pSourceList->iGetMaxString();i++){
					_variant_t var_;
					if (!m_bVFW[i]) {
						var_=m_pSourceList->szGetStringByNumber(i);
						CVideoCaptureList::eDeviceType devType = GetTypeDevice((_bstr_t)var_);
						if (devType != CVideoCaptureList::DEVICE_SCREEN_MON && devType != CVideoCaptureList::DEVICE_SCREEN_APP) {
							SafeArrayPutElement(psa, &n, &var_.GetVARIANT());
							n++;
						}
					}
				}
				rgsabound[0].lLbound = 0;
				rgsabound[0].cElements = n;
				SafeArrayRedim(psa,rgsabound);
				return VS_INTERFACE_OK;
			}
		case RUN_COMMAND:
			int ret = iGetDeviceList();
			g_DevStatus.SetStatus(DVS_SND_NOTPRESENT, false, ret!=0);
			return ret!=0 ? VS_INTERFACE_INTERNAL_ERROR : VS_INTERFACE_OK;
		}
	} else if (strncmp(pSection,_funcVCList2,sizeof(_funcVCList2)) == 0) {
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			{
				long i;
				long n=0;
				SAFEARRAYBOUND rgsabound[1];
				SAFEARRAY * psa;
				rgsabound[0].lLbound = 0;
				n=m_pSourceList->iGetMaxString();
				if(n<0)
					n=0;
				rgsabound[0].cElements = n;
				psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
				if(psa == NULL)
					return VS_INTERFACE_INTERNAL_ERROR;
				var->parray=psa;
				var->vt= VT_ARRAY | VT_VARIANT;
				n=0;
				for(i=0;i<m_pSourceList->iGetMaxString();i++){
					_variant_t var_;
					if (!m_bVFW[i]) {
						var_=m_pSourceList->szGetStringByNumber(i);
						CVideoCaptureList::eDeviceType devType = GetTypeDevice((_bstr_t)var_);
						std::map<eDeviceType, std::wstring>::iterator it = m_Device2Prefix.find(devType);
						if (it != m_Device2Prefix.end()) {
							std::wstring name((_bstr_t)var_);
							name.insert(0, it->second);
							var_.Clear();
							var_ = name.c_str();
						}
						SafeArrayPutElement(psa, &n, &var_.GetVARIANT());
						n++;
					}
				}
				rgsabound[0].lLbound = 0;
				rgsabound[0].cElements = n;
				SafeArrayRedim(psa,rgsabound);
				return VS_INTERFACE_OK;
			}
		case RUN_COMMAND:
			int ret = iGetDeviceList();
			g_DevStatus.SetStatus(DVS_SND_NOTPRESENT, false, ret!=0);
			return ret!=0 ? VS_INTERFACE_INTERNAL_ERROR : VS_INTERFACE_OK;
		}
	}
	return VS_INTERFACE_NO_FUNCTION;
}

bool CVideoCaptureList::UpdateDeviceModeList(const wchar_t *szName)
{
	bool bUpdate = false;
	if (m_bTryUpdateDevice) {
		devlist_iter i = m_DeviceModeList.find(szName);
		if (i == m_DeviceModeList.end()) return false;
		std::wstring devPath = i->second.pathDevice;
		if (wcsstr(devPath.c_str(), m_sDeviceTrouble.c_str()) != 0) {
			delete (i->second.pModeList);
			for (int j = 0; j < MAX_NUM_HW_ENCODERS; j++) {
				for (int k = 0; k < VIDEO_NUMLEVELS_MAX; k++) {
					delete [] i->second.pCaptureLevelList[j][k];
				}
			}
			m_DeviceModeList.erase(i);
			iGetDeviceModeList(szName, devPath.c_str());
			bUpdate = true;
			m_bTryUpdateDevice = false;
		}
	}
	return bUpdate;
}

bool CVideoCaptureList::IsAverMediaDevice(const wchar_t *szName)
{
	devlist_iter i = m_DeviceModeList.find(szName);
	if (i == m_DeviceModeList.end()) return false;
	std::wstring devPath = i->second.pathDevice;
	if (wcsstr(devPath.c_str(), m_sDeviceECU3.c_str()) != 0) {
		return true;
	}
	return false;
}

bool CVideoCaptureList::UpdateSenderLevel(unsigned char lvl, const bool encoderHardware)
{
	VS_AutoLock lock(this);
	if (lvl >= VIDEO_LEVEL_MAX && m_sndLevel >= VIDEO_LEVEL_MAX) {
		return false;
	}
	m_sndLevel = lvl;
	if (m_sndLevel > VIDEO_LEVEL_MAX) {
		m_sndLevel = VIDEO_LEVEL_MAX;
	}
	if (encoderHardware) {
		if (m_sndLevel < VIDEO_HWLEVEL_MIN) {
			m_sndLevel = VIDEO_HWLEVEL_MIN;
		}
		else {
			m_sndLevel = VIDEO_LEVEL_MAX;
		}
	}
	for (auto it : m_DeviceModeList) {
		SetCaptureLevelMode(&(it.second), it.first.c_str());
	}
	return true;
}

int CVideoCaptureList::iGetDeviceList()
{
	VS_AutoLock lock(this);
	return iInternalGetDeviceList();
}

CVideoCaptureList::eDeviceType CVideoCaptureList::ParseDeviceName(wchar_t *szName)
{
	std::wstring prefix(szName, 3);
	std::wstring name(szName);
	std::map<std::wstring,eDeviceType>::iterator it = m_Prefix2Device.find(prefix);
	if (it != m_Prefix2Device.end()) {
		name.erase(0, 3);
		wcscpy(szName, name.c_str());
		return (CVideoCaptureList::eDeviceType)it->second;
	}
	return CVideoCaptureList::DEVICE_WEB;
}

CVideoCaptureList::eCapturerType CVideoCaptureList::GetTypeCapturer(const wchar_t *szName)
{
	devlist_iter i = m_DeviceModeList.find(szName);
	if (i == m_DeviceModeList.end()) return CVideoCaptureList::CAPTURE_MF;
	return (CVideoCaptureList::eCapturerType)i->second.typeCapturer;
}

CVideoCaptureList::eDeviceType CVideoCaptureList::GetTypeDevice(const wchar_t *szName)
{
	devlist_iter i = m_DeviceModeList.find(szName);
	if (i == m_DeviceModeList.end()) return CVideoCaptureList::DEVICE_WEB;
	return (CVideoCaptureList::eDeviceType)i->second.typeDevice;
}

/**********************************/
CModeList* CVideoCaptureList::GetModeListByName(const wchar_t* szName)
{
	CModeList *pMode = NULL;
	devlist_iter iter = m_DeviceModeList.find(szName);
	if (iter != m_DeviceModeList.end()) {
		pMode = iter->second.pModeList;
	}
	return pMode;
}

/**********************************/
const wchar_t* CVideoCaptureList::GetSymbolicLinkByName(const wchar_t* szName)
{
	devlist_iter iter = m_DeviceModeList.find(szName);
	if (iter != m_DeviceModeList.end()) {
		return iter->second.pathDevice.c_str();
	}
	return 0;
}

/**********************************/
void CVideoCaptureList::SetAutoLevel(unsigned char lvl, eAspectRatio ar)
{
	m_uOptLevel = lvl;
	m_eAspectRatio = ar;
}

/**********************************/
void CVideoCaptureList::SetUseHardwareEncoder(bool bUseHWEnc)
{
	m_bUseHWEncoder = bUseHWEnc;
}


/**********************************/
tc_LevelModeState CVideoCaptureList::GetLevelState(const wchar_t* szName)
{
	tc_LevelModeState state;
	devlist_iter iter = m_DeviceModeList.find(szName);
	if (iter != m_DeviceModeList.end()) {
		int typeHW = (m_bUseHWEncoder) ? iter->second.iHWEncoderMode : 0;
		if (iter->second.pCaptureLevelList[typeHW][m_uOptLevel] != 0) {
			state = iter->second.pCaptureLevelList[typeHW][m_uOptLevel][m_eAspectRatio];
		}
	}
	return state;
}

/**********************************/
bool CVideoCaptureList::IsHWEncoderSupport(const wchar_t* szName)
{
	bool res = false;
	devlist_iter iter = m_DeviceModeList.find(szName);
	if (iter != m_DeviceModeList.end()) res = (iter->second.iHWEncoderMode > 0 && m_bUseHWEncoder);
	return res;
}

/**********************************/
int CVideoCaptureList::GetStereoMode(const wchar_t* szName)
{
	int res = 0;
	devlist_iter iter = m_DeviceModeList.find(szName);
	if (iter != m_DeviceModeList.end()) res = iter->second.iStereoMode;
	return res;
}

/**********************************/
int CVideoCaptureList::GetAutoModeHD(const wchar_t* szName)
{
	int mode = -1;
	devlist_iter iter = m_DeviceModeList.find(szName);
	if (iter != m_DeviceModeList.end()) {
		mode = iter->second.iHDVideoSourceMode;
	}
	return mode;
}

void CVideoCaptureList::FilterModeList(unsigned char baseLevel, unsigned char maxLevel, DeviceModeState *pDevMode, const wchar_t *szName,
									   std::multimap <int, int> & filterModes,
									   std::multimap <int, int> & filterExtendModes,
									   std::vector <std::multimap <int, int>> & hardwareModes,
									   std::multimap <int, int> & validModes)
{
	filterModes.clear();
	filterExtendModes.clear();
	validModes.clear();
	tc_levelVideo_t lvlDesc;
	if (m_pLevelCaps->GetLevelDesc(maxLevel, &lvlDesc)) {
		CModeList *pModeList = pDevMode->pModeList;
		CColorModeDescription cmd;
		int bestRating = INT_MAX;
		int num_modes = pModeList->iGetMaxMode();
		for (int i = 0; i < num_modes; i++) {
			pModeList->iGetModeDescription(i, &cmd);
			if (cmd.bIncorrect) continue;
			if (cmd.WidthBase <= 32 || cmd.HeightBase <= 32) continue;
			if (cmd.FrameIntMin == 0 && cmd.FrameIntMax == 0) continue;
			if (cmd.FrameIntMin == 0) cmd.FrameIntMin = cmd.FrameIntMax;
			if (cmd.FrameIntMax == 0) cmd.FrameIntMax = cmd.FrameIntMin;
			if (cmd.WidthBase & 0x7) {
				int dw = cmd.iGetdW();
				int w8 = (cmd.WidthMax / 8) * 8;
				int dw8 = w8 - cmd.WidthMin;
				if (dw8 < 0) dw8 = -dw8;
				if (dw8 % dw == 0) cmd.Width = w8;
				else continue;
			}
			if (cmd.HeightBase & 0x7) {
				int dh = cmd.iGetdH();
				int h8 = (cmd.HeightMax / 8) * 8;
				int dh8 = h8 - cmd.HeightMin;
				if (dh8 < 0) dh8 = -h8;
				if (dh8 % dh == 0) cmd.Height = h8;
				else continue;
			}
			pModeList->iUpdateModeDescription(i, &cmd);
			if (cmd.Color == CColorSpace::UNKNOWN) continue;
			if (cmd.Color == CColorSpace::STR0) pDevMode->iStereoMode = 1;
			int MBFrame = cmd.Width * cmd.Height / 256;
			std::pair <int, int> m(MBFrame, i);
			if (cmd.Color == CColorSpace::H264) {
				hardwareModes[ENCODER_H264_LOGITECH].insert(m);
			} else {
				if (m_pLevelCaps->GetLevelDesc(baseLevel, &lvlDesc)) {
					std::multimap <int, int> ::iterator it;
					CColorModeDescription cmd_v;
					bool insert = true;
					double baseFramerate = (double)lvlDesc.maxMBps / (double)lvlDesc.maxFrameSizeMB;
					double maxFramerate = 10000000.0 / (double)(cmd.FrameIntMin) + 0.05 /* ntsc correct */;
					if (validModes.count(MBFrame) > 0) {
						for (it = validModes.equal_range(MBFrame).first; it != validModes.equal_range(MBFrame).second; ++it) {
							pModeList->iGetModeDescription(it->second, &cmd_v);
							if (cmd == cmd_v) {
								double maxFramerate_v = 10000000.0 / (double)(cmd_v.FrameIntMin);
								if (cmd.Color <= cmd_v.Color && (maxFramerate >= maxFramerate_v || maxFramerate >= baseFramerate)) {
									it->second = i;
								}
								insert = false;
								break;
							}
						}
					}
					if (insert) {
						validModes.insert(m);
					}
					insert = true;
					if (filterModes.count(MBFrame) > 0) {
						for (it = filterModes.equal_range(MBFrame).first; it != filterModes.equal_range(MBFrame).second; ++it) {
							pModeList->iGetModeDescription(it->second, &cmd_v);
							if (cmd == cmd_v) {
								if (cmd.Color < cmd_v.Color && maxFramerate >= baseFramerate) {
									it->second = i;
								}
								insert = false;
								break;
							}
						}
					}
					if (insert) {
						if (maxFramerate >= baseFramerate) {
							filterModes.insert(m);
						}
					}
					insert = true;
					if (filterExtendModes.count(MBFrame) > 0) {
						for (it = filterExtendModes.equal_range(MBFrame).first; it != filterExtendModes.equal_range(MBFrame).second; ++it) {
							pModeList->iGetModeDescription(it->second, &cmd_v);
							if (cmd == cmd_v) {
								if (cmd.Color < cmd_v.Color && maxFramerate >= VIDEO_FRAMERATE_EXTEND_LIMIT) {
									it->second = i;
								}
								insert = false;
								break;
							}
						}
					}
					if (insert) {
						if (maxFramerate >= VIDEO_FRAMERATE_EXTEND_LIMIT) {
							filterExtendModes.insert(m);
						}
					}
				}
			}
		}
	}
}

/**********************************/
void CVideoCaptureList::SetCaptureLevelMode(DeviceModeState *pDevMode, const wchar_t *szName)
{
	std::multimap <int, int> validModes;
	std::multimap <int, int> filterModes;
	std::multimap <int, int> filterExtendModes;
	std::vector <std::multimap <int, int>> hardwareModes (MAX_NUM_HW_ENCODERS);

	std::multimap <int, int> ::iterator it, best[VS_VIDEOAR_NUM_MAX];

	unsigned char lvl;
	CColorModeDescription cmd;
	CModeList *pModeList = pDevMode->pModeList;

	/// try find base modes
	tc_levelVideo_t lvlDescBase;
	if (m_pLevelCaps->GetLevelDesc(VIDEO_LEVEL_MAX, &lvlDescBase)) {
		bool screenCapture(pDevMode->typeCapturer == CVideoCaptureList::CAPTURE_SCREEN);
		if (!screenCapture) {
			unsigned char actualLvl = m_pLevelAutoCaps->CheckLevel(m_sndLevel);
			if (actualLvl > VIDEO_LEVEL_MAX) {
				actualLvl = VIDEO_LEVEL_MAX;
			}
			FilterModeList(actualLvl, VIDEO_LEVEL_MAX, pDevMode, szName, filterModes, filterExtendModes, hardwareModes, validModes);
			m_pLevelCaps->GetLevelDesc(actualLvl, &lvlDescBase);
		}
		else {
			FilterModeList(VIDEO_LEVEL_MAX, VIDEO_LEVEL_MAX, pDevMode, szName, filterModes, filterExtendModes, hardwareModes, validModes);
		}
		if (validModes.empty()) return;
		int baseFramerate = lvlDescBase.maxMBps / lvlDescBase.maxFrameSizeMB;
		for (int iar = 0; iar < VS_VIDEOAR_NUM_MAX; iar++) {
			best[iar] = filterModes.end();
			double frameMBmax = 0.0;
			int arw = tc_AspectRatio[iar].arw;
			int arh = tc_AspectRatio[iar].arh;
			for (it = filterModes.begin(); it != filterModes.end(); ++it) {
				pModeList->iGetModeDescription(it->second, &cmd);
				if (pDevMode->typeCapturer == CVideoCaptureList::CAPTURE_SCREEN) {
					arh = 10000;
					arw = (cmd.Width * arh) / cmd.Height;
				}
				double sh = (double)cmd.Height / (double)arh;
				double sw = (double)cmd.Width / (double)arw;
				double k = 1.0;
				if (fabs(sh - sw) <= 1.0) {
					k += 0.3;
					int dmb = (cmd.Width * 16) / 256;
					if (it->first + dmb >= lvlDescBase.maxFrameSizeMB) {
						k += 1.0;
					}
				} else if (sw > sh) {
					k = sh / sw;
				} else {
					k = sw / sh;
				}
				double frameMB = (it->first * k) / (double)lvlDescBase.maxFrameSizeMB;
				if (frameMB >= frameMBmax) {
					frameMBmax = frameMB;
					best[iar] = it;
					if (k > 2.0) break;
				}
			}
			if (best[iar] == filterModes.end()) {
				best[iar] = validModes.begin(); /// init with lowest resolution
				int bestFramerate = 0;
				for (it = validModes.begin(); it != validModes.end(); ++it) {
					pModeList->iGetModeDescription(it->second, &cmd);
					int maxFramerate = (int)(10000000.0 / (double)(cmd.FrameIntMin) + 0.5);
					if (maxFramerate >= VIDEO_FRAMERATE_MIN_LIMIT || maxFramerate >= bestFramerate) {
						best[iar] = it;
						bestFramerate = maxFramerate;
					}
				}
				if (best[iar] == validModes.end()) return;
			}
		}
		/// HD Video Source
		if (m_pLevelCaps->GetLevelDesc(VIDEO_LEVEL_MAX, &lvlDescBase)) {
			if (validModes.count(lvlDescBase.maxFrameSizeMB) > 0) {
				double maxFramerate_hd = 0.0;
				for (it = validModes.equal_range(lvlDescBase.maxFrameSizeMB).first; it != validModes.equal_range(lvlDescBase.maxFrameSizeMB).second; ++it) {
					pModeList->iGetModeDescription(it->second, &cmd);
					double framerate = 10000000.0 / (double)(cmd.FrameIntMin);
					if (framerate > maxFramerate_hd) {
						pDevMode->iHDVideoSourceMode = it->second;
					}
				}
			}
		}
	}

	/// software , limit = VIDEO_LEVEL_MAX

	tc_levelVideo_t lvlDesc;
	tc_LevelModeState *pDescPrev[VS_VIDEOAR_NUM_MAX] = { 0 };
	for (lvl = 0; lvl <= m_pLevelCaps->GetMaxLevel(); lvl++) {
		if (m_pLevelCaps->GetLevelDesc(lvl, &lvlDesc)) {
			int maxLevelFramerate = lvlDesc.maxMBps / lvlDesc.maxFrameSizeMB;
			tc_LevelModeState *pCaptureLevel = pDevMode->pCaptureLevelList[ENCODER_SOFTWARE][lvl];
			if (pCaptureLevel == 0) {
				pDevMode->pCaptureLevelList[ENCODER_SOFTWARE][lvl] = new tc_LevelModeState [VS_VIDEOAR_NUM_MAX];
				pCaptureLevel = pDevMode->pCaptureLevelList[ENCODER_SOFTWARE][lvl];
			}
			for (int j = 0; j < VS_VIDEOAR_NUM_MAX; j++) {
				tc_LevelModeState *pDesc = &pCaptureLevel[j];
				if (lvl <= VIDEO_LEVEL_MAX) {
					int MBFrame, framerate;
					int width, height;
					pModeList->iGetModeDescription(best[j]->second, &cmd);
					int maxFramerate = (int)(10000000.0 / (double)(cmd.FrameIntMin) + 0.5);
					int maxMBFrame = cmd.Width * cmd.Height / 256;
					int arw = tc_AspectRatio[j].arw;
					int arh = tc_AspectRatio[j].arh;
					if (pDevMode->typeCapturer == CVideoCaptureList::CAPTURE_SCREEN) {
						arh = 10000;
						arw = (cmd.Width * arh) / cmd.Height;
					}
					double sh = (double)cmd.Height / (double)arh;
					double sw = (double)cmd.Width / (double)arw;
					if (sh == sw && maxMBFrame <= lvlDesc.maxFrameSizeMB) {
						width = cmd.Width;
						height = cmd.Height;
					} else {
						MBFrame = std::min(lvlDesc.maxFrameSizeMB, maxMBFrame);
						if (sw < sh) {
							double w = sqrt((double)(arw * MBFrame) / (double)arh) * 16.0;
							if (w > cmd.Width) w = cmd.Width;
							height = (int)(w / (double)arw * arh + 4) &~ 7;
							width = (int)w &~ 7;
						} else {
							double h = sqrt((double)(arh * MBFrame) / (double)arw) * 16.0;
							if (h > cmd.Height) h = cmd.Height;
							width = (int)(h / (double)arh * arw + 4) &~ 7;
							height = (int)h &~ 7;
						}
					}
					MBFrame = width * height / 256;
					framerate = (unsigned int)(lvlDesc.maxMBps / (double)MBFrame);
					if (framerate > maxFramerate) framerate = maxFramerate;
					if (framerate > maxLevelFramerate) framerate = maxLevelFramerate;
					pDesc->nVideoMode = best[j]->second;
					pDesc->nMBps = MBFrame * framerate;
					pDesc->nMBFrame = MBFrame;
					pDesc->nWidth = width;
					pDesc->nHeight = height;
					pDesc->nTypeHWEncoder = ENCODER_SOFTWARE;
					pDescPrev[j] = pDesc;
				} else {
					*pDesc = *pDescPrev[j];
				}
			}
		}
	}

	/// hardware

	for (int hwType = 1; hwType < MAX_NUM_HW_ENCODERS; hwType++) {
		if (!hardwareModes[hwType].empty()) {
			for (lvl = 0; lvl <= m_pLevelCaps->GetMaxLevel(); lvl++) {
				if (m_pLevelCaps->GetLevelDesc(lvl, &lvlDesc)) {
					tc_LevelModeState *pCaptureLevel = pDevMode->pCaptureLevelList[hwType][lvl];
					if (pCaptureLevel == 0) {
						pDevMode->pCaptureLevelList[hwType][lvl] = new tc_LevelModeState [VS_VIDEOAR_NUM_MAX];
						pCaptureLevel = pDevMode->pCaptureLevelList[hwType][lvl];
					}
					if (lvl < VIDEO_HWLEVEL_MIN) {
						for (int j = 0; j < VS_VIDEOAR_NUM_MAX; j++) {
							pCaptureLevel[j] = pDevMode->pCaptureLevelList[ENCODER_SOFTWARE][lvl][j];
						}
					} else {
						for (int j = 0; j < VS_VIDEOAR_NUM_MAX; j++) {
							double kmax = 0.0;
							tc_LevelModeState *pDesc = &pCaptureLevel[j];
							int arw = tc_AspectRatio[j].arw;
							int arh = tc_AspectRatio[j].arh;
							for (it = hardwareModes[hwType].begin(); it != hardwareModes[hwType].end(); ++it) {
								if (it->first > lvlDesc.maxFrameSizeMB) break;
								pModeList->iGetModeDescription(it->second, &cmd);
								double sh = (double)cmd.Height / (double)arh;
								double sw = (double)cmd.Width / (double)arw;
								double k = 1.0;
								if (sw > sh) {
									k = sh / sw;
								} else {
									k = sw / sh;
								}
								if (k >= kmax) {
									int maxFramerate = (int)(10000000.0 / (double)(cmd.FrameIntMin) + 0.5);
									int framerate = lvlDesc.maxMBps / it->first;
									if (framerate > maxFramerate) framerate = maxFramerate;
									kmax = k;
									pDesc->nVideoMode = it->second;
									pDesc->nMBps = it->first * framerate;
									pDesc->nMBFrame = it->first;
									pDesc->nWidth = cmd.Width;
									pDesc->nHeight = cmd.Height;
									pDesc->nTypeHWEncoder = hwType;
								}
							}
						}
					}
				}
			}
		}
	}

	//DumpCaptureLevelList(pDevMode, szName);
}

/**********************************/
void CVideoCaptureList::DumpCaptureLevelList(DeviceModeState *pDevMode, const wchar_t *szName)
{
	tc_levelVideo_t lvlDesc;
	FILE *f = fopen("capture_video_caps.txt", "a");
	fprintf(f, "\n================================================================");
	fprintf(f, "\n%S\n\n", szName);
	for (int typeHW = 0; typeHW < MAX_NUM_HW_ENCODERS; typeHW++) {
		fprintf(f, "===== %s MODES =====\n", (typeHW > 0) ? "HARDWARE" : "SOFTWARE");
		for (int lvl = 0; lvl <= m_pLevelCaps->GetMaxLevel(); lvl++) {
			if (m_pLevelCaps->GetLevelDesc(lvl, &lvlDesc)) {
				tc_LevelModeState *pLvlDesc = pDevMode->pCaptureLevelList[typeHW][lvl];
				if (pLvlDesc != 0) {
					fprintf(f, "LEVEL %10s  [%6d, %9d]: \n", lvlDesc.name, lvlDesc.maxFrameSizeMB, lvlDesc.maxMBps);
					for (int j = 0; j < VS_VIDEOAR_NUM_MAX; j++) {
						fprintf(f, "%d [%4dx%4d @ %2d, %6d, %9d, %3d]\n", j, pLvlDesc[j].nWidth, pLvlDesc[j].nHeight, pLvlDesc[j].nMBps / pLvlDesc[j].nMBFrame,
																		  pLvlDesc[j].nMBFrame, pLvlDesc[j].nMBps, pLvlDesc[j].nVideoMode);
					}
				}
			}
		}
	}
	fclose(f);
}

