#pragma once

#include "../fwd.h"
#include "../Relay/Types.h"
#include "std-generic/cpplib/string_view.h"

#include <chrono>

class VS_MediaFormat;
enum VS_Conference_Type : int;

namespace stream {

class Router
{
public:
	virtual ~Router() {};

	virtual void Stop() = 0;

	virtual bool AddConferencesCondition(ConferencesConditions* ccs) = 0;
	virtual void RemoveConferencesCondition(ConferencesConditions* ccs) = 0;
	virtual bool GetStatistics(RouterStatistics& stat) = 0;

	virtual bool CreateConference(string_view conference_name, VS_Conference_Type conference_type, const char* ssl_key = nullptr, unsigned max_participants = 200, bool write_logs = false, std::chrono::steady_clock::duration max_silence = std::chrono::seconds(30)) = 0;
	virtual void RemoveConference(string_view conference_name) = 0;
	virtual boost::signals2::connection ConnectToFrameSink(string_view conference_name, const FrameReceivedSignalType::slot_type& slot) = 0;

	virtual bool AddParticipant(string_view conference_name,
	                    string_view participant_name,
	                    string_view connected_participant_name,
	                    Buffer* snd_buffer = nullptr,
	                    bool write_logs = false,
	                    std::chrono::steady_clock::duration max_silence = std::chrono::seconds(30),
	                    const Track* tracks = nullptr,
	                    unsigned n_tracks = 255) = 0;
	virtual bool AddParticipant(string_view conference_name,
                        string_view participant_name,
                        Buffer* snd_buffer = nullptr,
                        bool write_logs = false,
                        std::chrono::steady_clock::duration max_silence = std::chrono::seconds(30)) = 0;
	virtual void RemoveParticipant(string_view conference_name, string_view participant_name) = 0;

	// Performs actions of both ConnectParticipantSender() and ConnectParticipantReceiver()
	virtual bool ConnectParticipant        (string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) = 0;
	// Connects two participants in a way that frames sent by participant_name will be passed to connected_participant_name (put into its stream::Buffer).
	// Arguments (tracks, n_tracks) can be used to limit this connection to specific tracks.
	virtual bool ConnectParticipantSender  (string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) = 0;
	// Connects two participants in a way that frames sent by connected_participant_name will be passed to participant_name (put into its stream::Buffer).
	// Arguments (tracks, n_tracks) can be used to limit this connection to specific tracks.
	virtual bool ConnectParticipantReceiver(string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) = 0;

	virtual bool DisconnectParticipant        (string_view conference_name, string_view participant_name) = 0;
	virtual bool DisconnectParticipant        (string_view conference_name, string_view participant_name, string_view connected_participant_name) = 0;
	virtual bool DisconnectParticipantSender  (string_view conference_name, string_view participant_name) = 0;
	virtual bool DisconnectParticipantSender  (string_view conference_name, string_view participant_name, string_view connected_participant_name) = 0;
	virtual bool DisconnectParticipantReceiver(string_view conference_name, string_view participant_name) = 0;
	virtual bool DisconnectParticipantReceiver(string_view conference_name, string_view participant_name, string_view connected_participant_name) = 0;

	virtual bool SetParticipantTracks        (string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) = 0;
	virtual bool SetParticipantSenderTracks  (string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) = 0;
	virtual bool SetParticipantReceiverTracks(string_view conference_name, string_view participant_name, string_view connected_participant_name, const Track* tracks = nullptr, unsigned n_tracks = 255) = 0;

	virtual bool SetParticipantCaps(string_view conference_name, string_view participant_name, const void* caps_buf, size_t buf_sz) = 0;
	virtual void SetParticipantSystemLoad(std::vector<ParticipantLoadInfo>&& load) = 0;
	virtual void SetParticipantFrameSizeMB(const char* conference_name, const char* participant_name_to, std::vector<ParticipantFrameSizeInfo>&& mb) = 0;
};

}
