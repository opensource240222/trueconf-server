#include "ScreenCapturer.h"

#include <cstring>

ScreenCapturer::ScreenCapturer() : left_(0), top_(0), width_(0), height_(0), m_bWin8OS(false)
{

}

ScreenCapturer::~ScreenCapturer()
{

}

bool ScreenCapturer::Init(int x, int y, size_t width, size_t height)
{
	Reset();

	EnumerateOutputs();

	bool res = width && height && InitAct(x, y, width, height);
	if (res)
	{
		left_ = x;
		top_ = y;
		width_ = width;
		height_ = height;
		type_ = ScreenCapturer::SCREEN_DISPLAY;
	}

	return res;
}

bool ScreenCapturer::Init(HWND hwnd)
{
	Reset();
	if (SetApplication(hwnd)) {
		type_ = ScreenCapturer::SCREEN_APPLICATION;
		return true;
	}
	Reset();
	return false;
}

bool ScreenCapturer::Init()
{
	Reset();

	GetScreenParams(left_, top_, width_, height_);

	if (InitAct(left_, top_, width_, height_)) {
		type_ = ScreenCapturer::SCREEN_DISPLAY;
		return true;
	}

	return false;
}

void ScreenCapturer::Reset()
{
	SetApplication(NULL);
	ResetAct();
	type_ = ScreenCapturer::SCREEN_NONE;
}

