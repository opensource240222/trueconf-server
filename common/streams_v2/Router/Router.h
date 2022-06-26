#pragma once

#include "RouterInternalInterface.h"
#include "../../streams/Router/Router.h"
#include "../../streams/Router/ConferencesConditionsSplitter.h"
#include "../../acs_v2/Service.h"
#include "Monitor.h"
#include "std-generic/cpplib/StrCompare.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

#include "std-generic/undef_windows.h" // this should be last

namespace stream {

class Handler;

class RouterV2 : public Router, public RouterInternalInterface
{
public:
	RouterV2(boost::asio::io_service& ios, std::string endpoint_name);
	~RouterV2();

	bool Start(acs::Service* acs);
	void Stop() override;

	bool AddConferencesCondition(ConferencesConditions* ccs) override;
	void RemoveConferencesCondition(ConferencesConditions* ccs) override;
	bool GetStatistics(RouterStatistics& stat) override;

	bool CreateConference(string_view conference_name, VS_Conference_Type conference_type, const char* ssl_key = nullptr, unsigned /*max_participants*/ = 200, bool write_logs = false, std::chrono::steady_clock::duration max_silence = std::chrono::seconds(30)) override;
	void RemoveConference(string_view conference_name) override;
	boost::signals2::connection ConnectToFrameSink(string_view conference_name, const FrameReceivedSignalType::slot_type& slot) override;

	bool AddParticipant(string_view conference_name,
	                    string_view participant_name,
	                    string_view connected_participant_name,
	                    Buffer* snd_buffer = nullptr,
	                    bool write_logs = false,
	                    std::chrono::steady_clock::duration max_silence = std::chrono::seconds(30),
	                    const Track* tracks = nullptr,
	                    unsigned n_tracks = 255) override;
	bool AddParticipant(string_view conference_name,
                        string_view participant_name,
                        Buffer* snd_buffer = nullptr,
                        bool write_logs = false,
                        std::chrono::steady_clock::duration max_silence = std::chrono::seconds(30)) override;
	void RemoveParticipant(string_view conference_name, string_view participant_name) override;

	bool ConnectParticipant        (string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) override;
	bool ConnectParticipantSender  (string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) override;
	bool ConnectParticipantReceiver(string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) override;

	bool DisconnectParticipant        (string_view conference_name, string_view participant_name) override;
	bool DisconnectParticipant        (string_view conference_name, string_view participant_name, string_view connected_participant_name) override;
	bool DisconnectParticipantSender  (string_view conference_name, string_view participant_name) override;
	bool DisconnectParticipantSender  (string_view conference_name, string_view participant_name, string_view connected_participant_name) override;
	bool DisconnectParticipantReceiver(string_view conference_name, string_view participant_name) override;
	bool DisconnectParticipantReceiver(string_view conference_name, string_view participant_name, string_view connected_participant_name) override;

	void GetMonitorInfo(Monitor::StreamReply& reply);

	bool SetParticipantTracks        (string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) override;
	bool SetParticipantSenderTracks  (string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) override;
	bool SetParticipantReceiverTracks(string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) override;

	bool SetParticipantCaps(string_view conference_name, string_view participant_name, const void* caps_buf, size_t buf_sz) override;
	void SetParticipantSystemLoad(std::vector<ParticipantLoadInfo>&& load) override;
	void SetParticipantFrameSizeMB(const char* conference_name, const char* participant_name_to, std::vector<ParticipantFrameSizeInfo>&& mb) override;

private:
	// RouterInternalInterface
	ConferencesConditions* GetCCS() override;
	const std::string& GetEndpointName() const override;
	const std::string& GetLogDirectory() const override;
	conference_ptr GetConference(string_view conference_name) override;
	void DeregisterConference(string_view conference_name) override;
	void NotifyRead(size_t bytes) override;
	void NotifyWrite(size_t bytes) override;
	void Timer(std::chrono::steady_clock::time_point now) override;

private:
	boost::asio::io_service::strand m_strand;
	const std::string m_ep_name;
	const std::string m_log_dir;
	ConferencesConditionsSplitter m_ccs;

	std::mutex m_mutex;
	vs::map<std::string, conference_ptr, vs::str_less> m_confs;
	std::atomic<uint64_t> m_total_read_bytes;
	std::atomic<uint64_t> m_total_write_bytes;
	uint64_t m_last_report_read_bytes;
	uint64_t m_last_report_write_bytes;
	std::chrono::steady_clock::time_point m_last_report_time;
	std::chrono::steady_clock::time_point m_last_svc_update_time;

	std::shared_ptr<Handler> m_handler;
};

}
