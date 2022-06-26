#include "ScreenCapturerDesktopDuplication.h"

#include <Windows.h>

ScreenCapturerDesktopDuplication::ScreenCapturerDesktopDuplication()
{
	screenBuf_ = 0;
	buf_ = 0;
	captureBuf_ = 0;
	hdcMonitor_ = 0;
	hdcCapture_ = 0;
	hBmp_ = 0;
	reinit_ = false;
	m_bWin8OS = true;
}

ScreenCapturerDesktopDuplication::~ScreenCapturerDesktopDuplication()
{
	Reset();
}

bool ScreenCapturerDesktopDuplication::InitAct(int x, int y, size_t width, size_t height)
{
	bool res = capturer_.Init(x, y, width, height);
	if (res) {
		if (!bInitCursor_) {
			screenBuf_ = new unsigned char [width * height * 4];
			memset(screenBuf_, 0x50, width * height * 4);
		}
		captureBuf_ = new unsigned char [width * height * 4];
		memset(captureBuf_, 0x50, width * height * 4);
		capturer_.GetActiveDescOutput(&desc_);
		char name[128];
		wcstombs(name, desc_.name, 32);
		hdcMonitor_ = CreateDC(name, 0, 0, 0);
		hdcCapture_ = CreateCompatibleDC(hdcMonitor_);
		BITMAPINFO bmi;
		memset(&bmi, 0, sizeof(bmi));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = static_cast<LONG>(width);
		bmi.bmiHeader.biHeight = static_cast<LONG>(height);
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = 4 * width * height;
		hBmp_ = CreateDIBSection(hdcMonitor_, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&buf_), NULL, 0);
		if (bInitCursor_) {
			screenBuf_ = buf_;
		}
		DeleteObject(SelectObject(hdcCapture_, hBmp_));
		memset(buf_, 0x50, width * height * 4);
	}
	return res;
}

void ScreenCapturerDesktopDuplication::ResetAct()
{
	reinit_ = false;
	capturer_.Reset();
	if (!bInitCursor_) delete [] screenBuf_;
	screenBuf_ = 0;
	delete [] captureBuf_; captureBuf_ = 0;
	if (hBmp_) DeleteObject(hBmp_); hBmp_ = 0;
	if (hdcCapture_) DeleteDC(hdcCapture_); hdcCapture_ = 0;
	if (hdcMonitor_) DeleteDC(hdcMonitor_); hdcMonitor_ = 0;
}

bool ScreenCapturerDesktopDuplication::EnumerateOutputs()
{
	return capturer_.EnumerateOutputs();
}

size_t ScreenCapturerDesktopDuplication::GetNumOutputs()
{
	return capturer_.GetNumOutputs();
}

void ScreenCapturerDesktopDuplication::GetDescOutput(size_t numOutput, ScreenCapturerDesc *desc)
{
	capturer_.GetDescOutput(numOutput, desc);
}

bool ScreenCapturerDesktopDuplication::UpdateScreen() {
	bool res = true;
	if (!reinit_) {
		unsigned char *dst = (bInitCursor_) ? captureBuf_ : screenBuf_;
		int ret = capturer_.GetScreen(dst, width_ * height_ * 4);
		if (bInitCursor_) {
			memcpy(screenBuf_, captureBuf_, width_ * height_ * 4);
			DrawCursor(0, hdcCapture_, desc_.offsetX, desc_.offsetY, true);
		}
		if (ret == -2) {
			reinit_ = true;
		}
	} else {
		ResetAct();
		reinit_ = true;
		res = EnumerateOutputs();
		if (res) {
			res = InitAct(left_, top_, width_, height_);
		}
		if (res) {
			reinit_ = false;
		}
	}
	return res;
}