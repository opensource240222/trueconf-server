#include "MediaSynch.h"
#include "VS_ConferenceMixer.h"
#include "IppLib2/VSVideoProcessingIpp.h"
#include "nvidia/include/nppi_geometry_transforms.h"
#include "std/cpplib/ASIOThreadPool.h"
#include "std/cpplib/VS_PerformanceMonitor.h"
#include "std-generic/cpplib/deleters.h"
#include <atomic>

MediaSynch::MediaSynch() : BalancingModule(balancing_module::Type::mediasynch),
						   m_queueInfoStreams(std::make_shared<queueRequestInfoStreams>())
{

}

MediaSynch::~MediaSynch()
{
	Release();
}

bool MediaSynch::Init(atomicReceivers *receivers)
{
	Release();
	m_receivers = receivers;
	m_releaseThread = false;
	auto nth = std::min<uint32_t>(16, std::thread::hardware_concurrency());
	m_synchPool = std::make_unique<vs::ASIOThreadPoolBalancing>([this] (bool reg) -> void { (reg) ? RegisterThread(balancing_module::Thread::synch) : UnregisterThread(); }, nth, "SynchPool");
	return true;
}

void MediaSynch::Release()
{
	Stop();
	m_queueInfoStreams.store(std::make_shared<queueRequestInfoStreams>());
	m_audioProcessingPool.clear();
	m_videoProcessingPool.clear();
	m_infoStreams.clear();
	m_receivers = nullptr;
}

void MediaSynch::Run()
{
	if (!m_synchThread.joinable()) {
		m_releaseThread = false;
		m_synchThread = std::thread([this] () {
			vs::SetThreadName("MediaSynch");
			RegisterThread(balancing_module::Thread::control);
			SynchThreadFunc();
			UnregisterThread();
		});
		vs::thread::SetPriority(m_synchThread, vs::thread::high);
	}
}

void MediaSynch::Stop()
{
	if (m_synchThread.joinable()) {
		m_releaseThread = true;
		m_checkData.set();
		m_synchThread.join();
	}
}

void MediaSynch::UpdateInfoStreams(const std::uintptr_t mixerHandle, const infoSenderStreams &infoStreams)
{
	auto pair = std::make_pair(mixerHandle, infoStreams);
	auto queue = m_queueInfoStreams.load();
	auto new_queue = std::make_shared<queueRequestInfoStreams>();
	do {
		*new_queue = *queue;
		new_queue->push_back(pair);
	} while (!m_queueInfoStreams.compare_exchange_weak(queue, new_queue));
}

void MediaSynch::PrepareBuffers()
{
	auto queueInfo = m_queueInfoStreams.exchange(std::make_shared<queueRequestInfoStreams>());
	if (queueInfo->empty()) {
		return;
	}
	for (const auto & it : *queueInfo) {
		auto mi = m_infoStreams.find(it.first);
		if (mi != m_infoStreams.end()) {
			m_infoStreams.erase(mi);
		}
		if (!it.second.empty()) {
			m_infoStreams.insert(it);
		}
	}
	auto appool = m_audioProcessingPool;
	auto vppool = m_videoProcessingPool;
	m_audioProcessingPool.clear();
	m_videoProcessingPool.clear();
	for (const auto & it : m_infoStreams) {
		for (auto & is : it.second) {
			if (is.second.dwAudioSampleRate != 0) {
				PrepareAudioBuffers(appool, is.first, is.second.dwAudioSampleRate, it.first);
			}
			if (is.second.dwVideoWidht != 0 && is.second.dwVideoHeight != 0) {
				PrepareVideoBuffers(vppool, is.first, is.second.dwVideoWidht, is.second.dwVideoHeight, it.first);
			}
		}
	}
	std::map<std::string, int32_t> mbPeers;
	for (const auto & it : m_videoProcessingPool) {
		int32_t mbmax(0);
		for (const auto & vpi : it.second) {
			int32_t mb = vpi.width * vpi.height / 256;
			mbmax = std::max(mbmax, mb);
		}
		if (mbmax > 0) {
			mbPeers[it.first] = mbmax;
		}
	}
	if (!mbPeers.empty()) {
		fireMbParticipantsUpdateSignal(mbPeers);
	}
}

void MediaSynch::PrepareAudioBuffers(synchAudioPool & pool, const std::string &id, uint32_t sr, std::uintptr_t handle)
{
	auto insertItem = [this, &pool, sr, handle, id]() -> void
	{
		media_synch::AudioProcessingItem api;
		api.frame.reset(new uint8_t[2 * sr * sizeof(uint16_t)], array_deleter<uint8_t>());
		api.sr = sr;
		api.hmixer.insert(handle);
		{
			/// swap resample
			auto old_ita = pool.find(id);
			if (old_ita != pool.end()) {
				for (auto & proc : old_ita->second) {
					if (proc.sr == sr) {
						api.rsmpl.swap(proc.rsmpl);
						break;
					}
				}
			}
		}
		if (!api.rsmpl) {
			api.rsmpl = std::make_shared<VS_FfmpegResampler>();
		}
		m_audioProcessingPool[id].push_back(api);
	};
	auto ita = m_audioProcessingPool.find(id);
	if (ita == m_audioProcessingPool.end()) {
		insertItem();
	}
	else {
		auto api = std::find_if(ita->second.begin(), ita->second.end(),
								[sr](const media_synch::AudioProcessingItem & item) -> bool
								{
									return (item.sr == sr);
								}
		);
		if (api == ita->second.end()) {
			insertItem();
		}
		else {
			api->hmixer.insert(handle);
		}
	}
}

void MediaSynch::PrepareVideoBuffers(synchVideoPool & pool, const std::string &id, uint32_t w, uint32_t h, std::uintptr_t handle)
{
	auto insertItem = [this, &pool, w, h, handle, id]() -> void
	{
		media_synch::VideoProcessingItem vpi;
		vpi.imageProc = std::make_shared<VSVideoProcessingIpp>();
		vpi.width = w;
		vpi.height = h;
		vpi.hmixer.insert(handle);
		{
			/// swap cache
			auto old_itv = pool.find(id);
			if (old_itv != pool.end()) {
				for (auto & proc : old_itv->second) {
					if (proc.width == vpi.width && proc.height == vpi.height) {
						vpi.cache.swap(proc.cache);
						break;
					}
				}
			}
		}
		if (!vpi.cache) {
			vpi.cache.reset(new media_synch::AtomicCacheVideo(2)); // to do
		}
		m_videoProcessingPool[id].push_back(vpi);
	};
	auto itv = m_videoProcessingPool.find(id);
	if (itv == m_videoProcessingPool.end()) {
		insertItem();
	}
	else {
		auto vpi = std::find_if(itv->second.begin(), itv->second.end(),
								[w, h](const media_synch::VideoProcessingItem & item) -> bool
								{
									return (item.width == w && item.height == h);
								}
		);
		if (vpi == itv->second.end()) {
			insertItem();
		}
		else {
			vpi->hmixer.insert(handle);
		}
	}
}

void MediaSynch::ProcessNonMultipleFrame(mixer_receiver::VideoBuffer *vb)
{
	int32_t w(vb->width &~1), h(vb->height &~1);
	if (w != vb->width || h != vb->height) {
		auto src = vb->buffer;
		auto dst = vb->buffer;
		/// Y
		for (int32_t i = 0; i < h; i++) {
			memmove(dst, src, w);
			dst += w;
			src += vb->width;
		}
		src = vb->buffer + vb->width * vb->height;
		dst = vb->buffer + w * h;
		/// U
		for (int32_t i = 0; i < h / 2; i++) {
			memmove(dst, src, w / 2);
			dst += w / 2;
			src += vb->width / 2;
		}
		/// V
		for (int32_t i = 0; i < h / 2; i++) {
			memmove(dst, src, w / 2);
			dst += w / 2;
			src += vb->width / 2;
		}
		vb->width = w;
		vb->height = h;
	}
}

std::shared_ptr<media_synch::VideoBuffer> MediaSynch::AllocateVideoBuffer(media_synch::VideoProcessingItem *vpi, std::uintptr_t context)
{
	avmuxer_cache::return_to_cache_video<media_synch::VideoBuffer> return_to_cache(vpi->cache);
	std::shared_ptr<media_synch::VideoBuffer> cb(vpi->cache->Get(), return_to_cache);
	if (!cb || cb->width != vpi->width || cb->height != vpi->height || cb->context != context) {
		cb.reset(new media_synch::VideoBuffer(vpi->width*vpi->height * 3 / 2, vpi->width, vpi->height, context), return_to_cache);
	}
	return cb;
}

std::shared_ptr<media_synch::VideoBuffer> MediaSynch::ProcessingSoftware(mixer_receiver::VideoBuffer *vb, media_synch::VideoProcessingItem *vpi, uint8_t **src)
{
	auto shared = AllocateVideoBuffer(vpi, vb->context);
	if (vpi->width != vb->width || vpi->height != vb->height) {
		uint8_t *dst[3] = { shared->buffer, shared->buffer + vpi->width * vpi->height, shared->buffer + 5 * vpi->width * vpi->height / 4 };
		double kw = (double) vpi->width / (double) vb->width;
		double kh = (double) vpi->height / (double) vb->height;
		if (!vpi->imageProc->ResampleCropI420(src, dst, vb->width, vb->height, vb->width, vpi->width, vpi->height, vpi->width, vb->width, vb->height, 0, 0, kw, kh, IPPI_INTER_LINEAR)) {
			shared.reset();
		}
	}
	else {
		memcpy(shared->buffer, vb->buffer, vb->width * vb->height * 3 / 2);
	}
	return shared;
}

std::shared_ptr<media_synch::VideoBuffer> MediaSynch::ProcessingHardware(mixer_receiver::VideoBuffer *vb, media_synch::VideoProcessingItem *vpi, uint8_t **src)
{
	std::shared_ptr<media_synch::VideoBuffer> shared;

#ifdef _WIN32
	shared = AllocateVideoBuffer(vpi, vb->context);
	cuCtxPushCurrent((CUcontext) vb->context);
	if (vpi->width != vb->width || vpi->height != vb->height) {
		NppStatus st(NPP_NO_ERROR);
		uint8_t *dst[3] = { shared->device, shared->device + vpi->width * vpi->height, shared->device + 5 * vpi->width * vpi->height / 4 };
		double kw = (double) vpi->width / (double) vb->width;
		double kh = (double) vpi->height / (double) vb->height;
		st = nppiResize_8u_C1R(src[0], { vb->width, vb->height }, vb->width, { 0, 0, vb->width, vb->height },
							   dst[0], vpi->width, { vpi->width, vpi->height },
							   kw, kh, NPPI_INTER_LINEAR);
		st = nppiResize_8u_C1R(src[1], { vb->width / 2, vb->height / 2 }, vb->width / 2, { 0, 0, vb->width / 2, vb->height / 2 },
							   dst[1], vpi->width / 2, { vpi->width / 2, vpi->height / 2 },
							   kw, kh, NPPI_INTER_LINEAR);
		st = nppiResize_8u_C1R(src[2], { vb->width / 2, vb->height / 2 }, vb->width / 2, { 0, 0, vb->width / 2, vb->height / 2 },
							   dst[2], vpi->width / 2, { vpi->width / 2, vpi->height / 2 },
							   kw, kh, NPPI_INTER_LINEAR);
		if (st == NPP_NO_ERROR) {
			cuMemcpyDtoHAsync(shared->buffer, (CUdeviceptr) shared->device, vpi->width * vpi->height * 3 / 2, (CUstream) vb->stream);
			cuStreamSynchronize((CUstream) vb->stream);
		}
		else {
			shared.reset();
		}
	}
	else {
		cuMemcpyDtoHAsync(shared->buffer, (CUdeviceptr) vb->buffer, vb->size, (CUstream) vb->stream);
		cuStreamSynchronize((CUstream) vb->stream);
	}
	cuCtxPopCurrent(NULL);
#endif

	return shared;
}

mixerVideoSynchBuffer MediaSynch::ProcessVideoStream(const std::string &id, mixer_receiver::VideoBuffer *vb)
{
	mixerVideoSynchBuffer synchBuffer;
	auto vpiPool = m_videoProcessingPool.find(id);
	if (vpiPool == m_videoProcessingPool.end()) {
		return synchBuffer;
	}
	bool ret(false);
	if (!vb->context) {
		ProcessNonMultipleFrame(vb);
	}
	uint8_t *src[3] = { vb->buffer, vb->buffer + vb->width * vb->height, vb->buffer + 5 * vb->width * vb->height / 4 };
	for (auto &vpi : vpiPool->second) {
		std::shared_ptr<media_synch::VideoBuffer> shared;
		if (!vb->context) {
			shared = ProcessingSoftware(vb, &vpi, src);
		}
		else {
			shared = ProcessingHardware(vb, &vpi, src);
		}
		if (shared) {
			shared->input_width = vb->width;
			shared->input_height = vb->height;
			for (const auto & handle : vpi.hmixer) {
				synchBuffer.push_back(std::make_pair(handle, shared));
			}
		}
	}
	return synchBuffer;
}

mixerAudioSynchBuffer MediaSynch::ProcessAudioStream(const std::string &id, mixer_receiver::AudioBuffer *ab)
{
	mixerAudioSynchBuffer synchBuffer;
	auto apiPool = m_audioProcessingPool.find(id);
	if (apiPool == m_audioProcessingPool.end()) {
		return synchBuffer;
	}
	for (auto &api : apiPool->second) {
		if (api.sr != ab->samplerate) {
			api.size = api.rsmpl->Process(ab->buffer.data(), api.frame.get(), ab->buffer.size(), ab->samplerate, api.sr);
		}
		else {
			memcpy(api.frame.get(), ab->buffer.data(), ab->buffer.size());
			api.size = ab->buffer.size();
		}
		if (api.size > 0) {
			for (const auto & handle : api.hmixer) {
				synchBuffer.push_back(std::make_pair(handle, &api));
			}
		}
	}
	return synchBuffer;
}

//#define MEDIASYNCH_PERF

void MediaSynch::SynchThreadFunc()
{
	uint64_t waitTime(0);
	uint64_t audioBufferMs(20), audioBufferMs40(40);
	uint64_t videoIntervalMs(16);
	uint64_t audioTimePrev(0), audioTime(0), videoTime(0), currentTime(0);
	mixer_receiver::AudioBuffer ab;
	vs::event synchEvent(false);
#ifdef MEDIASYNCH_PERF
	std::atomic_uint all_frames(0);
	std::atomic_uint last_frames(0);
	uint64_t lt_frames(0), ct_frames(0);
#endif
	m_synchPool->Start();
	while (true) {
		{
			auto t = m_checkData.wait_for(std::chrono::milliseconds(waitTime));
		}
		if (m_releaseThread) {
			break;
		}
		auto rcvs = m_receivers->load();
		PrepareBuffers();
		if (m_videoProcessingPool.empty() && m_audioProcessingPool.empty()) {
			waitTime = audioBufferMs;
			continue;
		}
		std::atomic_int counter(rcvs->size());
		std::vector<uint64_t> timestamps(counter.load(), std::numeric_limits<uint64_t>::max());
		uint32_t streamCount(0);
		currentTime = avmuxer::getTickMs();
		{
			/// check video
			if (currentTime < videoTime || rcvs->empty()) {
				synchEvent.set();
			} else {
				videoTime = currentTime + videoIntervalMs;
				for (auto &it : *rcvs) {
					auto id = it.first;
					auto receiver = it.second->rcv;
					m_synchPool->get_io_service().post(
#ifndef MEDIASYNCH_PERF
						[this, id, receiver, streamCount, track = it.second->track, &synchEvent, &counter, &timestamps] ()
#else
						[this, id, receiver, streamCount, &synchEvent, &counter, &timestamps, &all_frames]()
#endif
						{
							uint64_t nextTime(std::numeric_limits<uint64_t>::max());
							uint64_t currentTime = avmuxer::getTickMs();
							auto vb = receiver->CheckVideo(track, currentTime, nextTime);
							if (vb) {
								auto vsb = ProcessVideoStream(id, vb.get());
								if (!vsb.empty()) {
#ifdef MEDIASYNCH_PERF
									all_frames.fetch_add(1, std::memory_order_acq_rel);
#endif
									firePutVideoSignal(id, vsb);
								}
							}
							timestamps[streamCount] = nextTime;
							if (counter.fetch_sub(1, std::memory_order_acq_rel) == 1) {
								synchEvent.set();
							}
						}
					);
					streamCount++;
				}
			}
		}
		currentTime = avmuxer::getTickMs();
		{
			/// check audio
			if (audioTime == 0) {
				audioTimePrev = currentTime;
				audioTime = currentTime;
			}
			if (currentTime >= audioTime) {
				uint64_t dta = currentTime - audioTimePrev;
				uint64_t grabBufferMs((dta < 80) ? audioBufferMs : audioBufferMs40);
				uint64_t grabAudioMs = (dta / grabBufferMs) * grabBufferMs;
				while (grabAudioMs > 0) {
					std::map <std::string, mixerAudioSynchBuffer> asbPool;
					for (auto &it : *rcvs) {
						if (it.second->track != stream::Track::video) {
							continue;
						}
						mixer_receiver::AudioBuffer ab;
						it.second->rcv->CheckAudio(&ab, grabBufferMs);
						if (!ab.buffer.empty()) {
							auto asb = ProcessAudioStream(it.first, &ab);
							if (!asb.empty()) {
								asbPool[it.first] = std::move(asb);
							}
						}
					}
					if (!asbPool.empty()) {
						firePutAudioSignal(asbPool);
					}
					audioTimePrev += grabBufferMs;
					grabAudioMs -= grabBufferMs;
				}
				audioTime = audioTimePrev + audioBufferMs;
			}
			{
				/// wait video pool, calculate next grab time
				synchEvent.wait();
				if (!timestamps.empty()) {
					auto nextTime = std::min_element(timestamps.begin(), timestamps.end());
					videoTime = std::min(videoTime, *nextTime);
				}
			}
		}
		currentTime = avmuxer::getTickMs();
		{
			/// correct wait time
			waitTime = 0;
			uint64_t nt = std::min(videoTime, audioTime);
			if (nt > currentTime) {
				waitTime = nt - currentTime;
				if (waitTime > audioBufferMs) {
					waitTime = audioBufferMs;
				}
			}
		}
#ifdef MEDIASYNCH_PERF
		{
			ct_frames = avmuxer::getTickMs();
			if (lt_frames == 0) {
				lt_frames = ct_frames;
			}
			auto dt = ct_frames - lt_frames;
			double fps(0.0);
			if (dt >= 1000) {
				auto df = all_frames - last_frames;
				auto fps = static_cast<double>(df) / static_cast<double>(dt) * 1000.0;
				lt_frames = ct_frames;
				last_frames = all_frames.load();
				char tt[1024];
				sprintf(tt, "MediaSynch Performance: fps = %7.1f [peers = %3d], cpu = %5.2f, process = %5.2f, thread = %5.2f\n", fps, static_cast<int32_t>(rcvs->size()),
						VS_PerformanceMonitor::Instance().GetTotalProcessorTime(), VS_PerformanceMonitor::Instance().GetTotalProcessTime(),
						VS_PerformanceMonitor::Instance().GetTotalThreadTime(vs::GetThreadID()));
				OutputDebugString(tt);
			}
		}
#endif
		//{
		//	char tt[1024];
		//	sprintf(tt, "SyncTimestamps: ct = %12llu, vt = %12llu, at = %12llu [dta = %12llu], wt = %12llu\n", currentTime, videoTime, audioTime, audioTime - audioTimePrev, waitTime);
		//	OutputDebugString(tt);
		//}
	}
	m_synchPool->Stop();
}
