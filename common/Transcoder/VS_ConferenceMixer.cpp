#include "VS_ConferenceMixer.h"
#include "VS_MultiMixerTransceiverPass.h"

#include "AudioCodec.h"
#include "VS_FfmpegResampler.h"
#include "IppLib2/VSVideoProcessingIpp.h"
#include "streams/Protocol.h"
#include "streams/ParseSvcStream.h"
#include "Transcoder/VS_VideoCodecManager.h"
#include "Transcoder/VS_VideoCodecThreaded.h"
#include "std-generic/cpplib/deleters.h"

#include "std/cpplib/ASIOThreadPool.h"
#include "std/cpplib/VS_VideoLevelCaps.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

/* Mixer initialization. Pefroms full cycle of initialization with stopping and starting mixing process.
*  [in] mf  - contains audio/video format settings
*  [in] MixGrid - paremeters of output video frame
*
*/

const std::uintptr_t VS_ConferenceMixer::m_defaultHandle;

VS_ConferenceMixer::VS_ConferenceMixer() : BalancingModule(balancing_module::Type::mixer),
										   m_localEvents(cme_size, false),
										   m_mixerPool(std::make_shared<mixerPool>()),
										   m_receivers(std::make_shared<std::map<std::string, mixer_receiver::StreamInfo>>()),
										   m_is_inited(false)
{
	srand(avmuxer::getTickMs());
	m_key_frame_interval = 10000 + (rand() & 0xfff);
	m_key_frame_interval_min = 2000;
}

VS_ConferenceMixer::~VS_ConferenceMixer()
{
	Release();
}

bool VS_ConferenceMixer::Init(VS_MediaFormat *mf, VS_MixerGrid *MixGrid, const avmuxer::ScalableProperty &scalableProperty, uint32_t numThreads)
{
	Release();
	uint32_t nt(numThreads);
	if (nt == 0) {
		nt = std::min<uint32_t>(std::thread::hardware_concurrency(), 16);
	}
	m_mixerTasksPool = std::make_unique<vs::ASIOThreadPoolBalancing>([this] (bool reg) -> void { (reg) ? RegisterThread(balancing_module::Thread::layout) : UnregisterThread(); }, nt, "Mix Layout");
	m_workerTasksPool = std::make_unique<vs::ASIOThreadPoolBalancing>([this] (bool reg) -> void { (reg) ? RegisterThread(balancing_module::Thread::resampler) : UnregisterThread(); }, 1, "Mix Worker");
	m_commandTasksPool = std::make_unique<vs::ASIOThreadPool>(1, "Mix Command");
	m_strandCommand = std::make_unique<boost::asio::io_service::strand>(m_commandTasksPool->get_io_service());
	m_strandProcessLayout = std::make_unique<boost::asio::io_service::strand>(m_workerTasksPool->get_io_service());
	m_isNameOn = false;
	if (MixGrid) {
		m_mixerGrid = std::shared_ptr<VS_MixerGrid>(new VS_MixerGrid(*MixGrid));
	}
	m_mf.SetVideo(mf->dwVideoWidht, mf->dwVideoHeight, mf->dwVideoCodecFCC, mf->dwFps, mf->dwStereo, mf->dwSVCMode, mf->dwHWCodec);
	m_mf.SetAudio(mf->dwAudioSampleRate, mf->dwAudioCodecTag);
	m_scalableProperty = scalableProperty;
	m_is_inited.store(true);
	auto rcvs = m_receivers.load();
	for (auto & it : *rcvs) {
		StreamCreate(it.first, &it.second.mf, it.second.name);
		StreamSetActive(it.first, it.second.state == 1);
	}
	{
		/// insert default audio symmetric layout
		avmuxer::LayoutFormat lf(avmuxer::VideoLayout::symmetric, {});
		auto li = CreateMixer(lf);
		auto mi = m_mixerPool.load();
		auto ins = mi->insert(std::make_pair(m_defaultHandle, li));
		CreateAudioMixer(m_defaultHandle, ins.first->second.get());
	}
	Go();
	return true;
}

std::shared_ptr<avmuxer::LayoutItem> VS_ConferenceMixer::GetLayoutMixer(const std::uintptr_t handle)
{
	auto pool = m_mixerPool.load();
	auto it = pool->find(handle);
	if (it == pool->end()) {
		return std::shared_ptr<avmuxer::LayoutItem>();
	}
	return it->second;
}

std::shared_ptr<avmuxer::MixerItem<avmuxer::VideoItem>> VS_ConferenceMixer::GetVideoMixerItem(const std::uintptr_t handle, uint32_t fourcc)
{
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return std::shared_ptr<avmuxer::MixerItem<avmuxer::VideoItem>>();
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxVideoPool);
		auto vp = li->videoPool;
		if (!vp) {
			return std::shared_ptr<avmuxer::MixerItem<avmuxer::VideoItem>>();
		}
		auto mi = vp->mixerItem.find(fourcc);
		if (mi == vp->mixerItem.end()) {
			return std::shared_ptr<avmuxer::MixerItem<avmuxer::VideoItem>>();
		}
		return mi->second;
	}
}

std::shared_ptr<avmuxer::VideoPool> VS_ConferenceMixer::GetVideoPool(const std::uintptr_t handle)
{
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return std::shared_ptr<avmuxer::VideoPool>();
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxVideoPool);
		return li->videoPool;
	}
}

std::shared_ptr<avmuxer::AudioPool> VS_ConferenceMixer::GetAudioPool(const std::uintptr_t handle)
{
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return std::shared_ptr<avmuxer::AudioPool>();
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxAudioPool);
		return li->audioPool;
	}
}

std::shared_ptr<avmuxer::AudioPool> VS_ConferenceMixer::GetAudioPoolNotify(const std::uintptr_t handle)
{
	auto pool = m_mixerPool.load();
	if (pool->size() == 1 && handle == m_defaultHandle) {
		return std::shared_ptr<avmuxer::AudioPool>();
	}
	auto it = pool->find(handle);
	if (it == pool->end()) {
		return std::shared_ptr<avmuxer::AudioPool>();
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxAudioPool);
		return it->second->audioPool;
	}
}

std::shared_ptr<VS_MultiMixerVideo> VS_ConferenceMixer::GetVideoMixer(const std::uintptr_t handle)
{
	auto vp = GetVideoPool(handle);
	if (!vp) {
		return std::shared_ptr<VS_MultiMixerVideo>();
	}
	return vp->mixer;
}

avmuxer::VideoResampleItem VS_ConferenceMixer::CreateVideoResampleItem(int32_t w, int32_t h)
{
	avmuxer::VideoResampleItem vri;
	vri.w = w;
	vri.h = h;
	vri.cache.reset(new media_synch::AtomicCacheVideo(3));
	return vri;
}

avmuxer::AudioResampleItem VS_ConferenceMixer::CreateAudioResampleItem(int32_t sr)
{
	avmuxer::AudioResampleItem ari;
	ari.frame.reset(new uint8_t[2 * sr * sizeof(uint16_t)], array_deleter<uint8_t>());
	ari.rsmpl = std::make_shared<VS_FfmpegResampler>();
	return ari;
}

avmuxer::VideoItem* VS_ConferenceMixer::CreateVideoItem(VS_MediaFormat *mf, uint32_t bitrate, const avmuxer::LayoutFormat &lf)
{
	avmuxer::VideoItem *ci(new avmuxer::VideoItem());
	ci->vcm = std::make_shared<VS_VideoCodecThreaded>();
	if (ci->vcm->Init(*mf)) {
		mf->dwSVCMode = ci->vcm->GetScalableMode();
		auto l = [this, lf, svcMode = mf->dwSVCMode, mb = mf->GetFrameSizeMB()](uint8_t* frame, int32_t size, bool key, uint32_t timestamp, uint32_t fourcc)
		{
			CallbackPushVideo(lf, frame, svcMode, size, key, timestamp, mb, fourcc);
		};
		ci->vcm->m_firePushFrameQueue.connect(l);
		ci->vcm->ConfigAsync(bitrate, mf->dwFps * 100);
		ci->set_framerate = mf->dwFps;
		ci->set_bitrate = bitrate;
		load_balancing::RegisterBalancingModule(ci->vcm);
	}
	else {
		delete ci;
		ci = nullptr;
	}
	return ci;
}

avmuxer::AudioItem* VS_ConferenceMixer::CreateAudioItem(VS_MediaFormat *mf, const std::uintptr_t handle)
{
	avmuxer::AudioItem *ci(new avmuxer::AudioItem());
	ci->cdc = VS_RetriveAudioCodec(mf->dwAudioCodecTag, true);
	if (ci->cdc) {
		WAVEFORMATEX wf;
		wf.nSamplesPerSec = mf->dwAudioSampleRate;
		wf.cbSize = 0;
		wf.wBitsPerSample = 16;
		wf.nChannels = 1;
		wf.nBlockAlign = wf.wBitsPerSample * wf.nChannels / 8;
		wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
		wf.wFormatTag = (unsigned short)mf->dwAudioSampleRate;
		if (ci->cdc->Init(&wf) == 0) {
			ci->fifoframe.first = new uint8_t[mf->dwAudioBufferLen];
			ci->fifoframe.second = mf->dwAudioBufferLen;
			ci->fifo = new VS_FifoBuffer(2 * mf->dwAudioSampleRate * sizeof(uint16_t));
			ci->cdcframe = new uint8_t[mf->dwAudioBufferLen * 2];
		}
		else {
			delete ci;
			ci = nullptr;
		}
	}
	return ci;
}

void VS_ConferenceMixer::CorrectMediaFormatStream(VS_MediaFormat *mf)
{
	uint32_t mb(std::max(mf->GetFrameSizeMB(), m_scalableProperty.mbSpatial.begin()->first));
	for (const auto &it : m_scalableProperty.mbSpatial) {
		if (mb < it.first) {
			break;
		}
		mf->dwVideoWidht = it.second.w;
		mf->dwVideoHeight = it.second.h;
	}
}

void VS_ConferenceMixer::UpdateMixer(const std::uintptr_t handle, const avmuxer::LayoutFormat &lf)
{
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return;
	}
	if (!m_is_inited.load()) {
		return;
	}
	bool notifyLayout(false);
	auto audioMixer = (li->audioPool) ? li->audioPool->mixer.get() : nullptr;
	auto videoMixer = (li->videoPool) ? li->videoPool->mixer.get() : nullptr;
	auto rcvs = m_receivers.load();
	for (auto & it : *rcvs) {
		mixer_receiver::StreamInfo &si = it.second;
		if (si.state == 0) {
			continue;
		}
		if (audioMixer && lf.IsContainedPeer(it.first, true)) {
			std::lock_guard<std::mutex> lock(li->audioPool->mtx);
			notifyLayout |= audioMixer->AddRay(it.first, si.mf);
		}
		if (videoMixer && lf.IsContainedPeer(it.first)) {
			std::lock_guard<std::mutex> lock(li->videoPool->mtx);
			notifyLayout |= videoMixer->AddRay(it.first, (m_isNameOn) ? si.name : "", si.mf);
		}
	}
	if (!notifyLayout) {
		return;
	}
	SendNotifyLayoutData(handle);
}

bool VS_ConferenceMixer::CreateMixer(const std::uintptr_t handle, const avmuxer::LayoutFormat &lf)
{
	if (handle == m_defaultHandle) {
		/// handle = 0 - reserved for internal symmetric mixer
		return false;
	}
	bool modified(false), swapDefault(false);
	auto li = CreateMixer(lf);
	auto pool = m_mixerPool.load();
	auto new_pool = std::make_shared<decltype(pool)::element_type>();
	do {
		std::pair <std::uintptr_t, std::shared_ptr<avmuxer::LayoutItem>> pair;
		modified = false;
		swapDefault = false;
		*new_pool = *pool;
		if (lf.layout == avmuxer::VideoLayout::symmetric && lf.mb == 0) {
			auto symm = new_pool->find(m_defaultHandle);
			if (symm == new_pool->end()) {
				break;
			}
			pair = std::make_pair(handle, symm->second);
			new_pool->erase(symm);
			swapDefault = true;
		}
		else {
			pair = std::make_pair(handle, li);
		}
		auto it = new_pool->insert(pair);
		modified = it.second;
	} while (!m_mixerPool.compare_exchange_weak(pool, new_pool));
	SendNotifyLayoutData(m_defaultHandle);
	if (swapDefault) {
		SendNotifyLayoutData(handle);
	}
	return modified;
}

std::shared_ptr<avmuxer::LayoutItem> VS_ConferenceMixer::CreateMixer(const avmuxer::LayoutFormat &lf)
{
	auto li = std::make_shared<avmuxer::LayoutItem>();
	li->lf = lf;
	li->strand = std::make_shared<boost::asio::io_service::strand>(m_mixerTasksPool->get_io_service());
	return li;
}

void VS_ConferenceMixer::CreateVideoMixer(const std::uintptr_t handle, avmuxer::LayoutItem *layoutItem)
{
	VS_MixerGrid grid(*m_mixerGrid);
	uint32_t width(0), height(0);
	auto vp = std::make_shared<avmuxer::VideoPool>();
	if (layoutItem->lf.mb == 0) {
		width = m_mf.dwVideoWidht;
		height = m_mf.dwVideoHeight;
		for (const auto & mbl : m_scalableProperty.mbSpatial) {
			vp->resampleItem[mbl.first] = CreateVideoResampleItem(mbl.second.w, mbl.second.h);
		}
	}
	else {
		width = layoutItem->lf.width;
		height = layoutItem->lf.height;
		grid.W = width;
		grid.H = height;
		vp->resampleItem[layoutItem->lf.mb] = CreateVideoResampleItem(width, height);
	}
	for (auto & vri : vp->resampleItem) {
		vri.second.imageProc = std::make_shared<VSVideoProcessingIpp>();
	}
	vp->mixerFrame.reset(new uint8_t [width * height * 3 / 2], array_deleter<uint8_t>());
	memset(vp->mixerFrame.get(), 0, width * height);
	memset(vp->mixerFrame.get() + width * height, 0x80, width * height / 2);
	vp->mixer = std::shared_ptr<VS_MultiMixerVideo>((layoutItem->lf.layout != avmuxer::VideoLayout::asym) ? new VS_MultiMixerVideo(grid) : new VS_MultiMixerTransceiverPass());
	if (layoutItem->lf.layout == avmuxer::VideoLayout::special && !layoutItem->lf.mixerDesc.layout.empty()) {
		vp->mixer->SetLayout(layoutItem->lf.mixerDesc);
		vp->mixer->SetLayoutFixed(true);
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxVideoPool);
		layoutItem->videoPool = vp;
	}
	UpdateMixer(handle, layoutItem->lf);
}

std::shared_ptr<avmuxer::AudioPool> VS_ConferenceMixer::CreateAudioMixer()
{
	auto ap = std::make_shared<avmuxer::AudioPool>();
	ap->frameSize = 2 * m_mf.dwAudioSampleRate * sizeof(uint16_t);
	ap->mixerFrame.reset(new uint8_t[ap->frameSize], array_deleter<uint8_t>());
	memset(ap->mixerFrame.get(), 0, m_mf.dwAudioSampleRate * sizeof(uint16_t));
	for (const auto & sr : m_scalableProperty.srAudio) {
		ap->resampleItem[sr] = CreateAudioResampleItem(sr);
	}
	ap->mixer = std::shared_ptr<VS_MultiMixerAudio>(new VS_MultiMixerAudio());
	ap->mixer->ChangeSampleRate(m_mf.dwAudioSampleRate);
	return ap;
}

void VS_ConferenceMixer::CreateAudioMixer(const std::uintptr_t handle, avmuxer::LayoutItem *layoutItem)
{
	auto ap = CreateAudioMixer();
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxAudioPool);
		layoutItem->audioPool = ap;
	}
	UpdateMixer(handle, layoutItem->lf);
	if (handle != m_defaultHandle) {
		SendNotifyLayoutData(m_defaultHandle);
	}
}

bool VS_ConferenceMixer::DestroyMixer(const std::uintptr_t handle)
{
	bool modified(false);
	auto pool = m_mixerPool.load();
	auto new_pool = std::make_shared<decltype(pool)::element_type>();
	do {
		modified = false;
		*new_pool = *pool;
		auto it = new_pool->find(handle);
		if (it != new_pool->end()) {
			if (it->second->lf.layout == avmuxer::VideoLayout::symmetric && it->second->lf.mb == 0) {
				auto symm = new_pool->find(m_defaultHandle);
				if (symm == new_pool->end()) {
					auto pair = std::make_pair(m_defaultHandle, it->second);
					new_pool->insert(pair);
				}
			}
			new_pool->erase(it);
			modified = true;
		}
	} while (!m_mixerPool.compare_exchange_weak(pool, new_pool));
	SendNotifyLayoutData(handle);
	SendNotifyLayoutData(m_defaultHandle);
	return modified;
}

void VS_ConferenceMixer::DestroyVideoMixer(avmuxer::LayoutItem *layoutItem)
{
	layoutItem->videoPool.reset();
}

void VS_ConferenceMixer::DestroyAudioMixer(avmuxer::LayoutItem *layoutItem)
{
	auto mixer = layoutItem->audioPool->mixer;
	layoutItem->audioPool.reset();
	if (layoutItem->lf.layout == avmuxer::VideoLayout::symmetric) {
		layoutItem->audioPool = CreateAudioMixer();
		layoutItem->audioPool->mixer.swap(mixer);
	}
}

bool VS_ConferenceMixer::CreateVideoPool(const std::uintptr_t handle, uint32_t fourcc, int32_t *tl)
{
	bool ret(false);
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return false;
	}
	if (!li->videoPool) {
		CreateVideoMixer(handle, li.get());
	}
	VS_MediaFormat mf = m_mf;
	mf.dwVideoCodecFCC = fourcc;
	if (fourcc != VS_VCODEC_H264) {
		mf.dwHWCodec = ENCODER_SOFTWARE;
	}
	auto mi = std::make_shared<avmuxer::MixerItem<avmuxer::VideoItem>>();
	if (li->lf.layout != avmuxer::VideoLayout::asym) {
		for (auto & vri : li->videoPool->resampleItem) {
			uint32_t bitrate(0);
			auto mbl = m_scalableProperty.mbSpatial.find(vri.first);
			if (mbl != m_scalableProperty.mbSpatial.end()) {
				mf.dwVideoWidht = mbl->second.w;
				mf.dwVideoHeight = mbl->second.h;
				bitrate = mbl->second.bitrate;
			}
			else {
				mf.dwVideoWidht = li->lf.width;
				mf.dwVideoHeight = li->lf.height;
				bitrate = static_cast<uint32_t>(0.0017f * li->lf.width * li->lf.height);
			}
			auto vi = std::shared_ptr<avmuxer::VideoItem>(CreateVideoItem(&mf, bitrate, li->lf));
			if (vi) {
				mi->codecItem[vri.first] = vi;
				*tl = (mf.dwSVCMode & 0x0000ff00) ? 2 : 1;
				ret = true;
			}
		}
	}
	else {
		*tl = 2;
		ret = true;
	}
	if (ret) {
		std::lock_guard<std::recursive_mutex> lock(m_mtxVideoPool);
		li->videoPool->mixerItem[fourcc] = mi;
	}
	return ret;
}

bool VS_ConferenceMixer::DestroyVideoPool(const std::uintptr_t handle, uint32_t fourcc)
{
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return false;
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxVideoPool);
		auto vp = li->videoPool;
		if (!vp) {
			return false;
		}
		auto it = vp->mixerItem.find(fourcc);
		if (it != vp->mixerItem.end()) {
			assert(it->second->numPeers == 0);
			vp->mixerItem.erase(it);
			if (vp->mixerItem.empty()) {
				DestroyVideoMixer(li.get());
			}
			return true;
		}
	}
	return false;
}

bool VS_ConferenceMixer::CreateAudioPool(const std::uintptr_t handle, uint32_t twocc)
{
	bool ret(false);
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return false;
	}
	if (!li->audioPool) {
		CreateAudioMixer(handle, li.get());
	}
	VS_MediaFormat mf;
	auto mi = std::make_shared<avmuxer::MixerItem<avmuxer::AudioItem>>();
	for (const auto & sr : m_scalableProperty.srAudio) {
		mf.SetAudio(sr, twocc);
		auto ai = std::shared_ptr<avmuxer::AudioItem>(CreateAudioItem(&mf, handle));
		if (ai) {
			ai->mixframe.first = li->audioPool->resampleItem[sr].frame.get();
			ai->mixframe.second = &li->audioPool->resampleItem[sr].size;
			mi->codecItem[sr] = ai;
			ret = true;
		}
	}
	if (ret) {
		std::lock_guard<std::recursive_mutex> lock(m_mtxAudioPool);
		li->audioPool->mixerItem[twocc] = mi;
	}
	return ret;
}

bool VS_ConferenceMixer::DestroyAudioPool(const std::uintptr_t handle, uint32_t twocc)
{
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return false;
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxAudioPool);
		auto ap = li->audioPool;
		if (!ap) {
			return false;
		}
		auto it = ap->mixerItem.find(twocc);
		if (it == ap->mixerItem.end()) {
			return false;
		}
		assert(it->second->numPeers == 0);
		ap->mixerItem.erase(it);
		if (ap->mixerItem.empty()) {
			DestroyAudioMixer(li.get());
		}
	}
	return true;
}

bool VS_ConferenceMixer::AddVideoPeerStream(const std::uintptr_t handle, const VS_MediaFormat &mf, uint32_t *mb)
{
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return false;
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxVideoPool);
		auto vp = li->videoPool;
		if (!vp) {
			return false;
		}
		auto it = vp->mixerItem.find(mf.dwVideoCodecFCC);
		if (it != vp->mixerItem.end()) {
			auto mi = it->second.get();
			if (*mb == 0) {
				VS_MediaFormat fmt = mf;
				CorrectMediaFormatStream(&fmt);
				*mb = fmt.GetFrameSizeMB();
			}
			mi->numPeers++;
			EnableVideoLayer(true, handle, mf.dwVideoCodecFCC, *mb);
		}
		else {
			return false;
		}
	}
	return true;
}

bool VS_ConferenceMixer::AddAudioPeerStream(const std::uintptr_t handle, const VS_MediaFormat &mf)
{
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return false;
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxAudioPool);
		auto ap = li->audioPool;
		if (!ap) {
			return false;
		}
		auto it = ap->mixerItem.find(mf.dwAudioCodecTag);
		if (it != ap->mixerItem.end()) {
			auto mi = it->second.get();
			mi->numPeers++;
			EnableAudioLayer(true, handle, mf.dwAudioCodecTag, mf.dwAudioSampleRate);
		}
		else {
			return false;
		}
	}
	return true;
}

bool VS_ConferenceMixer::RemoveVideoPeerStream(const std::uintptr_t handle, uint32_t fourcc, uint32_t mb)
{
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return false;
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxVideoPool);
		if (!li->videoPool) {
			return false;
		}
		auto mi = li->videoPool->mixerItem[fourcc].get();
		assert(mi->numPeers > 0);
		mi->numPeers--;
	}
	return true;
}

bool VS_ConferenceMixer::RemoveAudioPeerStream(const std::uintptr_t handle, uint32_t twocc, uint32_t samplerate)
{
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return false;
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxAudioPool);
		if (!li->audioPool) {
			return false;
		}
		auto mi = li->audioPool->mixerItem[twocc].get();
		assert(mi->numPeers > 0);
		mi->numPeers--;
	}
	return true;
}

void VS_ConferenceMixer::EnableVideoLayer(bool enable, const std::uintptr_t handle, uint32_t fourcc, uint32_t mb)
{
	if (!m_is_inited.load()) {
		return;
	}
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return;
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxVideoPool);
		bool block(true);
		auto vp = li->videoPool;
		if (!vp) {
			return;
		}
		auto mi = vp->mixerItem.find(fourcc);
		if (mi != vp->mixerItem.end()) {
			auto cdc = mi->second->codecItem.find(mb);
			if (cdc != mi->second->codecItem.end()) {
				cdc->second->block = !enable;
			}
		}
		{
			for (auto & mi : vp->mixerItem) {
				auto ci = mi.second->codecItem.find(mb);
				if (ci != mi.second->codecItem.end()) {
					block = block && ci->second->block;
				}
			}
			auto cf = vp->resampleItem.find(mb);
			if (cf != vp->resampleItem.end()) {
				cf->second.block = block;
			}
		}
	}
}

void VS_ConferenceMixer::EnableAudioLayer(bool enable, const std::uintptr_t handle, uint32_t twocc, uint32_t samplerate)
{
	if (!m_is_inited.load()) {
		return;
	}
	auto li = GetLayoutMixer(handle);
	if (!li) {
		return;
	}
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxAudioPool);
		bool block(true);
		auto ap = li->audioPool;
		if (!ap) {
			return;
		}
		auto mi = ap->mixerItem.find(twocc);
		if (mi != ap->mixerItem.end()) {
			auto cdc = mi->second->codecItem.find(samplerate);
			if (cdc != mi->second->codecItem.end()) {
				cdc->second->block = !enable;
			}
		}
		{
			for (auto & mi : ap->mixerItem) {
				auto ci = mi.second->codecItem.find(samplerate);
				if (ci != mi.second->codecItem.end()) {
					block = block && ci->second->block;
				}
			}
			auto cf = ap->resampleItem.find(samplerate);
			if (cf != ap->resampleItem.end()) {
				cf->second.block = block;
			}
		}
	}
}

/* Changes some boolean settings of mixer
*  [in] flags  - control setting flags
*
*/
void VS_ConferenceMixer::SetFlags(int flags)
{
	m_isNameOn = (flags & NAME_ON) != 0;
}

void VS_ConferenceMixer::Release()
{
	if (!m_is_inited.load()) {
		return;
	}
	Stop();
	m_mixerPool.store(std::make_shared<mixerPool>());
	m_mf.SetZero();
	m_is_inited.store(false);
	m_mixerTasksPool.reset();
	m_commandTasksPool.reset();
	m_workerTasksPool.reset();
	m_strandCommand.reset();
	m_strandProcessLayout.reset();
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxCodecFramePool);
		m_codecFramePool.clear();
	}
}

////////////////////////
void VS_ConferenceMixer::StreamCreate(const std::string & handle, VS_MediaFormat *mf, const std::string & name)
{
	if (!m_is_inited.load()) {
		return;
	}
	mixer_receiver::StreamInfo info = { 0, *mf, name };
	auto pair = std::make_pair(handle, info);
	auto rcvs = m_receivers.load();
	auto new_rcvs = std::make_shared<decltype(rcvs)::element_type>();
	do {
		*new_rcvs = *rcvs;
		auto it = new_rcvs->insert(pair);
	} while (!m_receivers.compare_exchange_weak(rcvs, new_rcvs));
}

void VS_ConferenceMixer::StreamRelaese(const std::string & handle)
{
	if (!m_is_inited.load()) {
		return;
	}
	StreamSetActive(handle, false);
	auto rcvs = m_receivers.load();
	auto new_rcvs = std::make_shared<decltype(rcvs)::element_type>();
	do {
		*new_rcvs = *rcvs;
		auto it = new_rcvs->find(handle);
		if (it != new_rcvs->end()) {
			new_rcvs->erase(it);
		}
	} while (!m_receivers.compare_exchange_weak(rcvs, new_rcvs));
}

bool VS_ConferenceMixer::StreamChangeName(const std::string & handle, const std::string & name)
{
	if (!m_is_inited.load()) {
		return false;
	}
	auto rcvs = m_receivers.load();
	auto it = rcvs->find(handle);
	if (it == rcvs->end()) {
		return false;
	}
	it->second.name = name;
	if (it->second.state != 0) {
		auto mixerPool = m_mixerPool.load();
		for (auto & it : *mixerPool) {
			auto &li = it.second;
			if (!li) {
				continue;
			}
			if (li->videoPool) {
				std::lock_guard<std::mutex> lock(li->videoPool->mtx);
				li->videoPool->mixer->RenameRay(handle, name);
			}
		}
	}
	return true;
}

bool VS_ConferenceMixer::StreamSetActive(const std::string & handle, bool active)
{
	if (!m_is_inited.load()) {
		return false;
	}
	auto rcvs = m_receivers.load();
	auto it = rcvs->find(handle);
	if (it == rcvs->end()) {
		return false;
	}
	mixer_receiver::StreamInfo &si = it->second;
	if (active && si.state != 0) {
		return true;
	}
	if (!active && si.state == 0) {
		return true;
	}
	auto mixerPool = m_mixerPool.load();
	si.state ^= 1;
	for (auto & it : *mixerPool) {
		auto &li = it.second;
		bool notifyLayout(false);
		VS_MultiMixerAudio * audioMixer(nullptr);
		VS_MultiMixerVideo * videoMixer(nullptr);
		auto ap = li->audioPool;
		auto vp = li->videoPool;
		if (ap && (!ap->mixerItem.empty() || (li->lf.layout == avmuxer::VideoLayout::symmetric && li->lf.mb == 0))) {
			audioMixer = ap->mixer.get();
		}
		if (vp && !vp->mixerItem.empty()) {
			videoMixer = vp->mixer.get();
		}
		if (si.state == 1) {
			if (audioMixer && li->lf.IsContainedPeer(handle, true)) {
				std::lock_guard<std::mutex> lock(ap->mtx);
				notifyLayout = audioMixer->AddRay(handle, si.mf);
			}
			if (videoMixer && li->lf.IsContainedPeer(handle)) {
				std::lock_guard<std::mutex> lock(vp->mtx);
				notifyLayout = videoMixer->AddRay(handle, (m_isNameOn) ? si.name : "", si.mf);
			}
		}
		else {
			if (audioMixer) {
				std::lock_guard<std::mutex> lock(ap->mtx);
				notifyLayout = audioMixer->DeleteRay(handle);
			}
			if (videoMixer) {
				std::lock_guard<std::mutex> lock(vp->mtx);
				notifyLayout = videoMixer->DeleteRay(handle);
			}
		}
		if (!notifyLayout) {
			continue;
		}
		SendNotifyLayoutData(it.first);
	}
	return true;
}

void VS_ConferenceMixer::SendNotifyLayoutData(const std::uintptr_t & handle, const std::string & toPeer)
{
	if (!m_is_inited.load()) {
		return;
	}
	m_strandCommand->get_io_service().post(
		[weak_self = weak_from_this(), handle, to = toPeer]() -> void
		{
			if (auto self = weak_self.lock()) {
				self->FormLayoutData(handle, to);
			}
		}
	);
}

void VS_ConferenceMixer::FormLayoutData(const std::uintptr_t & handle, const std::string & to)
{
	infoSenderStreams infoStreams;
	auto vp = GetVideoPool(handle);
	if (vp) {
		avmuxer::LayoutControl ctrl;
		mixerVideoRaySize sizeRays;
		if (!to.empty()) {
			ctrl.toPeer.append(to);
		}
		{
			std::lock_guard<std::mutex> lock(vp->mtx);
			auto mixer = vp->mixer;
			ctrl.mixerWidht = mixer->GetMuxerOutWidth();
			ctrl.mixerHeight = mixer->GetMuxerOutHeight();
			ctrl.grid = mixer->GetRaysGrid();
			sizeRays = mixer->GetMuxerRaysSize();
		}
		fireNewLayoutList(handle, ctrl);
		for (const auto & it : sizeRays) {
			VS_MediaFormat mf;
			mf.SetZero();
			mf.SetVideo(it.second.first, it.second.second);
			infoStreams[it.first] = std::move(mf);
		}
	}
	auto ap = GetAudioPoolNotify(handle);
	if (ap) {
		int32_t sampleRate(0);
		std::set<std::string> handleRays;
		auto mixer = ap->mixer;
		{
			std::lock_guard<std::mutex> lock(ap->mtx);
			handleRays = mixer->GetHandleRays();
			sampleRate = mixer->GetSampleRate();
		}
		for (const auto & it : handleRays) {
			auto is = infoStreams.find(it);
			if (is == infoStreams.end()) {
				VS_MediaFormat mf;
				mf.SetZero();
				mf.SetAudio(sampleRate);
				infoStreams[it] = std::move(mf);
			}
			else {
				is->second.SetAudio(sampleRate);
			}
		}
	}
	fireUpdateInfoStreams(handle, infoStreams);
}

void VS_ConferenceMixer::ManageLayout(const std::uintptr_t handle, const avmuxer::LayoutControl & layoutControl)
{
	auto vp = GetVideoPool(handle);
	if (!vp) {
		return;
	}
	auto mixer = vp->mixer;
	auto lc = mixer->GetLayoutControl();
	if (!lc) {
		return;
	}
	int res(-1);
	{
		std::lock_guard<std::mutex> lock(vp->mtx);
		if (layoutControl.function == avmuxer::PriorityTypeLayoutFunction) {
			lc->SetPriorityLayoutType(layoutControl.type);
			res = 0;
		}
		else if (layoutControl.function == avmuxer::SetP0LayoutFunction) {
			res = lc->SetP0();
		}
		else if (layoutControl.function == avmuxer::SetP1LayoutFunction) {
			res = lc->SetP1(layoutControl.userId1);
		}
		else if (layoutControl.function == avmuxer::SetP2LayoutFunction) {
			res = lc->SetP2(layoutControl.userId1, layoutControl.userId2);
		}
		else if (layoutControl.function == avmuxer::SwapLayoutFunction) {
			res = lc->Swap(layoutControl.userId1, layoutControl.userId2);
		}
		else if (layoutControl.function == avmuxer::GetLayoutFunction) {
			res = 1;
		}
		if (res == 0) {
			mixer->Refresh();
		}
	}
	if (res == -1) {
		return;
	}
	SendNotifyLayoutData(handle, layoutControl.toPeer);
}

void VS_ConferenceMixer::PutAudio(const std::map <std::string, mixerAudioSynchBuffer> &frames)
{
	if (m_localLooper.joinable()) {
		auto mixerPool = m_mixerPool.load();
		for (auto & it : *mixerPool) {
			auto &li = it.second;
			if (!li) {
				continue;
			}
			auto audioPool = li->audioPool;
			if (!audioPool) {
				continue;
			}
			if (li->lf.mb != 0) {
				continue;
			}
			if (li->lf.layout == avmuxer::VideoLayout::asym) {
				continue;
			}
			if (audioPool->mixerItem.empty() && (li->lf.layout != avmuxer::VideoLayout::symmetric)) {
				continue;
			}
			for (const auto &fr : frames) {
				for (const auto &api : fr.second) {
					if (api.first != it.first) {
						continue;
					}
					std::lock_guard<std::mutex> lock(audioPool->mtx);
					audioPool->mixer->Add(api.second->frame.get(), api.second->size, fr.first);
				}
			}
		}
		m_localEvents.set(cme_checkAudio);
	}
}

void VS_ConferenceMixer::PutVideo(const std::string &callId, const mixerVideoSynchBuffer &frames)
{
	if (m_localLooper.joinable()) {
		auto mixerPool = m_mixerPool.load();
		for (auto & frame : frames) {
			auto it = mixerPool->find(frame.first);
			if (it == mixerPool->end()) {
				continue;
			}
			auto videoPool = it->second->videoPool;
			if (!videoPool) {
				continue;
			}
			if (videoPool->mixerItem.empty()) {
				continue;
			}
			if (it->second->lf.layout != avmuxer::VideoLayout::asym) {
				auto &vb = frame.second;
				std::lock_guard<std::mutex> lock(videoPool->mtx);
				videoPool->mixer->Add(callId, vb);
			}
		}
	}
}

void VS_ConferenceMixer::PutPassData(const std::string &callId, uint8_t *data, int32_t size, stream::Track type)
{
	if (m_localLooper.joinable()) {
		auto mixerPool = m_mixerPool.load();
		if (type == stream::Track::video) {
			for (auto & it : *mixerPool) {
				auto &li = it.second;
				if (!li) {
					continue;
				}
				auto videoPool = li->videoPool;
				if (!videoPool) {
					continue;
				}
				if (videoPool->mixerItem.empty()) {
					continue;
				}
				if (li->lf.layout == avmuxer::VideoLayout::asym) {
					std::lock_guard<std::mutex> lock(videoPool->mtx);
					//videoPool->mixer->Add(data, callId, 0, 0, 0, 0);
				}
			}
		}
		else if (type == stream::Track::audio) {

		}
	}
}

bool VS_ConferenceMixer::Go()
{
	if (!m_is_inited.load()) {
		return false;
	}
	if (!m_localLooper.joinable()) {
		m_mixerTasksPool->Start();
		m_commandTasksPool->Start();
		m_workerTasksPool->Start();
		m_localEvents.reset();
		m_localLooper = std::thread([this] () {
			vs::SetThreadName("MixerLoop");
			RegisterThread(balancing_module::Thread::control);
			Loop();
			UnregisterThread();
		});
		vs::thread::SetPriority(m_localLooper, vs::thread::high);
		dprint4("Start VS_ConferenceMixer thread\n");
	}
	return true;
}

void VS_ConferenceMixer::Stop()
{
	if (!m_is_inited.load()) {
		return;
	}
	dprint4("Stop VS_ConferenceMixer thread\n");
	m_workerTasksPool->Stop();
	m_commandTasksPool->Stop();
	m_mixerTasksPool->Stop();
	if (m_localLooper.joinable()) {
		m_localEvents.kill_listener();
		m_localLooper.join();
	}
	{
		auto rcvs = m_receivers.load();
		for (auto & it : *rcvs) {
			StreamSetActive(it.first, false);
		}
	}
}

int VS_ConferenceMixer::RequestKeyFrame(const std::uintptr_t handle, uint32_t fourcc)
{
	if (!m_is_inited.load()) {
		return -1;
	}
	auto mixerItem = GetVideoMixerItem(handle, fourcc);
	if (!mixerItem) {
		return -1;
	}
	for (auto & it : mixerItem->codecItem) {
		it.second->key_frame_requested = true;
	}
	return 0;
}

void VS_ConferenceMixer::SetBitrate(const std::uintptr_t handle, uint32_t fourcc, uint32_t bitrate, int32_t framerate)
{
	if (!m_is_inited.load()) {
		return;
	}
	auto mixerItem = GetVideoMixerItem(handle, fourcc);
	if (!mixerItem) {
		return;
	}
	for (auto & it : mixerItem->codecItem) {
		auto item = it.second;
		item->set_bitrate = static_cast<uint32_t>(bitrate * m_scalableProperty.mbSpatial[it.first].k + 0.5);
		item->set_framerate = framerate;
	}
}

void VS_ConferenceMixer::CallbackPushVideo(const avmuxer::LayoutFormat & lf, uint8_t *buffer, uint32_t svcMode, int size, bool keyframe, uint32_t timestamp, uint32_t mb, uint32_t fourcc)
{
	avmuxer::LayoutPayload payload(lf, fourcc, mb);
	auto frame = std::make_unique<avmuxer::FrameVideoThread>(buffer, size, svcMode, keyframe, timestamp, payload);
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxCodecFramePool);
		m_codecFramePool.push_back(std::move(frame));
	}
	m_localEvents.set(cme_checkVideoQueue);
}

void VS_ConferenceMixer::CheckVideoQueue(bool timeout)
{
	boost::shared_ptr<avmuxer::FrameVideoThread> frame;
	{
		std::lock_guard<std::recursive_mutex> lock(m_mtxCodecFramePool);
		if (!m_codecFramePool.empty()) {
			frame = std::move(m_codecFramePool.front());
			m_codecFramePool.pop_front();
			if (!m_codecFramePool.empty()) {
				m_localEvents.set(cme_checkVideoQueue);
			}
		}
	}
	if (frame && frame->buffer) {
		stream::SVCHeader h = { 0 };
		uint8_t *buffer(0);
		int32_t ns(0);
		{
			int32_t ss(0);
			buffer = ParseSvcStream(frame->buffer.get(), frame->size, frame->svcmode != 0, &ns, &h, &ss);
		}
		fireSendVideo(frame->payload, buffer, ns, h.temporal, frame->keyframe, frame->timestamp);
	}
}

std::shared_ptr<media_synch::VideoBuffer> VS_ConferenceMixer::AllocateVideoBuffer(const std::shared_ptr<media_synch::AtomicCacheVideo> &cache, int32_t width, int32_t height, std::uintptr_t context)
{
	avmuxer_cache::return_to_cache_video<media_synch::VideoBuffer> return_to_cache(cache);
	std::shared_ptr<media_synch::VideoBuffer> cb(cache->Get(), return_to_cache);
	if (!cb || cb->width != width || cb->height != height || cb->context != context) {
		cb.reset(new media_synch::VideoBuffer(width * height * 3 / 2, width, height, context), return_to_cache);
	}
	return cb;
}

void VS_ConferenceMixer::ProcessMixerLayout(const std::shared_ptr<avmuxer::VideoPool> &vp, uint32_t ct)
{
	for (auto & ri : vp->resampleItem) {
		auto &vri = ri.second;
		if (vri.block) {
			continue;
		}
		uint32_t ret(0);
		auto vb = AllocateVideoBuffer(vri.cache, vri.w, vri.h, 0);
		if (!vb) {
			continue;
		}
		{
			std::lock_guard<std::mutex> lock(vp->mtx);
			ret = vp->mixer->GetVideo(vb->buffer, vb->width, vb->height);
		}
		if (ret <= 0) {
			continue;
		}
		videoMixerItem mixerItem;
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxVideoPool);
			mixerItem = vp->mixerItem;
		}
		for (auto &mi : mixerItem) {
			auto ci = mi.second->codecItem.find(ri.first);
			if (ci == mi.second->codecItem.end()) {
				continue;
			}
			auto &vi = ci->second;
			auto &codec = vi->vcm;
			codec->ConfigAsync(vi->set_bitrate, vi->set_framerate * 100);
			auto dt = ct - vi->last_key_frame_time;
			bool key = (dt >= m_key_frame_interval) || (vi->key_frame_requested && dt >= m_key_frame_interval_min);
			if (key) {
				vi->last_key_frame_time = ct;
				vi->key_frame_requested = false;
			}
			codec->ConvertAsync(vb, key, ct);
		}
	}
	//{
	//	uint32_t jt = static_cast<uint32_t>(avmuxer::getTickMs());
	//	char tt[1024];
	//	sprintf(tt, "%p : j = %5u\n", vp.get(), jt - ct);
	//	OutputDebugString(tt);
	//}
}

void VS_ConferenceMixer::ProcessMixerAsymmetricLayout(const std::shared_ptr<avmuxer::VideoPool> &vp, const avmuxer::LayoutFormat &lf, uint32_t ct)
{
	uint32_t ret(false);
	uint8_t *muxframe(vp->mixerFrame.get());
	uint32_t mb(0), fourcc(0), tm(0);
	uint8_t tl(0);
	int32_t s(0);
	bool key(false);
	while (true) {
		{
			/// lock mixers for AddRay & DeleteRay & ChangeFormat
			std::lock_guard<std::mutex> lock(vp->mtx);
			ret = vp->mixer->Get(muxframe, s, mb, tl, key, tm, fourcc);
		}
		if (ret) {
			CallbackPushVideo(lf, muxframe, 0x0, s, key, tm, mb, fourcc);
		}
		else {
			break;
		}
	}
}

void VS_ConferenceMixer::CheckMixerVideo(uint32_t ct)
{
	auto mixerPool = m_mixerPool.load();
	for (auto &li : *mixerPool) {
		if (!li.second) {
			continue;
		}
		auto videoPool = li.second->videoPool;
		if (!videoPool) {
			continue;
		}
		if (videoPool->mixerItem.empty()) {
			continue;
		}
		/// layouts pool
		li.second->strand->post(
			[this, videoPool, lf = li.second->lf, ct] () -> void
			{
				if (lf.layout != avmuxer::VideoLayout::asym) {
					m_strandProcessLayout->post(
						[this, videoPool, ct] () -> void
						{
							ProcessMixerLayout(videoPool, ct);
						}
					);
				}
				else {
					ProcessMixerAsymmetricLayout(videoPool, lf, ct);
				}
			}
		);
	}
}

void VS_ConferenceMixer::CheckMixerAudio(uint32_t ct)
{
	uint32_t ret(0);
	std::shared_ptr<avmuxer::LayoutItem> symmetricLayout;
	std::shared_ptr<VS_MultiMixerAudio> symmetricMixer;
	auto mixerPool = m_mixerPool.load();
	for (auto &li : *mixerPool) {
		if (li.second->lf.mb != 0) {
			continue;
		}
		if (li.second->lf.layout != avmuxer::VideoLayout::symmetric) {
			// find symmetric mixer
			continue;
		}
		if (li.first == m_defaultHandle && mixerPool->size() == 1) {
			// only default (handle == 0) mixer exists
			continue;
		}
		symmetricLayout = li.second;
		break;
	}
	{
		/// Preparfe audio for symmetric mixer
		if (!symmetricLayout) {
			return;
		}
		auto audioPool = symmetricLayout->audioPool;
		if (!audioPool) {
			return;
		}
		symmetricMixer = audioPool->mixer;
		{
			std::lock_guard<std::mutex> lock(audioPool->mtx);
			ret = symmetricMixer->PrepareAudio(audioPool->frameSize);
		}
	}
	for (auto &li : *mixerPool) {
		if (!li.second) {
			continue;
		}
		auto audioPool = li.second->audioPool;
		if (!audioPool) {
			continue;
		}
		if (audioPool->mixerItem.empty()) {
			continue;
		}
		uint8_t *muxframe(audioPool->mixerFrame.get());
		int32_t s(audioPool->frameSize);
		if (li.second->lf.layout != avmuxer::VideoLayout::asym) {
			{
				/// lock mixers for AddRay & DeleteRay & ChangeFormat
				std::lock_guard<std::mutex> lock(audioPool->mtx);
				if (li.second->lf.layout == avmuxer::VideoLayout::sip) {
					s = symmetricMixer->GetAudio(*li.second->lf.peers.begin(), muxframe, ret);
				}
				else if (li.second->lf.layout == avmuxer::VideoLayout::symmetric) {
					s = symmetricMixer->GetAudio("", muxframe, ret);
				}
				else {
					s = audioPool->mixer->GetAudio(muxframe, s);
				}
			}
			if (s > 0) {
				for (auto &ari : audioPool->resampleItem) {
					ari.second.size = 0;
					if (!ari.second.block) {
						int32_t ars = ari.second.rsmpl->Process(muxframe, ari.second.frame.get(), s, m_mf.dwAudioSampleRate, ari.first);
						ari.second.size = std::max(0, ars);
					}
				}
				audioMixerItem mixerItem;
				{
					/// lock codecs map
					std::lock_guard<std::recursive_mutex> lock(m_mtxAudioPool);
					mixerItem = audioPool->mixerItem;
				}
				for (auto &mi : mixerItem) {
					for (auto &ci : mi.second->codecItem) {
						auto item = ci.second;
						if (!item->block) {
							item->fifo->AddData(item->mixframe.first, *item->mixframe.second);
							while (item->fifo->GetData(item->fifoframe.first, item->fifoframe.second)) {
								int32_t len = item->cdc->Convert(item->fifoframe.first, item->cdcframe + sizeof(int32_t), item->fifoframe.second);
								if (len > 0) {
									fireSendAudio(li.second->lf, mi.first, ci.first, item->cdcframe, len, item->fifoframe.second / 2, ct);
								}
							}
						}
					}
				}
			}
		}
		else {

		}
	}
}

void VS_ConferenceMixer::Loop()
{
	if (!m_is_inited.load()) {
		return;
	}
	bool exit = false;
	unsigned int curr_time = 0;
	unsigned int start_time = avmuxer::getTickMs();
	unsigned int dt_framerate = (int)(1000.0 / (double)m_mf.dwFps);
	unsigned int next_time = start_time;
	unsigned int dt_wait = 1;
	unsigned int cnt_skip_frame = 0;

	while (true) {
		uint32_t waitRes = m_localEvents.wait_for(std::chrono::milliseconds(dt_wait), false);
		switch (waitRes)
		{
		case vs::wait_result::time_to_die:
			exit = true;
			break;
		case cme_checkVideoQueue:
		{
			CheckVideoQueue(false);
			curr_time = avmuxer::getTickMs();
			int dt = next_time - curr_time;
			if (dt > 0) {
				dt_wait = (dt_wait < static_cast<unsigned int>(dt)) ? 0 : dt_wait - dt;
				CheckMixerAudio(curr_time);
				continue;
			}
			break;
		}
		case cme_checkAudio:
		{
			break;
		}
		case vs::wait_result::timeout:
		default:
			CheckVideoQueue(true);
			break;
		}

		curr_time = avmuxer::getTickMs();

		CheckMixerAudio(curr_time);
		if (curr_time >= next_time) {
			CheckMixerVideo(curr_time);
		}

		if (curr_time >= next_time) {
			next_time += dt_framerate;
		}
		curr_time = avmuxer::getTickMs();
		dt_wait = 0;
		if (curr_time < next_time) {
			dt_wait = next_time - curr_time;
			cnt_skip_frame = 0;
		}
		else {
			if (cnt_skip_frame >= 3) {
				cnt_skip_frame = 0;
				while (next_time <= curr_time) next_time += dt_framerate;
			}
			else {
				cnt_skip_frame++;
			}
		}
		if (exit) {
			break;
		}
	}

	return;
}
