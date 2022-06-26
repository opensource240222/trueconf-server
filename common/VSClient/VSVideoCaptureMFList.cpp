#include <Mfapi.h>
#include <Mfidl.h>
#include "VSVideoCaptureList.h"
#include "VS_Dmodule.h"

DEFINE_MEDIATYPE_GUID( MFVideoFormat_HDYC, FCC('HDYC') );
DEFINE_MEDIATYPE_GUID( MFVideoFormat_STR0, FCC('STR0') );

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT) {
		(*ppT)->Release();
		*ppT = NULL;
	}
}

void CModeListMediaFoundation::ResetList(void)
{
	UINT i;
	for (i = 0; i < m_ModeList.size(); i++) {
		IMFMediaType *pT = reinterpret_cast <IMFMediaType*> (m_ModeList[i].m_ColorModeID);
		SafeRelease(&pT);
	}
	m_ModeList.clear();
	m_iMaxMode = 0;
}

/*
MF_MT_FRAME_SIZE
MF_MT_AVG_BITRATE
MF_MT_MAJOR_TYPE = MFMediaType_Video
MF_MT_FRAME_RATE
MF_MT_FRAME_RATE_RANGE_MIN
MF_MT_FRAME_RATE_RANGE_MAX
MF_MT_SAMPLE_SIZE
MF_MT_INTERLACE_MODE
MF_MT_SUBTYPE
*/

int CModeListMediaFoundation::iAddMode(void* _pMode, int pinNumber, bool fixMjpeg, unsigned int limitMB)
{
	int ret = -1;
	UINT32 count = 0;
	UINT32 k = 0;
	IMFMediaType *pType = reinterpret_cast <IMFMediaType*> (_pMode);
	if (pType) {
		HRESULT hr = pType->GetCount(&count);
		if ( SUCCEEDED(hr) ) {
			GUID guid = { 0 };
			hr = pType->GetMajorType(&guid);
			if ( SUCCEEDED(hr) && IsEqualGUID(guid, MFMediaType_Video) ) {
				GUID guid_subtype = { 0 };
				hr = pType->GetGUID(MF_MT_SUBTYPE, &guid_subtype);
				if ( SUCCEEDED(hr) ) {
					int ColorTag;
					if ( IsEqualGUID(guid_subtype, MFVideoFormat_I420) ) ColorTag = CColorSpace::I420;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_H264) ) {
						ColorTag = CColorSpace::H264;
					}
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_IYUV) ) ColorTag = CColorSpace::IYUV;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_Y41P) ) ColorTag = CColorSpace::Y41P;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_YUY2) ) ColorTag = CColorSpace::YUY2;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_YVYU) ) ColorTag = CColorSpace::YVYU;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_UYVY) ) ColorTag = CColorSpace::UYVY;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_YV12) ) ColorTag = CColorSpace::YV12;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_RGB24) ) ColorTag = CColorSpace::RGB24;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_RGB32) ) ColorTag = CColorSpace::RGB32;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_ARGB32) ) ColorTag = CColorSpace::RGB32;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_MJPG) ) ColorTag = CColorSpace::MJPEG;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_DVSD) ) ColorTag = CColorSpace::DVSD;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_DVHD) ) ColorTag = CColorSpace::DVHD;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_DVSL) ) ColorTag = CColorSpace::DVSL;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_RGB8) ) ColorTag = CColorSpace::RGB8;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_RGB565) ) ColorTag = CColorSpace::RGB565;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_RGB555) ) ColorTag = CColorSpace::RGB555;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_HDYC) ) ColorTag = CColorSpace::HDYC;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_STR0) ) ColorTag = CColorSpace::STR0;
					else if ( IsEqualGUID(guid_subtype, MFVideoFormat_NV12) ) ColorTag = CColorSpace::NV12;
					else {
						ColorTag = CColorSpace::UNKNOWN;
					}
					UINT32 width = 0, height = 0;
					hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
					if ( SUCCEEDED(hr) ) {
						UINT32 den = 0, num = 0;
						hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &num, &den);
						if ( SUCCEEDED(hr) && num > 1 ) {
							double fps = (double)num / (double) den;
							__int64 frameIntervalMin = (__int64)(10000000.0 / fps + 0.5);
							__int64 frameIntervalMax = frameIntervalMin;
							hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE_RANGE_MAX, &num, &den);
							if ( SUCCEEDED(hr) ) {
								fps = (double)num / (double) den;
								frameIntervalMin = (__int64)(10000000.0 / fps + 0.5);
							}
							hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE_RANGE_MIN, &num, &den);
							if ( SUCCEEDED(hr) ) {
								fps = (double)num / (double) den;
								frameIntervalMax = (__int64)(10000000.0 / fps + 0.5);
							}
							bool bInterlace = false;
							UINT32 interlaceMode;
							hr = pType->GetUINT32(MF_MT_INTERLACE_MODE, &interlaceMode);
							if ( SUCCEEDED(hr) ) {
								bInterlace = interlaceMode != MFVideoInterlace_Progressive;
							}
							CColorMode cMode;
							cMode.SetColorMode((void*)(pType), ColorTag, height, width);
							cMode.SetVideoCaps(height, height, width, width, 1, 1, frameIntervalMin, frameIntervalMax, NULL, 0, bInterlace);
							cMode.SetPinNumber(pinNumber);
							cMode.SetInrorrect(false);
							bool bS = false;
							for (int i = 0; i < m_iMaxMode; i++) { /// break if FORMAT_VideoInfo and FORMAT_VideoInfo2 with the same parameters
								bS = (m_ModeList[i] == cMode);
								if (bS) break;
							}
							if (fixMjpeg) {
								if ((ColorTag != CColorSpace::MJPEG) && (width > 1280 && height > 720)) {
									bS = true;
								}
							}
							unsigned int MB(width * height / 256);
							if (MB > limitMB) {
								bS = true;
							}
							if (!bS) {
								m_ModeList.push_back(cMode);
								m_iMaxMode++;
								ret = 0;
							}
						}
					}
				}
			}
		}
	}
	return ret;
}

VS_VideoCaptureMediaFoundationList::VS_VideoCaptureMediaFoundationList(CVSInterface* pParentInterface, unsigned char sndLevel) : VS_VideoCaptureDirectShowList(pParentInterface, sndLevel)
{
	m_qDeviceMF.emplace_back(L"usb#vid_046d&pid_"); /// logitech vendor
	m_typeCapturer = CVideoCaptureList::CAPTURE_MF;
}

VS_VideoCaptureMediaFoundationList::~VS_VideoCaptureMediaFoundationList()
{

}

int VS_VideoCaptureMediaFoundationList::iInternalGetDeviceList()
{
	unsigned int i = 0, count = 0;
	m_pSourceList->ResetList();
	ResetModeList();
	VS_VideoCaptureDirectShowList::iInternalGetDeviceList();
 	HRESULT hr = g_MFStartup(MF_VERSION, MFSTARTUP_FULL);
 	if ( SUCCEEDED(hr) ) {
 		IMFAttributes *pAttributes = NULL;
 		hr = g_MFCreateAttributes(&pAttributes, 1);
 		if ( SUCCEEDED(hr) ) {
 			hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
 									  MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
 			if ( SUCCEEDED(hr) ) {
 				IMFActivate **ppDevices = NULL;
 				hr = g_MFEnumDeviceSources(pAttributes, &ppDevices, &count);
 				if ( SUCCEEDED(hr) && count > 0 ) {
					for (i = 0; i < count; i++) {
						WCHAR *szFriendlyName = 0;
						WCHAR *szSymbolicLink = 0;
						unsigned int cchName;
						hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, &cchName);
						if ( SUCCEEDED(hr) ) {
							hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &szSymbolicLink, &cchName);
							if (SUCCEEDED(hr)) {
								const wchar_t *p = wcsstr(szSymbolicLink, L"#{");
								int len(p - szSymbolicLink);
								bool skip(false);
								std::wstring strName(szFriendlyName);
								for (const auto& item : m_DeviceModeList) {
									if (wcsncmp(item.second.pathDevice.c_str(), szSymbolicLink, len) == 0) {
										skip = true;
										break;
									}
								}
								if (!skip) {
									auto it = m_DeviceModeList.find(szFriendlyName);
									if (it != m_DeviceModeList.end()) {
										wchar_t buff[256];
										_itow(m_DeviceModeList.size(), buff, 10);
										std::wstring str(it->first + std::wstring(L" #") + std::wstring(buff));
										strName = str;
									}
									m_pSourceList->iAddString((wchar_t*)strName.c_str());
									hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &szSymbolicLink, &cchName);
									iGetDeviceModeList(strName.c_str(), szSymbolicLink);
								}
							}
						}
						CoTaskMemFree(szFriendlyName);
						CoTaskMemFree(szSymbolicLink);
					}
 				}
 				for (i = 0; i < count; i++) {
 					SafeRelease(&ppDevices[i]);
 				}
 				CoTaskMemFree(ppDevices);
 			}
 		}
 		SafeRelease(&pAttributes);
 	}
 	g_MFShutdown();
	iGetScreenCaptureList();
 	return 0;
}

int VS_VideoCaptureMediaFoundationList::iGetDeviceModeList(const wchar_t *szName, const wchar_t *szDevPath)
{
	int dev_num = m_pSourceList->iFindString(szName);

	IMFMediaSource *pSource = NULL;
	IMFAttributes *pAttributes = NULL;
	HRESULT hr = g_MFCreateAttributes(&pAttributes, 2);
	if ( SUCCEEDED(hr) ) {
		hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
								  MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
		if ( SUCCEEDED(hr) ) {
			if (szDevPath != 0) {
				hr = pAttributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, szDevPath);
			} else {
				hr = pAttributes->SetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, szName);
			}
			if ( SUCCEEDED(hr) ) {
				hr = g_MFCreateDeviceSource(pAttributes, &pSource);
				if ( SUCCEEDED(hr) ) {

					DTRACE(VSTM_VCAPTURE, "");
					DTRACE(VSTM_VCAPTURE, "=================");
					DTRACE(VSTM_VCAPTURE, "Enumerate %S", szName);

					DeviceModeState DevMode;
					CModeList *pMode = new CModeListMediaFoundation();
					DevMode.pModeList = pMode;
					DevMode.pathDevice += szDevPath;
					DevMode.typeCapturer = CVideoCaptureList::CAPTURE_MF;
					DevMode.typeDevice = CVideoCaptureList::DEVICE_WEB;
					CModeListMediaFoundation *pModeListMediaFoundation = reinterpret_cast <CModeListMediaFoundation*> (pMode);
					bool fixMjpeg = false;
					if (!m_sDeviceFixMjpeg.empty() && wcsstr(DevMode.pathDevice.c_str(), m_sDeviceFixMjpeg.c_str()) != 0) {
						fixMjpeg = true;
					}
					unsigned int limitMB(-1);
					for (const auto& it : m_mDeviceLimit) {
						if (wcsstr(szDevPath, it.first.c_str()) != 0) {
							limitMB = it.second;
							break;
						}
					}
					hr = EnumerateCaptureFormats(pSource, pModeListMediaFoundation, fixMjpeg, limitMB);
					if ( SUCCEEDED(hr) ) {
						SetCaptureLevelMode(&DevMode, szName);
						m_DeviceModeList.emplace(szName, DevMode);
						m_bVFW[dev_num] = false;
					} else {
						delete pMode;
					}
				}
			}
		}
	}
	SafeRelease(&pAttributes);
	SafeRelease(&pSource);

	return 0;
}

HRESULT VS_VideoCaptureMediaFoundationList::EnumerateCaptureFormats(IMFMediaSource *pSource, CModeListMediaFoundation *pModeListMediaFoundation, bool fixMjpeg, unsigned int limitMB)
{
	HRESULT hr = S_FALSE;
	unsigned long cTypes = 0, cStreams = 0;
	unsigned int i = 0, j = 0;
	bool ret = false;

    IMFPresentationDescriptor *pPD = NULL;
    IMFStreamDescriptor *pSD = NULL;
    IMFMediaTypeHandler *pHandler = NULL;
    IMFMediaType *pType = NULL;

	hr = pSource->CreatePresentationDescriptor(&pPD);
	if ( SUCCEEDED(hr) ) {
		hr = pPD->GetStreamDescriptorCount(&cStreams);
		if ( SUCCEEDED(hr) ) {

			DTRACE(VSTM_VCAPTURE, "Num Streams = %u", cStreams);

			for (j = 0; j < cStreams; j++) {
				BOOL fSelected;
				hr = pPD->GetStreamDescriptorByIndex(j, &fSelected, &pSD);
				if ( SUCCEEDED(hr) ) {

					DTRACE(VSTM_VCAPTURE, "===== Streams %u =====", j);

					hr = pSD->GetMediaTypeHandler(&pHandler);
					if ( SUCCEEDED(hr) ) {
						hr = pHandler->GetMediaTypeCount(&cTypes);
						if ( SUCCEEDED(hr) ) {

							DTRACE(VSTM_VCAPTURE, "Num Formats = %u", cTypes);

							for (i = 0; i < cTypes; i++) {
								int res = -1;
								hr = pHandler->GetMediaTypeByIndex(i, &pType);
								if ( SUCCEEDED(hr) ) {

									//DTRACE(VSTM_VCAPTURE, "++++++ Format %u ++++++", i);

									res = pModeListMediaFoundation->iAddMode(pType, j, fixMjpeg, limitMB);
									ret |= (res == 0);
								}
								if (res != 0) {
									SafeRelease(&pType);
								}
							}
						}
					}
				}
			}
		}
	}

    SafeRelease(&pPD);
    SafeRelease(&pSD);
    SafeRelease(&pHandler);

	return (ret) ? S_OK : E_FAIL;
}