#include "VS_RTSPBroadcastMediaPeer.h"
#include "VS_RelayMediaSource.h"
#include "VS_RTSPBroadcastParameters.h"

#include <UsageEnvironment.hh>
#include <Live555Thread.hh>
#include <FrameBufferSource.hh>
#include <LateFrameDropper.hh>
#include <TimestampSetter.hh>
#include <H264VideoStreamFramer.hh>
#include <H264VideoStreamDiscreteFramer.hh>
#include <H264InterFrameDropper.hh>
#include <H264AccessUnitDelimiterInjector.hh>
#include <H264VideoRTPSink.hh>
#include <VP8VideoRTPSink.hh>
#include <uLawAudioFilter.hh>
#include <L16AudioRTPSink.hh>
#include <G711uAudioRTPSink.hh>
#include <G711aAudioRTPSink.hh>
#include <MPEG1or2AudioRTPSink.hh>
#include <MPEG4GenericRTPSink.hh>
#include <GSMAudioStreamFramer.hh>
#include <GSMAudioRTPSink.hh>
#include <FrameMetadataPrinter.hh>
#include <H264FrameMetadataPrinter.hh>
#include <StreamReplicator.hh>
#include <StreamSynchronizer.hh>
#include <GroupsockHelper.hh>
#include <timeval_helper.hh>

#include "../MediaParserLib/VS_AACParser.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/numerical.h"
#include "std-generic/cpplib/string_view.h"
#include "std/debuglog/VS_Debug.h"

#include <boost/algorithm/string/predicate.hpp>

#include <cstdlib>
#include <iostream>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

static inline std::pair<unsigned short/*width*/, unsigned short/*height*/> ResolutionFromCodec(string_view codec)
{
	const auto opt_pos = codec.find('+');
	if (opt_pos == codec.npos)
		return {1280, 720};

	const auto opt = codec.substr(opt_pos + 1);
	if (boost::iequals(opt, string_view("720p")))
		return {1280, 720};
	if (boost::iequals(opt, string_view("480p")))
		return {864, 480};
	if (boost::iequals(opt, string_view("360p")))
		return {640, 360};

	return {1280, 720};
}

static inline unsigned SampleRateFromCodec(string_view codec)
{
	const auto opt_pos = codec.find('+');
	if (opt_pos == codec.npos)
		return 16000;

	const std::string opt(codec.substr(opt_pos + 1));
	char* num_end = nullptr;
	unsigned result = std::strtoul(opt.c_str(), &num_end, 10);
	if (result != 0 && num_end && boost::iequals(string_view(opt).substr(num_end - opt.c_str()), "Hz"))
		return result;

	return 16000;
}

class VS_Live555MP3RTPSink : public MPEG1or2AudioRTPSink
{
public:
	static VS_Live555MP3RTPSink* createNew(UsageEnvironment& env, Groupsock* RTPgs, Boolean packFrames)
	{
		return new VS_Live555MP3RTPSink(env, RTPgs, packFrames);
	}

private:
	VS_Live555MP3RTPSink(UsageEnvironment& env, Groupsock* RTPgs, Boolean packFrames)
		: MPEG1or2AudioRTPSink(env, RTPgs)
		, fPackFrames(packFrames)
	{
	}

	Boolean frameCanAppearAfterPacketStart(unsigned char const* frameStart, unsigned numBytesInFrame) const override
	{
		return fPackFrames;
	}

	Boolean fPackFrames;
};

VS_Live555ServerMediaSubsession::VS_Live555ServerMediaSubsession(VS_RTSPBroadcastMediaPeer* peer)
	: OnDemandServerMediaSubsession(*peer->m_env, False)
	, m_peer(peer)
{
}

const std::string& VS_Live555ServerMediaSubsession::GetCodec() const
{
	return m_peer->m_codec;
}

VS_RTSPSourceType VS_Live555ServerMediaSubsession::GetSourceType() const
{
	return m_peer->m_src_type;
}

FramedSource* VS_Live555ServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
	return m_peer->CreateLive555Source(clientSessionId, estBitrate);
}

RTPSink* VS_Live555ServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	return m_peer->CreateLive555Sink(rtpGroupsock, rtpPayloadTypeIfDynamic, inputSource);
}

StreamSynchronizer* VS_Live555StreamSynchronizerPool::GetSynchronizer(UsageEnvironment& env, unsigned clientSessionId)
{
	auto it = m_data.find(clientSessionId);
	if (it == m_data.end())
	{
		std::unique_ptr<StreamSynchronizer, Medium_deleter> sync(StreamSynchronizer::createNew(env));
		it = m_data.emplace(clientSessionId, std::move(sync)).first;
	}
	return it->second.get();
}

VS_RTSPBroadcastMediaPeer::VS_RTSPBroadcastMediaPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env)
	: VS_MediaPeerBase(std::move(peer_id), std::move(part_id))
	, m_codec(std::move(codec))
	, m_src_type(src_type)
	, m_media_source(media_source)
	, m_parameters(parameters)
	, m_live555_thread(live555_thread)
	, m_env(env)
	, m_started(false)
	, m_fb(nullptr)
{
	m_type = vs_media_peer_type_rtsp;
	Live555Thread::PauseGuard guard(m_live555_thread);
	m_fb = FrameBufferSource::createNew(*m_env, m_parameters->fb_max_size_bytes, m_parameters->fb_max_size_us);
	m_fb->setUnderflowCallback([this]() {
		if (Start())
			m_fb->setUnderflowCallback(nullptr);
	});
}

VS_RTSPBroadcastMediaPeer::~VS_RTSPBroadcastMediaPeer()
{
	Live555Thread::PauseGuard guard(m_live555_thread);
	m_replicator.reset();
}

VS_Live555ServerMediaSubsession* VS_RTSPBroadcastMediaPeer::CreateLive555SMSS()
{
	Live555Thread::PauseGuard guard(m_live555_thread);
	return new VS_Live555ServerMediaSubsession(this);
}

void VS_RTSPBroadcastMediaPeer::SetSynchronizerPool(VS_Live555StreamSynchronizerPool* sync_pool)
{
	m_sync_pool = sync_pool;
}

bool VS_RTSPBroadcastMediaPeer::Start()
{
	m_started = m_started || DoStart();
	return m_started;
}

bool VS_RTSPBroadcastMediaPeer::DoStart()
{
	auto media_source(m_media_source.lock());
	if (!media_source)
		return false;
	media_source->PeerConnect(shared_from_this());
	dstream3 << "RTSPMediaPeer(" << m_codec << "): Connected\n";
	return true;
}

void VS_RTSPBroadcastMediaPeer::Stop()
{
	if (!m_started)
		return;
	auto media_source(m_media_source.lock());
	if (!media_source)
		return;
	media_source->PeerDisconnect(m_peer_id.c_str());
	m_observer->ReadyToDie(m_part_id.c_str(), m_peer_id.c_str());
}

void VS_RTSPBroadcastMediaPeer::OnSendError(void* opaque)
{
	reinterpret_cast<VS_RTSPBroadcastMediaPeer*>(opaque)->OnSendError();
}

void VS_RTSPBroadcastMediaPeer::OnSendError()
{
	dstream1 << "RTSPMediaPeer(" << m_codec << "): Network error: " << m_env->getResultMsg();
}

void VS_RTSPBroadcastMediaPeer::ReceiveVideo(const char* /*peer_name*/, const char* /*stream_id*/, const unsigned char* /*pFrame*/, int /*size*/, bool /*isKey*/, unsigned int /*timestamp*/)
{
}

void VS_RTSPBroadcastMediaPeer::ReceiveAudio(const char* /*peer_name*/, const unsigned char* /*buf*/, int /*sz*/, unsigned int /*timestamp*/)
{
}

void VS_RTSPBroadcastMediaPeer::ChangeAudioRcvPayload(const char* /*plname*/, const int /*plfreq*/)
{
}

void VS_RTSPBroadcastMediaPeer::ChangeRcvFrameResolution(const char* /*peer_name*/, const char* /*stream_id*/, const char* /*plname*/, uint8_t /*pltype*/, unsigned short /*width*/, unsigned short /*height*/)
{
}

class VS_RTSPBroadcastVideoPeer : public VS_RTSPBroadcastMediaPeer
{
protected:
	VS_RTSPBroadcastVideoPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned short width, unsigned short height);
	~VS_RTSPBroadcastVideoPeer();

	void RequestKeyFrame(unsigned delay_us = 0, unsigned count = 1);

private:

	void PutVideo(unsigned char *pFrame, unsigned long size, bool isKey, unsigned int timestamp) override;
	void PutCompressedAudio(unsigned char* /*buf*/, unsigned long /*sz*/, unsigned int /*n_samples*/, unsigned int /*timestamp*/) override
	{
	}
	void PutUncompressedAudio(unsigned char* /*buf*/, unsigned long /*sz*/, unsigned int /*n_samples*/) override
	{
	}

	static void DoRequestKeyFrame(void* opaque);
	void DoRequestKeyFrame();

protected:
	unsigned short m_width;
	unsigned short m_height;

private:
	struct timeval m_last_pt;
	unsigned int m_last_timestamp;
	TaskToken m_key_frame_request_task;
	unsigned m_extra_key_frames;
};

VS_RTSPBroadcastVideoPeer::VS_RTSPBroadcastVideoPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned short width, unsigned short height)
	: VS_RTSPBroadcastMediaPeer(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env)
	, m_width(width)
	, m_height(height)
	, m_last_pt({0, 0})
	, m_last_timestamp(0)
	, m_key_frame_request_task(0)
	, m_extra_key_frames(0)
{

}

VS_RTSPBroadcastVideoPeer::~VS_RTSPBroadcastVideoPeer()
{
	Live555Thread::PauseGuard guard(m_live555_thread);
	m_env->taskScheduler().unscheduleDelayedTask(m_key_frame_request_task);
}

void VS_RTSPBroadcastVideoPeer::RequestKeyFrame(unsigned delay_us, unsigned count)
{
	count = std::max(count, 1u);
	m_extra_key_frames = std::max(m_extra_key_frames, count - 1);
	if (delay_us > 0)
	{
		Live555Thread::PauseGuard guard(m_live555_thread);
		if (m_key_frame_request_task)
			m_env->taskScheduler().unscheduleDelayedTask(m_key_frame_request_task);
		m_key_frame_request_task = m_env->taskScheduler().scheduleDelayedTask(delay_us, &VS_RTSPBroadcastVideoPeer::DoRequestKeyFrame, this);
	}
	else
		DoRequestKeyFrame();
}

void VS_RTSPBroadcastVideoPeer::DoRequestKeyFrame(void* opaque)
{
	reinterpret_cast<VS_RTSPBroadcastVideoPeer*>(opaque)->DoRequestKeyFrame();
}

void VS_RTSPBroadcastVideoPeer::DoRequestKeyFrame()
{
	m_key_frame_request_task = 0;
	if (m_started)
		m_observer->KeyFrameRequestForPeer(m_part_id.c_str(), m_peer_id.c_str());
}

void VS_RTSPBroadcastVideoPeer::PutVideo(unsigned char *pFrame, unsigned long size, bool isKey, unsigned int timestamp)
{
	if (!m_started)
		return;

	if (isKey && m_extra_key_frames > 0)
	{
		--m_extra_key_frames;
		m_observer->KeyFrameRequestForPeer(m_part_id.c_str(), m_peer_id.c_str());
	}

	struct timeval pt;
	if (m_last_pt == timeval{ 0, 0 })
		gettimeofday(&pt, NULL);
	else
	{
		const unsigned last_frame_duration_ms = timestamp >= m_last_timestamp
			? timestamp - m_last_timestamp
			: (std::numeric_limits<unsigned>::max() - m_last_timestamp + 1) + timestamp;
		if (last_frame_duration_ms >= 1000)
		{
			// Workaround for limiting timestamp difference to 1000ms in clients which makes us to think that received frame is from the past.
			dstream2 << "RTSPMediaPeer(" << m_codec << "): Video timestamp incremented by " << last_frame_duration_ms << "ms, re-syncing to current time.";
			gettimeofday(&pt, NULL);
		}
		else
			pt = m_last_pt + std::chrono::milliseconds(last_frame_duration_ms);
	}
	m_fb->insertFrame(pFrame, size, pt, 0/*can't predict the future*/);
	m_last_pt = pt;
	m_last_timestamp = timestamp;
}

class VS_RTSPBroadcastAudioPeer : public VS_RTSPBroadcastMediaPeer
{
protected:
	VS_RTSPBroadcastAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned sample_rate);

	bool DoStart() override;

private:
	void PutVideo(unsigned char* /*pFrame*/, unsigned long /*size*/, bool /*isKey*/, unsigned /*timestamp*/) override
	{
	}
	void PutCompressedAudio(unsigned char *buf, unsigned long sz, unsigned int n_samples, unsigned int timestamp) override;
	void PutUncompressedAudio(unsigned char* /*buf*/, unsigned long /*sz*/, unsigned int /*n_samples*/) override
	{
	}

protected:
	unsigned m_sample_rate;

private:
	struct timeval m_next_pt;
};

VS_RTSPBroadcastAudioPeer::VS_RTSPBroadcastAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned sample_rate)
	: VS_RTSPBroadcastMediaPeer(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env)
	, m_sample_rate(sample_rate)
	, m_next_pt({0, 0})
{
}

bool VS_RTSPBroadcastAudioPeer::DoStart()
{
	if (!VS_RTSPBroadcastMediaPeer::DoStart())
		return false;
	VS_MediaFormat mf;
	mf.SetVideo(0, 0, 0, 0, 0, 0);
	m_observer->ChangeVideoSendPayload(m_part_id.c_str(), m_peer_id.c_str(), m_type, mf);
	return true;
}

void VS_RTSPBroadcastAudioPeer::PutCompressedAudio(unsigned char *buf, unsigned long sz, unsigned int n_samples, unsigned int timestamp)
{
	if (!m_started)
		return;

	struct timeval now;
	gettimeofday(&now, NULL);
	if (m_next_pt == timeval{ 0, 0 })
		m_next_pt = now;
	const struct timeval diff(m_next_pt - now);
	if (abs(diff) > std::chrono::microseconds(m_parameters->frame_timeout_us))
	{
		dstream2 << "RTSPMediaPeer(" << m_codec << "): Adjusting audio timestamp by " << diff << "s\n";
		m_next_pt = now;
	}
	// Apparently there are 4 bytes of garbage(?) at the beginning of the buffer
	const unsigned frame_duration_us = clamp_cast<unsigned>((n_samples*1000000ll)/m_sample_rate);
	m_fb->insertFrame(buf + sizeof(int32_t), sz, m_next_pt, frame_duration_us);
	m_next_pt += std::chrono::microseconds(frame_duration_us);
}

class VS_RTSPBroadcastPCMAudioPeer : public VS_RTSPBroadcastMediaPeer
{
public:
	VS_RTSPBroadcastPCMAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env);

	bool DoStart() override;

private:
	FramedSource* CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate) override;
	RTPSink* CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;

	void PutVideo(unsigned char* /*pFrame*/, unsigned long /*size*/, bool /*isKey*/, unsigned /*timestamp*/) override
	{
	}
	void PutCompressedAudio(unsigned char* /*buf*/, unsigned long /*sz*/, unsigned int /*n_samples*/, unsigned int /*timestamp*/) override
	{
	}
	void PutUncompressedAudio(unsigned char *buf, unsigned long sz, unsigned int n_samples) override;

private:
	struct timeval m_next_pt;
	static const unsigned c_sample_rate = 16000;
};

VS_RTSPBroadcastPCMAudioPeer::VS_RTSPBroadcastPCMAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env)
	: VS_RTSPBroadcastMediaPeer(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env)
	, m_next_pt({0, 0})
{
	m_fb->allowPartialDelivery(false);
	Live555Thread::PauseGuard guard(m_live555_thread);
	FramedSource* source = m_fb;
	TimestampSetter* ts = nullptr;
	source = LateFrameDropper::createNew(*m_env, source, m_parameters->frame_timeout_max_audio_us, m_parameters->frame_timeout_us);
	source = ts = TimestampSetter::createNew(*m_env, source);
	ts->appendAction(new SetConstantDuration(0));
	source = NetworkFromHostOrder16::createNew(*m_env, source);
	m_replicator.reset(StreamReplicator::createNew(*m_env, source, False, parameters->replicator_safety_timeout_us));
}

bool VS_RTSPBroadcastPCMAudioPeer::DoStart()
{
	if (!VS_RTSPBroadcastMediaPeer::DoStart())
		return false;
	VS_MediaFormat mf;
	mf.SetVideo(0, 0, 0, 0, 0, 0);
	m_observer->ChangeVideoSendPayload(m_part_id.c_str(), m_peer_id.c_str(), m_type, mf);
	return true;
}

FramedSource* VS_RTSPBroadcastPCMAudioPeer::CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate)
{
	estBitrate = 16*c_sample_rate/1024;
	FramedSource* source = m_replicator->createStreamReplica();
	if (m_sync_pool)
		source = m_sync_pool->GetSynchronizer(*m_env, clientSessionId)->synchronizeSource(source);
	return source;
}

RTPSink* VS_RTSPBroadcastPCMAudioPeer::CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	auto sink = L16AudioRTPSink::createNew(*m_env, rtpGroupsock, rtpPayloadTypeIfDynamic, c_sample_rate, 1, False);
	sink->enableSendRetries(m_parameters->send_retry_timeout_us, m_parameters->send_retry_interval_us);
	sink->setOnSendErrorFunc(&VS_RTSPBroadcastMediaPeer::OnSendError, this);
	return sink;
}

void VS_RTSPBroadcastPCMAudioPeer::PutUncompressedAudio(unsigned char *buf, unsigned long sz, unsigned int n_samples)
{
	if (!m_started)
		return;

	struct timeval now;
	gettimeofday(&now, NULL);
	const struct timeval diff(m_next_pt - now);
	if (abs(diff) > std::chrono::microseconds(m_parameters->frame_timeout_us))
	{
		dstream2 << "RTSPMediaPeer(" << m_codec << "): Adjusting audio timestamp by " << diff << "s\n";
		m_next_pt = now;
	}
	const unsigned frame_duration_us = clamp_cast<unsigned>((n_samples*1000000ll)/c_sample_rate);
	m_fb->insertFrame(buf, sz, m_next_pt, frame_duration_us);
	m_next_pt += std::chrono::microseconds(frame_duration_us);
}

class VS_RTSPBroadcastH264VideoPeer : public VS_RTSPBroadcastVideoPeer
{
public:
	VS_RTSPBroadcastH264VideoPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned short width, unsigned short height);

private:
	bool DoStart() override;

	FramedSource* CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate) override;
	RTPSink* CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;

private:
	H264VideoStreamFramer* m_framer;
};

VS_RTSPBroadcastH264VideoPeer::VS_RTSPBroadcastH264VideoPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned short width, unsigned short height)
	: VS_RTSPBroadcastVideoPeer(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env, width, height)
{
	m_fb->allowPartialDelivery(true);
	Live555Thread::PauseGuard guard(m_live555_thread);
	FramedSource* source = m_fb;
	TimestampSetter* ts = nullptr;
	//source = FrameMetadataPrinter::createNew(*m_env, source, "H264 fb:      ");
	source = LateFrameDropper::createNew(*m_env, source, m_parameters->frame_timeout_max_audio_us, m_parameters->frame_timeout_us);
	//source = FrameMetadataPrinter::createNew(*m_env, source, "H264 dropper: ");
	source = m_framer = H264VideoStreamFramer::createNew(*m_env, source, False, True);
	//source = H264FrameMetadataPrinter::createNew(*m_env, source, "H264 framer:  ");
	source = ts = TimestampSetter::createNew(*m_env, source);
	ts->appendAction(new LimitDuration(42)); // Just need duration to be non-zero for last NALU of the frame
	//source = H264FrameMetadataPrinter::createNew(*m_env, source, "H264 Tsetter: ");
	//source = H264AccessUnitDelimiterInjector::createNew(*m_env, source);
	//source = H264FrameMetadataPrinter::createNew(*m_env, source, "H264 AUD inj: ");
	m_replicator.reset(StreamReplicator::createNew(*m_env, source, False, parameters->replicator_safety_timeout_us));
}

bool VS_RTSPBroadcastH264VideoPeer::DoStart()
{
	if (!VS_RTSPBroadcastMediaPeer::DoStart())
		return false;
	VS_MediaFormat mf;
	mf.SetVideo(m_width, m_height, VS_VCODEC_H264);
	m_observer->ChangeVideoSendPayload(m_part_id.c_str(), m_peer_id.c_str(), m_type, mf);
	return true;
}

FramedSource* VS_RTSPBroadcastH264VideoPeer::CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate)
{
	estBitrate = 1024; //FIXME
	FramedSource* source = m_replicator->createStreamReplica();
	//source = H264InterFrameDropper::createNew(*m_env, source);
	if (m_sync_pool)
		source = m_sync_pool->GetSynchronizer(*m_env, clientSessionId)->synchronizeSource(source);
	source = H264VideoStreamDiscreteFramer::createNew(*m_env, source);
	return source;
}

RTPSink* VS_RTSPBroadcastH264VideoPeer::CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	RequestKeyFrame(m_parameters->key_frame_delay_us, 2);
	u_int8_t* vps = nullptr; // unused
	unsigned int vps_size = 0; // unused
	u_int8_t* sps = nullptr;
	unsigned int sps_size = 0;
	u_int8_t* pps = nullptr;
	unsigned int pps_size = 0;
	m_framer->getVPSandSPSandPPS(vps, vps_size, sps, sps_size, pps, pps_size);
	auto sink = H264VideoRTPSink::createNew(*m_env, rtpGroupsock, rtpPayloadTypeIfDynamic, sps, sps_size, pps, pps_size);
	sink->enableSendRetries(m_parameters->send_retry_timeout_us, m_parameters->send_retry_interval_us);
	sink->setOnSendErrorFunc(&VS_RTSPBroadcastMediaPeer::OnSendError, this);
	sink->setXDimensions(m_width, m_height);
	return sink;
}

class VS_RTSPBroadcastVP8VideoPeer : public VS_RTSPBroadcastVideoPeer
{
public:
	VS_RTSPBroadcastVP8VideoPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned short width, unsigned short height);

private:
	bool DoStart() override;

	FramedSource* CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate) override;
	RTPSink* CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;
};

VS_RTSPBroadcastVP8VideoPeer::VS_RTSPBroadcastVP8VideoPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned short width, unsigned short height)
	: VS_RTSPBroadcastVideoPeer(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env, width, height)
{
	m_fb->allowPartialDelivery(false);
	Live555Thread::PauseGuard guard(m_live555_thread);
	FramedSource* source = m_fb;
	TimestampSetter* ts = nullptr;
	//source = FrameMetadataPrinter::createNew(*m_env, source, "VP8 fb:      ");
	source = LateFrameDropper::createNew(*m_env, source, m_parameters->frame_timeout_max_audio_us, m_parameters->frame_timeout_us);
	//source = FrameMetadataPrinter::createNew(*m_env, source, "VP8 dropper: ");
	source = ts = TimestampSetter::createNew(*m_env, source);
	ts->appendAction(new SetConstantDuration(0));
	//source = FrameMetadataPrinter::createNew(*m_env, source, "VP8 Tsetter: ");
	m_replicator.reset(StreamReplicator::createNew(*m_env, source, False, parameters->replicator_safety_timeout_us));
}

bool VS_RTSPBroadcastVP8VideoPeer::DoStart()
{
	if (!VS_RTSPBroadcastMediaPeer::DoStart())
		return false;
	VS_MediaFormat mf;
	mf.SetVideo(m_width, m_height, VS_VCODEC_VPX);
	m_observer->ChangeVideoSendPayload(m_part_id.c_str(), m_peer_id.c_str(), m_type, mf);
	return true;
}

FramedSource* VS_RTSPBroadcastVP8VideoPeer::CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate)
{
	estBitrate = 512; //FIXME
	FramedSource* source = m_replicator->createStreamReplica();
	if (m_sync_pool)
		source = m_sync_pool->GetSynchronizer(*m_env, clientSessionId)->synchronizeSource(source);
	return source;
}

RTPSink* VS_RTSPBroadcastVP8VideoPeer::CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	RequestKeyFrame(m_parameters->key_frame_delay_us, 2);
	// Hack to increase OutPacketBuffer::maxSize only for VP8 sinks
	auto saved_OutPacketBuffer_maxSize(OutPacketBuffer::maxSize);
	OutPacketBuffer::maxSize = 256*1024;
	auto sink = VP8VideoRTPSink::createNew(*m_env, rtpGroupsock, rtpPayloadTypeIfDynamic);
	OutPacketBuffer::maxSize = saved_OutPacketBuffer_maxSize;
	sink->enableSendRetries(m_parameters->send_retry_timeout_us, m_parameters->send_retry_interval_us);
	sink->setOnSendErrorFunc(&VS_RTSPBroadcastMediaPeer::OnSendError, this);
	sink->setXDimensions(m_width, m_height);
	return sink;
}

class VS_RTSPBroadcastG711uAudioPeer : public VS_RTSPBroadcastAudioPeer
{
public:
	VS_RTSPBroadcastG711uAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env);

private:
	bool DoStart() override;

	FramedSource* CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate) override;
	RTPSink* CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;
};

VS_RTSPBroadcastG711uAudioPeer::VS_RTSPBroadcastG711uAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env)
	: VS_RTSPBroadcastAudioPeer(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env, 8000)
{
	m_fb->allowPartialDelivery(false);
	Live555Thread::PauseGuard guard(m_live555_thread);
	FramedSource* source = m_fb;
	TimestampSetter* ts = nullptr;
	//source = FrameMetadataPrinter::createNew(*m_env, source, "G711u fb:      ");
	source = LateFrameDropper::createNew(*m_env, source, m_parameters->frame_timeout_max_audio_us, m_parameters->frame_timeout_us);
	//source = FrameMetadataPrinter::createNew(*m_env, source, "G711u dropper: ");
	source = ts = TimestampSetter::createNew(*m_env, source);
	ts->appendAction(new SetConstantDuration(0));
	//source = FrameMetadataPrinter::createNew(*m_env, source, "G711u Tsetter: ");
	m_replicator.reset(StreamReplicator::createNew(*m_env, source, False, parameters->replicator_safety_timeout_us));
}

bool VS_RTSPBroadcastG711uAudioPeer::DoStart()
{
	if (!VS_RTSPBroadcastAudioPeer::DoStart())
		return false;
	m_observer->ChangeAudioSendPayload(m_part_id.c_str(), m_peer_id.c_str(), "PCMU", m_sample_rate);
	return true;
}

FramedSource* VS_RTSPBroadcastG711uAudioPeer::CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate)
{
	estBitrate = 64; // = 8*m_sample_rate/1024;
	FramedSource* source = m_replicator->createStreamReplica();
	if (m_sync_pool)
		source = m_sync_pool->GetSynchronizer(*m_env, clientSessionId)->synchronizeSource(source);
	return source;
}

RTPSink* VS_RTSPBroadcastG711uAudioPeer::CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	auto sink = G711uAudioRTPSink::createNew(*m_env, rtpGroupsock, False);
	sink->enableSendRetries(m_parameters->send_retry_timeout_us, m_parameters->send_retry_interval_us);
	sink->setOnSendErrorFunc(&VS_RTSPBroadcastMediaPeer::OnSendError, this);
	return sink;
}

class VS_RTSPBroadcastG711aAudioPeer : public VS_RTSPBroadcastAudioPeer
{
public:
	VS_RTSPBroadcastG711aAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env);

private:
	bool DoStart() override;

	FramedSource* CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate) override;
	RTPSink* CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;
};

VS_RTSPBroadcastG711aAudioPeer::VS_RTSPBroadcastG711aAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env)
	: VS_RTSPBroadcastAudioPeer(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env, 8000)
{
	m_fb->allowPartialDelivery(false);
	Live555Thread::PauseGuard guard(m_live555_thread);
	FramedSource* source = m_fb;
	TimestampSetter* ts = nullptr;
	//source = FrameMetadataPrinter::createNew(*m_env, source, "G711a fb:      ");
	source = LateFrameDropper::createNew(*m_env, source, m_parameters->frame_timeout_max_audio_us, m_parameters->frame_timeout_us);
	//source = FrameMetadataPrinter::createNew(*m_env, source, "G711a dropper: ");
	source = ts = TimestampSetter::createNew(*m_env, source);
	ts->appendAction(new SetConstantDuration(0));
	//source = FrameMetadataPrinter::createNew(*m_env, source, "G711a Tsetter: ");
	m_replicator.reset(StreamReplicator::createNew(*m_env, source, False, parameters->replicator_safety_timeout_us));
}

bool VS_RTSPBroadcastG711aAudioPeer::DoStart()
{
	if (!VS_RTSPBroadcastAudioPeer::DoStart())
		return false;
	m_observer->ChangeAudioSendPayload(m_part_id.c_str(), m_peer_id.c_str(), "PCMA", m_sample_rate);
	return true;
}

FramedSource* VS_RTSPBroadcastG711aAudioPeer::CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate)
{
	estBitrate = 64; // = 8*m_sample_rate/1024;
	FramedSource* source = m_replicator->createStreamReplica();
	if (m_sync_pool)
		source = m_sync_pool->GetSynchronizer(*m_env, clientSessionId)->synchronizeSource(source);
	return source;
}

RTPSink* VS_RTSPBroadcastG711aAudioPeer::CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	auto sink = G711aAudioRTPSink::createNew(*m_env, rtpGroupsock, False);
	sink->enableSendRetries(m_parameters->send_retry_timeout_us, m_parameters->send_retry_interval_us);
	sink->setOnSendErrorFunc(&VS_RTSPBroadcastMediaPeer::OnSendError, this);
	return sink;
}

class VS_RTSPBroadcastMP3AudioPeer : public VS_RTSPBroadcastAudioPeer
{
public:
	VS_RTSPBroadcastMP3AudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned sample_rate);

private:
	bool DoStart() override;

	FramedSource* CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate) override;
	RTPSink* CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;
};

VS_RTSPBroadcastMP3AudioPeer::VS_RTSPBroadcastMP3AudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned sample_rate)
	: VS_RTSPBroadcastAudioPeer(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env, sample_rate)
{
	m_fb->allowPartialDelivery(false);
	Live555Thread::PauseGuard guard(m_live555_thread);
	FramedSource* source = m_fb;
	TimestampSetter* ts = nullptr;
	//source = FrameMetadataPrinter::createNew(*m_env, source, "MP3 fb:      ");
	source = LateFrameDropper::createNew(*m_env, source, m_parameters->frame_timeout_max_audio_us, m_parameters->frame_timeout_us);
	//source = FrameMetadataPrinter::createNew(*m_env, source, "MP3 dropper: ");
	source = ts = TimestampSetter::createNew(*m_env, source);
	ts->appendAction(new SetConstantDuration(0));
	//source = FrameMetadataPrinter::createNew(*m_env, source, "MP3 Tsetter: ");
	m_replicator.reset(StreamReplicator::createNew(*m_env, source, False, parameters->replicator_safety_timeout_us));
}

bool VS_RTSPBroadcastMP3AudioPeer::DoStart()
{
	if (!VS_RTSPBroadcastAudioPeer::DoStart())
		return false;
	m_observer->ChangeAudioSendPayload(m_part_id.c_str(), m_peer_id.c_str(), "MP3", m_sample_rate);
	return true;
}

FramedSource* VS_RTSPBroadcastMP3AudioPeer::CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate)
{
	estBitrate = 64; //FIXME
	FramedSource* source = m_replicator->createStreamReplica();
	if (m_sync_pool)
		source = m_sync_pool->GetSynchronizer(*m_env, clientSessionId)->synchronizeSource(source);
	return source;
}

RTPSink* VS_RTSPBroadcastMP3AudioPeer::CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	auto sink = VS_Live555MP3RTPSink::createNew(*m_env, rtpGroupsock, False);
	sink->enableSendRetries(m_parameters->send_retry_timeout_us, m_parameters->send_retry_interval_us);
	sink->setOnSendErrorFunc(&VS_RTSPBroadcastMediaPeer::OnSendError, this);
	sink->setXSampleRate(m_sample_rate);
	// Shitty Wowza doesn't recognize MP3 streams without a=rtpmap: line
	sink->forceRtpmapLine() = True;
	return sink;
}

class VS_RTSPBroadcastAACAudioPeer : public VS_RTSPBroadcastAudioPeer
{
public:
	VS_RTSPBroadcastAACAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned sample_rate);

private:
	bool DoStart() override;

	FramedSource* CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate) override;
	RTPSink* CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;
};

VS_RTSPBroadcastAACAudioPeer::VS_RTSPBroadcastAACAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env, unsigned sample_rate)
	: VS_RTSPBroadcastAudioPeer(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env, sample_rate)
{
	m_fb->allowPartialDelivery(false);
	Live555Thread::PauseGuard guard(m_live555_thread);
	FramedSource* source = m_fb;
	TimestampSetter* ts = nullptr;
	//source = FrameMetadataPrinter::createNew(*m_env, source, "AAC fb:      ");
	source = LateFrameDropper::createNew(*m_env, source, m_parameters->frame_timeout_max_audio_us, m_parameters->frame_timeout_us);
	//source = FrameMetadataPrinter::createNew(*m_env, source, "AAC dropper: ");
	source = ts = TimestampSetter::createNew(*m_env, source);
	ts->appendAction(new SetConstantDuration(0));
	//source = FrameMetadataPrinter::createNew(*m_env, source, "AAC Tsetter: ");
	m_replicator.reset(StreamReplicator::createNew(*m_env, source, False, parameters->replicator_safety_timeout_us));
}

bool VS_RTSPBroadcastAACAudioPeer::DoStart()
{
	if (!VS_RTSPBroadcastAudioPeer::DoStart())
		return false;
	m_observer->ChangeAudioSendPayload(m_part_id.c_str(), m_peer_id.c_str(), "AAC", m_sample_rate);
	return true;
}

FramedSource* VS_RTSPBroadcastAACAudioPeer::CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate)
{
	estBitrate = 64; //FIXME
	FramedSource* source = m_replicator->createStreamReplica();
	if (m_sync_pool)
		source = m_sync_pool->GetSynchronizer(*m_env, clientSessionId)->synchronizeSource(source);
	return source;
}

RTPSink* VS_RTSPBroadcastAACAudioPeer::CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	uint8_t config[5] = {};
	size_t config_size = sizeof(config);
	MakeMPEG4AudioConfig(config, config_size, 2/*AAC Low Complexity*/, MPEG4FrequencyIndexFromSamplingRate(m_sample_rate), 1, m_sample_rate);
	std::ostringstream s;
	s << std::hex << std::setfill('0');
	for (size_t i = 0; i < config_size; ++i)
		s << std::setw(2) << static_cast<unsigned>(config[i]);
	auto sink = MPEG4GenericRTPSink::createNew(*m_env, rtpGroupsock, rtpPayloadTypeIfDynamic, m_sample_rate, "audio", "AAC-hbr", s.str().c_str(), 1);
	sink->enableSendRetries(m_parameters->send_retry_timeout_us, m_parameters->send_retry_interval_us);
	sink->setOnSendErrorFunc(&VS_RTSPBroadcastMediaPeer::OnSendError, this);
	sink->setXSampleRate(m_sample_rate);
	return sink;
}

class VS_RTSPBroadcastGSMAudioPeer : public VS_RTSPBroadcastAudioPeer
{
public:
	VS_RTSPBroadcastGSMAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env);

private:
	bool DoStart() override;

	FramedSource* CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate) override;
	RTPSink* CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;
};

VS_RTSPBroadcastGSMAudioPeer::VS_RTSPBroadcastGSMAudioPeer(std::string&& codec, VS_RTSPSourceType src_type, std::string&& peer_id, std::string&& part_id, std::weak_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env)
	: VS_RTSPBroadcastAudioPeer(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env, 8000)
{
	m_fb->allowPartialDelivery(true);
	Live555Thread::PauseGuard guard(m_live555_thread);
	FramedSource* source = m_fb;
	TimestampSetter* ts = nullptr;
	//source = FrameMetadataPrinter::createNew(*m_env, source, "GSM fb:      ");
	source = LateFrameDropper::createNew(*m_env, source, m_parameters->frame_timeout_max_audio_us, m_parameters->frame_timeout_us);
	//source = FrameMetadataPrinter::createNew(*m_env, source, "GSM dropper: ");
	source = GSMAudioStreamFramer::createNew(*m_env, source, 1);
	//source = FrameMetadataPrinter::createNew(*m_env, source, "GSM framer:  ");
	source = ts = TimestampSetter::createNew(*m_env, source);
	ts->appendAction(new SetConstantDuration(0));
	//source = FrameMetadataPrinter::createNew(*m_env, source, "GSM Tsetter: ");
	m_replicator.reset(StreamReplicator::createNew(*m_env, source, False, parameters->replicator_safety_timeout_us));
}

bool VS_RTSPBroadcastGSMAudioPeer::DoStart()
{
	if (!VS_RTSPBroadcastAudioPeer::DoStart())
		return false;
	m_observer->ChangeAudioSendPayload(m_part_id.c_str(), m_peer_id.c_str(), "GSM", m_sample_rate);
	return true;
}

FramedSource* VS_RTSPBroadcastGSMAudioPeer::CreateLive555Source(unsigned clientSessionId, unsigned& estBitrate)
{
	estBitrate = 14; // 13200 b/s per RFC3551 section 4.5.8
	FramedSource* source = m_replicator->createStreamReplica();
	if (m_sync_pool)
		source = m_sync_pool->GetSynchronizer(*m_env, clientSessionId)->synchronizeSource(source);
	return source;
}

RTPSink* VS_RTSPBroadcastGSMAudioPeer::CreateLive555Sink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	auto sink = GSMAudioRTPSink::createNew(*m_env, rtpGroupsock);
	sink->enableSendRetries(m_parameters->send_retry_timeout_us, m_parameters->send_retry_interval_us);
	sink->setOnSendErrorFunc(&VS_RTSPBroadcastMediaPeer::OnSendError, this);
	return sink;
}

std::shared_ptr<VS_RTSPBroadcastMediaPeer> VS_RTSPBroadcastMediaPeer::Create(std::string&& codec, VS_RTSPSourceType src_type, std::shared_ptr<VS_RelayMediaSource> media_source, std::shared_ptr<VS_RTSPBroadcastParameters> parameters, Live555Thread& live555_thread, UsageEnvironment* env)
{
	std::string peer_id_suffix;
	media_source->GenerateNewPeerId(peer_id_suffix);
	std::string peer_id;
	peer_id.reserve(5 + codec.size() + 1 + peer_id_suffix.size());
	peer_id.append("rtsp:").append(codec.begin(), codec.end()).append(":").append(peer_id_suffix);
	std::string part_id;
	if (static_cast<bool>(src_type & VS_RTSPSourceType::Mix))
		part_id.clear();
	else if (static_cast<bool>(src_type & VS_RTSPSourceType::Speaker))
	{
		if (media_source->GetConfType() == CT_MULTISTREAM && (media_source->GetConfSubType() == GCST_ALL_TO_OWNER || media_source->GetConfSubType() == GCST_PENDING_ALL_TO_OWNER))
			part_id = media_source->GetPodiumPartsConf();
		else
			part_id = media_source->GetConfOwner();
	}

	if      (boost::istarts_with(codec, string_view("H264")))
	{
		unsigned short width;
		unsigned short height;
		std::tie(width, height) = ResolutionFromCodec(codec);
		return std::make_shared<VS_RTSPBroadcastH264VideoPeer>(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env, width, height);
	}
	else if (boost::istarts_with(codec, string_view("VP8")))
	{
		unsigned short width;
		unsigned short height;
		std::tie(width, height) = ResolutionFromCodec(codec);
		return std::make_shared<VS_RTSPBroadcastVP8VideoPeer>(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env, width, height);
	}
	else if (boost::iequals(codec, string_view("L16")))
		return std::make_shared<VS_RTSPBroadcastPCMAudioPeer>(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env);
	else if (boost::iequals(codec, string_view("G711Ulaw64k")))
		return std::make_shared<VS_RTSPBroadcastG711uAudioPeer>(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env);
	else if (boost::iequals(codec, string_view("G711Alaw64k")))
		return std::make_shared<VS_RTSPBroadcastG711aAudioPeer>(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env);
	else if (boost::istarts_with(codec, string_view("MP3")))
		return std::make_shared<VS_RTSPBroadcastMP3AudioPeer>(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env, SampleRateFromCodec(codec));
	else if (boost::istarts_with(codec, string_view("AAC")))
		return std::make_shared<VS_RTSPBroadcastAACAudioPeer>(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env, SampleRateFromCodec(codec));
#if 0
	else if (boost::iequals(codec, string_view("GSM")))
		return std::make_shared<VS_RTSPBroadcastGSMAudioPeer>(std::move(codec), src_type, std::move(peer_id), std::move(part_id), media_source, parameters, live555_thread, env);
#endif
	else
		return nullptr;
}

#undef DEBUG_CURRENT_MODULE
