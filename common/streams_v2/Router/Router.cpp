#include "Router.h"
#include "Handler.h"
#include "Conference.h"
#include "../../streams/Statistics.h"
#include "../../streams/Router/Types.h"
#include "../../streams/Router/VS_StreamsSVCStatistics.h"
#include "../../std/cpplib/event.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/MakeShared.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/Globals.h"

#include <boost/filesystem/operations.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_STREAMS

namespace stream {

const auto c_svc_update_interval = std::chrono::seconds(1);

RouterV2::RouterV2(boost::asio::io_service& ios, std::string endpoint_name)
	: m_strand(ios)
	, m_ep_name(std::move(endpoint_name))
	, m_log_dir(vs::GetLogDirectory() + stream::c_streams_logs_directory_name)
	, m_total_read_bytes(0)
	, m_total_write_bytes(0)
	, m_last_report_read_bytes(0)
	, m_last_report_write_bytes(0)
	, m_last_report_time(std::chrono::steady_clock::now())
	, m_last_svc_update_time(std::chrono::steady_clock::now())
{
}

RouterV2::~RouterV2()
{
}

bool RouterV2::Start(acs::Service* acs)
{
	boost::system::error_code ec;
	boost::filesystem::create_directories(m_log_dir, ec);
	if (ec)
	{
		dstream0 << "Can't create directory '" << m_log_dir << "': " << ec.message();
		return false;
	}

	VS_MainSVCStatistics::CleanStatDirectory();
	m_handler = std::make_shared<Handler>(this);
	acs->AddHandler("StreamRouter Handler", m_handler);

	return true;
}

void RouterV2::Stop()
{
	m_handler = nullptr;

	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	for (const auto& kv : m_confs)
		kv.second->Stop();
	m_confs.clear();
}

bool RouterV2::AddConferencesCondition(ConferencesConditions* ccs)
{
	return m_ccs.ConnectToSplitter(ccs);
}

void RouterV2::RemoveConferencesCondition(ConferencesConditions* ccs)
{
	m_ccs.DisconnectFromSplitter(ccs);
}

bool RouterV2::GetStatistics(RouterStatistics& stat)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);

	unsigned streams = 0;
	for (const auto& kv : m_confs)
		streams += kv.second->GetParticipantCount() * 2;
	stat.streams = streams;

	const auto now = std::chrono::steady_clock::now();
	const auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - m_last_report_time);

	auto total_read_bytes = m_total_read_bytes.load(std::memory_order_relaxed);
	auto total_write_bytes = m_total_write_bytes.load(std::memory_order_relaxed);
	stat.in_bytes = total_read_bytes;
	stat.out_bytes = total_write_bytes;
	stat.in_byterate = static_cast<double>(total_read_bytes - m_last_report_read_bytes) / elapsed.count();
	stat.out_byterate = static_cast<double>(total_write_bytes - m_last_report_write_bytes) / elapsed.count();

	m_last_report_read_bytes = total_read_bytes;
	m_last_report_write_bytes = total_write_bytes;
	m_last_report_time = now;

	return true;
}

bool RouterV2::CreateConference(string_view conference_name, VS_Conference_Type conference_type, const char* ssl_key, unsigned /*max_participants*/, bool write_logs, std::chrono::steady_clock::duration max_silence)
{
	conference_ptr conf = nullptr;
	{std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	auto it = m_confs.find(conference_name);
	if (it != m_confs.end())
		return false;

	conf = vs::MakeShared<RouterConference<boost::asio::ip::tcp::socket>>(m_strand, this, std::string(conference_name), conference_type, ssl_key, write_logs, max_silence);
	m_confs.emplace(conference_name, conf);
	}

	if(conf) conf->Start();	// will call RouterV2::GetConference indirectly, and lock(m_mutex)
	return true;
}

void RouterV2::RemoveConference(string_view conference_name)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	auto it = m_confs.find(conference_name);
	if (it == m_confs.end())
		return;
	it->second->Stop();
	m_confs.erase(it);
}

boost::signals2::connection RouterV2::ConnectToFrameSink(string_view conference_name, const FrameReceivedSignalType::slot_type& slot)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->ConnectToFrameSink(slot) : boost::signals2::connection();
}

bool RouterV2::AddParticipant(string_view conference_name, string_view participant_name, string_view connected_participant_name, Buffer* snd_buffer, bool write_logs, std::chrono::steady_clock::duration max_silence, const Track* tracks, unsigned n_tracks)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->AddParticipant(participant_name, connected_participant_name, snd_buffer, write_logs, max_silence, tracks, n_tracks) : false;
}

bool RouterV2::AddParticipant(string_view conference_name, string_view participant_name, Buffer* snd_buffer, bool write_logs, std::chrono::steady_clock::duration max_silence)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->AddParticipant(participant_name, snd_buffer, write_logs, max_silence) : false;
}

void RouterV2::RemoveParticipant(string_view conference_name, string_view participant_name)
{
	auto conf = GetConference(conference_name);
	if (conf)
		conf->RemoveParticipant(participant_name);
}

bool RouterV2::ConnectParticipant(string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->ConnectParticipant(participant_name, connected_participant_name, tracks, n_tracks) : false;
}

bool RouterV2::ConnectParticipantSender(string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->ConnectParticipantSender(participant_name, connected_participant_name, tracks, n_tracks) : false;
}

bool RouterV2::ConnectParticipantReceiver(string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->ConnectParticipantReceiver(participant_name, connected_participant_name, tracks, n_tracks) : false;
}

bool RouterV2::DisconnectParticipant(string_view conference_name, string_view participant_name)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->DisconnectParticipant(participant_name) : false;
}

bool RouterV2::DisconnectParticipant(string_view conference_name, string_view participant_name, string_view connected_participant_name)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->DisconnectParticipantSender(participant_name, connected_participant_name) : false;
}
bool RouterV2::DisconnectParticipantSender(string_view conference_name, string_view participant_name)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->DisconnectParticipantSender(participant_name) : false;
}
bool RouterV2::DisconnectParticipantSender(string_view conference_name, string_view participant_name, string_view connected_participant_name)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->DisconnectParticipantSender(participant_name, connected_participant_name) : false;
}
bool RouterV2::DisconnectParticipantReceiver(string_view conference_name, string_view participant_name)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->DisconnectParticipantReceiver(participant_name) : false;
}

bool RouterV2::DisconnectParticipantReceiver(string_view conference_name, string_view participant_name, string_view connected_participant_name)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->DisconnectParticipantReceiver(participant_name, connected_participant_name) : false;
}

void RouterV2::GetMonitorInfo(Monitor::StreamReply & reply)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ ready.set(); };
		for (auto& elem : m_confs)
		{
			Monitor::StreamReply::Conference conference;
			elem.second->FillConfsMonitorStruct(conference);
			reply.conferences.push_back(conference);

			elem.second->FillPartsMonitorStruct(reply);
		}
	});
	ready.wait();
}

bool RouterV2::SetParticipantTracks(string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->SetParticipantTracks(participant_name, connected_participant_name, tracks, n_tracks) : false;
}

bool RouterV2::SetParticipantSenderTracks(string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->SetParticipantSenderTracks(participant_name, connected_participant_name, tracks, n_tracks) : false;
}

bool RouterV2::SetParticipantReceiverTracks(string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->SetParticipantReceiverTracks(participant_name, connected_participant_name, tracks, n_tracks) : false;
}

bool RouterV2::SetParticipantCaps(string_view conference_name, string_view participant_name, const void* caps_buf, size_t buf_sz)
{
	auto conf = GetConference(conference_name);
	return conf ? conf->SetParticipantCaps(participant_name, caps_buf, buf_sz) : false;
}

void RouterV2::SetParticipantSystemLoad(std::vector<ParticipantLoadInfo>&& load)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		for (auto & it : load) {
			VS_MainSVCStatistics::UpdateSystemLoad(it.conference_name.c_str(), it.participant_name.c_str(), it.load);
		}
		load.clear();
	});
	ready.wait();
}

void RouterV2::SetParticipantFrameSizeMB(const char* conference_name, const char* participant_name_to, std::vector<ParticipantFrameSizeInfo>&& mb)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		VS_MainSVCStatistics::UpdateFrameSizeMB(conference_name, participant_name_to, mb);
	});
	ready.wait();
}

ConferencesConditions* RouterV2::GetCCS()
{
	return &m_ccs;
}

const std::string& RouterV2::GetEndpointName() const
{
	return m_ep_name;
}

const std::string& RouterV2::GetLogDirectory() const
{
	return m_log_dir;
}

auto RouterV2::GetConference(string_view conference_name) -> conference_ptr
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	auto it = m_confs.find(conference_name);
	return it != m_confs.end() ? it->second : nullptr;
}

void RouterV2::DeregisterConference(string_view conference_name)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	auto it = m_confs.find(conference_name);
	if (it == m_confs.end())
		return;
	m_confs.erase(it);
}

void RouterV2::NotifyRead(size_t bytes)
{
	m_total_read_bytes += bytes;
}

void RouterV2::NotifyWrite(size_t bytes)
{
	m_total_write_bytes += bytes;
}

void RouterV2::Timer(std::chrono::steady_clock::time_point now)
{
	// Since all conferences use the same strand, calls to this function are syncronized
	if (now - m_last_svc_update_time > c_svc_update_interval)
	{
		m_last_svc_update_time = now;
		VS_MainSVCStatistics::UpdateStatistics(&m_ccs);
	}
}

}
