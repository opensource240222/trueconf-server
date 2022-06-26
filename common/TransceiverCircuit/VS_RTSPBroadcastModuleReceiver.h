#pragma once

#include "VS_RTSPBroadcastSession.h"
#include "TransceiverLib/VS_RelayModule.h"
#include "std/cpplib/VS_Lock.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/macro_utils.h"

#include <Live555Thread.hh>
#include <Medium_deleter.hh>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <string>

class VS_MediaSourceCollection;

class UsageEnvironment;
class RTSPServer;

namespace { namespace mi = boost::multi_index; }

struct UsageEnvironment_deleter { void operator()(UsageEnvironment* p) const; };

class VS_RTSPBroadcastModuleReceiver : public VS_RelayModule
{
public:
	VS_RTSPBroadcastModuleReceiver(const boost::shared_ptr<VS_MediaSourceCollection>& media_source_collection);
	virtual ~VS_RTSPBroadcastModuleReceiver();

	bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase>& msg) override;

private:
	boost::shared_ptr<VS_MediaSourceCollection> m_media_source_collection;

	void StartRTSPBroadcast(std::string conf_name, const char* url, const char* description_utf8, string_view enabled_codecs, const char* helper_program);
	void StopRTSPBroadcast(string_view conf_name);
	void AnnounceRTSPBroadcast(string_view conf_name, string_view announce_id, const char* url, const char* username, const char* password, bool rtp_over_tcp, string_view enabled_codecs, unsigned keepalive_timeout, unsigned retries, unsigned retry_delay);

	bool InitLive555();
	void OnAnnounceStatusChange(const std::string& conf_name, const std::string& announce_id, bool is_active, string_view reason);

	std::mutex m_sessions_mutex;
	struct session_info
	{
		VS_FORWARDING_CTOR3(session_info, conf_name, url, impl) {}
		std::string conf_name;
		std::string url;
		std::unique_ptr<VS_RTSPBroadcastSession> impl;
	};
	typedef mi::indexed_by<
		mi::ordered_unique<mi::tag<struct conf_name_tag>, mi::member<session_info, std::string, &session_info::conf_name>, vs::str_less> ,
		mi::ordered_unique<mi::tag<struct url_tag>, mi::member<session_info, std::string, &session_info::url>, vs::str_less>
	> sessions_indices;
	mi::multi_index_container<session_info, sessions_indices> m_sessions;

	std::mutex m_live555_init_mutex;
	std::unique_ptr<TaskScheduler> m_scheduler;
	std::unique_ptr<UsageEnvironment, UsageEnvironment_deleter> m_env;
	std::unique_ptr<Live555Thread> m_live555_thread;
	std::unique_ptr<RTSPServer, Medium_deleter> m_rtsp_server;

};
