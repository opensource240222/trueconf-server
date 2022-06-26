#include "CMixerPool.h"
#include "ThreadDecompressor.h"
#include "MediaSynch.h"
#include "std/cpplib/MakeShared.h"

CMixerPool::CMixerPool() 
	: m_confmix(vs::MakeShared<VS_ConferenceMixer>())
	, m_receivers(std::make_shared<mixerReseivers>())
{
	m_thread_decomp.reset(new ThreadDecompressor());
	m_media_synch.reset(new MediaSynch());
	m_mgrid.arW = 16;
	m_mgrid.arH = 9;
	m_mgrid.wMultp = 8;
	m_mgrid.hMultp = 2;
	m_mf.SetZero();
	m_IsInit = false;
	m_State.store(false);
}

CMixerPool::~CMixerPool()
{
	m_thread_decomp->Stop();
	m_media_synch->Stop();
	m_confmix->Stop();
	m_receivers.store(std::make_shared<mixerReseivers>());
}

void CMixerPool::SetBitrate(const std::uintptr_t handle, uint32_t fourcc, uint32_t bitrate, int32_t framerate)
{
	m_confmix->SetBitrate(handle, fourcc, bitrate, framerate);
}

void CMixerPool::EnableVideoLayer(bool enable, const std::uintptr_t handle, uint32_t fourcc, uint32_t mb)
{
	m_confmix->EnableVideoLayer(enable, handle, fourcc, mb);
}

void CMixerPool::EnableAudioLayer(bool enable, const std::uintptr_t handle, uint32_t twocc, uint32_t samplerate)
{
	m_confmix->EnableAudioLayer(enable, handle, twocc, samplerate);
}

long CMixerPool::RequestKeyFrame(const std::uintptr_t handle, uint32_t fourcc)
{
	m_confmix->RequestKeyFrame(handle, fourcc);
	return 0;
}

void CMixerPool::SetState()
{
	if (!m_IsInit) {
		return;
	}
	bool temp_state = m_State.load();
	auto rcvs = m_receivers.load();
	int rec_size = rcvs->size();
	if (!temp_state) {
		if (rec_size > 0) {
			m_media_synch->Run();
			m_thread_decomp->Go();
			m_confmix->Go();
			temp_state = true;
		}
	}
	else {
		if (rec_size == 0) {
			m_thread_decomp->Stop();
			m_media_synch->Stop();
			m_confmix->Stop();
			temp_state = false;
		}
	}
	m_State.store(temp_state);
}

long CMixerPool::Init(VS_MediaFormat *mf, const avmuxer::ScalableProperty & property, VS_ConfMixerCallback *callback)
{
	if (!mf)
		return -1;
	if (m_IsInit)
		return 1;
	m_scalableProperty = property;
	m_mgrid.W = mf->dwVideoWidht;
	m_mgrid.H = mf->dwVideoHeight;

	// register balancing modules
	load_balancing::RegisterBalancingModule(m_confmix);
	load_balancing::RegisterBalancingModule(m_thread_decomp);
	load_balancing::RegisterBalancingModule(m_media_synch);
	// media synch signals
	m_media_synch->firePutAudioSignal.connect([this](const std::map <std::string, mixerAudioSynchBuffer> &frames) { m_confmix->PutAudio(frames); });
	m_media_synch->firePutVideoSignal.connect([this](const std::string &id, const mixerVideoSynchBuffer &frames) { m_confmix->PutVideo(id, frames); });
	m_media_synch->fireMbParticipantsUpdateSignal.connect([callback](const std::map<std::string, int32_t> &mbs) { callback->UpdateParticipantsMb(mbs); });
	// mixer synch signals
	m_confmix->fireUpdateInfoStreams.connect([this](const std::uintptr_t handle, const infoSenderStreams & infoStreams) { m_media_synch->UpdateInfoStreams(handle, infoStreams); });
	m_confmix->fireNewLayoutList.connect([callback](const std::uintptr_t handle, const avmuxer::LayoutControl &lc) { callback->NewLayoutList(handle, lc); });
	m_confmix->fireSendVideo.connect([callback](const avmuxer::LayoutPayload &payload, uint8_t* data, int size, uint8_t tl, bool key, uint32_t tm) { callback->CallbackVideo(payload, data, size, tl, key, tm); });
	m_confmix->fireSendAudio.connect([callback](const avmuxer::LayoutFormat &format, uint32_t twocc, uint32_t samplerate, uint8_t* data, int size, uint32_t samples, uint32_t timestamp) { callback->CallbackAudio(format, twocc, samplerate, data, size, samples, timestamp); });
	// decompressor signals
	m_thread_decomp->fireUpdateLoadStatistic.connect([callback](const avmuxer::LoadStatistic &load) { callback->UpdateLoadDecompressor(load); });
	m_thread_decomp->fireKeyFrameRequest.connect([callback] (const std::string &id) { callback->UpdateParticipantKeyRequest(id); });

	m_media_synch->Init(&m_receivers);
	m_confmix->Init(mf, &m_mgrid, m_scalableProperty);
	m_confmix->SetFlags(NAME_ON);
	m_thread_decomp->Init(&m_receivers);
	m_mf = *mf;
	m_IsInit = true;
	return 0;
}

bool CMixerPool::CreateMixer(const std::uintptr_t handle, const avmuxer::LayoutFormat &lf)
{
	if (!m_IsInit) {
		return false;
	}
	return m_confmix->CreateMixer(handle, lf);
}

bool CMixerPool::DestroyMixer(const std::uintptr_t handle)
{
	if (!m_IsInit) {
		return false;
	}
	return m_confmix->DestroyMixer(handle);
}

bool CMixerPool::CreateVideoPool(const std::uintptr_t handle, uint32_t fourcc, int32_t *tl)
{
	if (!m_IsInit) {
		return false;
	}
	return m_confmix->CreateVideoPool(handle, fourcc, tl);
}

bool CMixerPool::DestroyVideoPool(const std::uintptr_t handle, uint32_t fourcc)
{
	if (!m_IsInit) {
		return false;
	}
	return m_confmix->DestroyVideoPool(handle, fourcc);
}

bool CMixerPool::CreateAudioPool(const std::uintptr_t handle, uint32_t twocc)
{
	if (!m_IsInit) {
		return false;
	}
	return m_confmix->CreateAudioPool(handle, twocc);
}

bool CMixerPool::DestroyAudioPool(const std::uintptr_t handle, uint32_t twocc)
{
	if (!m_IsInit) {
		return false;
	}
	return m_confmix->DestroyAudioPool(handle, twocc);
}

bool CMixerPool::AddVideoStream(const std::uintptr_t handle, const VS_MediaFormat &mf, uint32_t *mb)
{
	if (!m_IsInit) {
		return false;
	}
	return m_confmix->AddVideoPeerStream(handle, mf, mb);
}

bool CMixerPool::RemoveVideoStream(const std::uintptr_t handle, uint32_t fourcc, uint32_t mb)
{
	if (!m_IsInit) {
		return false;
	}
	return m_confmix->RemoveVideoPeerStream(handle, fourcc, mb);
}

bool CMixerPool::AddAudioStream(const std::uintptr_t handle, const VS_MediaFormat &mf)
{
	if (!m_IsInit) {
		return false;
	}
	return m_confmix->AddAudioPeerStream(handle, mf);
}

bool CMixerPool::RemoveAudioStream(const std::uintptr_t handle, uint32_t twocc, uint32_t samplerate)
{
	if (!m_IsInit) {
		return false;
	}
	return m_confmix->RemoveAudioPeerStream(handle, twocc, samplerate);
}

void CMixerPool::Release()
{
	m_media_synch->Release();
	m_thread_decomp->Stop();
	m_confmix->Release();
	m_media_synch->firePutAudioSignal.disconnect_all_slots();
	m_media_synch->firePutVideoSignal.disconnect_all_slots();
	m_media_synch->fireMbParticipantsUpdateSignal.disconnect_all_slots();
	m_confmix->fireUpdateInfoStreams.disconnect_all_slots();
	m_confmix->fireNewLayoutList.disconnect_all_slots();
	m_confmix->fireSendVideo.disconnect_all_slots();
	m_confmix->fireSendAudio.disconnect_all_slots();
	m_thread_decomp->fireUpdateLoadStatistic.disconnect_all_slots();
	m_mf.SetZero();
	m_IsInit = false;
}

void CMixerPool::ChangeMixerParam(VS_MediaFormat *mf)
{
	m_mgrid.W = mf->dwVideoWidht;
	m_mgrid.H = mf->dwVideoHeight;
	m_confmix->Init(mf, &m_mgrid, m_scalableProperty);
	m_mf = *mf;
}

std::shared_ptr<MixerTrackReceiver> CMixerPool::GetReceiver(string_view id)
{
	auto rcvs = m_receivers.load();
	auto it = rcvs->find(std::string(id));
	if (it == rcvs->end()) {
		return std::shared_ptr<MixerTrackReceiver>();
	}
	return it->second;
}

std::shared_ptr<MixerTrackReceiver> CMixerPool::CreateReceiver(string_view id, string_view dn, std::shared_ptr<VS_MixerReceiver>& rcv, stream::Track track, stream::TrackType type, VS_MediaFormat mf)
{
	bool modified(false);
	auto rcv_info = std::make_shared<MixerTrackReceiver>(track, type);
	rcv_info->rcv = rcv;
	auto pair = std::make_pair(std::string(id), rcv_info);
	auto rcvs = m_receivers.load();
	auto new_rcvs = std::make_shared<decltype(rcvs)::element_type>();
	do {
		modified = false;
		*new_rcvs = *rcvs;
		auto it = new_rcvs->insert(pair);
		modified = it.second;
	} while (!m_receivers.compare_exchange_weak(rcvs, new_rcvs));
	if (!modified) {
		return std::shared_ptr<MixerTrackReceiver>();
	}
	mf.dwAudioSampleRate = m_mf.dwAudioSampleRate;
	m_confmix->StreamCreate(std::string(id), &mf, std::string(dn));
	SetState();
	return rcv_info;
}

long CMixerPool::ReceiverInit(const char* callId, VS_MediaFormat *mf, unsigned long capsFourcc, string_view streamName)
{
	if (!callId || !mf) {
		return -1;
	}
	auto rcv = std::make_shared<VS_MixerReceiver>();
	rcv->Init(mf, capsFourcc, (char*) callId, std::string(streamName).c_str());
	auto rcv_info = CreateReceiver(callId, streamName, rcv, stream::Track::video, stream::TrackType::video, *mf);
	return (rcv_info) ? 0 : 1;
}

long CMixerPool::ReceiverSetActive(const char* callId, bool IsActive)
{
	if (!callId) {
		return -1;
	}
	auto info = GetReceiver(callId);
	if (!info) {
		return 1;
	}
	info->rcv->SetActive(IsActive);
	m_confmix->StreamSetActive(callId, IsActive);
	return 0;
}

long CMixerPool::ReceiverDisconnect(const char* callId)
{
	if (!callId) {
		return -1;
	}
	bool modified(false);
	auto rcvs = m_receivers.load();
	auto new_rcvs = std::make_shared<decltype(rcvs)::element_type>();
	do {
		modified = false;
		*new_rcvs = *rcvs;
		auto it = new_rcvs->find(callId);
		if (it != new_rcvs->end()) {
			new_rcvs->erase(it);
			modified = true;
		}
	} while (!m_receivers.compare_exchange_weak(rcvs, new_rcvs));
	if (!modified) {
		return 1;
	}
	m_confmix->StreamRelaese(callId);
	SetState();
	return 0;
}

long CMixerPool::ReceiverChangeName(const char *callId, const char* StreamName)
{
	if (!callId) {
		return -1;
	}
	auto info = GetReceiver(callId);
	if (!info) {
		return 1;
	}
	m_confmix->StreamChangeName(callId, (StreamName != nullptr) ? StreamName : "");
	return 0;
}

long CMixerPool::PutData(const char* callId, stream::Track track, unsigned char* data, int size)
{
	if (!callId) {
		return -1;
	}
	auto info = GetReceiver(callId);
	if (!info) {
		return 1;
	}
	info->rcv->PushData(data, size, track);
	m_thread_decomp->SetNewDataEvent();
	return 0;
}

long CMixerPool::PutPassData(const char* callId, stream::Track track, unsigned char* data, int size)
{
	if (!callId) {
		return -1;
	}
	m_confmix->PutPassData(callId, data, size, track);
	return 0;
}

void CMixerPool::ReinitAudioRcv(const char *part, const VS_MediaFormat&fmt)
{
	auto info = GetReceiver(part);
	if (!info) {
		return;
	}
	info->rcv->SetNewAudioFmt(fmt);
}

bool CMixerPool::AddReceiverTrack(string_view receiverId, string_view uniqueId, string_view dn, stream::Track track, stream::TrackType type)
{
	auto info = GetReceiver(receiverId);
	if (!info) {
		return false;
	}
	info->rcv->AddTrack(track, type);
	std::string id(uniqueId);
	if (id.empty()) {
		id = mixer_receiver::GetUniqueId(receiverId, track);
	}
	auto uniq_info = GetReceiver(id);
	if (uniq_info) {
		if (uniq_info->rcv == info->rcv) {
			return false;
		}
		uniq_info->rcvs_queue.push_back(uniq_info->rcv);
		uniq_info->rcv = info->rcv;
	}
	else {
		uniq_info = CreateReceiver(id, "", info->rcv, track, type, VS_MediaFormat());
		if (!uniq_info) {
			return false;
		}
	}
	//std::string dn_full = std::string(dn) + " of " + std::string(receiverId);
	m_confmix->StreamSetActive(id, uniq_info->rcv->Activated());
	m_confmix->StreamChangeName(id, std::string(dn));
	return true;
}

bool CMixerPool::RemoveReceiverTrack(string_view receiverId, string_view uniqueId, stream::Track track)
{
	auto info = GetReceiver(receiverId);
	if (!info) {
		return false;
	}
	info->rcv->RemoveTrack(track);
	std::string id(uniqueId);
	if (id.empty()) {
		id = mixer_receiver::GetUniqueId(receiverId, track);
	}
	auto uniq_info = GetReceiver(id);
	if (!uniq_info) {
		return false;
	}
	auto &rcvs = uniq_info->rcvs_queue;
	rcvs.erase(std::remove_if(rcvs.begin(), rcvs.end(), [ptr = info->rcv](const std::weak_ptr<VS_MixerReceiver> &weak) -> bool { return weak.expired() || weak.lock() == ptr; }), rcvs.end());
	if (uniq_info->rcv != info->rcv) {
		/// don't change source
		return false;
	}
	if (!rcvs.empty()) {
		/// if other sources in queue
		uniq_info->rcv = rcvs.front().lock();
		rcvs.front().reset();
		m_confmix->StreamSetActive(id, uniq_info->rcv->Activated());
		return true;
	}
	/// if last source
	return ReceiverDisconnect(id.c_str()) == 0;
}

void CMixerPool::ManageLayout(const std::uintptr_t handleMixer, const avmuxer::LayoutControl & layoutControl)
{
	m_confmix->ManageLayout(handleMixer, layoutControl);
}

std::string CMixerPool::GetLoudestHandle()
{
	auto rcvs = m_receivers.load();
	if (rcvs->empty()) {
		return std::string();
	}
	auto loudest = rcvs->begin();
	for (auto it = rcvs->begin(); it != rcvs->end(); ++it) {
		if (it->second->track != stream::Track::video) {
			continue;
		}
		if (it->second->rcv->GetVol() > loudest->second->rcv->GetVol()) {
			loudest = it;
		}
	}
	if (loudest->second->rcv->GetVol() > 200) {
		return loudest->first;
	}
	return std::string();
}
