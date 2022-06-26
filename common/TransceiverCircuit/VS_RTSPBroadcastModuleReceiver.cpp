#include "VS_RTSPBroadcastModuleReceiver.h"
#include "VS_MediaSourceCollection.h"
#include "VS_RelayMediaSource.h"
#include "VS_Live555RTSPServer.h"

#include "../TransceiverLib/VS_RTSPBroadcastRelayMessage.h"
#include "TransceiverLib/VS_ControlRelayMessage.h"
#include "../net/EndpointRegistry.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_Utils.h"
#include "../std/debuglog/VS_Debug.h"
#include "std-generic/cpplib/hton.h"

#include <BasicUsageEnvironment.hh>
#include <PollTaskScheduler.hh>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/make_shared.hpp>

#include <functional>
#include <cstring>
#include <memory>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

void UsageEnvironment_deleter::operator()(UsageEnvironment* p) const
{
	if (p && !p->reclaim())
		dstream1 << "RTSPModule: Failed to free Live555 environment at 0x" << p;
}

VS_RTSPBroadcastModuleReceiver::VS_RTSPBroadcastModuleReceiver(const boost::shared_ptr<VS_MediaSourceCollection>& media_source_collection)
	: VS_RelayModule(VS_RTSPBroadcastRelayMessage::module_name)
	, m_media_source_collection(media_source_collection)
	, m_env(nullptr)
{
}

bool VS_RTSPBroadcastModuleReceiver::InitLive555()
{
	std::lock_guard<std::mutex> lock(m_live555_init_mutex);
	if (!m_scheduler)
	{
		m_scheduler.reset(PollTaskScheduler::createNew(10));
		if (!m_scheduler)
		{
			dstream1 << "RTSPModule: Failed to create live555 scheduler\n";
			return false;
		}
	}
	if (!m_env)
	{
		m_env.reset(BasicUsageEnvironment::createNew(*m_scheduler));
		if (!m_env)
		{
			dstream1 << "RTSPModule: Failed to create live555 UsageEnvironment\n";
			return false;
		}
	}
	if (!m_live555_thread)
	{
		m_live555_thread.reset(new Live555Thread(m_env.get()));
		if (!m_live555_thread)
		{
			dstream1 << "RTSPModule: Failed to create live555 thread\n";
			return false;
		}
	}
	if (!m_rtsp_server)
	{
		char secret[33];
		VS_GenKeyByMD5(secret);

		unsigned client_timeout = 10;
		VS_RegistryKey conf_key(false, CONFIGURATION_KEY);
		conf_key.GetValue(&client_timeout, sizeof(client_timeout), VS_REG_INTEGER_VT, "RTSP Inactive Client Timeout");

		Port live555_port(0);
		m_rtsp_server.reset(VS_Live555RTSPServer::createNew(*m_env, live555_port, NULL, client_timeout, secret));
		if (!m_rtsp_server)
		{
			dstream1 << "RTSPModule: Failed to create RTSP server on port: " << m_env->getResultMsg();
			return false;
		}

		const net::port port = vs_ntohs(live555_port.num());
		dstream0 << "RTSPModule: RTSP server started: port=" << port << ", client_timeout=" << client_timeout;

		auto msg = boost::make_shared<VS_ControlRelayMessage>();
		msg->MakeLive555Info(port, secret);
		SendMsg(msg);
	}
	return true;
}

VS_RTSPBroadcastModuleReceiver::~VS_RTSPBroadcastModuleReceiver()
{
	// Destructor could have done it too, but it needs to be done in this specific order
	if (m_live555_thread)
	{
		Live555Thread::PauseGuard guard(*m_live555_thread);
		m_sessions.clear();
		m_rtsp_server.reset();
	}
	m_live555_thread.reset();
	m_env.reset();
	m_scheduler.reset();
}

bool VS_RTSPBroadcastModuleReceiver::ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase>& msg_in)
{
	std::unique_ptr<VS_RTSPBroadcastRelayMessage> msg(new VS_RTSPBroadcastRelayMessage);
	if(msg->SetMessage(msg_in->GetMess()))
	{
		switch(msg->GetMessageType())
		{
		case VS_RTSPBroadcastRelayMessage::e_StartRTSPBroadcast:
			StartRTSPBroadcast(std::string(msg->GetConferenceName()), msg->GetURL(), msg->GetDescription(), msg->GetEnabledCodecs(), msg->GetHelperProgram());
			break;
		case VS_RTSPBroadcastRelayMessage::e_StopRTSPBroadcast:
			StopRTSPBroadcast(msg->GetConferenceName());
			break;
		case VS_RTSPBroadcastRelayMessage::e_AnnounceRTSPBroadcast:
			AnnounceRTSPBroadcast(
				msg->GetConferenceName(),
				msg->GetAnnounceID(),
				msg->GetURL(),
				msg->GetUsername(),
				msg->GetPassword(),
				msg->GetRTPOverTCP(),
				msg->GetEnabledCodecs(),
				msg->GetKepaliveTimeout(),
				msg->GetRetries(),
				msg->GetRetryDelay()
			);
			break;
		default:
			dstream2 << "RTSPModule: Unknown relay message: type=" << msg->GetMessageType();
		}
	}
	return true;
}

void VS_RTSPBroadcastModuleReceiver::StartRTSPBroadcast(std::string conf_name, const char* url, const char* description_utf8, string_view enabled_codecs, const char* helper_program)
{
	dstream4 << "RTSPModule: StartRTSPBroadcast:"
		<<  " conf_name=" << conf_name
		<< ", url=\"" << url << '\"'
		<< ", description_utf8=\"" << description_utf8 << '\"'
		<< ", enabled_codecs=\"" << enabled_codecs << '\"'
		<< '\n';
	if (conf_name.empty())
		return;

	auto source = m_media_source_collection->GetMediaSource(conf_name.c_str());
	if (!source)
		return;

	if (!InitLive555())
		return;

	std::lock_guard<std::mutex> lock(m_sessions_mutex);
	// Ignore request to start already started conference
	if (m_sessions.get<conf_name_tag>().count(conf_name) > 0)
	{
		dstream2 << "RTSPModule: Streaming of conf=" << conf_name << " already started\n";
		return;
	}

	auto& url_idx = m_sessions.get<url_tag>();
	auto it = url_idx.find(string_view(url));
	// We will stop broadcast on the same url as in the request if there is one
	if (it == url_idx.end())
		it = url_idx.emplace(conf_name, url, nullptr).first;
	assert(it != url_idx.end());

	url_idx.modify(it, [&](session_info& s) {
		s.conf_name = std::move(conf_name);
		// IMPORTANT: we must delete previous session impl (if any) before creating new one
		s.impl.reset();
		s.impl.reset(new VS_RTSPBroadcastSession(url, description_utf8, enabled_codecs, helper_program, source, *m_live555_thread, m_env.get(), m_rtsp_server.get()));
	});
}

void VS_RTSPBroadcastModuleReceiver::StopRTSPBroadcast(string_view conf_name)
{
	dstream4 << "RTSPModule: StopRTSPBroadcast: conf_name=" << conf_name;
	std::lock_guard<std::mutex> lock(m_sessions_mutex);
	auto& name_idx = m_sessions.get<conf_name_tag>();
	auto it = name_idx.find(conf_name);
	if (it == name_idx.end())
		return;
	name_idx.erase(it);
}

void VS_RTSPBroadcastModuleReceiver::AnnounceRTSPBroadcast(string_view conf_name, string_view announce_id, const char* url, const char* username, const char* password, bool rtp_over_tcp, string_view enabled_codecs, unsigned keepalive_timeout, unsigned retries, unsigned retry_delay)
{
	dstream4 << "RTSPModule: AnnounceRTSPBroadcast:"
		<<  " conf_name=" << conf_name
		<< ", announce_id=" << announce_id
		<< ", url=\"" << url << '\"'
		<< ", username " << (username ? "present" : "not present")
		<< ", password " << (password ? "present" : "not present")
		<< ", rtp_over_tcp=" << std::boolalpha << rtp_over_tcp
		<< ", enabled_codecs=\"" << enabled_codecs << '\"'
		<< ", keepalive_timeout=" << keepalive_timeout
		<< ", retries=" << retries
		<< ", retry_delay=" << retry_delay
		<< '\n';
	std::lock_guard<std::mutex> lock(m_sessions_mutex);
	auto& name_idx = m_sessions.get<conf_name_tag>();
	auto it = name_idx.find(conf_name);
	if (it == name_idx.end())
		return;
	it->impl->Announce(url, username, password, rtp_over_tcp, enabled_codecs, keepalive_timeout, retries, retry_delay, std::bind(&VS_RTSPBroadcastModuleReceiver::OnAnnounceStatusChange, this, std::string(conf_name), std::string(announce_id), std::placeholders::_1, std::placeholders::_2));
}

void VS_RTSPBroadcastModuleReceiver::OnAnnounceStatusChange(const std::string& conf_name, const std::string& announce_id, bool is_active, string_view reason)
{
	boost::shared_ptr<VS_RTSPBroadcastRelayMessage> msg(boost::make_shared<VS_RTSPBroadcastRelayMessage>());
	msg->MakeAnnounceStatusReport(conf_name, announce_id, is_active, reason);
	SendMsg(msg);
}
