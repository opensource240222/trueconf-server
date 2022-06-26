#pragma once

#include <cinttypes>

#ifdef _WIN32
#include "nvidia/include/cuda.h"
#endif

namespace media_synch
{
	struct VideoBuffer
	{
		uint8_t* buffer = nullptr;
		uint8_t* device = nullptr;
		int32_t input_width = 0;
		int32_t input_height = 0;
		int32_t width = 0;
		int32_t height = 0;
		int32_t size = 0;
		std::uintptr_t context = 0;
		VideoBuffer(int32_t s, int32_t w, int32_t h, std::uintptr_t ctx = 0)
			: width(w), height(h), size(s), context(ctx)
		{
			if (context) {
#ifdef _WIN32
				cuCtxPushCurrent((CUcontext)context);
				cuMemAlloc((CUdeviceptr*)&device, s);
				cuCtxPopCurrent(NULL);
#endif
			}
			buffer = new uint8_t[s];
		}
		~VideoBuffer()
		{
			if (context) {
				if (device) {
#ifdef _WIN32
					cuCtxPushCurrent((CUcontext)context);
					cuMemFree((CUdeviceptr)device);
					cuCtxPopCurrent(NULL);
#endif
				}
			}
			delete[] buffer;
		}
		VideoBuffer(const VideoBuffer&) = delete;
		VideoBuffer& operator=(const VideoBuffer&) = delete;
	};
}