#ifndef BASE_WIN_SCREEN_CAPTURER_H
#define BASE_WIN_SCREEN_CAPTURER_H

#include "ScreenCapturer.h"

class BaseWinScreenCapturer : public ScreenCapturer
{
public :

	BaseWinScreenCapturer();
	virtual ~BaseWinScreenCapturer();
	void Reset();
	bool GetApplication(int &x, int &y, size_t &width, size_t &height);
	bool UpdateApplication();
	bool GetMaxScreenParams(size_t &width, size_t &height);

protected:

	void DrawCursor(HWND hwndSrc, HDC hdcDest, int offX, int offY, bool bAero);

private:

	void GetScreenParams(int &x, int &y, size_t &width, size_t &height);
	bool SetApplication(HWND hwnd);
	void ClearBuffers();
	void CreateBuffers();
	void DetectOverlapWindow(HWND hwndApp, RECT rectAppp, DWORD threadApp, DWORD processApp, bool bAero);
	bool GetApplicationRect(HWND hwndApp, bool bAero, RECT *rectApp, POINT *offset = nullptr);
	virtual void ResetAct() {};

protected:

	bool bInitCursor_;
	DWORD processApp_, threadApp_;
	HWND hWnd_;
	HDC hdc_;
	HDC hdcWnd_;
	HBITMAP hBmp_;
	int copyOffset_;
	unsigned char *buff_;
};

#endif
