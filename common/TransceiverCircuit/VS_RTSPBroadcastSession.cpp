#include "VS_RTSPBroadcastSession.h"

#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/clib/vs_time.h"
#include "std/debuglog/VS_Debug.h"
#include "std/Globals.h"
#include "std/VS_TransceiverInfo.h"

#include <UsageEnvironment.hh>
#include <Live555Thread.hh>
#include <RTSPServer.hh>
#include <StreamAnnouncer.hh>
#include <StreamSynchronizer.hh>
#include <DigestAuthentication.hh>

#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "std-generic/compat/iomanip.h"
#include <cassert>
#include <iostream>
#include <thread>
#include "std-generic/cpplib/ThreadUtils.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

#define HELPER_PROCESS_EXIT_TIMEOUT_MS (5000)

static std::string GetSessionCodecNames(ServerMediaSession* sms)
{
	std::string result;
	result.reserve(256);
	ServerMediaSubsessionIterator smss_it(*sms);
	ServerMediaSubsession* smss;
	while (NULL != (smss = smss_it.next()))
	{
		assert(dynamic_cast<VS_Live555ServerMediaSubsession*>(smss));
		const auto our_smss = static_cast<VS_Live555ServerMediaSubsession*>(smss);
		if      (static_cast<bool>(our_smss->GetSourceType() & VS_RTSPSourceType::Mix))
			result += "mix:";
		else if (static_cast<bool>(our_smss->GetSourceType() & VS_RTSPSourceType::Speaker))
			result += "speaker:";
		result += our_smss->GetCodec();
		result += ' ';
	}
	if (!result.empty())
		result.pop_back();
	return result;
}

VS_RTSPBroadcastSession::VS_RTSPBroadcastSession(const char* url, const char* description_utf8, string_view enabled_codecs, const char* helper_program, const std::shared_ptr<VS_RelayMediaSource>& media_source, Live555Thread& live555_thread, UsageEnvironment* env, RTSPServer* rtsp_server)
	: m_media_source(media_source)
	, m_live555_thread(live555_thread)
	, m_env(env)
	, m_rtsp_server(rtsp_server)
	, m_sms(nullptr)
	, m_sms_default(nullptr)
{
	VS_MediaFormat conf_mf;
	m_media_source->GetPeerMediaFormat(&conf_mf, vs_media_peer_type_rtsp);
	const unsigned frame_duration_us = 1000000 / (conf_mf.dwFps > 0 ? conf_mf.dwFps : 15);
	m_parameters = std::make_shared<VS_RTSPBroadcastParameters>(frame_duration_us);

	auto ds = dstream3;

	{
		Live555Thread::PauseGuard guard(m_live555_thread);
		m_sync_pool.reset(new VS_Live555StreamSynchronizerPool());

		m_sms = ServerMediaSession::createNew(*m_env, std::string(url).append("/all").c_str(), description_utf8, description_utf8);
		AddSubsession(m_sms, "H264+720p",   VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "H264+480p",   VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "H264+360p",   VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "H264+720p",   VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "H264+480p",   VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "H264+360p",   VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "VP8+720p",    VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "VP8+480p",    VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "VP8+360p",    VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "VP8+720p",    VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "VP8+480p",    VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "VP8+360p",    VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "MP3+48000Hz", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "MP3+44100Hz", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "MP3+32000Hz", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "MP3+22050Hz", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "MP3+16000Hz", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "MP3+48000Hz", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "MP3+44100Hz", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "MP3+32000Hz", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "MP3+22050Hz", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "MP3+16000Hz", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "AAC+48000Hz", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "AAC+44100Hz", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "AAC+32000Hz", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "AAC+22050Hz", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "AAC+16000Hz", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "AAC+48000Hz", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "AAC+44100Hz", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "AAC+32000Hz", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "AAC+22050Hz", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "AAC+16000Hz", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "G711Ulaw64k", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "G711Ulaw64k", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "G711Alaw64k", VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "G711Alaw64k", VS_RTSPSourceType::Speaker);
		AddSubsession(m_sms, "L16",         VS_RTSPSourceType::Mix    );
		AddSubsession(m_sms, "L16",         VS_RTSPSourceType::Speaker);
		m_rtsp_server->addServerMediaSession(m_sms);

		m_sms_default = ServerMediaSession::createNew(*m_env, url, description_utf8, description_utf8);
		AddSubsessions(m_sms_default, !enabled_codecs.empty() ? enabled_codecs : "H264+720p MP3+16000Hz");
		m_rtsp_server->addServerMediaSession(m_sms_default);

		ds << "RTSPSession: Initialized streaming: url=\"" << url << "\", codecs={" << GetSessionCodecNames(m_sms) << "}, default_codecs={" << GetSessionCodecNames(m_sms_default) << '}';
	}

	if (helper_program && *helper_program)
	{
		std::string helper_process_log;
		m_helper_process = std::make_shared<VS_ChildProcess>(helper_program);

		// redirect output
		if (ds.enabled())
		{
			auto now = std::time(0);
			tm now_tm;
			std::ostringstream log_name;
			log_name << vs::GetLogDirectory() << ts::LOG_DIRECTORY_NAME << "/rtsp_helper_" << vs::put_time(localtime_r(&now, &now_tm), "%Y-%m-%d_(%H-%M-%S)") << ".log";
			helper_process_log = log_name.str();
			if (!m_helper_process->RedirectOutputToFile(helper_process_log.c_str(), true))
			{
				helper_process_log.clear();
				dstream0 << "Failed to redirect output for RTSP Helper Program! (Transceiver PID = " << m_helper_process->GetPID() << ")";
			}
		}
		if (m_helper_process->Start())
		{
			auto ds2 = dstream3;
			ds2 << "Started RTSP Helper Program (PID: " << m_helper_process->GetPID() << ")";
			if (!helper_process_log.empty())
				ds2 << ", output is redirected to file: " << helper_process_log;
		}
		else
		{
			dprint0("Starting RTSP Helper Program failed.");
			m_helper_process = nullptr;
		}
	}
}

void VS_RTSPBroadcastSession::AddSubsession(ServerMediaSession* sms, string_view codec, VS_RTSPSourceType src_type)
{
	assert(src_type == VS_RTSPSourceType::Default || src_type == VS_RTSPSourceType::Mix || src_type == VS_RTSPSourceType::Speaker);
	if (src_type == VS_RTSPSourceType::Default)
	{
		const bool is_asym_conf = m_media_source->GetConfType() == CT_MULTISTREAM && (m_media_source->GetConfSubType() == GCST_ALL_TO_OWNER || m_media_source->GetConfSubType() == GCST_PENDING_ALL_TO_OWNER);
		src_type = is_asym_conf ? VS_RTSPSourceType::Speaker : VS_RTSPSourceType::Mix;
	}

	std::pair<std::string, VS_RTSPSourceType> lookup_key(codec, src_type);
	auto it = m_peers.find(lookup_key);
	if (it == m_peers.end())
	{
		const bool is_asym_conf = m_media_source->GetConfType() == CT_MULTISTREAM && (m_media_source->GetConfSubType() == GCST_ALL_TO_OWNER || m_media_source->GetConfSubType() == GCST_PENDING_ALL_TO_OWNER);
		if (src_type == (is_asym_conf ? VS_RTSPSourceType::Speaker : VS_RTSPSourceType::Mix))
			src_type |= VS_RTSPSourceType::Default;
		auto peer = VS_RTSPBroadcastMediaPeer::Create(std::string(codec), src_type, m_media_source, m_parameters, m_live555_thread, m_env);
		if (!peer)
			return;
		peer->SetSynchronizerPool(m_sync_pool.get());
		it = m_peers.emplace(std::move(lookup_key), std::move(peer)).first;
	}
	sms->addSubsession(it->second->CreateLive555SMSS());
}

void VS_RTSPBroadcastSession::AddSubsessions(ServerMediaSession* sms, string_view codecs)
{
	typedef boost::algorithm::split_iterator<string_view::const_iterator> split_iterator;
	for (split_iterator codec_it(codecs, boost::algorithm::token_finder([](char x) { return x == ' '; }, boost::algorithm::token_compress_on)); !codec_it.eof(); ++codec_it)
	{
		string_view codec(&*codec_it->begin(), codec_it->end() - codec_it->begin());
		if      (boost::iequals(codec, string_view("H264")))
			codec = "H264+720p";
		else if (boost::iequals(codec, string_view("VP8")))
			codec = "VP8+720p";
		else if (boost::iequals(codec, string_view("MP3")))
			codec = "MP3+16000Hz";
		else if (boost::iequals(codec, string_view("AAC")))
			codec = "AAC+16000Hz";
		AddSubsession(sms, codec);
	}
}

void VS_RTSPBroadcastSession::Announce(const char* url, const char* username, const char* password, bool rtp_over_tcp, string_view enabled_codecs, unsigned keepalive_timeout, unsigned retries, unsigned retry_delay, announce_status_cb&& cb)
{
	VS_RegistryKey conf_key(false, CONFIGURATION_KEY);
	unsigned rtsp_timeout = 30;
	conf_key.GetValue(&rtsp_timeout, sizeof(rtsp_timeout), VS_REG_INTEGER_VT, "RTSP Announce Request Timeout");

	std::unique_ptr<Authenticator> authenticator;
	if (username && *username && password && *password)
		authenticator.reset(new Authenticator(username, password));
	m_announces.emplace_back(new announce_info());
	auto& announce = m_announces.back();
	announce->parent = this;
	announce->cb = std::move(cb);
	announce->retries = retries;
	announce->retry_delay = retry_delay;
	announce->retry_token = 0;

	auto ds = dstream3;
	ds << "RTSPSession: Initialized publishing: remote_url=\"" << url << "\", mode=" << (rtp_over_tcp ? "TCP" : "UDP");

	Live555Thread::PauseGuard guard(m_live555_thread);

	ServerMediaSession* sms = nullptr;
	if (enabled_codecs.empty())
	{
		ds << ", same codecs as in streaming";
		sms = m_sms_default;
	}
	else
	{
		sms = ServerMediaSession::createNew(*m_env, m_sms->name(), m_sms->info(), m_sms->description());
		sms->deleteWhenUnreferenced() = True;
		AddSubsessions(sms, enabled_codecs);
		ds << ", codecs={" << GetSessionCodecNames(sms) << '}';
	}

	announce->announcer.reset(StreamAnnouncer::createNew(*m_env, sms, url, rtp_over_tcp, authenticator.get(), rtsp_timeout, keepalive_timeout));
	if (announce->announcer->state() == StreamAnnouncer::state_initial)
	{
		announce->announcer->setCompletionHandler(VS_RTSPBroadcastSession::OnAnnounceCompletion, announce.get());
		announce->announcer->setLivenessTimeoutHandler(VS_RTSPBroadcastSession::OnAnnounceTimeout, announce.get());
		announce->announcer->start();
		ds << ", success";
	}
	else
	{
		OnAnnounceCompletion(announce.get());
		ds << ", failure";
	}
	ds << '\n';
}

void VS_RTSPBroadcastSession::OnAnnounceCompletion(void* opaque)
{
	announce_info* const announce(reinterpret_cast<announce_info*>(opaque));
	announce->parent->OnAnnounceCompletion(announce);
}

void VS_RTSPBroadcastSession::OnAnnounceCompletion(announce_info* announce)
{
	switch (announce->announcer->state())
	{
	case StreamAnnouncer::state_local_error:
	case StreamAnnouncer::state_remote_error:
	{
		const std::string error_msg = m_env->getResultMsg();
		dstream1 << "RTSPSession: Announce failed: " << error_msg;
		if (announce->retries > 0)
		{
			m_env->taskScheduler().rescheduleDelayedTask(announce->retry_token, announce->retry_delay * 1000000, &VS_RTSPBroadcastSession::RetryAnnounce, announce);
			--announce->retries;
		}
		if (announce->cb)
			announce->cb(false, error_msg);
	}
		break;
	case StreamAnnouncer::state_success:
		if (announce->cb)
			announce->cb(true, "");
		break;
	default:
		dstream1 << "RTSPSession: OnAnnounceCompletion: unexpected state\n";
		if (announce->cb)
			announce->cb(false, "Internal error: unexpected state");
		break;
	}
}

void VS_RTSPBroadcastSession::OnAnnounceTimeout(void* opaque)
{
	announce_info* const announce(reinterpret_cast<announce_info*>(opaque));
	announce->parent->OnAnnounceTimeout(announce);
}

void VS_RTSPBroadcastSession::OnAnnounceTimeout(announce_info* announce)
{
	const std::string error_msg = m_env->getResultMsg();
	dstream1 << "RTSPSession: Announce liveness timeout: " << error_msg;
	announce->announcer->stop();
	if (announce->retries > 0)
	{
		m_env->taskScheduler().rescheduleDelayedTask(announce->retry_token, announce->retry_delay * 1000000, &VS_RTSPBroadcastSession::RetryAnnounce, announce);
		--announce->retries;
	}
	if (announce->cb)
		announce->cb(false, error_msg);
}

void VS_RTSPBroadcastSession::RetryAnnounce(void* opaque)
{
	announce_info* const announce(reinterpret_cast<announce_info*>(opaque));
	announce->parent->RetryAnnounce(announce);
}

void VS_RTSPBroadcastSession::RetryAnnounce(announce_info* announce)
{
	dstream1 << "RTSPSession: Retying announce, remaining retries: " << announce->retries;
	announce->announcer->stop();
	announce->announcer->start();
}

VS_RTSPBroadcastSession::~VS_RTSPBroadcastSession()
{
	dstream4 << "RTSPSession: Stopping\n";

	for (auto& ppeer: m_peers)
		if (ppeer.second)
			ppeer.second->Stop();

	Live555Thread::PauseGuard guard(m_live555_thread);
	m_rtsp_server->deleteServerMediaSession(m_sms_default);
	m_rtsp_server->deleteServerMediaSession(m_sms);
	// Need to delete them explicitly now while live555 thread is paused
	for (auto& announce: m_announces)
		m_env->taskScheduler().unscheduleDelayedTask(announce->retry_token);
	m_announces.clear();
	m_peers.clear();
	m_sync_pool.reset();

	if (m_helper_process)
	{
		auto helper = m_helper_process;
		// run a background thread which will stop the RTSP helper process
		std::thread rtsp_helper_stopper([helper](void) {
			vs::SetThreadName("RTSPHelperStop");

			if (!helper)
				return;

			if (helper->Alive())
			{
				vs::SleepFor(std::chrono::milliseconds(HELPER_PROCESS_EXIT_TIMEOUT_MS));
				if (helper->Alive())
				{
					dprint0("RTSP Helper Program refused to stop (PID: %lu). Terminating...", helper->GetPID());
					helper->Terminate(1);
				}
				else
				{
					int exit_code = 0;
					if (helper->GetExitCode(exit_code))
						dprint3("RTSP Helper Program stopped normally with exit code %d (PID: %lu).", exit_code, helper->GetPID());
				}
			}
			else
			{
				int exit_code = 0;
				if (helper->GetExitCode(exit_code))
					dprint0("RTSP Helper Program stopped before receiving termination request (PID: %lu, exit code: %d).", helper->GetPID(), exit_code);
			}
		});

		rtsp_helper_stopper.detach();
	}
}
