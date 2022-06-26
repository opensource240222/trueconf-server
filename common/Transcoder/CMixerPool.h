#pragma once

#include "mixer/VS_MultiMixerVideo.h"
#include "VS_MixerReceiver.h"
#include "VS_ConferenceMixer.h"

#include <memory>

class CMixerPool
{

private:

	std::shared_ptr<VS_ConferenceMixer>	m_confmix;
	std::shared_ptr<ThreadDecompressor> m_thread_decomp;
	std::shared_ptr<MediaSynch> m_media_synch;
	atomicReceivers				m_receivers;
	VS_MediaFormat				m_mf;
	VS_MixerGrid				m_mgrid;
	avmuxer::ScalableProperty	m_scalableProperty;
	bool						m_IsInit;
	std::atomic_bool			m_State;

public:

	CMixerPool();
	~CMixerPool();
	long Init(VS_MediaFormat *mf, const avmuxer::ScalableProperty & property, VS_ConfMixerCallback *callback);
	void Release();
	bool IsInit() {
		return m_IsInit;
	}
	void SetState();

	long ReceiverInit(const char* callId, VS_MediaFormat *mf, unsigned long capsFourcc, string_view streamName);
	long ReceiverSetActive(const char* callId, bool IsActive);
	long ReceiverDisconnect(const char *callId);
	long ReceiverChangeName(const char *callId, const char* StreamName);
	void ChangeMixerParam(VS_MediaFormat *mf);
	long PutData(const char* callId, stream::Track track, unsigned char* data, int size);
	long PutPassData(const char* callId, stream::Track track, unsigned char* data, int size);
	void ReinitAudioRcv(const char *part, const VS_MediaFormat&fmt);
	bool AddReceiverTrack(string_view receiverId, string_view uniqueId, string_view dn, stream::Track track, stream::TrackType type);
	bool RemoveReceiverTrack(string_view receiverId, string_view uniqueId, stream::Track track);

	void SetBitrate(const std::uintptr_t handle, uint32_t fourcc, uint32_t bitrate, int32_t framerate);
	long RequestKeyFrame(const std::uintptr_t handle, uint32_t fourcc);
	bool CreateMixer(const std::uintptr_t handle, const avmuxer::LayoutFormat &lf);
	bool DestroyMixer(const std::uintptr_t handle);
	bool CreateVideoPool(const std::uintptr_t handle, uint32_t fourcc, int32_t *tl);
	bool DestroyVideoPool(const std::uintptr_t handle, uint32_t fourcc);
	bool CreateAudioPool(const std::uintptr_t handle, uint32_t twocc);
	bool DestroyAudioPool(const std::uintptr_t handle, uint32_t twocc);
	bool AddVideoStream(const std::uintptr_t handle, const VS_MediaFormat &mf, uint32_t *mb);
	bool RemoveVideoStream(const std::uintptr_t handle, uint32_t fourcc, uint32_t mb);
	bool AddAudioStream(const std::uintptr_t handle, const VS_MediaFormat &mf);
	bool RemoveAudioStream(const std::uintptr_t handle, uint32_t twocc, uint32_t samplerate);
	void EnableVideoLayer(bool enable, const std::uintptr_t handle, uint32_t fourcc, uint32_t mb);
	void EnableAudioLayer(bool enable, const std::uintptr_t handle, uint32_t twocc, uint32_t samplerate);
	void ManageLayout(const std::uintptr_t handleMixer, const avmuxer::LayoutControl & layoutControl);

	std::string GetLoudestHandle();

private:

	std::shared_ptr<MixerTrackReceiver> GetReceiver(string_view id);
	std::shared_ptr<MixerTrackReceiver> CreateReceiver(string_view id, string_view dn, std::shared_ptr<VS_MixerReceiver> &rcv, stream::Track track, stream::TrackType type, VS_MediaFormat mf);

};

