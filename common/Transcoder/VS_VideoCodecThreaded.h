#pragma once

#include "std/cpplib/event.h"

#include <boost/signals2/signal.hpp>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

#include "LoadBalancing/BalancingModule.h"

#include "nvidia/include/cuda.h"

class VideoCodec;
class VS_MediaFormat;

namespace media_synch
{
	struct VideoBuffer;
}

class VS_VideoCodecThreaded : public balancing_module::BalancingModule
{

public:

	VS_VideoCodecThreaded();
	~VS_VideoCodecThreaded();
	bool Init(const VS_MediaFormat &mf);
	uint32_t GetScalableMode() const;
	void Release();
	bool ConfigAsync(int32_t bitrate, int32_t framerate);
	void ConvertAsync(std::shared_ptr<media_synch::VideoBuffer> in, bool key, uint32_t timestamp);

public:

	typedef boost::signals2::signal<void(uint8_t* /* frame */, int32_t /* size */, bool /* key */, uint32_t /* timestamp */, uint32_t /*fourcc*/)> PushFrameSignal;
	PushFrameSignal m_firePushFrameQueue;

private:

	void EncodeRoutine(uint32_t w, uint32_t h, uint32_t fourcc, bool svc);
	int32_t Encode(uint8_t *raw, uint8_t *encoded, bool &key, uint8_t &tl);
	void SendData(uint8_t *encoded, int32_t size, bool svc, bool key, uint8_t tl, uint64_t tm, uint32_t fourcc);

private:

	std::thread m_encodeThread;
	std::unique_ptr<VideoCodec> m_codec;
	std::shared_ptr<media_synch::VideoBuffer> m_srcFrame;
	std::mutex m_mtxEncode;
	std::mutex m_mtxSwap;
	vs::event m_checkData { false };
	std::atomic<bool> m_init { false };
	std::atomic<uint32_t> m_scalableMode { 0 };
	uint32_t m_timestamp = 0;
	bool m_key = false;

};
