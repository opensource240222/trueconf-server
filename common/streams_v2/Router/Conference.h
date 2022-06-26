#pragma once

#include "../../streams/fwd.h"
#include "../../streams/Handshake.h"
#include "../../streams/Statistics.h"
#include "../../streams/Relay/Types.h"
#include "../../streams/Router/CSVLogger.h"
#include "SecureLib/VS_StreamCrypter.h"
#include "std-generic/cpplib/string_view.h"
#include "Monitor.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include "std-generic/compat/functional.h"
#include <atomic>
#include <chrono>
#include <memory>
#include <string>

class VS_MediaFormat;
enum VS_Conference_Type : int;

namespace { namespace mi = boost::multi_index; }

namespace stream {

class RouterInternalInterface;

template <class Socket = boost::asio::ip::tcp::socket>
class RouterConference : public std::enable_shared_from_this<RouterConference<Socket>>
{
public:
	~RouterConference();

	void Start();
	void Stop();

	boost::signals2::connection ConnectToFrameSink(const FrameReceivedSignalType::slot_type& slot);

	bool AddParticipant(string_view participant_name,
	                    string_view connected_participant_name,
	                    Buffer* snd_buffer,
	                    bool write_logs,
	                    std::chrono::steady_clock::duration max_silence,
	                    const Track* tracks,
	                    unsigned n_tracks);
	bool AddParticipant(string_view participant_name,
                        Buffer* snd_buffer,
                        bool write_logs,
                        std::chrono::steady_clock::duration max_silence);
	void RemoveParticipant(string_view participant_name);

	void SetParticipantConnection(string_view participant_name, Socket&& socket, ClientType type, const uint8_t mtracks[32]);

	bool ConnectParticipant        (string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks);
	bool ConnectParticipantSender  (string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks);
	bool ConnectParticipantReceiver(string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks);

	bool DisconnectParticipant        (string_view participant_name);
	bool DisconnectParticipant        (string_view participant_name, string_view connected_participant_name);
	bool DisconnectParticipantSender  (string_view participant_name);
	bool DisconnectParticipantSender  (string_view participant_name, string_view connected_participant_name);
	bool DisconnectParticipantReceiver(string_view participant_name);
	bool DisconnectParticipantReceiver(string_view participant_name, string_view connected_participant_name);

	bool SetParticipantTracks        (string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks);
	bool SetParticipantSenderTracks  (string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks);
	bool SetParticipantReceiverTracks(string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks);

	bool SetParticipantCaps(string_view participant_name, const void* caps_buf, size_t buf_sz);

	void FillConfsMonitorStruct(Monitor::StreamReply::Conference& conf);
	void FillPartsMonitorStruct(Monitor::StreamReply & reply);

	unsigned GetParticipantCount() const
	{
		return m_n_parts.load(std::memory_order_relaxed);
	}

protected:
	RouterConference(boost::asio::io_service::strand &strand, RouterInternalInterface *router, std::string name, VS_Conference_Type type, const char *ssl_key, bool write_logs, std::chrono::steady_clock::duration max_silence);

private:
	enum direction { send = 0, recv = 1 };
	struct connection_info;
	struct receiver_info;
	struct participant_info;
	using participant_ptr = boost::intrusive_ptr<participant_info>;
	struct participant_name_less
	{
		typedef int is_transparent;
		bool operator()(participant_ptr l, participant_ptr r) const;
		bool operator()(participant_ptr l, string_view r) const;
		bool operator()(string_view l, participant_ptr r) const;
	};

	participant_ptr GetParticipant(string_view name);
	participant_ptr CreateParticipant(string_view name, Buffer* snd_buffer, bool write_logs, std::chrono::steady_clock::duration max_silence);
	void DeleteParticipant(participant_ptr part);

	void WriteHSReply(participant_ptr part, direction dir);
	void WriteFrame(participant_ptr part);
	void ReadFrameHeader(participant_ptr part);
	void ReadFrameBody(participant_ptr part);
	void CloseConnection(participant_ptr part, direction dir);

	void StartWrite(participant_ptr part);
	bool PrepareWriteFrame(participant_ptr part);
	bool PrepareWriteStatistics(participant_ptr part);
	void ProcessWriteFrame(participant_ptr part);
	void ProcessReadFrame(participant_ptr part);

	StreamStatistics* RetrieveInstantSendStatistic(participant_ptr part);
	void CalculateInstantSendStatistic(participant_ptr part);
	bool CalculateBandwidth(participant_ptr part);

	void ScheduleTimer();

private:
	boost::asio::io_service::strand& m_strand;
	boost::asio::steady_timer m_timer;
	RouterInternalInterface* const m_router;
	const std::string m_name;
	const VS_Conference_Type m_type;

	// TODO: MSVC140: Replace with std::set<participant_ptr, participant_name_less>
	struct participant_name_extractor
	{
		using result_type = std::string;
		const std::string& operator()(participant_ptr p) const;
	};
	mi::multi_index_container<participant_ptr, mi::indexed_by<
		mi::ordered_unique<participant_name_extractor, vs::less<>>
	>> m_parts;
	FrameReceivedSignalType m_signal_FrameReceived;
	uint16_t num_participants = 0;
	std::chrono::steady_clock::time_point m_start_time;
	std::chrono::system_clock::time_point m_conf_start = std::chrono::system_clock::now();
	std::atomic<unsigned> m_n_parts;
	VS_StreamCrypter m_crypter;
	CSVLog m_log_file;
	CSVLogger m_logger;
	ConferenceStatistics m_stat;

	const std::chrono::steady_clock::duration m_max_silence;
	std::chrono::steady_clock::time_point m_last_active_time;

	std::unique_ptr<net::HandshakeHeader, array_deleter<char>> m_hs_reply;
	struct LogPrefix;
};
extern template class RouterConference<>;

}
