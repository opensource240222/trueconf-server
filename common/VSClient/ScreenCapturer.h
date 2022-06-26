#ifndef SCREEN_CAPTURER_H
#define SCREEN_CAPTURER_H

#include <stddef.h>
#include <windows.h>

struct ScreenCapturerDesc
{
	int widthCaptureArea;
	int heightCaptureArea;
	int offsetX;
	int offsetY;
	wchar_t name[256];
};

class ScreenCapturer
{
public:

	enum eScreenType
	{
		SCREEN_NONE = 0,
		SCREEN_DISPLAY = 1,
		SCREEN_APPLICATION,
	};

	ScreenCapturer();
	virtual ~ScreenCapturer();

	// Init without parameters is for capturing the entire desktop area
	bool Init(int x, int y, size_t width, size_t height);
	bool Init(HWND hwnd);
	bool Init();
	bool Inited() const {return screenBuf_ != 0;}
	ScreenCapturer::eScreenType Type() { return type_; }
	void Reset();
	const unsigned char* GetScreen() const {return screenBuf_;}
	int Left() const {return left_;}
	int Top() const {return top_;}
	size_t Width() const {return width_;}
	size_t Height() const {return height_;}
	virtual bool UpdateScreen() = 0;
	virtual bool UpdateApplication() = 0;
	virtual bool GetMaxScreenParams(size_t &width, size_t &height) = 0;
	virtual bool EnumerateOutputs() { return false; }
	virtual size_t GetNumOutputs() { return 0; }
	virtual void GetDescOutput(size_t numOutput, ScreenCapturerDesc *desc) {};

protected:

	int left_, top_;
	size_t width_, height_;
	unsigned char *screenBuf_;
	ScreenCapturer::eScreenType type_;
	bool m_bWin8OS;

private:

	virtual bool InitAct(int x, int y, size_t width, size_t height) = 0;
	virtual void ResetAct() = 0;
	virtual void GetScreenParams(int &x, int &y, size_t &width, size_t &height) = 0;
	virtual bool SetApplication(HWND hwnd) = 0;
};

#endif
