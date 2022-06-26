#include "ScreenCapturerGDI.h"

struct Impl
{
	RECT descArea;
	RECT captureArea;
	int offsetX;
	int offsetY;
	int shiftX;
	int shiftY;
	HBITMAP hBmp;
	unsigned char *buff;
	wchar_t name[128];
	bool interSect;
public :
	Impl() : hBmp(0) {};
	~Impl()
	{
		if (hBmp) DeleteObject(hBmp);
	}
};

BOOL CALLBACK MonitorEnumScreenCapture(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	ScreenCapturerGDI *pCapturer = reinterpret_cast <ScreenCapturerGDI*> (dwData);
	MONITORINFOEX mi;
	mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &mi);
	pCapturer->SetDescOutput(lprcMonitor, mi.szDevice);
	return TRUE;
}

ScreenCapturerGDI::ScreenCapturerGDI()
{
	screenBuf_ = 0;
	hdcScreen_ = 0;
	hdcCapture_ = 0;
}

ScreenCapturerGDI::~ScreenCapturerGDI()
{
	Reset();
}

bool ScreenCapturerGDI::EnumerateOutputs()
{
	Reset();
	BOOL ret = EnumDisplayMonitors(NULL, NULL, MonitorEnumScreenCapture, (LPARAM)this);
	return true;
}

size_t ScreenCapturerGDI::GetNumOutputs()
{
	return pimpl_.size();
}

void ScreenCapturerGDI::SetDescOutput(RECT *rectMonitor, char *deviceName)
{
	Impl *p = new Impl;

	mbstowcs(p->name, deviceName, 32);
	CopyRect(&(p->descArea), rectMonitor);
	p->captureArea.left = 0;
	p->captureArea.right = rectMonitor->right - rectMonitor->left;
	p->captureArea.top = 0;
	p->captureArea.bottom = rectMonitor->bottom - rectMonitor->top;
	p->offsetX = rectMonitor->left;
	p->offsetY = rectMonitor->top;
	p->shiftX = rectMonitor->left;
	p->shiftY = rectMonitor->top;

	pimpl_.push_back(p);
}

void ScreenCapturerGDI::GetDescOutput(size_t numOutput, ScreenCapturerDesc *desc)
{
	if (desc && numOutput < pimpl_.size()) {
		desc->offsetX = pimpl_[numOutput]->offsetX;
		desc->offsetY = pimpl_[numOutput]->offsetY;
		desc->widthCaptureArea = pimpl_[numOutput]->descArea.right - pimpl_[numOutput]->descArea.left;
		desc->heightCaptureArea = pimpl_[numOutput]->descArea.bottom - pimpl_[numOutput]->descArea.top;
		wcsncpy_s(desc->name, pimpl_[numOutput]->name, 32);
	}
}

bool ScreenCapturerGDI::InitAct(int x, int y, size_t width, size_t height)
{
	if (pimpl_.size() > 0) {
		if ( (hdcScreen_ = GetDC(NULL)) == NULL) {
			return false;
		}
		if ( (hdcCapture_ = CreateCompatibleDC(hdcScreen_)) == NULL) {
			ReleaseDC(NULL, hdcScreen_);
			hdcScreen_ = 0;
			return false;
		}
	}
	bool ret = false;
	RECT target = {x, y, x + width, y + height}, bufRect;
	for (size_t i = 0; i < pimpl_.size(); i++) {
		PRECT desc = &(pimpl_[i]->descArea);
		pimpl_[i]->interSect = false;
		if (IntersectRect(&bufRect, &target, desc) != 0) {
			pimpl_[i]->interSect = true;
			bufRect.left -= desc->left;
			bufRect.right -= desc->left;
			bufRect.top -= desc->top;
			bufRect.bottom -= desc->top;
			pimpl_[i]->captureArea = bufRect;
			pimpl_[i]->offsetX = desc->left + bufRect.left - x;
			pimpl_[i]->offsetY = desc->top + bufRect.top - y;
			ret = true;

			BITMAPINFO bmi;
			memset(&bmi, 0, sizeof(bmi));
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = static_cast<LONG>(width);
			bmi.bmiHeader.biHeight = static_cast<LONG>(height);
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biSizeImage = 4 * width * height;

			pimpl_[i]->hBmp = CreateDIBSection(hdcScreen_, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&(pimpl_[i]->buff)), NULL, 0);
		}
	}

	if (ret) {
		screenBuf_ = new unsigned char [width * height * 4];
		memset(screenBuf_, 0x50, width * height * 4);
		for (size_t i = 0; i < pimpl_.size(); i++) {
			DeleteObject(SelectObject(hdcScreen_, pimpl_[i]->hBmp));
		}
	}

	return true;
}

void ScreenCapturerGDI::ResetAct()
{
	for (size_t i = 0; i < pimpl_.size(); i++) {
		delete pimpl_[i];
	}
	pimpl_.clear();
	delete [] screenBuf_; screenBuf_ = 0;
	if (hdcCapture_) DeleteDC(hdcCapture_); hdcCapture_ = 0;
	if (hdcScreen_) ReleaseDC(NULL, hdcScreen_); hdcScreen_ = 0;
}

bool ScreenCapturerGDI::UpdateScreen()
{
	bool ret = false;
	for (size_t i = 0; i < pimpl_.size(); ++i) {
		if (pimpl_[i]->interSect) {
			HBITMAP oldBm = (HBITMAP)SelectObject(hdcCapture_, pimpl_[i]->hBmp);
			BitBlt(hdcCapture_, 0, 0, width_, height_, hdcScreen_, pimpl_[i]->shiftX, pimpl_[i]->shiftY, SRCCOPY);
			SelectObject(hdcCapture_, oldBm);
			DrawCursor(NULL, hdcCapture_, pimpl_[i]->descArea.left, pimpl_[i]->descArea.top, true);
			memcpy(screenBuf_, pimpl_[i]->buff, width_ * height_ * 4);
			ret = true;
		}
	}
	return ret;
}
