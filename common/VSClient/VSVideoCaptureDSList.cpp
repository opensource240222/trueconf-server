
#include <atlbase.h>
#include <dshow.h>
#include <dvdmedia.h>
#include "VSDirecShow.h"
#include "VSVideoCaptureList.h"

/****************************************************************************
 * CModeListDirectShow
 ****************************************************************************/

/**
 **************************************************************************
 ****************************************************************************/
void CModeListDirectShow::ResetList(void)
{
	UINT i;
	for (i = 0; i < m_ModeList.size(); i++) {
		delete (CMediaType*)(m_ModeList[i].m_ColorModeID);
	}
	m_ModeList.clear();
	m_iMaxMode = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CModeListDirectShow::iAddMode(void* _pMode, void* _pVideoCaps, __int64 *pFrameIntList, int sizeList, int pinNumber, bool fixMjpeg, unsigned int limitMB)
{
	AM_MEDIA_TYPE *pMode = reinterpret_cast<CMediaType*>(_pMode);
	VIDEO_STREAM_CONFIG_CAPS *pVideoCaps = (VIDEO_STREAM_CONFIG_CAPS*)_pVideoCaps;

	if ((pMode->formattype == FORMAT_VideoInfo) || (pMode->formattype == FORMAT_VideoInfo2)) {
		int ColorTag;
		bool bInterlace = false;
		CMediaType* pMediaType= new CMediaType(*pMode);
		pMediaType->SetFormat(pMode->pbFormat, pMode->cbFormat);
		REFERENCE_TIME *pAvgTimePerFrame;
		BITMAPINFOHEADER *pbmiHeader;
		if (pMode->formattype == FORMAT_VideoInfo) {
			VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)(pMediaType->pbFormat);
			pAvgTimePerFrame=&pVih->AvgTimePerFrame;
			pbmiHeader=&pVih->bmiHeader;
		} else {
			VIDEOINFOHEADER2 *pVih = (VIDEOINFOHEADER2*)(pMediaType->pbFormat);
			pAvgTimePerFrame = &pVih->AvgTimePerFrame;
			pbmiHeader = &pVih->bmiHeader;
			bInterlace = pVih->dwInterlaceFlags & (AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOnly);
		}
		if (pMode->subtype == MEDIASUBTYPE_CLPL) ColorTag = CColorSpace::CLPL;
		else if (pMode->subtype.Data1 == CColorSpace::FCC_STR0) ColorTag = CColorSpace::STR0;
		else if (pMode->subtype.Data1 == mmioFOURCC('I', '4', '2', '0')) ColorTag = CColorSpace::I420;
		else if (pMode->subtype.Data1 == mmioFOURCC('H', 'D', 'Y', 'C')) ColorTag = CColorSpace::HDYC;
		else if (pMode->subtype.Data1 == mmioFOURCC('H', '2', '6', '4')) ColorTag = CColorSpace::H264;
		else if (pMode->subtype.Data1 == mmioFOURCC('Y', 'V', '1', '2'))  ColorTag = CColorSpace::YV12;
		else if (pMode->subtype == MEDIASUBTYPE_IYUV) ColorTag = CColorSpace::IYUV;
		else if (pMode->subtype == MEDIASUBTYPE_YVU9) ColorTag = CColorSpace::YVU9;
		else if (pMode->subtype == MEDIASUBTYPE_Y411) ColorTag = CColorSpace::Y411;
		else if (pMode->subtype == MEDIASUBTYPE_Y41P) ColorTag = CColorSpace::Y41P;
		else if (pMode->subtype == MEDIASUBTYPE_YUY2) ColorTag = CColorSpace::YUY2;
		else if (pMode->subtype == MEDIASUBTYPE_YVYU) ColorTag = CColorSpace::YVYU;
		else if (pMode->subtype == MEDIASUBTYPE_UYVY) ColorTag = CColorSpace::UYVY;
		else if (pMode->subtype == MEDIASUBTYPE_Y211) ColorTag = CColorSpace::Y211;
		//else if (pMode->subtype == MEDIASUBTYPE_YV12) ColorTag=CColorSpace::YV12;
		else if (pMode->subtype == MEDIASUBTYPE_CLJR) ColorTag = CColorSpace::CLJR;
		else if (pMode->subtype == MEDIASUBTYPE_IF09) ColorTag = CColorSpace::IF09;
		else if (pMode->subtype == MEDIASUBTYPE_CPLA) ColorTag = CColorSpace::CPLA;
		else if (pMode->subtype == MEDIASUBTYPE_RGB24) ColorTag = CColorSpace::RGB24;
		else if (pMode->subtype == MEDIASUBTYPE_RGB32) ColorTag = CColorSpace::RGB32;
		else if (pMode->subtype == MEDIASUBTYPE_MJPG) ColorTag = CColorSpace::MJPEG;
		else if (pMode->subtype == MEDIASUBTYPE_dvsd) ColorTag = CColorSpace::DVSD;
		else if (pMode->subtype == MEDIASUBTYPE_dvhd) ColorTag = CColorSpace::DVHD;
		else if (pMode->subtype == MEDIASUBTYPE_dvsl) ColorTag = CColorSpace::DVSL;
		else if (pMode->subtype == MEDIASUBTYPE_RGB1) ColorTag = CColorSpace::RGB1;
		else if (pMode->subtype == MEDIASUBTYPE_RGB4) ColorTag = CColorSpace::RGB4;
		else if (pMode->subtype == MEDIASUBTYPE_RGB8) ColorTag = CColorSpace::RGB8;
		else if (pMode->subtype == MEDIASUBTYPE_RGB565) ColorTag = CColorSpace::RGB565;
		else if (pMode->subtype == MEDIASUBTYPE_RGB555) ColorTag = CColorSpace::RGB555;
		else if (pMode->subtype == MEDIASUBTYPE_NV12) ColorTag = CColorSpace::NV12;
		else if (pMode->subtype.Data1 == CColorSpace::FCC_BJPG) ColorTag = CColorSpace::BJPG;
		else
			ColorTag = CColorSpace::UNKNOWN;

		*pAvgTimePerFrame = 10000000 / 30;

		/// check FrameInterval
		if (pVideoCaps->MinFrameInterval < 0 || pVideoCaps->MinFrameInterval > 10000000) pVideoCaps->MinFrameInterval = __int64(10000000.0 / (double)30 + 0.5);
		if (pVideoCaps->MaxFrameInterval < 0 || pVideoCaps->MaxFrameInterval > 10000000) pVideoCaps->MaxFrameInterval = __int64(10000000.0 / (double)10 + 0.5);
		if (pVideoCaps->OutputGranularityX < 0 || pVideoCaps->OutputGranularityX > 4096) pVideoCaps->OutputGranularityX = 0;
		if (pVideoCaps->OutputGranularityY < 0 || pVideoCaps->OutputGranularityY > 3112) pVideoCaps->OutputGranularityY = 0;

		bool bIncorrectMode = false;
		if (pFrameIntList) {
			/// correct some drivers bugs for framelist
			if (pFrameIntList[0] < pFrameIntList[sizeList-1]) {
				/// reorder list
				for (int i = 0; i < sizeList / 2; i++) {
					__int64 tmp = pFrameIntList[i];
					pFrameIntList[i] = pFrameIntList[sizeList-1-i];
					pFrameIntList[sizeList-1-i] = tmp;
				}
			}
			if (pFrameIntList[0] < pVideoCaps->MinFrameInterval ||
				pFrameIntList[sizeList-1] > pVideoCaps->MaxFrameInterval) {
					bIncorrectMode = true;
			} else {
				if (pVideoCaps->MaxFrameInterval > pFrameIntList[0]) pVideoCaps->MaxFrameInterval = pFrameIntList[0];
				if (pVideoCaps->MinFrameInterval < pFrameIntList[sizeList-1]) pVideoCaps->MinFrameInterval = pFrameIntList[sizeList-1];
			}
		}

		bool bS = false;

		CColorMode cMode;
		cMode.SetColorMode((void*)(pMediaType),ColorTag,pbmiHeader->biHeight,pbmiHeader->biWidth);
		cMode.SetVideoCaps(pVideoCaps->MinOutputSize.cy, pVideoCaps->MaxOutputSize.cy,
						   pVideoCaps->MinOutputSize.cx, pVideoCaps->MaxOutputSize.cx,
						   pVideoCaps->OutputGranularityX, pVideoCaps->OutputGranularityY,
						   pVideoCaps->MinFrameInterval, pVideoCaps->MaxFrameInterval, pFrameIntList, sizeList, bInterlace);
		cMode.SetPinNumber(pinNumber);
		cMode.SetInrorrect(bIncorrectMode);

		for (int i = 0; i < m_iMaxMode; i++) { /// break if FORMAT_VideoInfo and FORMAT_VideoInfo2 with the same parameters
			bS = (m_ModeList[i] == cMode);
			if (bS) break;
		}

		if (fixMjpeg) {
			if ((ColorTag != CColorSpace::MJPEG) && (pbmiHeader->biWidth > 1280 && pbmiHeader->biHeight > 720)) {
				bS = true;
			}
		}
		unsigned int MB(pbmiHeader->biWidth * pbmiHeader->biHeight / 256);
		if (MB > limitMB) {
			bS = true;
		}

		if (!bS) {
			m_ModeList.push_back(cMode);
			m_iMaxMode++;
		}

		DeleteMediaType(pMode);
		return (bS) ? -2 : 0;
	}
	DeleteMediaType(pMode);
	return -2;
}

/****************************************************************************
 * VS_VideoCaptureDirectShowList
 ****************************************************************************/

VS_VideoCaptureDirectShowList::VS_VideoCaptureDirectShowList(CVSInterface* pParentInterface, unsigned char sndLevel) : CVideoCaptureList(pParentInterface, sndLevel)
{
	m_DevMonikerList.clear();
	m_typeCapturer = CVideoCaptureList::CAPTURE_DS;
}

VS_VideoCaptureDirectShowList::~VS_VideoCaptureDirectShowList()
{
	ResetMonikerList();
}

int VS_VideoCaptureDirectShowList::iInternalGetDeviceList(void)
{
	HRESULT hr;
	ICreateDevEnum *pDevEnum = NULL;
	m_pSourceList->ResetList();
	ResetModeList();
	ResetMonikerList();
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,IID_ICreateDevEnum, (void **)&pDevEnum);
	if ( SUCCEEDED(hr) && pDevEnum) {
		IEnumMoniker *pClassEnum = NULL;
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
		if ( SUCCEEDED(hr) && pClassEnum) {
			ULONG cFetched;
			IMoniker *pMoniker = NULL;
			while (pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK) {
				bool bMediaFoundation = false;
				IPropertyBag *pProp = NULL;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pProp);
				if ( SUCCEEDED(hr) ) {
					_variant_t varName, varPath;
					hr = pProp->Read(L"DevicePath", &varPath, 0);
					if (!SUCCEEDED(hr)) {
						varPath.Clear();
					} else {
						for (size_t idx = 0; idx < m_qDeviceMF.size(); idx++) {
							bMediaFoundation = ( wcsstr(varPath.bstrVal, m_qDeviceMF[idx].c_str()) != 0 );
							if (bMediaFoundation) break;
						}
					}
					if (!bMediaFoundation) {
						hr = pProp->Read(L"FriendlyName", &varName, 0);
						if ( SUCCEEDED(hr) ) {
							int n;
							n = m_pSourceList->iFindString(varName.bstrVal);
							if (n >= 0) {
								char buff[256];
								_bstr_t str(varName.bstrVal);
								_itoa(m_pSourceList->iGetMaxString(), buff, 10);
								str += " (";
								str += buff;
								str += ")";
								varName = str;
							}
							m_pSourceList->iAddString(varName.bstrVal);
							m_DevMonikerList.push_back(pMoniker);
							pMoniker->AddRef();
							VS_VideoCaptureDirectShowList::iGetDeviceModeList((bstr_t)varName, (bstr_t)varPath);
						}
					}
					VariantClear(&varName);
					pProp->Release();
				}
				pMoniker->Release();
			}
			pClassEnum->Release();
		}
		pDevEnum->Release();
	}
	if (m_typeCapturer == CVideoCaptureList::CAPTURE_DS) {
		iGetScreenCaptureList();
	}
	return 0;
}

int VS_VideoCaptureDirectShowList::iGetDeviceModeList(const wchar_t *szName, const wchar_t *szDevPath){
	int ret = -1;
	int dev_num = m_pSourceList->iFindString(szName);
	IBaseFilter *pCaptureFilter = _iConnect(dev_num);
	m_bVFW[dev_num] = true;

	if (pCaptureFilter) {
		bool fixMjpeg = false;
		DeviceModeState DevMode;
		CModeList *pMode = new CModeListDirectShow();
		DevMode.pModeList = pMode;
		DevMode.pathDevice += szDevPath;
		DevMode.typeCapturer = CVideoCaptureList::CAPTURE_DS;
		DevMode.typeDevice = CVideoCaptureList::DEVICE_WEB;
		if (!m_sDeviceFixMjpeg.empty() && wcsstr(DevMode.pathDevice.c_str(), m_sDeviceFixMjpeg.c_str()) != 0) {
			fixMjpeg = true;
		}
		CModeListDirectShow *pModeListDirectShow = reinterpret_cast <CModeListDirectShow*> (pMode);
		int iNumPin = 0;
		while (true) {
			IPin *pCapPin = GetPin(pCaptureFilter, iNumPin, PINDIR_OUTPUT);
			if (pCapPin == NULL) {
				if (iNumPin == 0) {
					delete pMode;
				} else {
					SetCaptureLevelMode(&DevMode, szName);
					m_DeviceModeList.emplace(szName, DevMode);
				}
				break;
			}
			GUID PinCategory;
			HRESULT hr = GetPinCategory(pCapPin, &PinCategory);
			if (PIN_CATEGORY_CAPTURE == PinCategory) {
				IAMStreamConfig *pStreamConfig;
				HRESULT hr = pCapPin->QueryInterface(IID_IAMStreamConfig,(void **)&pStreamConfig);
				if (hr == S_OK && pStreamConfig != NULL) {
					int iNum = 0, iSize = 0, i;
					pStreamConfig->GetNumberOfCapabilities(&iNum, &iSize);
					if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS)){
						for (i = 0; i < iNum; i++){
							VIDEO_STREAM_CONFIG_CAPS vc;
							AM_MEDIA_TYPE *pMt = NULL;
							hr = pStreamConfig->GetStreamCaps(i,&pMt,(BYTE*)&vc);
							if (hr == S_OK && pMt) {
								if (pMt->formattype == FORMAT_VideoInfo || pMt->formattype == FORMAT_VideoInfo2) {
									int x = vc.InputSize.cx;
									int y = vc.InputSize.cy;
									AM_MEDIA_TYPE *pMt_ = CreateMediaType(pMt);
									if (pMt_ && x >= 0 && y >= 0) {
										if (x == 0) x = vc.MaxCroppingSize.cx;
										if (y == 0) y = vc.MaxCroppingSize.cy;
										if (pMt_->formattype == FORMAT_VideoInfo){
											VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)(pMt_->pbFormat);
											pVih->bmiHeader.biHeight=y;
											pVih->bmiHeader.biWidth=x;
											pVih->bmiHeader.biSizeImage=DIBSIZE(pVih->bmiHeader);
										} else {
											VIDEOINFOHEADER2 *pVih = (VIDEOINFOHEADER2*)(pMt_->pbFormat);
											pVih->bmiHeader.biHeight=y;
											pVih->bmiHeader.biWidth=x;
											pVih->bmiHeader.biSizeImage=DIBSIZE(pVih->bmiHeader);
										}
										__int64 *pFrameIntList = 0;
										long listSize = 0;
										IAMVideoControl* pVideoControl = 0;
										hr = pCaptureFilter->QueryInterface(IID_IAMVideoControl, (void**)&pVideoControl);
										if (hr == S_OK && pVideoControl) {
											SIZE frameSize = {x, y};
											hr = pVideoControl->GetFrameRateList(pCapPin, i, frameSize, &listSize, &pFrameIntList);
											if (hr != S_OK || listSize <= 0) {
												/// TO DO : may be AV
												if (listSize > 0) CoTaskMemFree(pFrameIntList);
												pFrameIntList = 0;
												listSize = 0;
											}
										}
										if (pVideoControl) pVideoControl->Release(); pVideoControl = 0;
										unsigned int limitMB(-1);
										for (const auto& it : m_mDeviceLimit) {
											if (wcsstr(szDevPath, it.first.c_str()) != 0) {
												limitMB = it.second;
												break;
											}
										}
										pModeListDirectShow->iAddMode((void*)pMt_, (void*)&vc, pFrameIntList, listSize, iNumPin, fixMjpeg, limitMB);
										CoTaskMemFree(pFrameIntList);
										pFrameIntList = 0;
									}
								}
								DeleteMediaType(pMt);
							}
						}
					}
					pStreamConfig->Release();
					m_bVFW[dev_num] = false;
				}
				ret = 0;
			}
			iNumPin++;
		}
		pCaptureFilter->Release();
	}

	return ret;
}

IBaseFilter* VS_VideoCaptureDirectShowList::_iConnect(int device_number)
{
	HRESULT hr;
	IBaseFilter *pCaptureFilter = NULL;

	if (device_number >= 0 && device_number < (int)m_DevMonikerList.size()) {
		hr = m_DevMonikerList[device_number]->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCaptureFilter);
	}

	return pCaptureFilter;
}

IBaseFilter* VS_VideoCaptureDirectShowList::GetCaptureByName(const wchar_t* szName)
{
	int dev_number = m_pSourceList->iFindString(szName);
	return _iConnect(dev_number);
}




