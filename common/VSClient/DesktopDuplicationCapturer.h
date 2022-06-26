#ifndef DESKTOP_DUPLICATION_CAPTURER_H
#define DESKTOP_DUPLICATION_CAPTURER_H

#include <windows.h>
#include <vector>

struct ScreenCapturerDesc;
struct DXGI_OUTDUPL_FRAME_INFO;
struct IDXGIOutputDuplication;
struct DD_Impl;

class DesktopDuplicationCapturer
{
public:

	DesktopDuplicationCapturer();
	~DesktopDuplicationCapturer();

	bool Init(int x, int y, size_t width, size_t height);
	void Reset();
	int GetScreen(unsigned char *buf, size_t size);
	bool EnumerateOutputs();
	size_t GetNumOutputs();
	void GetDescOutput(size_t numOutput, ScreenCapturerDesc *desc);
	void GetActiveDescOutput(ScreenCapturerDesc *desc);

private:

	size_t width_, height_;
	size_t size_;
	void *libDX_, *libD3D_;
	size_t numOutputs_;
	std::vector <DD_Impl*> pimpl_;
	struct MousePointer *pmouse_;

	void GetMousePointer(IDXGIOutputDuplication *output, DXGI_OUTDUPL_FRAME_INFO *frameInfo);
};

#endif
