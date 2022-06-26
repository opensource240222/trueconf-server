#ifndef SCREEN_CAPTURER_GDI_H
#define SCREEN_CAPTURER_GDI_H

#include "BaseWinScreenCapturer.h"
#include <windows.h>
#include <vector>

struct Impl;

class ScreenCapturerGDI : public BaseWinScreenCapturer
{
public:

	ScreenCapturerGDI();
	~ScreenCapturerGDI();
	bool UpdateScreen();
	bool EnumerateOutputs();
	size_t GetNumOutputs();
	void GetDescOutput(size_t numOutput, ScreenCapturerDesc *desc);
	void SetDescOutput(RECT *rectMonitor, char *deviceName);

private:

	std::vector <Impl*> pimpl_;
	HDC hdcScreen_;
	HDC hdcCapture_;

	virtual bool InitAct(int x, int y, size_t width, size_t height);
	virtual void ResetAct();
};

#endif