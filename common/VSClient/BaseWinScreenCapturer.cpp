
#include "BaseWinScreenCapturer.h"
#include "VS_Dmodule.h"
#include <Windows.h>
#include <Uxtheme.h>
#include <Dwmapi.h>

#include <algorithm>
#include <climits>

typedef BOOL (*ISCOMPOSITIONACTIVE)(void);
typedef HRESULT (WINAPI *DWMGETWINDOWATTRIBUTE)(HWND, DWORD, PVOID, DWORD);
ISCOMPOSITIONACTIVE g_IsCompositionActive = 0;
DWMGETWINDOWATTRIBUTE g_DwmGetWindowAttribute = 0;

HBITMAP getCursorHBITMAP(HBITMAP *maskBmp, POINT *pos, LPCTSTR lpCursorName = 0)
{
    CURSORINFO pci;
    ICONINFO iconinfo;
    HBITMAP result;
    pci.cbSize = sizeof(pci);
	if (!lpCursorName) {
		GetCursorInfo(&pci);
	} else {
		GetCursorPos(pos);
		pci.ptScreenPos = *pos;
		pci.hCursor = LoadCursor(0, lpCursorName);
	}
	if (pci.hCursor && GetIconInfo(pci.hCursor, &iconinfo)) {
        result = iconinfo.hbmColor;
		*pos = pci.ptScreenPos;
		pos->x -= iconinfo.xHotspot;
		pos->y -= iconinfo.yHotspot;
		if (maskBmp) {
            *maskBmp = iconinfo.hbmMask;
		}
	}  else {
        result = NULL;
	}
    return result;
}

BaseWinScreenCapturer::BaseWinScreenCapturer() : hWnd_(0), hdc_(0), hdcWnd_(0), hBmp_(0), threadApp_(0), processApp_(0),
												 buff_(0), copyOffset_(0), bInitCursor_(true)
{
	g_IsCompositionActive = 0;
	g_DwmGetWindowAttribute = 0;
	HINSTANCE lib = LoadLibrary("UxTheme.dll");
	if (lib) {
		(FARPROC &)g_IsCompositionActive = GetProcAddress(lib, "IsCompositionActive");
		FreeLibrary(lib);
	}
	lib = LoadLibrary("Dwmapi.dll");
	if (lib) {
		(FARPROC &)g_DwmGetWindowAttribute = GetProcAddress(lib, "DwmGetWindowAttribute");
		FreeLibrary(lib);
	}
};

BaseWinScreenCapturer::~BaseWinScreenCapturer()
{
	Reset();
};

void BaseWinScreenCapturer::Reset()
{
	ClearBuffers();
	if (hdcWnd_) DeleteObject(hdcWnd_); hdcWnd_ = 0;
	if (hdc_) ReleaseDC(hWnd_, hdc_); hdc_ = 0;
	threadApp_ = 0;
	processApp_ = 0;
	hWnd_ = 0;
}

void BaseWinScreenCapturer::ClearBuffers()
{
	buff_ = 0;
	if (hBmp_) DeleteObject(hBmp_); hBmp_ = 0;
	width_ = 0;
	height_ = 0;
	copyOffset_ = 0;
	if (hdc_) {
		delete [] screenBuf_; screenBuf_ = 0;
	}
}

void BaseWinScreenCapturer::GetScreenParams(int &x, int &y, size_t &width, size_t &height)
{
	if (EnumerateOutputs()) {
		size_t numOutputs = GetNumOutputs();
		ScreenCapturerDesc desc;
		int top		= INT_MAX;
		int left	= INT_MAX;
		int right	= INT_MIN;
		int bottom	= INT_MIN;
		for (size_t i = 0; i < numOutputs; i++) {
			GetDescOutput(i, &desc);
			left	= std::min(left, desc.offsetX);
			top		= std::min(top, desc.offsetY);
			right	= std::max(right, desc.offsetX + desc.widthCaptureArea);
			bottom	= std::max(bottom, desc.offsetY + desc.heightCaptureArea);
		}
		x = left;
		y = top;
		width = right - left;
		height = bottom - top;
	}
}

bool BaseWinScreenCapturer::GetMaxScreenParams(size_t &width, size_t &height)
{
	bool ret = false;
	if (EnumerateOutputs()) {
		size_t numOutputs = GetNumOutputs();
		ScreenCapturerDesc desc;
		width = 0;
		height = 0;
		size_t sqscreen = 0;
		for (size_t i = 0; i < numOutputs; i++) {
			GetDescOutput(i, &desc);
			size_t sq = desc.widthCaptureArea * desc.heightCaptureArea;
			if (sqscreen < sq) {
				width = desc.widthCaptureArea;
				height = desc.heightCaptureArea;
				sqscreen = sq;
			}
		}
		ret = (sqscreen != 0);
	}
	return ret;
}

bool BaseWinScreenCapturer::GetApplicationRect(HWND hwndApp, bool bAero, RECT *rectApp, POINT *offset)
{
	bool res = false;
	if (offset) {
		offset->x = 0;
		offset->y = 0;
	}
	if (g_DwmGetWindowAttribute && bAero) {
		HRESULT hr = g_DwmGetWindowAttribute(hwndApp, DWMWA_EXTENDED_FRAME_BOUNDS, rectApp, sizeof(RECT));
		if (hr == S_OK) {
			res = true;
			if (offset) {
				RECT rect;
				if (GetWindowRect(hwndApp, &rect) == TRUE) {
					offset->x = rectApp->left - rect.left;
					offset->y = rectApp->top - rect.top;
				}
			}
		}
	} else {
		res = (GetWindowRect(hwndApp, rectApp) == TRUE);
	}
	return res;
}

void BaseWinScreenCapturer::DetectOverlapWindow(HWND hwndApp, RECT rectApp, DWORD threadApp, DWORD processApp, bool bAero)
{
	HWND hwnd = hwndApp;
	while ( (hwnd = GetNextWindow(hwnd, GW_HWNDPREV)) != NULL ) {
		if (IsWindowVisible(hwnd)) {
			RECT rect;
			bool res = GetApplicationRect(hwnd, bAero, &rect);
			if (res && (IsRectEmpty(&rect) == FALSE)) {
				RECT dst;
				if (IntersectRect(&dst, &rectApp, &rect) == TRUE) {
					DWORD dwExStyle = (DWORD)GetWindowLong(hwnd, GWL_EXSTYLE);
					DWORD pid = 0, tid = 0;
					tid = GetWindowThreadProcessId(hwnd, &pid);
					if ((dwExStyle & WS_EX_TOOLWINDOW) && (pid == processApp && tid == threadApp)) {
						if (bAero) {
							//DTRACE(VSTM_NHP_OTHER, "pid = %x(%x), pidapp = %x(%x),app = %x, p = %x, ex = %x", pid, tid, processApp, threadApp, hwndApp, GetParent(hwnd), dwExStyle);
							HDC dc = GetWindowDC(hwnd);
							if (dc) {
								OffsetRect(&dst, -rectApp.left, -rectApp.top);
								int w = dst.right - dst.left;
								int h = dst.bottom - dst.top;
								int xoff = std::max<int>(rectApp.left - rect.left, 0);
								int yoff = std::max<int>(rectApp.top - rect.top, 0);
								BitBlt(hdcWnd_, dst.left, dst.top, w, h, dc, xoff, yoff, SRCCOPY);
								ReleaseDC(hwnd, dc);
							}
						}
					} else {
						if (!bAero) {
							//DTRACE(VSTM_NHP_OTHER, "pid = %x(%x), pidapp = %x(%x),app = %x, p = %x, ex = %x", pid, tid, processApp, threadApp, hwndApp, GetParent(hwnd), dwExStyle);
							OffsetRect(&dst, -rectApp.left, -rectApp.top);
							int pitch_copy = (dst.right - dst.left) * 4;
							int pitch_src = (rectApp.right - rectApp.left) * 4;
							int height_src = rectApp.bottom - rectApp.top;
							unsigned char *pbuff = buff_ + dst.left * 4 + (height_src - dst.top - 1) * pitch_src;
							for (int j = dst.top; j < dst.bottom; j++) {
								memset(pbuff, 0x50, pitch_copy);
								pbuff -= pitch_src;
							}
							if (EqualRect(&dst, &rectApp) == TRUE) break;
						}
					}
					//if (pid != processApp && tid != threadApp) {
					//	if (!bAero) {
					//		//DTRACE(VSTM_NHP_OTHER, "pid = %x(%x), pidapp = %x(%x),app = %x, p = %x, ex = %x", pid, tid, processApp, threadApp, hwndApp, GetParent(hwnd), dwExStyle);
					//		OffsetRect(&dst, -rectApp.left, -rectApp.top);
					//		int pitch_copy = (dst.right - dst.left) * 4;
					//		int pitch_src = (rectApp.right - rectApp.left) * 4;
					//		int height_src = rectApp.bottom - rectApp.top;
					//		unsigned char *pbuff = buff_ + dst.left * 4 + (height_src - dst.top - 1) * pitch_src;
					//		for (int j = dst.top; j < dst.bottom; j++) {
					//			memset(pbuff, 0x50, pitch_copy);
					//			pbuff -= pitch_src;
					//		}
					//		if (EqualRect(&dst, &rectApp) == TRUE) break;
					//	}
					//} else {
					//	if (bAero && (dwExStyle & WS_EX_LAYERED) == 0x0) {
					//		//DTRACE(VSTM_NHP_OTHER, "pid = %x(%x), pidapp = %x(%x),app = %x, p = %x, ex = %x", pid, tid, processApp, threadApp, hwndApp, GetParent(hwnd), dwExStyle);
					//		HDC dc = GetWindowDC(hwnd);
					//		if (dc) {
					//			OffsetRect(&dst, -rectApp.left, -rectApp.top);
					//			int w = dst.right - dst.left;
					//			int h = dst.bottom - dst.top;
					//			int xoff = max((rectApp.left - rect.left), 0);
					//			int yoff = max((rectApp.top - rect.top), 0);
					//			BitBlt(hdcWnd_, dst.left, dst.top, w, h, dc, xoff, yoff, SRCCOPY);
					//			ReleaseDC(hwnd, dc);
					//		}
					//	}
					//}
				}
			}
		}
	}
}

void BaseWinScreenCapturer::DrawCursor(HWND hwndSrc, HDC hdcDest, int offX, int offY, bool bAero)
{
	if (bInitCursor_) {
		POINT pos;
		BITMAP bm;
		HBITMAP hCursorBmp, hCursorMaskBmp;
		hCursorBmp = getCursorHBITMAP(&hCursorMaskBmp, &pos, 0);
		if (!hCursorBmp || !hCursorMaskBmp) {
			if (hCursorBmp) DeleteObject(hCursorBmp); hCursorBmp = 0;
			if (hCursorMaskBmp) DeleteObject(hCursorMaskBmp); hCursorMaskBmp = 0;
			hCursorBmp = getCursorHBITMAP(&hCursorMaskBmp, &pos, IDC_ARROW);
		}
		if (hCursorBmp && hCursorMaskBmp) {
			bool drawCursor = true;
			if (hwndSrc) {
				drawCursor = false;
				RECT rect;
				if (GetApplicationRect(hwndSrc, bAero, &rect)) {
					drawCursor = (pos.x >= rect.left && pos.y >= rect.top && pos.x <= rect.right && pos.y <= rect.bottom);
					pos.x -= rect.left;
					pos.y -= rect.top;
				}
			} else {
				pos.x -= offX;
				pos.y -= offY;
			}
			if (drawCursor) {
				HDC hdcCursor = CreateCompatibleDC(hdcDest);
				if (hdcCursor) {
					HBITMAP oldBm = (HBITMAP)SelectObject(hdcCursor, hCursorBmp);
					GetObject(hCursorBmp, sizeof(bm), &bm);
					MaskBlt(hdcDest, pos.x, pos.y, bm.bmWidth, bm.bmHeight, hdcCursor, 0, 0, hCursorMaskBmp, 0, 0, MAKEROP4(SRCPAINT, SRCCOPY));
					SelectObject(hdcCursor, oldBm);
					DeleteDC(hdcCursor);
				}
			}
		}
		if (hCursorBmp) DeleteObject(hCursorBmp); hCursorBmp = 0;
		if (hCursorMaskBmp) DeleteObject(hCursorMaskBmp); hCursorMaskBmp = 0;
	}
}

void BaseWinScreenCapturer::CreateBuffers()
{

}

bool BaseWinScreenCapturer::SetApplication(HWND hwnd)
{
	bool res = false;
	Reset();
	if (!hwnd) return true;
	ResetAct();
	hWnd_ = hwnd;
	if (hWnd_) {
		threadApp_ = GetWindowThreadProcessId(hWnd_, &processApp_);
		hdc_ = GetWindowDC(hWnd_);
		if (hdc_) {
			hdcWnd_ = CreateCompatibleDC(hdc_);
			if (hdcWnd_) {
				RECT rc;
				bool bAero = (g_IsCompositionActive && (g_IsCompositionActive() == TRUE));
				res = GetApplicationRect(hWnd_, bAero, &rc);
				if (res) {
					width_ = rc.right - rc.left;
					height_ = rc.bottom - rc.top;
					BITMAPINFO bmi;
					memset(&bmi, 0, sizeof(bmi));
					bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					bmi.bmiHeader.biWidth = static_cast<LONG>(width_);
					bmi.bmiHeader.biHeight = static_cast<LONG>(height_);
					bmi.bmiHeader.biPlanes = 1;
					bmi.bmiHeader.biBitCount = 32;
					bmi.bmiHeader.biCompression = BI_RGB;
					bmi.bmiHeader.biSizeImage = 4 * bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight;
					hBmp_ = CreateDIBSection(hdc_, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&buff_), NULL, 0);
					if (hBmp_) {
						DeleteObject(SelectObject(hdcWnd_, hBmp_));
					}
					screenBuf_ = new unsigned char[width_ * height_ * 4];
					memset(screenBuf_, 0x50, width_ * height_ * 4);
					if (bAero) PrintWindow(hWnd_, hdc_, 0);
				}
			}
		}
	}
	if (!res) {
		Reset();
	}
	return (hdcWnd_ != 0);
}

bool BaseWinScreenCapturer::UpdateApplication()
{
	bool ret = false;
	if (hBmp_) {
		RECT rc;
		POINT offset;
		bool bAero = (g_IsCompositionActive && (g_IsCompositionActive() == TRUE));
		bool res = GetApplicationRect(hWnd_, bAero, &rc, &offset);
		if (res) {
			int w = rc.right - rc.left;
			int h = rc.bottom - rc.top;
			if (width_ != w || height_ != h) {
				if (hdc_) {
					ClearBuffers();
					width_ = w;
					height_ = h;
					BITMAPINFO bmi;
					memset(&bmi, 0, sizeof(bmi));
					bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					bmi.bmiHeader.biWidth = static_cast<LONG>(w);
					bmi.bmiHeader.biHeight = static_cast<LONG>(h);
					bmi.bmiHeader.biPlanes = 1;
					bmi.bmiHeader.biBitCount = 32;
					bmi.bmiHeader.biCompression = BI_RGB;
					bmi.bmiHeader.biSizeImage = 4 * bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight;
					hBmp_ = CreateDIBSection(hdc_, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&buff_), NULL, 0);
					if (hBmp_) {
						DeleteObject(SelectObject(hdcWnd_, hBmp_));
					}
					if (!hBmp_) {
						ClearBuffers();
						return false;
					}
					screenBuf_ = new unsigned char [w * h * 4];
					memset(screenBuf_, 0x50, w * h * 4);
					if (bAero) PrintWindow(hWnd_, hdc_, 0);
				}
			}
			HBITMAP oldBm = (HBITMAP)SelectObject(hdcWnd_, hBmp_);
			BitBlt(hdcWnd_, 0, 0, width_, height_, hdc_, offset.x, offset.y, SRCCOPY);
			SelectObject(hdcWnd_, oldBm);
			DetectOverlapWindow(hWnd_, rc, threadApp_, processApp_, (bAero || m_bWin8OS));
			DrawCursor(hWnd_, hdcWnd_, 0, 0, bAero);
			memcpy(screenBuf_, buff_, width_ * height_ * 4);
			ret = true;
		}
	}
	return ret;
}

