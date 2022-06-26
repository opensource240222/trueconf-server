#pragma once

#include "VideoCodec.h"
#include "nvidia/nvenc/NvEncoder.h"
#include "nvidia/nvdec/NvDecoder.h"
#include "nvidia/include/cuda.h"

#include <deque>
#include <memory>

class VS_NvidiaVideoCodec : public VideoCodec
{

protected:

	CUcontext m_cuContext;
	bool m_externalContext;

private:

	bool InitCuda(int32_t deviceID);
	virtual bool CheckCuda();

public:

	VS_NvidiaVideoCodec(int CodecId, bool IsCoder);
	VS_NvidiaVideoCodec(int CodecId, bool IsCoder, int deviceId);
	virtual ~VS_NvidiaVideoCodec();
};

class VS_NvidiaVideoEncoder : public VS_NvidiaVideoCodec
{

private:

	NvEncoder *m_pEncoder = nullptr;
	std::deque<std::vector<uint8_t>> m_encodeFrames;
	bool m_requestKeyFrame;

protected:

	bool TryReconfigure(bool key);

public:

	VS_NvidiaVideoEncoder(int CodecId);
	VS_NvidiaVideoEncoder(int CodecId, int deviceId);
	virtual ~VS_NvidiaVideoEncoder();
	virtual int Init(int w, int h, uint32_t ColorMode = 0, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10) override;
	virtual void Release() override;
	virtual int Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param) override;
	virtual bool SetSVCMode(uint32_t &param) override;

};

class VS_NvidiaVideoDecoder : public VS_NvidiaVideoCodec
{

private:

	struct DecodingFrame
	{
		uint8_t* buffer = nullptr;
		int32_t size = 0;
		CUcontext context = 0;
		DecodingFrame(uint8_t *src, int32_t s, CUcontext ctx, CUstream stream) : context(ctx), size(s)
		{
			if (context) {
				cuCtxPushCurrent(context);
				cuMemAlloc((CUdeviceptr*) &buffer, s);
				cuMemcpyDtoDAsync((CUdeviceptr) buffer, (CUdeviceptr) src, s, stream);
				cuStreamSynchronize(stream);
				cuCtxPopCurrent(NULL);
			}
			else {
				buffer = new uint8_t[s];
				memcpy(buffer, src, s);
			}
		}
		~DecodingFrame()
		{
			if (context) {
				if (buffer) {
					cuCtxPushCurrent(context);
					cuMemFree((CUdeviceptr) buffer);
					cuCtxPopCurrent(NULL);
				}
			}
			else {
				delete[] buffer;
			}
		}
	};

	NvDecoder * m_pDecoder = nullptr;
	std::deque<DecodingFrame> m_decodeFrames;
	int m_numDecodeFrames = 0;
	CUstream m_cuvidStream = 0;
	bool m_bDeviceMemory = false;

protected:

public:

	VS_NvidiaVideoDecoder(int CodecId);
	VS_NvidiaVideoDecoder(int CodecId, int deviceId);
	virtual ~VS_NvidiaVideoDecoder();
	virtual int Init(int w, int h, uint32_t ColorMode = 0, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10) override;
	virtual int InitExtended(const base_Param &settings) override;
	virtual void Release() override;
	virtual int Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param) override;

private:

	int InitInternal(int w, int h, uint32_t ColorMode, bool deviceMemory, Dim *resizeRect);

};
