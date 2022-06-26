
#include "VSVideoCaptureList.h"
#include "VSCapture.h"
#include "ScreenCapturerFactory.h"

void CModeListScreenCapture::ResetList()
{
	UINT i;
	for (i = 0; i < m_ModeList.size(); i++) {
		ScreenCapturerDesc *pT = reinterpret_cast <ScreenCapturerDesc*> (m_ModeList[i].m_ColorModeID);
		delete pT;
	}
	m_ModeList.clear();
	m_iMaxMode = 0;
}

int CModeListScreenCapture::iAddMode(void* _pMode)
{
	ScreenCapturerDesc *pType = reinterpret_cast <ScreenCapturerDesc*> (_pMode);
	int ColorTag = CColorSpace::RGB32;
	__int64 frameIntervalMin = __int64(10000000.0 / (double)30 + 0.5);
	__int64 frameIntervalMax = __int64(10000000.0 / (double)5 + 0.5);
	CColorMode cMode;
	cMode.SetColorMode(0, ColorTag, pType->heightCaptureArea, pType->widthCaptureArea);
	cMode.SetVideoCaps(pType->heightCaptureArea, pType->heightCaptureArea,
					   pType->widthCaptureArea, pType->widthCaptureArea,
					   1, 1, frameIntervalMin, frameIntervalMax, NULL, 0, false);
	cMode.SetPinNumber(0);
	cMode.SetInrorrect(false);
	m_ModeList.push_back(cMode);
	m_iMaxMode++;
	return 0;
}

int CVideoCaptureList::iGetScreenCaptureList()
{
	ScreenCapturer *pDesktopCapture = ScreenCapturerFactory::Create(CapturerType::GDI);

	if (pDesktopCapture) {
		if (pDesktopCapture->EnumerateOutputs()) {
			size_t numOutputs = pDesktopCapture->GetNumOutputs();
			for (size_t i = 0; i <= numOutputs; i++) {
				ScreenCapturerDesc *desc = new ScreenCapturerDesc();
				wchar_t szFriendlyName[256];
				if (i < numOutputs) {
					pDesktopCapture->GetDescOutput(i, desc);
					swprintf(szFriendlyName, L"%s %s [%dx%d]", VS_CaptureDeviceScreen::_nameScreenCapture, desc->name, desc->widthCaptureArea, desc->heightCaptureArea);
				} else {
					pDesktopCapture->GetDescOutput(0, desc);
					swprintf(szFriendlyName, L"%s %s", VS_CaptureDeviceScreen::_nameScreenCapture, VS_CaptureDeviceScreen::_nameApplicationCapture);
				}
				m_pSourceList->iAddString(szFriendlyName);
				int dev_num = m_pSourceList->iFindString(szFriendlyName);
				m_bVFW[dev_num] = false;
				DeviceModeState DevMode;
				CModeList *pMode = new CModeListScreenCapture();
				DevMode.pModeList = pMode;
				DevMode.typeCapturer = CVideoCaptureList::CAPTURE_SCREEN;
				DevMode.typeDevice = CVideoCaptureList::DEVICE_SCREEN_MON;
				if (i == numOutputs) {
					DevMode.typeDevice = CVideoCaptureList::DEVICE_SCREEN_APP;
				}
				CModeListScreenCapture *pModeListScreenCapture = reinterpret_cast <CModeListScreenCapture*> (pMode);
				pModeListScreenCapture->iAddMode(desc);
				SetCaptureLevelMode(&DevMode, szFriendlyName);
				m_DeviceModeList.emplace(szFriendlyName, DevMode);
			}
		}
	}

	delete pDesktopCapture;
	return 0;
}

