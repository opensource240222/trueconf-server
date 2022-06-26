#include "DesktopDuplicationCapturer.h"

#include <d3d11_1.h>
#include <dxgi1_2.h>

const size_t DEFAULT_MAX_NUM_OUTPUTS = 32;

struct ScreenCapturerDesc
{
	int widthCaptureArea;
	int heightCaptureArea;
	int offsetX;
	int offsetY;
	wchar_t name[256];
};

struct MousePointer
{
	DXGI_OUTDUPL_POINTER_SHAPE_INFO ShapeInfo;
	bool visible;
	POINT position;
	BYTE *pShapeBuffer;
	UINT BufferSize;
public :
	MousePointer()
	{
		BufferSize = 0;
		pShapeBuffer = 0;
		position.x = 0;
		position.y = 0;
		memset(&ShapeInfo, 0, sizeof(DXGI_OUTDUPL_POINTER_SHAPE_INFO));
	}
	~MousePointer()
	{
		delete [] pShapeBuffer;
	}
};

struct DD_Impl
{
	IDXGIOutputDuplication *output;
	ID3D11Device *device;
	ID3D11DeviceContext *ctx;
	int offsX, offsY;
	RECT captureArea;
	RECT descArea;
	WCHAR DeviceName[32];
	bool interSect;
public:
	DD_Impl() : output(0), device(0), ctx(0) {}
	~DD_Impl()
	{
		if (output) output->Release();
		if (ctx) ctx->Release();
		if (device) device->Release();
	}
};

DesktopDuplicationCapturer::DesktopDuplicationCapturer() : width_(0), height_(0), size_(0), libDX_(0), libD3D_(0), pmouse_(0)
{
	libDX_ = LoadLibraryA("dxgi.dll");
	libD3D_ = LoadLibraryA("d3d11.dll");
}

DesktopDuplicationCapturer::~DesktopDuplicationCapturer()
{
	Reset();

	if (libDX_) {
		FreeLibrary(static_cast<HMODULE>(libDX_));
		libDX_ = 0;
	}

	if (libD3D_) {
		FreeLibrary(static_cast<HMODULE>(libD3D_));
		libD3D_ = 0;
	}
}

bool DesktopDuplicationCapturer::EnumerateOutputs()
{
	if (libDX_ && libD3D_) {
		HRESULT hr = S_OK;

		Reset();

		HRESULT(__stdcall *createFactory)(const IID &, void **);
		HRESULT(__stdcall *createDevice)(IDXGIAdapter *, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL *, UINT, UINT, ID3D11Device **, D3D_FEATURE_LEVEL *, ID3D11DeviceContext **);

		reinterpret_cast<FARPROC &>(createFactory) = GetProcAddress(static_cast<HMODULE>(libDX_), "CreateDXGIFactory1");
		reinterpret_cast<FARPROC &>(createDevice) = GetProcAddress(static_cast<HMODULE>(libD3D_), "D3D11CreateDevice");

		if (createFactory && createDevice) {
			IDXGIFactory1 *fact;
			hr = createFactory(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&fact));
			if ( SUCCEEDED(hr) ) {
				IDXGIAdapter1 *adapter;
				size_t numAdapter = 0;
				while (fact->EnumAdapters1(numAdapter++, &adapter) == S_OK) {
					IDXGIOutput *output;
					DXGI_OUTPUT_DESC desc;
					size_t numOutput = 0;
					while (adapter->EnumOutputs(numOutput++, &output) == S_OK) {
						output->GetDesc(&desc);

						DD_Impl *p = new DD_Impl();

						CopyRect(&(p->descArea), &desc.DesktopCoordinates);
						SetRect(&(p->captureArea), 0, 0,
								desc.DesktopCoordinates.right - desc.DesktopCoordinates.left,
								desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top);
						p->offsX = desc.DesktopCoordinates.left;
						p->offsY = desc.DesktopCoordinates.top;

						hr = createDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, D3D11_CREATE_DEVICE_SINGLETHREADED,
							NULL, 0, D3D11_SDK_VERSION, &p->device, NULL, &p->ctx);

						if ( !SUCCEEDED(hr) ) {
							Reset();
							output->Release();
							adapter->Release();
							fact->Release();
							return false;
						}

						IDXGIOutput1 *output1;
						HRESULT hres = output->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&output1));
						output->Release();
						if (hres != S_OK) {
							Reset();
							adapter->Release();
							fact->Release();
							return false;
						}

						hres = output1->DuplicateOutput(p->device, &p->output);
						output1->Release();
						if (hres != S_OK) {
							Reset();
							adapter->Release();
							fact->Release();
							return false;
						}

						wcsncpy_s(p->DeviceName, desc.DeviceName, 32);
						p->interSect = false;

						pimpl_.push_back(p);
						numOutputs_ = pimpl_.size();
					}
					adapter->Release();
				}
				fact->Release();
			}
		}

	}
	if (numOutputs_ == 0) {
		Reset();
	}
	return (numOutputs_ > 0);
}

size_t DesktopDuplicationCapturer::GetNumOutputs()
{
	return numOutputs_;
}

void DesktopDuplicationCapturer::GetDescOutput(size_t numOutput, ScreenCapturerDesc *desc)
{
	if (desc && numOutput < numOutputs_) {
		desc->widthCaptureArea = pimpl_[numOutput]->descArea.right - pimpl_[numOutput]->descArea.left;
		desc->heightCaptureArea = pimpl_[numOutput]->descArea.bottom - pimpl_[numOutput]->descArea.top;
		desc->offsetX = pimpl_[numOutput]->descArea.left;
		desc->offsetY = pimpl_[numOutput]->descArea.top;
		wcsncpy_s(desc->name, pimpl_[numOutput]->DeviceName, 32);
	}
}

bool DesktopDuplicationCapturer::Init(int x, int y, size_t width, size_t height)
{
	bool ret = false;
	// find out which outputs are needed for the given desktop rect
	RECT target = {x, y, x + width, y + height}, bufRect;

	for (size_t i = 0; i < numOutputs_; i++) {
		PRECT desc = &(pimpl_[i]->descArea);
		pimpl_[i]->interSect = false;
		// the output is for capture
		if (IntersectRect(&bufRect, &target, desc) != 0) {
			pimpl_[i]->interSect = true;
			bufRect.left -= desc->left;
			bufRect.right -= desc->left;
			bufRect.top -= desc->top;
			bufRect.bottom -= desc->top;
			pimpl_[i]->captureArea = bufRect;
			pimpl_[i]->offsX = desc->left + bufRect.left - x;
			pimpl_[i]->offsY = desc->top + bufRect.top - y;
			ret = true;
		}
	}

	if (ret) {
		pmouse_ = new MousePointer;
	}

	width_ = width; height_ = height;
	size_ = width * height * 4; // format is BGRA
	return ret;
}

void DesktopDuplicationCapturer::GetActiveDescOutput(ScreenCapturerDesc *desc)
{
	for (size_t i = 0; i < numOutputs_; i++) {
		if (pimpl_[i]->interSect) {
			GetDescOutput(i, desc);
		}
	}
}

void DesktopDuplicationCapturer::Reset()
{
	delete pmouse_;
	pmouse_ = 0;
	for (auto &it : pimpl_) {
		delete it;
	}
	pimpl_.clear();
	numOutputs_ = 0;
}

void DesktopDuplicationCapturer::GetMousePointer(IDXGIOutputDuplication *output, DXGI_OUTDUPL_FRAME_INFO *frameInfo)
{
	if (frameInfo->LastMouseUpdateTime.QuadPart) {
		pmouse_->position.x = frameInfo->PointerPosition.Position.x;
		pmouse_->position.y = frameInfo->PointerPosition.Position.y;
		pmouse_->visible = frameInfo->PointerPosition.Visible != 0;
	}
	if (frameInfo->PointerShapeBufferSize == 0) return;
	if (frameInfo->PointerShapeBufferSize > pmouse_->BufferSize) {
		delete [] pmouse_->pShapeBuffer;
		pmouse_->pShapeBuffer = 0;
		pmouse_->pShapeBuffer = new (std::nothrow) BYTE [frameInfo->PointerShapeBufferSize];
		if (!pmouse_->pShapeBuffer) {
			pmouse_->BufferSize = 0;
			return;
		}
		pmouse_->BufferSize = frameInfo->PointerShapeBufferSize;
	}
	UINT BufferSizeRequired;
	HRESULT hr = output->GetFramePointerShape(frameInfo->PointerShapeBufferSize, reinterpret_cast<VOID*>(pmouse_->pShapeBuffer), &BufferSizeRequired, &(pmouse_->ShapeInfo));
	if ( FAILED(hr) ) {
		delete [] pmouse_->pShapeBuffer;
		pmouse_->pShapeBuffer = 0;
		pmouse_->BufferSize = 0;
		return;
	}
}

int DesktopDuplicationCapturer::GetScreen(unsigned char *buf, size_t size)
{
	HRESULT hr = S_OK;
	int result = 0;
	if (pimpl_.empty() || !buf || size < size_)
		return -1;

	DXGI_OUTDUPL_FRAME_INFO frameInfo = {};
	IDXGIResource *desktop;

	for (size_t i = 0; i < numOutputs_; ++i) {
		if (pimpl_[i]->interSect) {
			IDXGIOutputDuplication *output = pimpl_[i]->output;
			hr = output->AcquireNextFrame(0, &frameInfo, &desktop);
			bool res = (hr == S_OK) && (frameInfo.AccumulatedFrames != 0);
			if (res) {
				res = false;
				//if (i == 0) GetMousePointer(output, &frameInfo);
				ID3D11Texture2D *src;
				if (desktop->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&src)) == S_OK) {
					RECT target = pimpl_[i]->captureArea;
					ID3D11Texture2D *dst;
					D3D11_TEXTURE2D_DESC desc = {};
					desc.Width = target.right - target.left;
					desc.Height = target.bottom - target.top;
					desc.MipLevels = 1;
					desc.ArraySize = 1;
					desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
					desc.SampleDesc.Count = 1;
					desc.SampleDesc.Quality = 0;
					desc.Usage = D3D11_USAGE_STAGING;
					desc.BindFlags = 0;
					desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
					desc.MiscFlags = 0;
					if (pimpl_[i]->device->CreateTexture2D(&desc, NULL, &dst) == S_OK) {
						D3D11_BOX copybox;
						copybox.left = target.left;
						copybox.top = target.top;
						copybox.right = target.right;
						copybox.bottom = target.bottom;
						copybox.front = 0;
						copybox.back = 1;
						pimpl_[i]->ctx->CopySubresourceRegion(dst, 0, 0, 0, 0, src, 0, &copybox);
						src->Release();
						desktop->Release();
						output->ReleaseFrame();
						D3D11_MAPPED_SUBRESOURCE mapped;
						hr = pimpl_[i]->ctx->Map(dst, 0, D3D11_MAP_READ, 0, &mapped);
						if (SUCCEEDED(hr)) {
							size_t height = target.bottom - target.top;
							size_t width = (target.right - target.left) * 4;
							unsigned char *pSrc = (unsigned char*)mapped.pData + (height - 1) * mapped.RowPitch;
							unsigned char *pDst = buf + pimpl_[i]->offsX * 4 + (height_ - height + pimpl_[i]->offsY) * width_ * 4;
							for (size_t j = 0; j < height; j++) {
								memcpy(pDst, pSrc, width);
								pSrc -= mapped.RowPitch;
								pDst += width_ * 4;
							}
							pimpl_[i]->ctx->Unmap(dst, 0);
							//if (pmouse_->visible) {
							//	DXGI_OUTDUPL_POINTER_SHAPE_INFO *si = &pmouse_->ShapeInfo;
							//	unsigned int *pDst = (unsigned int*)(buf + pmouse_->position.x * 4 + pmouse_->position.y * width_ * 4);
							//	switch (si->Type)
							//	{
							//		case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
							//		{
							//			int w = si->Width;
							//			int h = si->Height / 2;
							//			for (int row = 0; row < h; ++row) {
							//				BYTE mask = 0x80;
							//				for (int col = 0; col < w; ++col) {
							//					BYTE AndMask = pmouse_->pShapeBuffer[(col / 8) + row * si->Pitch] & mask;
							//					BYTE XorMask = pmouse_->pShapeBuffer[(col / 8) + ((row + (si->Height / 2)) * si->Pitch)] & mask;
							//					UINT AndMask32 = (AndMask) ? 0xFFFFFFFF : 0xFF000000;
							//					UINT XorMask32 = (XorMask) ? 0x00FFFFFF : 0x00000000;

							//					pDst[row*width_ + col] = (pDst[row*width_ + col] & AndMask32) ^ XorMask32;

							//					if (mask == 0x01) {
							//						mask = 0x80;
							//					} else {
							//						mask = mask >> 1;
							//					}
							//				}
							//			}
							//			break;
							//		}
							//		case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
							//		{
							//			break;
							//		}
							//		case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
							//		{
							//			break;
							//		}
							//	}
							//}
							res = true;
						}
						dst->Release();
					}
					else {
						src->Release();
						desktop->Release();
						output->ReleaseFrame();
					}
				}
				else {
					desktop->Release();
					output->ReleaseFrame();
				}
			}
			else {
				if (hr == S_OK) {
					desktop->Release();
				}
				output->ReleaseFrame();
				if (DXGI_ERROR_ACCESS_LOST == hr) {
					result = -2;
				}
			}
			output->ReleaseFrame();
		}
	}

	return result;
}
