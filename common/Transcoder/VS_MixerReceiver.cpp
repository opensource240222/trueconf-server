#include "VS_MixerReceiver.h"
#include "VS_AudioSource.h"
#include "std/debuglog/VS_Debug.h"
#include "Transcoder/VS_RetriveVideoCodec.h"

#include "../MediaParserLib/VS_VPXParser.h"
#include "../MediaParserLib/VS_XCCParser.h"
#include "../MediaParserLib/VS_H265Parser.h"
#include "../MediaParserLib/VS_H264Parser.h"
#include "../MediaParserLib/VS_H263Parser.h"

#include <cinttypes>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

namespace mixer_receiver
{

	TrackDescription::TrackDescription(stream::TrackType type) : media_type(type)
	{
		predictor.Init(20);
		audiodurr.Init(8);
		frame_interval.Init(60);
		frame_size.Init(60);
		if (type == stream::TrackType::video || type == stream::TrackType::slide) {
			cache.reset(new mixer_receiver::AtomicCacheVideo(1));
			cycle_buffer.maxFrames = mixer_receiver::max_buffer_lenght;
			cycle_buffer.cb.resize(cycle_buffer.maxFrames);
		}
	}

	mixer_receiver::UniqueVideoBuffer TrackDescription::AllocateVideoBuffer(bool& emptyCache)
	{
		avmuxer_cache::return_to_cache_video<mixer_receiver::VideoBuffer> return_to_cache(cache);
		mixer_receiver::UniqueVideoBuffer cb(cache->Get(), return_to_cache);
		if (!cb || cb->width != mf.dwVideoWidht || cb->height != mf.dwVideoHeight || cb->context != context_device) {
			delete cb.release(); /// we do not delete the deleter
			cb.reset(new mixer_receiver::VideoBuffer(buffer_s, mf.dwVideoWidht, mf.dwVideoHeight, context_device));
			emptyCache = true;
		}
		return cb;
	}

	void TrackDescription::CorrectCacheLenght(uint64_t due, uint64_t ct, double vi, int32_t cycleBufferLenght, bool emptyCache)
	{
		int32_t dt = due - ct;
		int32_t count = static_cast<int32_t>(static_cast<double>(dt) / vi);
		int32_t cache_s = static_cast<int32_t>(cache->Size());
		count = std::max(count, cycleBufferLenght + 1);
		count = std::min(count, mixer_receiver::max_buffer_lenght);
		if (cache_s > count) {
			cache->DecreaseSize(1);
			//{
			//	char tt[1024];
			//	sprintf(tt, "AllocateVideoBuffer Decrease: %p: cache_s = %2u, cycle_buffer_s = %2d, dt = %5d, count = %2d, empty = %d\n",
			//			this, cache_s, cycle_buffer_s, dt, count, emptyCache ? 1 : 0);
			//	OutputDebugString(tt);
			//}
		}
		else if (cache_s < count) {
			cache->IncreaseSize(1);
			//{
			//	char tt[1024];
			//	sprintf(tt, "AllocateVideoBuffer Increase: %p: cache_s = %2u, cycle_buffer_s = %2d, dt = %5d, count = %2d, empty = %d\n",
			//			this, cache_s, cycle_buffer_s, dt, count, emptyCache ? 1 : 0);
			//	OutputDebugString(tt);
			//}
		}
	}

	PacketPtr TrackDescription::GetPacket(bool &needKey, uint32_t &numPackets)
	{
		std::lock_guard<std::mutex> lock(mtx_packets);
		numPackets = 0;
		needKey = false;
		if (media_type == stream::TrackType::video) {
			if (queue_bytes >= queue_limit_bytes) {
				packets.clear();
				queue_bytes = 0;
				needKey = true;
			}
		}
		if (packets.empty()) {
			return PacketPtr();
		}
		auto packet = std::move(packets.front());
		packets.pop_front();
		queue_bytes -= packet->m_size;
		numPackets = packets.size();
		return packet;
	}

}

VS_MixerReceiver::VS_MixerReceiver()
{
	m_new_audio_mf.SetZero();
	m_audioSource = std::make_unique<VS_AudioSource>();
}

VS_MixerReceiver::~VS_MixerReceiver()
{
	Release();
}

int VS_MixerReceiver::Init(VS_MediaFormat *mf, unsigned long capsFourcc, char* callId, const char* /*StreamName*/)
{
	Release();
	m_CallId = callId;
	{
		// init audio
		m_audioSource->Init(mf);
		// init track::audio desc
		auto desc = std::make_shared<mixer_receiver::TrackDescription>(stream::TrackType::audio);
		desc->mf = *mf;
		{
			std::lock_guard<std::mutex> lock(m_mtxTracks);
			m_tracksDesc[stream::Track::audio] = std::move(desc);
		}
	}
	{
		// init video
		// init track::video desc
		auto desc = std::make_shared<mixer_receiver::TrackDescription>(stream::TrackType::video);
		desc->mf = *mf;
		desc->buffer_s = mf->dwVideoWidht * mf->dwVideoHeight * 3 / 2;
		if (mf->IsVideoValid_WithoutMultiplicity8()) {
			desc->vc.reset(VS_RetriveVideoCodec(*mf, false));
			if (desc->vc) {
				desc->context_device = VS_GetContextDevice(desc->vc.get());
				base_Param settings;
				settings.width = mf->dwVideoWidht;
				settings.height = mf->dwVideoHeight;
				settings.color_space = FOURCC_I420;
				settings.device_memory = (desc->context_device != 0);
				desc->vc->InitExtended(settings);
				{
					std::lock_guard<std::mutex> lock(m_mtxTracks);
					m_tracksDesc[stream::Track::video] = std::move(desc);
				}
			}
		}
		else {

		}
	}
	m_capsFourcc = capsFourcc;
	dprint3("Receiver %s Init : sr = %d, ac = %u, vc = %u, w = %d, h = %d\n", m_CallId.c_str(), mf->dwAudioSampleRate, mf->dwAudioCodecTag, mf->dwVideoCodecFCC, mf->dwVideoWidht, mf->dwVideoHeight);
	m_IsInited = true;
	return 0;
}

bool VS_MixerReceiver::AddTrack(stream::Track track, stream::TrackType type)
{
	auto desc = GetTrackDescription(track);
	if (desc) {
		return false;
	}
	desc = std::make_shared<mixer_receiver::TrackDescription>(type);
	{
		std::lock_guard<std::mutex> lock(m_mtxTracks);
		m_tracksDesc[track] = std::move(desc);
	}
	return true;
}

bool VS_MixerReceiver::RemoveTrack(stream::Track track)
{
	std::lock_guard<std::mutex> lock(m_mtxTracks);
	return m_tracksDesc.erase(track) > 0;
}

mixer_receiver::TrackDescriprionPtr VS_MixerReceiver::GetTrackDescription(stream::Track track)
{
	std::lock_guard<std::mutex> lock(m_mtxTracks);
	auto it = m_tracksDesc.find(track);
	if (it == m_tracksDesc.end()) {
		return mixer_receiver::TrackDescriprionPtr();
	}
	return it->second;
}

void VS_MixerReceiver::SetActive(bool set)
{
	m_IsNeedDecoding = set;
}

bool VS_MixerReceiver::Activated() const
{
	return m_IsNeedDecoding;
}

void VS_MixerReceiver::Release()
{
	m_IsInited = false;
	m_tracksDesc.clear();
	m_audioSource->Release();
	dprint3("Receiver Release %s\n", m_CallId.c_str());
}

long VS_MixerReceiver::PushData(unsigned char* buff, int size, stream::Track track)
{
	if (!m_IsInited) {
		return -1;
	}
	if (!m_IsNeedDecoding) {
		return 0;
	}
	auto desc = GetTrackDescription(track);
	if (desc) {
		std::lock_guard<std::mutex> lock(desc->mtx_packets);
		desc->packets.emplace_back(std::make_unique<mixer_receiver::PushFrame>(buff, size, track));
		desc->queue_bytes += size;
	}
	return 0;
}

int VS_MixerReceiver::ReceiveAudio()
{
	auto desc = GetTrackDescription(stream::Track::audio);
	if (!desc) {
		return -1;
	}
	bool need_key(false);
	uint32_t num_packets(0);
	auto frame = desc->GetPacket(need_key, num_packets);
	if (!frame || !frame->m_buff) {
		return -1;
	}
	if (m_new_audio_mf.IsAudioValid()) {
		if (m_skip_audio_frames_counter == 0) {
			ChangeAudioFormat(desc.get(), m_new_audio_mf);
			m_new_audio_mf.SetZero();
		}
		else {
			--m_skip_audio_frames_counter;
		}
	}
	{
		std::lock_guard<std::mutex> lock(m_mtxAudioLock);
		m_audioSource->AddData(reinterpret_cast<char*>(frame->m_buff.get()), frame->m_size);
	}
	return (num_packets == 0) ? -1 : 0;
}

int VS_MixerReceiver::ReceiveVideo(stream::Track track, uint32_t &packets, bool &needKey)
{
	bool getFrame(false);
	auto desc = GetTrackDescription(track);
	if (!desc) {
		return -1;
	}
	if (desc->media_type != stream::TrackType::video && desc->media_type != stream::TrackType::slide) {
		return -1;
	}
	auto frame = desc->GetPacket(needKey, packets);
	if (!frame || !frame->m_buff) {
		return -1;
	}
	if (desc->media_type == stream::TrackType::video) {
		getFrame = ProcessCompressedData(desc.get(), std::move(frame), needKey);
	}
	else {
		getFrame = ProcessUncompressedData(desc.get(), std::move(frame));
	}
	if (getFrame) {
		return -1;
	}
	return (packets == 0) ? -1 : 0;
}

bool VS_MixerReceiver::ProcessCompressedData(mixer_receiver::TrackDescription *desc, mixer_receiver::PacketPtr &&frame, bool &needKey)
{
	if (!desc->vs_input.Add(frame->m_buff.get(), frame->m_size, stream::Track::video)) {
		needKey = true;
		return false;
	}
	frame.reset();
	unsigned long usize(0), vi(0);
	bool key(false);
	uint8_t* rcvFrame(nullptr);
	auto type = desc->vs_input.GetRef(rcvFrame, usize, vi, key);
	if (type != stream::Track::video) {
		return false;
	}
	int ret = CheckMediaFormat(desc, rcvFrame, key, usize);
	if (ret >= 0 || ret == -5) {
		int decompress_size(desc->buffer_s);
		bool empty_cache(false);
		auto cb = desc->AllocateVideoBuffer(empty_cache);
		if (ret >= 0) {
			VS_VideoCodecParam prm;
			prm.dec.FrameSize = usize;
			decompress_size = desc->vc->Convert(rcvFrame, cb->buffer, &prm);
		}
		else {
			int plane = desc->buffer_s * 2 / 3;
			memset(cb->buffer, 0xc8, plane);
			memset(cb->buffer + plane, 0x80, desc->buffer_s - plane);
		}
		uint64_t ct = avmuxer::getTickMs();
		uint64_t dueTime = GetSynchronizationTime(desc, ct, vi);
		CorrectBuffersLenght(desc, ct, dueTime, vi, usize, empty_cache);
		if (decompress_size > 0) {
			StorageVideoBuffer(desc, std::move(cb), dueTime);
		}
	}
	desc->vs_input.Reset(type); // clear used data
	return true;
}

bool VS_MixerReceiver::ProcessUncompressedData(mixer_receiver::TrackDescription* desc, mixer_receiver::PacketPtr&& frame)
{
	unsigned long vi(1000 / 30);
	bool empty_cache(false);
	int32_t source_s(frame->m_size - 2 * sizeof(uint32_t));
	{
		VS_MediaFormat mf;
		auto p = frame->m_buff.get() + source_s;
		uint32_t w = *reinterpret_cast<uint32_t*>(p);
		uint32_t h = *reinterpret_cast<uint32_t*>(p + sizeof(uint32_t));
		mf.SetVideo(w, h, FOURCC_I420);
		ChangeVideoFormat(desc, mf);
	}
	auto cb = desc->AllocateVideoBuffer(empty_cache);
	memcpy(cb->buffer, frame->m_buff.get(), source_s);
	frame.reset();
	uint64_t ct = avmuxer::getTickMs();
	desc->last_fill_time = ct;
	uint64_t dueTime(ct);
	if (desc->media_type != stream::TrackType::slide) {
		dueTime = GetSynchronizationTime(desc, ct, vi);
	}
	CorrectBuffersLenght(desc, ct, dueTime, vi, 0, empty_cache);
	StorageVideoBuffer(desc, std::move(cb), dueTime);

	return true;
}

uint64_t VS_MixerReceiver::GetSynchronizationTime(mixer_receiver::TrackDescription* desc, uint64_t ct, unsigned long videoInterval)
{
	// die time for video frame
	uint64_t vInt = static_cast<uint64_t>(videoInterval);
	int CurrBuffDurr = m_audioSource->GetCurrBuffDurr();
	// CurrBuffDurr = 140; // SMIrnov: comment at 17.02.15
	if (CurrBuffDurr < 100)
		CurrBuffDurr = 100;
	if ((ct - desc->last_fill_time) > 2000 || CurrBuffDurr - desc->prev_buffer_duration > 2000 || vInt >= 1000) // clear stat
	{
		desc->predictor.Clear();
		desc->audiodurr.Clear();
		desc->last_frame_time = ct + CurrBuffDurr;
		dprint2("%.10s | Clear Stat vint = %4" PRIu64 ", lft=%" PRIu64, m_CallId.c_str(), vInt, desc->last_frame_time);
		vInt = 0;
	}
	desc->prev_buffer_duration = CurrBuffDurr;
	desc->last_fill_time = ct;
	desc->audiodurr.Snap(CurrBuffDurr);
	desc->audiodurr.GetAverage(CurrBuffDurr);
	desc->predictor.Snap(ct + CurrBuffDurr);

	uint64_t DueTime = ct;
	double predictedTime = 0.;
	if (!desc->predictor.GetPredictedFromLast(-0.5, predictedTime)) {
		if (desc->cycle_buffer.writeCount.load() == 1)
			DueTime = desc->last_frame_time;
		else
			DueTime = desc->last_frame_time + vInt;
	}
	else {
		DueTime = int(predictedTime * 0.5 - 5.0) + (desc->last_frame_time + vInt) / 2;
	}
	if (DueTime > desc->last_frame_time + vInt * 2)
		DueTime = desc->last_frame_time + vInt * 2;
	if (DueTime < desc->last_frame_time + 1)
		DueTime = desc->last_frame_time + 1;
	//if (DueTime < desc->last_frame_time + vInt/2)
	//	DueTime = desc->last_frame_time + vInt/2;
	desc->last_frame_time = DueTime;

	return DueTime;
}

void VS_MixerReceiver::CorrectBuffersLenght(mixer_receiver::TrackDescription *desc, uint64_t ct, uint64_t dueTime, unsigned long videoInterval, unsigned long compressedSize, bool emptyCache)
{
	double avgVideoInterval(1000.0 / 15.0);
	double avgFrameSize(10240); // 10KB
	desc->frame_interval.Snap(static_cast<double>(videoInterval));
	desc->frame_interval.GetAverage(avgVideoInterval);
	desc->frame_size.Snap(static_cast<double>(compressedSize));
	desc->frame_size.GetAverage(avgFrameSize);
	desc->queue_limit_bytes = static_cast<int64_t>(4 * std::max(static_cast<double>(mixer_receiver::min_video_queue_bitrate), avgFrameSize * 1000.0 / avgVideoInterval));
	int32_t buffer_s = desc->cycle_buffer.writeCount.load() - desc->cycle_buffer.readCount.load();
	desc->CorrectCacheLenght(dueTime, ct, avgVideoInterval, buffer_s, emptyCache);
}

void VS_MixerReceiver::StorageVideoBuffer(mixer_receiver::TrackDescription* desc, mixer_receiver::UniqueVideoBuffer&& vb, uint64_t dueTime)
{
	vb->timestamp = dueTime;
	{
		int32_t head(desc->cycle_buffer.writeCount.load());
		int32_t tail(desc->cycle_buffer.readCount.load());
		desc->cycle_buffer.cb[head % desc->cycle_buffer.maxFrames] = std::move(vb);
		head += (head - tail < desc->cycle_buffer.maxFrames - 1);
		desc->cycle_buffer.writeCount.store(head);
		//dprint4("%.10s | cd= %4d, pred= %4d, dif= %4d, vi= %4d, f=%3d, dt=%3d \n",
		//	m_CallId, CurrBuffDurr, DueTime - CurrTime, m_CycleTime[(m_EndCount - 1) % VIDEO_FRAMES_MAX] - desc->last_frame_time, vInt, m_EndCount - m_StartCount, dt);
	}
}

void VS_MixerReceiver::CheckAudio(mixer_receiver::AudioBuffer *ab, uint64_t dt)
{
	std::lock_guard<std::mutex> lock(m_mtxAudioLock);
	uint32_t sr(m_audioSource->GetFreq());
	int32_t bytes = 2 * sr * static_cast<uint32_t>(dt) / 1000;
	ab->buffer.resize(bytes);
	ab->samplerate = sr;
	if (m_audioSource->GetData(reinterpret_cast<char*>(ab->buffer.data()), bytes) == 0) {
		ab->buffer.clear();
		ab->samplerate = 0;
	}
}

mixer_receiver::UniqueVideoBuffer VS_MixerReceiver::CheckVideo(stream::Track track, const uint64_t ct, uint64_t &nexttime)
{
	auto desc = GetTrackDescription(track);
	if (!desc) {
		return mixer_receiver::UniqueVideoBuffer();
	}
	auto *cbi = &desc->cycle_buffer;
	int32_t head(cbi->writeCount.load());
	int32_t tail(cbi->readCount.load());
	if (head <= tail) {
		assert(head == tail);
		return mixer_receiver::UniqueVideoBuffer();
	}
	int32_t idx(tail % cbi->maxFrames);
	auto vb = std::move(cbi->cb[idx]);
	if (vb) {
		int32_t dt = static_cast<int32_t>(ct) - static_cast<int32_t>(vb->timestamp);
		while (head > (tail + 1) && (dt > 100)) {
			/// skip older frames, lifetime > 100ms
			cbi->readCount.store(++tail);
			idx = tail % cbi->maxFrames;
			vb = std::move(cbi->cb[idx]);
			dt = static_cast<int32_t>(ct) - static_cast<int32_t>(vb->timestamp);
			head = cbi->writeCount.load();
		}
		if (vb->timestamp <= ct) {
			/// frame for draw
			if (head > (tail + 1)) {
				/// next check draw if exist
				nexttime = cbi->cb[(tail + 1) % cbi->maxFrames]->timestamp;
			}
			cbi->readCount.store(++tail);
		}
		else {
			nexttime = vb->timestamp;
			cbi->cb[idx] = std::move(vb);
		}
	}
	return vb;

}

int VS_MixerReceiver::CheckMediaFormat(mixer_receiver::TrackDescription* desc, uint8_t* buffer, bool key, uint32_t size)
{
	if (!key) {
		if (desc->wait_key_frame) {
			return -1;
		}
		else {
			return 0;
		}
	}
	desc->wait_key_frame = true;

	VS_MediaFormat mf = desc->mf;
	int width = mf.dwVideoWidht, height = mf.dwVideoHeight, fps = mf.dwFps, stereo = mf.dwStereo;
	uint32_t fourcc = mf.dwVideoCodecFCC;
	int ret = -1, num_threads = 0;

	if (m_capsFourcc & (0x00000020 | 0x00000040 | 0x00000080)) {
		ret = ResolutionFromBitstream_VPX(buffer, size, width, height, num_threads);
	}
	if (ret == 0) {
		if (num_threads == 1) fourcc = VS_VCODEC_VPX;
		else if (num_threads == 2) fourcc = VS_VCODEC_VPXSTEREO;
		else fourcc = VS_VCODEC_VPXHD;
	}
	else {
		if (m_capsFourcc & 0x00000002) ret = ResolutionFromBitstream_H264(buffer, size, width, height);
		if (ret == 0 || ret == -2) {
			fourcc = VS_VCODEC_H264;
		}
		else {
			if (m_capsFourcc & 0x00000100) ret = ResolutionFromBitstream_H265(buffer, size, width, height);
			if (ret == 0 || ret == -2) {
				fourcc = VS_VCODEC_H265;
			}
			else {
				if (m_capsFourcc & 0x00000001) ret = ResolutionFromBitstream_XCC(buffer, size, width, height);
				if (ret == 0) {
					fourcc = VS_VCODEC_XC02;
				}
				else {
					if (m_capsFourcc & 0x00000004) ret = ResolutionFromBitstream_H263(buffer, size, width, height);
					if (ret == 0) {
						fourcc = VS_VCODEC_H263P;
					}
				}
			}
		}
	}
	if (ret < 0) {
		if (ret == -2) {
			ret = 0;
		}
		desc->wait_key_frame = (ret < 0);
		return ret;
	}
	mf.SetVideo(width, height, fourcc, fps, stereo);
	ret = ChangeVideoFormat(desc, mf);
	desc->wait_key_frame = false;

	return ret;
}

int VS_MixerReceiver::ChangeVideoFormat(mixer_receiver::TrackDescription* desc, VS_MediaFormat& mf)
{
	if (!m_IsInited) {
		return -1;
	}
	if (desc->mf.VideoEq(mf)) {
		return 0;
	}
	desc->mf = mf;
	desc->buffer_s = ((mf.dwVideoWidht + 1) & ~1) * ((mf.dwVideoHeight + 1) & ~1) * 3 / 2;
	desc->vc.reset(VS_RetriveVideoCodec(mf, false));
	if (desc->vc) {
		desc->context_device = VS_GetContextDevice(desc->vc.get());
		base_Param settings;
		settings.width = mf.dwVideoWidht;
		settings.height = mf.dwVideoHeight;
		settings.color_space = FOURCC_I420;
		settings.device_memory = (desc->context_device != 0);
		desc->vc->InitExtended(settings);
	}
	return 1;
}

int VS_MixerReceiver::ChangeAudioFormat(mixer_receiver::TrackDescription* desc, VS_MediaFormat& mf)
{
	if (!m_IsInited) {
		return -1;
	}
	if (desc->mf.AudioEq(mf)) {
		return 0;
	}
	desc->mf = mf;
	{
		std::lock_guard<std::mutex> lock(m_mtxAudioLock);
		m_audioSource->ReInit(&mf);
	}
	return 1;
}

void VS_MixerReceiver::SetNewAudioFmt(const VS_MediaFormat& fmt)
{
	auto desc = GetTrackDescription(stream::Track::audio);
	if (desc) {
		std::lock_guard<std::mutex> lock(desc->mtx_packets);
		m_skip_audio_frames_counter = desc->packets.size();
		m_new_audio_mf = fmt;
	}
}

uint32_t VS_MixerReceiver::GetVol()
{
	return m_audioSource->GetLevel();
}
