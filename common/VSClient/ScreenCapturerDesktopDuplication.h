#ifndef SCREEN_CAPTURER_DESKTOP_DUPLICATION_H
#define SCREEN_CAPTURER_DESKTOP_DUPLICATION_H

#include "BaseWinScreenCapturer.h"
#include "DesktopDuplicationCapturer.h"

class ScreenCapturerDesktopDuplication : public BaseWinScreenCapturer
{
public:

	ScreenCapturerDesktopDuplication();
	~ScreenCapturerDesktopDuplication();
	bool UpdateScreen();
	bool EnumerateOutputs();
	size_t GetNumOutputs();
	void GetDescOutput(size_t numOutput, ScreenCapturerDesc *desc);

private:

	DesktopDuplicationCapturer capturer_;
	HDC hdcMonitor_;
	HDC hdcCapture_;
	HBITMAP hBmp_;
	unsigned char *buf_;
	unsigned char *captureBuf_;
	ScreenCapturerDesc desc_;
	bool reinit_;

	virtual bool InitAct(int x, int y, size_t width, size_t height);
	virtual void ResetAct();
};

#endif
