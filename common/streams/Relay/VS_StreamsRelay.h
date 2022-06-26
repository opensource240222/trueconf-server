#pragma once

#include "../fwd.h"
#include "../Router/ConferencesConditions.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/string_view.h"

#include <boost/signals2/connection.hpp>

#include <string>
#include <vector>

class VS_CircuitStreamRelayInterface;

namespace vs_relaymodule{

class VS_StreamsRelay : public stream::ConferencesConditions
{
public:
	explicit VS_StreamsRelay(stream::Router* sr);
	~VS_StreamsRelay();

	bool ConnectToConference(string_view conf_name, const std::shared_ptr<VS_CircuitStreamRelayInterface>& circuit);
	void DisconnectFromConference(string_view conf_name, const std::shared_ptr<VS_CircuitStreamRelayInterface>& circuit);
	void DisconnectFromConference(string_view conf_name);

private:
	stream::Router* const m_stream_router;
	struct SubscriberInfo
	{
		VS_FORWARDING_CTOR3(SubscriberInfo, conf_name, circuit, frame_transmit_connection) {}
		std::string conf_name;
		std::weak_ptr<VS_CircuitStreamRelayInterface> circuit;
		boost::signals2::connection frame_transmit_connection;

		friend bool operator<(const SubscriberInfo& info, string_view conf_name) { return info.conf_name < conf_name; }
		friend bool operator<(string_view conf_name, const SubscriberInfo& info) { return conf_name < info.conf_name; }
	};
	using subscribers_t = std::vector<SubscriberInfo>;
	vs::atomic_shared_ptr<const subscribers_t> m_subscribers;

	template <class F>
	void ForEachCircuit(string_view conf_name, F&& f);

	// stream::ConferencesConditions
	void CreateConference(const char* conf_name) override;
	void RemoveConference(const char* conf_name, const stream::ConferenceStatistics& cs) override;
	void AddParticipant(const char* conf_name, const char* part_name) override;
	void RemoveParticipant(const char* conf_name, const char* part_name, const stream::ParticipantStatistics& ps) override;
	int RestrictBitrate(const char* conf_name, const char* part_name, int bitrate) override;
	int RestrictBitrateSVC(const char* conf_name, const char* part_name, int v_bitrate, int bitrate, int old_bitrate) override;
	void RequestKeyFrame(const char* conf_name, const char* part_name) override;
	void SetParticipantCaps(const char* conf_name, const char* part_name, const void* caps_buf, size_t buf_sz) override;
};

}
