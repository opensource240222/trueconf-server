#pragma once

#include "VS_MediaPeerBase.h"
#include "VS_RTSPBroadcastCommon.h"
#include "std-generic/cpplib/string_view.h"

#include <OnDemandServerMediaSubsession.hh>
#include <Medium_deleter.hh>

#include "std-generic/compat/memory.h"

#include <map>
#include <memory>
#include <string>

class VS_RelayMediaSource;
class VS_RTSPBroadcastMediaPeer;
class VS_RTSPBroadcastParameters;

class UsageEnvironment;
class Live555Thread;
class FrameBufferSource;
class StreamReplicator;
class StreamSynchronizer;

class VS_Live555ServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
	explicit VS_Live555ServerMediaSubsession(VS_RTSPBroadcastMediaPeer* peer);

	const std::string& GetCodec() const;
	VS_RTSPSourceType GetSourceType() const;

	FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate) override;
	RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;

private:
	VS_RTSPBroadcastMediaPeer* m_peer;
};

class VS_Live555StreamSynchronizerPool
{
public:
	StreamSynchronizer* GetSynchronizer(UsageEnvironment& env, unsigned clientSessionId);

private:
	std::map<unsigned, std::unique_ptr<StreamSynchronizer, Medium_deleter>> m_data;
};

class VS_RTSPBroadcastMediaPeer : public VS_MediaPeerBase, public vs::enable_shared_from_this<VS_RTSPBroadcastMediaPeer>
{
public:
	static std::shared_ptr<VS_RTSPBroadcastMediaPeer> Create(std::string&& codec, VS_RTSPSourceType src_type, std::shared_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env);

	bool Start();
	void Stop();
	VS_Live555ServerMediaSubsession* CreateLive555SMSS();
	void SetSynchronizerPool(VS_Live555StreamSynchronizerPool* sync_pool);

	static void OnSendError(void* opaque);
	void OnSendError();

protected:
	VS_RTSPBroadcastMediaPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env);
	~VS_RTSPBroadcastMediaPeer();

	virtual bool DoStart();

private:
	friend class VS_Live555ServerMediaSubsession;
	virtual FramedSource* CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate) = 0;
	virtual RTPSink* CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) = 0;

	void ReceiveVideo(const char* peer_name, const char* stream_id, const unsigned char* pFrame, int size, bool isKey, unsigned int timestamp) override;
	void ReceiveAudio(const char* peer_name, const unsigned char* buf, int sz, unsigned int timestamp) override;
	void ChangeAudioRcvPayload(const char* plname, const int plfreq) override;
	void ChangeRcvFrameResolution(const char* peer_name, const char* stream_id, const char* plname, uint8_t pltype, unsigned short width, unsigned short height) override;

protected:
	std::string m_codec;
	VS_RTSPSourceType m_src_type;
	std::weak_ptr<VS_RelayMediaSource> m_media_source;
	std::shared_ptr<VS_RTSPBroadcastParameters> m_parameters;
	Live555Thread& m_live555_thread;
	UsageEnvironment* m_env;
	VS_Live555StreamSynchronizerPool* m_sync_pool;

	bool m_started;
	FrameBufferSource* m_fb;
	std::unique_ptr<StreamReplicator, Medium_deleter> m_replicator;
};
