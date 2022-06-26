#include "Conference.h"
#include "RouterInternalInterface.h"
#include "../../streams/Protocol.h"
#include "../../streams/Router/Buffer.h"
#include "../../streams/Router/DefaultBuffer.h"
#include "../../streams/Router/ParticipantStatisticsCalculator.h"
#include "../../streams/Router/Types.h"
#include "../../streams/Router/ConferencesConditions.h"
#include "../../std/cpplib/event.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/ignore.h"
#include "../../std/debuglog/VS_Debug.h"
#include "net/QoSSettings.h"

#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>

#include "std-generic/compat/memory.h"
#include <array>
#include <cassert>
#include <map>

#define DEBUG_CURRENT_MODULE VS_DM_STREAMS

#define STREAMS_VERBOSE_LOGS 0

namespace stream {

static const auto c_statistics_send_interval = std::chrono::seconds(2);
static const auto c_timer_interval = std::chrono::seconds(1);
static const char direction_prefix[2][7] = { "send: ", "recv: " };

template <class Socket>
struct RouterConference<Socket>::connection_info
{
	Socket socket;
	unsigned socket_id = 1; // used to identify operations executed on a previous socket
	FrameHeader header = {};
	std::unique_ptr<char[]> body;
	uint8_t mtracks[32] = {};
	time_t last_connect = 0;
	time_t last_disconnect = 0;
	std::string protocol;
	net::QoSFlowSharedPtr qos_flow;

	explicit connection_info(boost::asio::io_service& ios) : socket(ios) {}
};

template <class Socket>
struct RouterConference<Socket>::receiver_info
{
	uint8_t wanted_mtracks[32];
};

template <class Socket>
struct RouterConference<Socket>::participant_info
{
	std::atomic<unsigned> ref_count {0}; // TODO: We can use non-atomic counter if we make all completion handlers that capture participant_ptr non-copyable
	std::string name;
	connection_info snd;
	connection_info rcv;
	bool snd_write_in_progress = false;
	bool rcv_read_in_progress = false;
	std::unique_ptr<Buffer> snd_buffer;
	std::map<participant_ptr, receiver_info> receivers;
	ParticipantStatistics stat = {};
	ParticipantStatisticsCalculator stat_calc;
	std::vector<uint8_t> received_statistic_buffer;
	ParticipantBandwidthInfo bandwidth_info;
	CSVLog log_file;
	std::chrono::steady_clock::time_point last_statistics_send_time;

	std::chrono::steady_clock::duration max_silence;
	std::chrono::steady_clock::time_point last_active_time;
	std::chrono::system_clock::time_point creation_time = std::chrono::system_clock::now();

	std::shared_ptr<RouterConference> conf; // Used to avoid passing two "shared pointers" to every callback

	connection_info& connection(direction dir)
	{
		return dir == send ? snd : rcv;
	}

	explicit participant_info(boost::asio::io_service& ios) : snd(ios), rcv(ios) {}
	participant_info(const participant_info&) = delete;
	participant_info& operator=(const participant_info&) = delete;

	friend void intrusive_ptr_add_ref(participant_info* p)
	{
		p->ref_count.fetch_add(1, std::memory_order_release);
	}

	friend void intrusive_ptr_release(participant_info* p)
	{
		if (p->ref_count.fetch_sub(1, std::memory_order_acquire) == 1)
			delete p;
	}
};

template <class Socket>
bool RouterConference<Socket>::participant_name_less::operator()(participant_ptr l, participant_ptr r) const
{
	return l->name < r->name;
}

template <class Socket>
bool RouterConference<Socket>::participant_name_less::operator()(participant_ptr l, string_view r) const
{
	return l->name < r;
}

template <class Socket>
bool RouterConference<Socket>::participant_name_less::operator()(string_view l, participant_ptr r) const
{
	return l < r->name;
}

template <class Socket>
const std::string& RouterConference<Socket>::participant_name_extractor::operator()(participant_ptr p) const
{
	return p->name;
}

template <class Socket>
std::string GetRemoteIP(const Socket& socket)
{
	std::string result;
	boost::system::error_code ec;
	const auto ep = socket.remote_endpoint(ec);
	if (ec)
		return result;
	result = ep.address().to_string(ec);
	if (ec)
	{
		result.clear();
		return result;
	}
	return result;
}

template <class Socket>
boost::system::error_code SetFastSocket(Socket& socket)
{
	boost::system::error_code ec;
	socket.set_option(boost::asio::ip::tcp::no_delay(true), ec);
#if defined(_WIN32)
	if (!ec)
	{
		boost::asio::ip::tcp::socket::send_buffer_size so_sndbuf;
		socket.get_option(so_sndbuf, ec);
		if (!ec && so_sndbuf.value() != 0x10000 && so_sndbuf.value() != 0)
		{
			so_sndbuf = 58400;
			socket.set_option(so_sndbuf, ec);
		}
	}
#endif
	return ec;
}

template <class Socket>
struct RouterConference<Socket>::LogPrefix
{
	LogPrefix(const RouterConference* conf_, string_view participant_name_ = {})
		: conference_name(conf_->m_name)
		, participant_name(participant_name_)
	{}
	string_view conference_name;
	string_view participant_name;

	template <class CharT, class Traits>
	friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, LogPrefix x)
	{
		s << "C[" << x.conference_name << "]: ";
		if (!x.participant_name.empty())
			s << "P[" << x.participant_name << "]: ";
		return s;
	}
};

template <class Socket>
RouterConference<Socket>::RouterConference(boost::asio::io_service::strand& strand, RouterInternalInterface* router, std::string name, VS_Conference_Type type, const char* ssl_key, bool write_logs, std::chrono::steady_clock::duration max_silence)
	: m_strand(strand)
	, m_timer(strand.get_io_service())
	, m_router(router)
	, m_name(std::move(name))
	, m_type(type)
	, m_n_parts(0)
	, m_stat()
	, m_max_silence(max_silence)
	, m_last_active_time(std::chrono::steady_clock::now())
	, m_hs_reply(CreateHandshakeResponse(nullptr))
{
	m_crypter.Init(ssl_key);
	if (write_logs)
		m_log_file.Open(m_router->GetLogDirectory(), m_name);
}

template <class Socket>
RouterConference<Socket>::~RouterConference()
{
#if STREAMS_VERBOSE_LOGS
	dstream4 << LogPrefix(this) << "delete";
#endif
	m_crypter.Free();
}

template <class Socket>
void RouterConference<Socket>::Start()
{
	m_start_time = std::chrono::steady_clock::now();
	ScheduleTimer();
	m_router->GetCCS()->CreateConference(m_name.c_str());
}

template <class Socket>
void RouterConference<Socket>::Stop()
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
#if STREAMS_VERBOSE_LOGS
		dstream4 << LogPrefix(this) << "stop";
#endif

		m_timer.cancel();
		for (const auto& part : m_parts)
		{
			CloseConnection(part, send);
			CloseConnection(part, recv);
			part->receivers.clear();
		}

		assert(m_n_parts.load(std::memory_order_relaxed) == m_parts.size());
		m_parts.clear();
		m_n_parts.store(0, std::memory_order_relaxed);

		m_router->GetCCS()->RemoveConference(m_name.c_str(), m_stat);
	});
	ready.wait();
}

template <class Socket>
boost::signals2::connection RouterConference<Socket>::ConnectToFrameSink(const FrameReceivedSignalType::slot_type& slot)
{
	// boost::signals2::signal is thread-safe, no custom synchronization required.
	return m_signal_FrameReceived.connect(slot);
}

template <class Socket>
bool RouterConference<Socket>::AddParticipant(string_view participant_name,
					string_view connected_participant_name,
					Buffer* snd_buffer,
					bool write_logs,
					std::chrono::steady_clock::duration max_silence,
					const Track* tracks,
					unsigned n_tracks)
{
	uint8_t ftracks[256];
	if (!TracksToFTracks(ftracks, tracks, n_tracks))
		return false;
	uint8_t mtracks[32];
	FTracksToMTracks(mtracks, ftracks);

	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto it = m_parts.find(participant_name);
		if (it != m_parts.end())
			return;

		const bool self_connected = participant_name == connected_participant_name;

		participant_ptr connected_part;
		if (!self_connected)
		{
			connected_part = GetParticipant(connected_participant_name);
			if (!connected_part)
				return;
		}

		auto part = CreateParticipant(participant_name, snd_buffer, write_logs, max_silence);
		if (self_connected)
			connected_part = part;
		memcpy(part->receivers[connected_part].wanted_mtracks, mtracks, 32);
		memcpy(connected_part->receivers[part].wanted_mtracks, mtracks, 32);
		m_parts.insert(part);
		m_n_parts.fetch_add(1, std::memory_order_relaxed);

		m_router->GetCCS()->AddParticipant(m_name.c_str(), part->name.c_str());
		result = true;
	});
	ready.wait();
	return result;
}

template <class Socket>
bool RouterConference<Socket>::AddParticipant(string_view participant_name,
					Buffer* snd_buffer,
					bool write_logs,
					std::chrono::steady_clock::duration max_silence)
{
	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto it = m_parts.find(participant_name);
		if (it != m_parts.end())
			return;

		auto part = CreateParticipant(participant_name, snd_buffer, write_logs, max_silence);
		m_parts.insert(part);
		m_n_parts.fetch_add(1, std::memory_order_relaxed);

		m_router->GetCCS()->AddParticipant(m_name.c_str(), part->name.c_str());
		result = true;
	});
	ready.wait();
	return result;
}

template <class Socket>
void RouterConference<Socket>::RemoveParticipant(string_view participant_name)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;

		DeleteParticipant(part);
	});
	ready.wait();
}

template <class Socket>
void RouterConference<Socket>::SetParticipantConnection(string_view participant_name, Socket&& socket, ClientType type, const uint8_t mtracks[32])
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;

		const direction dir = type == ClientType::sender ? send : recv;
		auto& ci = part->connection(dir);

		CloseConnection(part, dir);
		if (dir == send)
			++part->stat.receiverNConns;
		else
			++part->stat.senderNConns;
		ci.last_connect = time(0);
		auto qos_flow = net::QoSSettings::GetInstance().GetTCStreamQoSFlow(socket.remote_endpoint(vs::ignore<boost::system::error_code>()).address().is_v6());
		if (qos_flow && qos_flow->AddSocket(socket.native_handle()))
		{
			ci.qos_flow = qos_flow;
		}
		ci.socket = std::move(socket);

		memcpy(ci.mtracks, mtracks, sizeof(ci.mtracks));
#if STREAMS_VERBOSE_LOGS
		dstream4 << LogPrefix(this, part->name) << direction_prefix[dir] << "new connection: socket_id=" << ci.socket_id;
#endif

		auto ec = SetFastSocket(ci.socket);
		if (ec)
			dstream4 << LogPrefix(this, part->name) << direction_prefix[dir] << "SetFastSocket failed: " << ec.message();

		WriteHSReply(part, dir);

		if (dir == send)
		{
			part->stat_calc.InitSndStatCalc(mtracks);
			part->last_statistics_send_time = std::chrono::steady_clock::now();
			part->snd.protocol = "TCP";
		}
		else
		{
			part->stat_calc.InitRcvStatCalc(mtracks);
			ReadFrameHeader(part);
			part->rcv.protocol = "TCP";
		}
	});
	ready.wait();
}

template <class Socket>
bool RouterConference<Socket>::ConnectParticipant(string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	uint8_t ftracks[256];
	if (!TracksToFTracks(ftracks, tracks, n_tracks))
		return false;
	uint8_t mtracks[32];
	FTracksToMTracks(mtracks, ftracks);

	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;
		auto connected_part = GetParticipant(connected_participant_name);
		if (!connected_part)
			return;

		memcpy(part->receivers[connected_part].wanted_mtracks, mtracks, 32);
		memcpy(connected_part->receivers[part].wanted_mtracks, mtracks, 32);
		result = true;
	});
	ready.wait();
	return result;
}

template <class Socket>
bool RouterConference<Socket>::ConnectParticipantSender(string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	uint8_t ftracks[256];
	if (!TracksToFTracks(ftracks, tracks, n_tracks))
		return false;
	uint8_t mtracks[32];
	FTracksToMTracks(mtracks, ftracks);

	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;
		auto connected_part = GetParticipant(connected_participant_name);
		if (!connected_part)
			return;

		memcpy(part->receivers[connected_part].wanted_mtracks, mtracks, 32);
		result = true;
	});
	ready.wait();
	return result;
}

template <class Socket>
bool RouterConference<Socket>::ConnectParticipantReceiver(string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	return ConnectParticipantSender(connected_participant_name, participant_name, tracks, n_tracks);
}

template <class Socket>
bool RouterConference<Socket>::DisconnectParticipant(string_view participant_name)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;

		for (const auto& p : m_parts)
			p->receivers.erase(part);
		part->receivers.clear();
	});
	ready.wait();
	return true;
}

template <class Socket>
bool RouterConference<Socket>::DisconnectParticipant(string_view participant_name, string_view connected_participant_name)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;
		auto connected_part = GetParticipant(connected_participant_name);
		if (!connected_part)
			return;

		part->receivers.erase(connected_part);
		connected_part->receivers.erase(part);
	});
	ready.wait();
	return true;
}

template <class Socket>
bool RouterConference<Socket>::DisconnectParticipantSender(string_view participant_name)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;

		part->receivers.clear();
	});
	ready.wait();
	return true;
}

template <class Socket>
bool RouterConference<Socket>::DisconnectParticipantSender(string_view participant_name, string_view connected_participant_name)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;
		auto connected_part = GetParticipant(connected_participant_name);
		if (!connected_part)
			return;

		part->receivers.erase(connected_part);
	});
	ready.wait();
	return true;
}

template <class Socket>
bool RouterConference<Socket>::DisconnectParticipantReceiver(string_view participant_name)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;

		for (const auto& p : m_parts)
			p->receivers.erase(part);
	});
	ready.wait();
	return true;
}

template <class Socket>
bool RouterConference<Socket>::DisconnectParticipantReceiver(string_view participant_name, string_view connected_participant_name)
{
	return DisconnectParticipantSender(connected_participant_name, participant_name);
}

template <class Socket>
bool RouterConference<Socket>::SetParticipantTracks(string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	uint8_t ftracks[256];
	if (!TracksToFTracks(ftracks, tracks, n_tracks))
		return false;
	uint8_t mtracks[32];
	FTracksToMTracks(mtracks, ftracks);

	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;
		auto connected_part = GetParticipant(connected_participant_name);
		if (!connected_part)
			return;

		auto p_cp_it = part->receivers.find(connected_part);
		if (p_cp_it == part->receivers.end())
			return;

		auto cp_p_it = connected_part->receivers.find(part);
		if (cp_p_it == connected_part->receivers.end())
			return;

		memcpy(p_cp_it->second.wanted_mtracks, mtracks, 32);
		memcpy(cp_p_it->second.wanted_mtracks, mtracks, 32);
		result = true;
	});
	ready.wait();
	return result;
}

template <class Socket>
bool RouterConference<Socket>::SetParticipantSenderTracks(string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	uint8_t ftracks[256];
	if (!TracksToFTracks(ftracks, tracks, n_tracks))
		return false;
	uint8_t mtracks[32];
	FTracksToMTracks(mtracks, ftracks);

	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;
		auto connected_part = GetParticipant(connected_participant_name);
		if (!connected_part)
			return;

		auto p_cp_it = part->receivers.find(connected_part);
		if (p_cp_it == part->receivers.end())
			return;

		memcpy(p_cp_it->second.wanted_mtracks, mtracks, 32);
		result = true;
	});
	ready.wait();
	return result;
}

template <class Socket>
bool RouterConference<Socket>::SetParticipantReceiverTracks(string_view participant_name, string_view connected_participant_name, const Track* tracks, unsigned n_tracks)
{
	return SetParticipantSenderTracks(connected_participant_name, participant_name, tracks, n_tracks);
}

template <class Socket>
bool RouterConference<Socket>::SetParticipantCaps(string_view participant_name, const void* caps_buf, size_t buf_sz)
{
	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto part = GetParticipant(participant_name);
		if (!part)
			return;

		m_router->GetCCS()->SetParticipantCaps(m_name.c_str(), part->name.c_str(), caps_buf, buf_sz);
		result = true;
	});
	ready.wait();
	return result;
}

template <class Socket>
void RouterConference<Socket>::FillConfsMonitorStruct(Monitor::StreamReply::Conference& conf)
{
	conf.name = m_name;
	conf.creation_time = m_conf_start;
	conf.num_participants = m_n_parts;
	conf.max_absence_ms = m_max_silence;
}

template <class Socket>
void RouterConference<Socket>::FillPartsMonitorStruct(Monitor::StreamReply & reply)
{
	for (auto& elem : m_parts)
	{
		Monitor::StreamReply::Participant participant;
		elem->stat_calc.FillMonitorStruct(participant);
		participant.conf_name = elem->conf->m_name;
		participant.creation_time = elem->creation_time;
		participant.part_name = elem->name;
		participant.receiver = elem->rcv.protocol;
		participant.sender = elem->snd.protocol;

		participant.r_last_connect = elem->rcv.last_connect;
		participant.r_last_disconnect = elem->rcv.last_disconnect;
		participant.r_remote_addr = GetRemoteIP(elem->rcv.socket);
		participant.r_local_addr = GetRemoteIP(elem->snd.socket);

		participant.s_last_connect = elem->snd.last_connect;
		participant.s_last_disconnect = elem->snd.last_disconnect;
		participant.s_remote_addr = GetRemoteIP(elem->snd.socket);
		participant.s_local_addr = GetRemoteIP(elem->rcv.socket);

		reply.participants.push_back(participant);
	}
}

template <class Socket>
auto RouterConference<Socket>::GetParticipant(string_view name) -> participant_ptr
{
	assert(m_strand.running_in_this_thread());

	auto it = m_parts.find(name);
	return it != m_parts.end() ? *it : nullptr;
}

template <class Socket>
auto RouterConference<Socket>::CreateParticipant(string_view name, Buffer* snd_buffer, bool write_logs, std::chrono::steady_clock::duration max_silence) -> participant_ptr
{
	assert(m_strand.running_in_this_thread());

	participant_ptr part(new participant_info(m_strand.get_io_service()));
	part->conf = this->shared_from_this();
	part->name = std::string(name);
	part->snd_buffer.reset(snd_buffer);
	if (!part->snd_buffer)
		part->snd_buffer = vs::make_unique<DefaultBuffer>();
	if (part->snd_buffer->Init(m_name, part->name))
	{
		if (m_crypter.IsValid())
			part->snd_buffer->SetStreamCrypter(&m_crypter);
		part->snd_buffer->SetParticipantStatisticsInterface(&part->stat_calc);
	}
	if (write_logs)
		part->log_file.Open(m_router->GetLogDirectory(), m_name, part->name);
	part->max_silence = max_silence;
	VS_RegistryKey(false, CONFIGURATION_KEY).GetString(part->bandwidth_info.loggedParticipant, "p2p bitrate logged participant");
	if (part->bandwidth_info.loggedParticipant != part->name) {
		part->bandwidth_info.loggedParticipant.clear();
	}
	part->last_active_time = std::chrono::steady_clock::now();

#if STREAMS_VERBOSE_LOGS
	dstream4 << LogPrefix(this, part->name) << "new";
#endif
	return part;
}

template <class Socket>
void RouterConference<Socket>::DeleteParticipant(participant_ptr part)
{
	assert(m_strand.running_in_this_thread());

#if STREAMS_VERBOSE_LOGS
	dstream4 << LogPrefix(this, part->name) << "delete";
#endif

	part->snd_buffer->Destroy(m_name, part->name);
	for (const auto& p : m_parts)
		p->receivers.erase(part);
	part->receivers.clear();
	CloseConnection(part, send);
	CloseConnection(part, recv);

	m_parts.erase(part->name);
	auto n_parts = m_n_parts.fetch_sub(1, std::memory_order_relaxed);
	assert(n_parts >= 1);
	(void)n_parts; // Silence warning in release build

	m_router->GetCCS()->RemoveParticipant(m_name.c_str(), part->name.c_str(), part->stat);
}

template <class Socket>
void RouterConference<Socket>::WriteHSReply(participant_ptr part, direction dir)
{
	assert(m_strand.running_in_this_thread());
	assert(!(dir == send && part->snd_write_in_progress));

	auto& ci = part->connection(dir);
	assert(ci.socket.is_open());

	if (dir == send)
		part->snd_write_in_progress = true;
	const auto buffer = boost::asio::buffer(m_hs_reply.get(), sizeof(net::HandshakeHeader) + m_hs_reply->body_length + 1);
	boost::asio::async_write(ci.socket, buffer, m_strand.wrap(
		[this, part, socket_id = ci.socket_id, dir] (const boost::system::error_code& ec, size_t bytes_transferred)
		{
			auto& ci = part->connection(dir);
			if (socket_id != ci.socket_id)
				return; // This was an operation on a closed socket
			if (ec == boost::asio::error::operation_aborted)
				return;

			if (dir == send)
				part->snd_write_in_progress = false;
			if (ec)
			{
				dstream3 << LogPrefix(this, part->name) << direction_prefix[dir] << "write failed (hs reply): " << ec.message();
				CloseConnection(part, dir);
				return;
			}
			m_router->NotifyWrite(bytes_transferred);
#if STREAMS_VERBOSE_LOGS
			dstream4 << LogPrefix(this, part->name) << direction_prefix[dir] << "write ok (hs reply)";
#endif
			if (dir == send)
				StartWrite(part);
		}
	));
}

template <class Socket>
void RouterConference<Socket>::WriteFrame(participant_ptr part)
{
	assert(m_strand.running_in_this_thread());
	assert(!part->snd_write_in_progress);
	assert(part->snd.socket.is_open());
	assert(part->snd.body != nullptr);
	assert(part->snd.header.length > 0);

	part->snd_write_in_progress = true;
	std::array<boost::asio::const_buffer, 2> buffer = {{
		{ &part->snd.header, sizeof(FrameHeader) },
		{ part->snd.body.get(), part->snd.header.length },
	}};
#if STREAMS_VERBOSE_LOGS
	dstream4 << LogPrefix(this, part->name) << direction_prefix[send] << "write start: size=" << boost::asio::buffer_size(buffer);
#endif
	boost::asio::async_write(part->snd.socket, buffer, m_strand.wrap(
		[this, part, socket_id = part->snd.socket_id] (const boost::system::error_code& ec, size_t bytes_transferred)
		{
			if (socket_id != part->snd.socket_id)
				return; // This was an operation on a closed socket
			if (ec == boost::asio::error::operation_aborted)
				return;

			part->snd_write_in_progress = false;
			if (ec)
			{
				dstream3 << LogPrefix(this, part->name) << direction_prefix[send] << "write failed: " << ec.message();
				CloseConnection(part, send);
				return;
			}
			const size_t write_size = sizeof(FrameHeader) + part->snd.header.length;
			if (bytes_transferred != write_size)
			{
				dstream3 << LogPrefix(this, part->name) << direction_prefix[send] << "wrong write size: " << bytes_transferred << " != " << write_size;
				CloseConnection(part, send);
				return;
			}
			m_router->NotifyWrite(bytes_transferred);
#if STREAMS_VERBOSE_LOGS
			dstream4 << LogPrefix(this, part->name) << direction_prefix[send] << "write ok: size=" << bytes_transferred;
#endif

			ProcessWriteFrame(part);
		}
	));
}

template <class Socket>
void RouterConference<Socket>::ReadFrameHeader(participant_ptr part)
{
	assert(m_strand.running_in_this_thread());
	assert(!part->rcv_read_in_progress);
	assert(part->rcv.socket.is_open());

	part->rcv_read_in_progress = true;
	const auto buffer = boost::asio::buffer(&part->rcv.header, sizeof(FrameHeader));
#if STREAMS_VERBOSE_LOGS
	dstream4 << LogPrefix(this, part->name) << direction_prefix[recv] << "read start (header): size=" << boost::asio::buffer_size(buffer);
#endif
	boost::asio::async_read(part->rcv.socket, buffer, m_strand.wrap(
		[this, part, socket_id = part->rcv.socket_id] (const boost::system::error_code& ec, size_t bytes_transferred)
		{
			if (socket_id != part->rcv.socket_id)
				return; // This was an operation on a closed socket
			if (ec == boost::asio::error::operation_aborted)
				return;

			part->rcv_read_in_progress = false;
			if (ec)
			{
				dstream3 << LogPrefix(this, part->name) << direction_prefix[recv] << "read failed (header): " << ec.message();
				CloseConnection(part, recv);
				return;
			}
			const size_t read_size = sizeof(FrameHeader);
			if (bytes_transferred != read_size)
			{
				dstream3 << LogPrefix(this, part->name) << direction_prefix[recv] << "wrong read size (header): " << bytes_transferred << " != " << read_size;
				CloseConnection(part, recv);
				return;
			}
			m_router->NotifyRead(bytes_transferred);
#if STREAMS_VERBOSE_LOGS
			dstream4 << LogPrefix(this, part->name) << direction_prefix[recv] << "read ok (header): size=" << bytes_transferred << ", track=" << static_cast<int>(part->rcv.header.track);
#endif

			if (part->rcv.header.track == Track{} || !IsInMTracks(part->rcv.mtracks, part->rcv.header.track))
			{
				dstream3 << LogPrefix(this, part->name) << direction_prefix[recv] << "wrong track: " << static_cast<int>(part->rcv.header.track);
				CloseConnection(part, recv);
				return;
			}

			const size_t body_size = part->rcv.header.length;
			if (body_size > 0)
			{
				part->rcv.body = vs::make_unique_default_init<char[]>(body_size);
				ReadFrameBody(part);
			}
			else
				ReadFrameHeader(part);
		}
	));
}

template <class Socket>
void RouterConference<Socket>::ReadFrameBody(participant_ptr part)
{
	assert(m_strand.running_in_this_thread());
	assert(!part->rcv_read_in_progress);
	assert(part->rcv.socket.is_open());
	assert(part->rcv.body != nullptr);
	assert(part->rcv.header.length > 0);

	part->rcv_read_in_progress = true;
	const auto buffer = boost::asio::buffer(part->rcv.body.get(), part->rcv.header.length);
#if STREAMS_VERBOSE_LOGS
	dstream4 << LogPrefix(this, part->name) << direction_prefix[recv] << "read start (body): size=" << boost::asio::buffer_size(buffer);
#endif
	boost::asio::async_read(part->rcv.socket, buffer, m_strand.wrap(
		[this, part, socket_id = part->rcv.socket_id] (const boost::system::error_code& ec, size_t bytes_transferred)
		{
			if (socket_id != part->rcv.socket_id)
				return; // This was an operation on a closed socket
			if (ec == boost::asio::error::operation_aborted)
				return;

			part->rcv_read_in_progress = false;
			if (ec)
			{
				dstream3 << LogPrefix(this, part->name) << direction_prefix[recv] << "read failed (body): " << ec.message();
				CloseConnection(part, recv);
				return;
			}
			const size_t read_size = part->rcv.header.length;
			if (bytes_transferred != read_size)
			{
				dstream3 << LogPrefix(this, part->name) << direction_prefix[recv] << "wrong read size (body): " << bytes_transferred << " != " << read_size;
				CloseConnection(part, recv);
				return;
			}
			m_router->NotifyRead(bytes_transferred);
#if STREAMS_VERBOSE_LOGS
			dstream4 << LogPrefix(this, part->name) << direction_prefix[recv] << "read ok (body): size=" << bytes_transferred;
#endif

			const auto cksum = GetFrameBodyChecksum(part->rcv.body.get(), part->rcv.header.length);
			if (cksum != part->rcv.header.cksum)
			{
				dstream3 << LogPrefix(this, part->name) << direction_prefix[recv] << "wrong frame checksum: " << static_cast<int>(cksum) << " != " << static_cast<int>(part->rcv.header.cksum);
				CloseConnection(part, recv);
				return;
			}

			ProcessReadFrame(part);
		}
	));
}

template <class Socket>
void RouterConference<Socket>::CloseConnection(participant_ptr part, direction dir)
{
	assert(m_strand.running_in_this_thread());

	auto& ci = part->connection(dir);

	if (ci.socket.is_open())
	{
		char* const ip_str = dir == send ? part->stat.receiverIp : part->stat.senderIp;
		const size_t ip_str_sz = dir == send ? sizeof(part->stat.receiverIp) : sizeof(part->stat.senderIp);
		ip_str[GetRemoteIP(ci.socket).copy(ip_str, ip_str_sz - 1)] = '\0';

		boost::system::error_code ec;
		if (ci.qos_flow != nullptr)
		{
			ci.qos_flow->RemoveSocket(ci.socket.native_handle());
			ci.qos_flow = nullptr;
		}
		ci.socket.close(ec);
		if (ec)
			dstream4 << LogPrefix(this, part->name) << direction_prefix[dir] << "close failed: " << ec.message();
		++ci.socket_id;
		ci.last_disconnect = time(0);

#if STREAMS_VERBOSE_LOGS
		dstream4 << LogPrefix(this, part->name) << direction_prefix[dir] << "close ok: socket_id=" << ci.socket_id;
#endif
	}

	if (dir == send)
	{
		part->snd_write_in_progress = false;
		part->stat_calc.FreeSndStatCalc(&part->stat);
	}
	else
	{
		part->rcv_read_in_progress = false;
		part->stat_calc.FreeRcvStatCalc(&part->stat);
	}
}

template <class Socket>
void RouterConference<Socket>::StartWrite(participant_ptr part)
{
	assert(m_strand.running_in_this_thread());

	if (part->snd_write_in_progress || !part->snd.socket.is_open())
		return;

	if (PrepareWriteFrame(part))
		WriteFrame(part);
}

template <class Socket>
bool RouterConference<Socket>::PrepareWriteStatistics(participant_ptr part)
{
	assert(m_strand.running_in_this_thread());
	assert(!part->snd_write_in_progress);

	if (!IsInMTracks(part->snd.mtracks, Track::old_command))
	   return false;

	const auto now = std::chrono::steady_clock::now();
	if (now - part->last_statistics_send_time < c_statistics_send_interval)
		return false;
	part->last_statistics_send_time = now;

	size_t stat_size = 0;
	for (const auto& kv : part->receivers)
		stat_size += kv.first->stat_calc.GetSndStatisticsSize();
	if (stat_size == 0)
		return false;

	auto body = vs::make_unique_default_init<char[]>(stat_size);
	size_t offset = 0;
	for (const auto& kv : part->receivers)
	{
		offset += kv.first->stat_calc.FormSndStatistics(reinterpret_cast<StreamStatistics*>(body.get() + offset), stat_size - offset);
		assert(offset <= stat_size);
	}
	assert(offset == stat_size);

	if (m_crypter.IsValid())
	{
		uint32_t enc_size = stat_size * 2 + 16;
		auto enc_body = vs::make_unique<char[]>(enc_size);
		if (m_crypter.Encrypt(reinterpret_cast<unsigned char*>(body.get()), stat_size, reinterpret_cast<unsigned char*>(enc_body.get()), &enc_size))
		{
			body = std::move(enc_body);
			stat_size = enc_size;
		}
		else
			return false;
	}

	const auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start_time).count();
	part->snd.header.length = stat_size;
	part->snd.header.tick_count = ticks;
	part->snd.header.track = Track::old_command;
	part->snd.header.cksum = GetFrameBodyChecksum(body.get(), stat_size);
	part->snd.body = std::move(body);
	return true;
}

template <class Socket>
bool RouterConference<Socket>::PrepareWriteFrame(participant_ptr part)
{
	assert(m_strand.running_in_this_thread());
	assert(!part->snd_write_in_progress);

	uint32_t tick_count;
	Track track;
	std::unique_ptr<char[]> body;
	size_t size;
	switch (part->snd_buffer->GetFrame(tick_count, track, body, size))
	{
	case Buffer::Status::success:
#if STREAMS_VERBOSE_LOGS
		dstream4 << LogPrefix(this, part->name) << "GetFrame(" << tick_count << ", " << static_cast<int>(track) << ", ..., " << size << ") ok";
#endif
		part->snd.header.length = size;
		part->snd.header.tick_count = tick_count;
		part->snd.header.track = track;
		part->snd.header.cksum = GetFrameBodyChecksum(body.get(), size);
		part->snd.body = std::move(body);
		return true;
	case Buffer::Status::non_fatal:
		break;
	case Buffer::Status::fatal:
		dstream4 << LogPrefix(this, part->name) << "GetFrame() fatal error";
		DeleteParticipant(part);
		break;
	}
	return false;
}

template <class Socket>
void RouterConference<Socket>::ProcessWriteFrame(participant_ptr part)
{
	assert(m_strand.running_in_this_thread());

	const auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start_time).count();

	m_logger.LogFrameWrite(part->snd.header, part->name.c_str(), &part->log_file, &m_log_file);
	part->stat_calc.SndWrite(part->snd.header, ticks);

	StartWrite(part);
}

template <class Socket>
void RouterConference<Socket>::ProcessReadFrame(participant_ptr part)
{
	assert(m_strand.running_in_this_thread());

	const auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start_time).count();
	const auto& header = part->rcv.header;
	const size_t body_size = header.length;

	m_logger.LogFrameRead(header, part->name.c_str(), &part->log_file, &m_log_file);
	part->stat_calc.RcvRead(header, ticks);
	m_signal_FrameReceived(m_name.c_str(), part->name.c_str(), &header, part->rcv.body.get());

	std::vector<participant_ptr> receivers;
	receivers.reserve(part->receivers.size());
	for (const auto& kv : part->receivers)
		if (IsInMTracks(kv.second.wanted_mtracks, header.track))
			receivers.push_back(kv.first);

	size_t remaining_receivers = receivers.size();
	for (const auto& connected_part : receivers)
	{
		std::unique_ptr<char[]> body;
		assert(remaining_receivers > 0);
		if (--remaining_receivers == 0)
			body = std::move(part->rcv.body);
		else
		{
			body = vs::make_unique_default_init<char[]>(body_size);
			memcpy(body.get(), part->rcv.body.get(), body_size);
		}

		switch (connected_part->snd_buffer->PutFrame(ticks, header.track, std::move(body), body_size))
		{
		case Buffer::Status::success:
#if STREAMS_VERBOSE_LOGS
			dstream4 << LogPrefix(this, connected_part->name) << "PutFrame(" << ticks << ", " << static_cast<int>(header.track) << ", ..., " << body_size << ") ok, from: " << part->name;
#endif
			StartWrite(connected_part);
			connected_part->stat_calc.SndPutBuffer(header, ticks, connected_part->snd_buffer.get());
			break;
		case Buffer::Status::non_fatal:
#if STREAMS_VERBOSE_LOGS
			dstream4 << LogPrefix(this, connected_part->name) << "PutFrame(" << ticks << ", " << static_cast<int>(header.track) << ", ..., " << body_size << ") non-fatal error, from: " << part->name;
#endif
			StartWrite(connected_part);
			break;
		case Buffer::Status::fatal:
			dstream3 << LogPrefix(this, connected_part->name) << "PutFrame(" << ticks << ", " << static_cast<int>(header.track) << ", ..., " << body_size << ") fatal error, from: " << part->name;
			DeleteParticipant(connected_part);
			break;
		}
	}

	ReadFrameHeader(part);
}

template<class Socket>
StreamStatistics * stream::RouterConference<Socket>::RetrieveInstantSendStatistic(participant_ptr part)
{
	auto rcv_size(part->stat_calc.GetSndStatisticsSize());
	if (part->received_statistic_buffer.size() < rcv_size) {
		part->received_statistic_buffer.resize(rcv_size);
	}
	bool video(true);
	auto rcv_stat = reinterpret_cast<stream::StreamStatistics*>(part->received_statistic_buffer.data());
	if (part->stat_calc.FormSndStatistics(rcv_stat, rcv_size, &video) <= 0) {
		return nullptr;
	}
	return rcv_stat;
}

template<class Socket>
void stream::RouterConference<Socket>::CalculateInstantSendStatistic(participant_ptr part)
{
	auto rcv_stat = RetrieveInstantSendStatistic(part);
	if (!rcv_stat) {
		return;
	}
	part->bandwidth_info.queueBytes = part->snd_buffer->GetQueueBytes();
	part->bandwidth_info.queueLenght = part->snd_buffer->GetFrameCount();
	part->bandwidth_info.receivedBytes = rcv_stat->allWriteBytesBand;
}

template<class Socket>
bool stream::RouterConference<Socket>::CalculateBandwidth(participant_ptr part)
{
	assert(m_strand.running_in_this_thread());

	if (!part->snd_buffer->Ready()) {
		return false;
	}
	CalculateInstantSendStatistic(part);
	return CalculateParticipantBandwidth(&part->bandwidth_info, 20, false);
}

template <class Socket>
void RouterConference<Socket>::ScheduleTimer()
{
	m_timer.expires_from_now(c_timer_interval);
	m_timer.async_wait(m_strand.wrap(
		[this, self = this->shared_from_this()](const boost::system::error_code& ec)
		{
			if (ec == boost::asio::error::operation_aborted)
				return;

			const auto now = std::chrono::steady_clock::now();
			m_router->Timer(now);

			if (!m_parts.empty())
				m_last_active_time = now;
			else if (now - m_last_active_time > m_max_silence)
			{
#if STREAMS_VERBOSE_LOGS
			dstream4 << LogPrefix(this) << "delete by inactivity";
#endif
				m_router->DeregisterConference(m_name);
				return;
			}

			const auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start_time).count();
			for (auto it = m_parts.begin(); it != m_parts.end(); )
			{
				const auto part = *it;

				if (part->snd.socket.is_open() || part->rcv.socket.is_open())
					part->last_active_time = now;
				else if (now - part->last_active_time > part->max_silence)
				{
#if STREAMS_VERBOSE_LOGS
					dstream4 << LogPrefix(this, part->name) << "delete by inactivity";
#endif
					++it;
					DeleteParticipant(part);
					continue;
				}

				part->stat_calc.ProcessingSend(ticks, part->snd_buffer.get());
				part->stat_calc.ProcessingReceive(ticks);
				StartWrite(part);
				if ((m_type == VS_Conference_Type::CT_PRIVATE) && CalculateBandwidth(part)) {
					assert(part->receivers.size() == 1);
					auto sender = part->receivers.begin()->first;
					m_router->GetCCS()->RestrictBitrate(m_name.c_str(), sender->name.c_str(), part->bandwidth_info.restrictBitrate);
				}

				++it;
			}

			ScheduleTimer();
		}
	));
}

}

#undef DEBUG_CURRENT_MODULE
