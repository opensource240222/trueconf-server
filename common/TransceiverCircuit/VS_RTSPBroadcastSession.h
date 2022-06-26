#pragma once

#include "VS_RelayMediaSource.h"
#include "VS_RTSPBroadcastCommon.h"
#include "VS_RTSPBroadcastParameters.h"
#include "VS_RTSPBroadcastMediaPeer.h"
#include "std/cpplib/VS_ChildProcess.h"

#include <Medium_deleter.hh>

#include <functional>
#include <map>
#include <memory>
#include <string>

class UsageEnvironment;
class RTSPServer;
class ServerMediaSession;
class Live555Thread;
class StreamAnnouncer;
class StreamSynchronizer;
class Authenticator;

class VS_RTSPBroadcastSession
{
public:
	VS_RTSPBroadcastSession(const char* url, const char* description_utf8, string_view enabled_codecs, const char* helper_program, const std::shared_ptr<VS_RelayMediaSource>& media_source, Live555Thread& live555_thread, UsageEnvironment* env, RTSPServer* rtsp_server);
	~VS_RTSPBroadcastSession();

	using announce_status_cb = std::function<void(bool, string_view)>;
	void Announce(const char* url, const char* username, const char* password, bool rtp_over_tcp, string_view enabled_codecs, unsigned keepalive_timeout, unsigned retries, unsigned retry_delay, announce_status_cb&& cb);
	template <class Callback>
	void Announce(const char* url, const char* username, const char* password, bool rtp_over_tcp, string_view enabled_codecs, unsigned keepalive_timeout, unsigned retries, unsigned retry_delay, Callback cb)
	{
		Announce(url, username, password, rtp_over_tcp, enabled_codecs, keepalive_timeout, retries, retry_delay, announce_status_cb(cb));
	}

private:
	struct announce_info;
	static void OnAnnounceCompletion(void* opaque);
	void OnAnnounceCompletion(announce_info* announce);
	static void OnAnnounceTimeout(void* opaque);
	void OnAnnounceTimeout(announce_info* announce);
	static void RetryAnnounce(void* opaque);
	void RetryAnnounce(announce_info* announce);

	void AddSubsession(ServerMediaSession* sms, string_view codec, VS_RTSPSourceType src_type = VS_RTSPSourceType::Default);
	void AddSubsessions(ServerMediaSession* sms, string_view codecs);

private:
	std::shared_ptr<VS_RelayMediaSource> m_media_source;
	std::shared_ptr<VS_RTSPBroadcastParameters> m_parameters;
	Live555Thread& m_live555_thread;
	UsageEnvironment* m_env;
	RTSPServer* m_rtsp_server;
	ServerMediaSession* m_sms;
	ServerMediaSession* m_sms_default;
	std::unique_ptr<VS_Live555StreamSynchronizerPool> m_sync_pool;
	std::map<std::pair<std::string /*codec*/, VS_RTSPSourceType>, std::shared_ptr<VS_RTSPBroadcastMediaPeer>> m_peers;
	std::shared_ptr<VS_ChildProcess> m_helper_process;

	struct announce_info
	{
		VS_RTSPBroadcastSession* parent;
		announce_status_cb cb;
		std::unique_ptr<StreamAnnouncer, Medium_deleter> announcer;
		unsigned retries;
		unsigned retry_delay;
		TaskToken retry_token;
	};
	std::vector<std::unique_ptr<announce_info>> m_announces;
};
