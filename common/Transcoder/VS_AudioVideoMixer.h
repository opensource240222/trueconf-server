#pragma once

#include "std-generic/compat/memory.h"
#include <deque>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/signals2.hpp>

#include "VSAudioVad.h"
#include "std/cpplib/EventArray.h"
#include "std/cpplib/event.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"
#include "VS_AVMixerIface.h"
#include "std/VS_TimeDeviation.h"
#include "std/VS_FifoBuffer.h"
#include "streams/Protocol.h"
#include "IppLib2/VSVideoProcessingIpp.h"
#include "AudioCodec.h"
#include "std-generic/cpplib/AtomicCache.h"
#include "std-generic/cpplib/string_view.h"
#include "nvidia/include/cuda.h"
#include "VideoBuffer.h"
#include "mixer/VS_MultiMixerVideo.h"
#include "mixer/VS_MultiMixerAudio.h"

#define NAME_ON (0x01)

class VS_VideoCodecThreaded;
class VS_FfmpegResampler;

namespace vs
{
	class ASIOThreadPool;
}

namespace avmuxer_cache
{

	const unsigned maxCacheLenght = 30;

	template <typename T>
	struct return_to_cache_video
	{
		typedef vs::AtomicCache<T*, std::default_delete<T>, maxCacheLenght> AtomicCacheVideo;
	private:
		std::weak_ptr<AtomicCacheVideo> m_cache;
	public:
		return_to_cache_video() { }
		return_to_cache_video(const std::shared_ptr<AtomicCacheVideo> &cache) : m_cache(cache) { }
		void operator() (T* vb) const { auto cache = m_cache.lock(); if (cache) cache->Put(vb); }
	};

}

namespace media_synch
{
	typedef vs::AtomicCache<VideoBuffer*, std::default_delete<VideoBuffer>, avmuxer_cache::maxCacheLenght> AtomicCacheVideo;

	struct VideoProcessingItem
	{
		std::shared_ptr<AtomicCacheVideo> cache;
		int32_t width = 0;
		int32_t height = 0;
		std::set<std::uintptr_t> hmixer;
		std::shared_ptr<VSVideoProcessingIpp> imageProc;
		VideoProcessingItem() { }
		~VideoProcessingItem() { }
	};

	struct AudioProcessingItem
	{
		std::shared_ptr<uint8_t> frame;
		int32_t size = 0;
		int32_t sr = 0;
		std::set<std::uintptr_t> hmixer;
		std::shared_ptr<VS_FfmpegResampler> rsmpl;
		AudioProcessingItem() { }
		~AudioProcessingItem() { }
	};

}

namespace avmuxer {

	typedef boost::signals2::signal<void(uint32_t baseBitrate, uint32_t maxBitrate, int32_t framerate)> SetBitrateSignal;
	typedef boost::signals2::signal<void()> RequestKeyFrameSignal;

	template <typename T>
	struct MixerItem
	{
		std::map<uint32_t /* mbframe or samplerate */, std::shared_ptr<T>> codecItem;
		int32_t numPeers = 0;
	};

	struct VideoItem
	{
		std::shared_ptr<VS_VideoCodecThreaded> vcm;
		unsigned int set_bitrate = 0;
		unsigned int set_framerate = 0;
		unsigned int last_key_frame_time = 0;
		bool key_frame_requested = false;
		bool block = true;
	};

	struct VideoResampleItem
	{
		std::shared_ptr<media_synch::AtomicCacheVideo> cache;
		std::shared_ptr<VSVideoProcessingIpp> imageProc;
		int32_t w = 0;
		int32_t h = 0;
		bool block = true;
	};

	struct VideoPool
	{
		std::map<uint32_t /* fourcc */, std::shared_ptr<MixerItem<VideoItem>>> mixerItem;
		std::map<uint32_t /* mbframe */, avmuxer::VideoResampleItem, std::greater<uint32_t>> resampleItem;
		std::shared_ptr<VS_MultiMixerVideo> mixer;
		std::shared_ptr<uint8_t> mixerFrame;
		std::mutex mtx;
	};

	struct AudioItem
	{
		AudioCodec *cdc = nullptr;
		VS_FifoBuffer *fifo = nullptr;
		std::pair <uint8_t*, int32_t> fifoframe = std::make_pair(nullptr, 0);
		std::pair <uint8_t*, int32_t*> mixframe = std::make_pair(nullptr, nullptr);
		uint8_t *cdcframe = nullptr;
		bool block = true;
	public:
		~AudioItem()
		{
			delete cdc;
			delete fifo;
			delete[] fifoframe.first;
			delete[] cdcframe;
		}
	};

	struct AudioResampleItem
	{
		std::shared_ptr<VS_FfmpegResampler> rsmpl;
		std::shared_ptr<uint8_t> frame;
		int32_t size = 0;
		bool block = true;
	};

	struct AudioPool
	{
		std::map<uint32_t /* twocc */, std::shared_ptr<MixerItem<AudioItem>>> mixerItem;
		std::map<uint32_t /* samplerate */, avmuxer::AudioResampleItem> resampleItem;
		std::shared_ptr<VS_MultiMixerAudio> mixer;
		std::shared_ptr<uint8_t> mixerFrame;
		int32_t frameSize = 0;
		std::mutex mtx;
	};

	struct LayoutItem
	{
		avmuxer::LayoutFormat lf;
		std::shared_ptr<avmuxer::VideoPool> videoPool;
		std::shared_ptr<avmuxer::AudioPool> audioPool;
		std::shared_ptr<boost::asio::io_service::strand> strand;
	};

	struct FrameVideoThread
	{
		std::unique_ptr<uint8_t[]> buffer;
		bool			keyframe = false;
		uint32_t		timestamp = 0;
		int32_t			size = 0;
		uint32_t		svcmode = 0;
		LayoutPayload	payload;
	public:
		FrameVideoThread() {}
		FrameVideoThread(uint8_t *p, int32_t s, uint32_t svc, bool key, uint32_t t, LayoutPayload pl)
			: keyframe(key), timestamp(t), size(s), svcmode(svc), payload(pl)
		{
			buffer = vs::make_unique_default_init<uint8_t[]>(s);
			if (buffer) {
				memcpy(buffer.get(), p, s);
			}
		}
		~FrameVideoThread() {};
	};

}

namespace mixer_receiver
{

	inline std::string GetUniqueId(string_view id, stream::Track track)
	{
		return std::string(id) + "-<%%>-" + std::to_string(stream::id(track));
	}

	struct StreamInfo
	{
		int					state = 0;
		VS_MediaFormat		mf;
		std::string			name;
	};

	struct PushFrame
	{
		std::unique_ptr<uint8_t[]>	m_buff;
		int32_t						m_size;
		stream::Track				m_track;
		PushFrame() : m_size(0), m_track()
		{
		}
		PushFrame(uint8_t* buff, int32_t size, stream::Track track)
		{
			m_buff = vs::make_unique_default_init<uint8_t[]>(size);
			memcpy(m_buff.get(), buff, size);
			m_size = size;
			m_track = track;
		}
		~PushFrame()
		{
		};
	};

	struct VideoBuffer
	{
		uint8_t* buffer = nullptr;
		int32_t size = 0;
		int32_t width = 0;
		int32_t height = 0;
		uint64_t timestamp = 0;
		std::uintptr_t context = 0;
		std::uintptr_t stream = 0;
		VideoBuffer(int32_t s, int32_t w, int32_t h, std::uintptr_t ctx = 0) : size(s), width(w), height(h), context(ctx)
		{
			if (context) {
#ifdef _WIN32
				cuCtxPushCurrent((CUcontext) context);
				cuMemAlloc((CUdeviceptr*) &buffer, s);
				cuStreamCreate((CUstream*) &stream, 0);
				cuCtxPopCurrent(NULL);
#endif
			}
			else {
				buffer = new uint8_t[s];
			}
		}
		~VideoBuffer()
		{
			if (context) {
#ifdef _WIN32
				if (buffer) {
					cuCtxPushCurrent((CUcontext) context);
					cuMemFree((CUdeviceptr) buffer);
					if (stream) {
						cuStreamDestroy((CUstream) stream);
					}
					cuCtxPopCurrent(NULL);
				}
#endif
			}
			else {
				delete[] buffer;
			}
		};
	};

	struct AudioBuffer
	{
		std::vector<uint8_t> buffer;
		int32_t samplerate = 0;
	};

	typedef vs::AtomicCache<VideoBuffer*, std::default_delete<VideoBuffer>, avmuxer_cache::maxCacheLenght> AtomicCacheVideo;
	typedef std::unique_ptr<VideoBuffer, avmuxer_cache::return_to_cache_video<VideoBuffer>> UniqueVideoBuffer;

	struct CycleBuffer
	{
		std::vector<UniqueVideoBuffer> cb;
		int32_t maxFrames = 0;
		std::atomic<int32_t> readCount { 0 };
		std::atomic<int32_t> writeCount { 0 };
		void Clear()
		{
			maxFrames = 0;
			readCount.store(0);
			writeCount.store(0);
			cb.clear();
		}
		void Detach()
		{
			readCount.store(1);
			writeCount.store(1);
		}
	};

}

typedef std::map<std::uintptr_t /* handle */, std::shared_ptr<avmuxer::LayoutItem>> mixerPool;
typedef std::map<uint32_t, std::shared_ptr< avmuxer::MixerItem<avmuxer::VideoItem> >> videoMixerItem;
typedef std::map<uint32_t, std::shared_ptr< avmuxer::MixerItem<avmuxer::AudioItem> >> audioMixerItem;
typedef std::deque<std::unique_ptr<avmuxer::FrameVideoThread>> videoFramePool;
typedef std::map<std::string /* stream id */, VS_MediaFormat /* mf */> infoSenderStreams;
typedef std::map<std::uintptr_t /* handle */, infoSenderStreams /* map info */> mixerInfoStreams;
typedef std::deque<std::pair<std::uintptr_t /* handle */, infoSenderStreams /* info */>> queueRequestInfoStreams;
typedef std::vector<std::pair<std::uintptr_t /* handle */, media_synch::AudioProcessingItem* /* audio item */>> mixerAudioSynchBuffer;
typedef std::vector<std::pair<std::uintptr_t /* handle */, std::shared_ptr<media_synch::VideoBuffer>>> mixerVideoSynchBuffer;

typedef vs::atomic_shared_ptr<mixerPool> atomicMixerPool;
typedef vs::atomic_shared_ptr<std::map<std::string, mixer_receiver::StreamInfo>> atomicMixerReceivers;
typedef vs::atomic_shared_ptr<queueRequestInfoStreams> atomicQueueRequestInfoStreams;