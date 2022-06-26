#include "VS_StreamsRelay.h"
#include "VS_CircuitStreamRelayInterface.h"
#include "../Router/Router.h"
#include "std-generic/cpplib/scope_exit.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace vs_relaymodule{

VS_StreamsRelay::VS_StreamsRelay(stream::Router* sr)
	: m_stream_router(sr)
	, m_subscribers(std::make_shared<subscribers_t>())
{
	if(m_stream_router)
		m_stream_router->AddConferencesCondition(this);
}

VS_StreamsRelay::~VS_StreamsRelay()
{
	if(m_stream_router)
		m_stream_router->RemoveConferencesCondition(this);

	const auto subscribers = m_subscribers.load();
	for (const auto& info : *subscribers)
		info.frame_transmit_connection.disconnect();
}

bool VS_StreamsRelay::ConnectToConference(string_view conf_name, const std::shared_ptr<VS_CircuitStreamRelayInterface>& circuit)
{
	if(!m_stream_router || !circuit)
		return false;

	auto connection = m_stream_router->ConnectToFrameSink(conf_name,
		[w_circuit = std::weak_ptr<VS_CircuitStreamRelayInterface>(circuit)] (const char* conf_name, const char* part, const stream::FrameHeader* frame_head, const void* frame_data)
		{
			if (const auto circuit = w_circuit.lock())
				circuit->TransmitFrame(conf_name, part, frame_head, frame_data);
		});
	if (!connection.connected())
		return false;
	VS_SCOPE_EXIT { connection.disconnect(); };

	auto subscribers = m_subscribers.load();
	auto new_subscribers = std::make_shared<subscribers_t>();
	do
	{
		const auto range = std::equal_range(subscribers->begin(), subscribers->end(), conf_name);
		if (std::any_of(range.first, range.second, [&] (const SubscriberInfo& info) { return info.circuit.lock() == circuit; }))
			return false; // Trying to connect a second time.

		// Insert the new element before the first one with "larger" conference name.
		// This will ensure that the list is sorted by conference name.
		new_subscribers->clear();
		new_subscribers->reserve(subscribers->size() + 1);
		new_subscribers->insert(new_subscribers->end(), subscribers->begin(), range.first);
		new_subscribers->emplace_back(conf_name, circuit, connection);
		new_subscribers->insert(new_subscribers->end(), range.first, subscribers->end());
	} while (!m_subscribers.compare_exchange_strong(subscribers, new_subscribers));

	// We have successfully inserted the new entry, now we need to ensure that TransmitFrame connection wont be disconnected.
	connection = {};
	return true;
}

void VS_StreamsRelay::DisconnectFromConference(string_view conf_name, const std::shared_ptr<VS_CircuitStreamRelayInterface>& circuit)
{
	if (!circuit)
		return;

	auto subscribers = m_subscribers.load();
	auto new_subscribers = std::make_shared<subscribers_t>();
	while (true)
	{
		const auto range = std::equal_range(subscribers->begin(), subscribers->end(), conf_name);
		const auto it = std::find_if(range.first, range.second, [&] (const SubscriberInfo& info) { return info.circuit.lock() == circuit; });
		if (it == range.second)
			return; // No entry for spefified conf_name and circuit.

		assert(it != subscribers->end());
		assert(subscribers->size() >= 1);

		new_subscribers->clear();
		new_subscribers->reserve(subscribers->size() - 1);
		new_subscribers->insert(new_subscribers->end(), subscribers->begin(), it);
		new_subscribers->insert(new_subscribers->end(), std::next(it), subscribers->end());

		if (m_subscribers.compare_exchange_strong(subscribers, new_subscribers))
		{
			// We have successfully removed the entry, now we need to disconnect TransmitFrame connection.
			it->frame_transmit_connection.disconnect();
			break;
		}
	}
}

void VS_StreamsRelay::DisconnectFromConference(string_view conf_name)
{
	auto subscribers = m_subscribers.load();
	auto new_subscribers = std::make_shared<subscribers_t>();
	while (true)
	{
		const auto range = std::equal_range(subscribers->begin(), subscribers->end(), conf_name);
		const size_t n_to_remove = std::distance(range.first, range.second);
		assert(n_to_remove <= subscribers->size());
		if (n_to_remove == 0)
			return;

		new_subscribers->clear();
		new_subscribers->reserve(subscribers->size() - n_to_remove);
		new_subscribers->insert(new_subscribers->end(), subscribers->begin(), range.first);
		new_subscribers->insert(new_subscribers->end(), range.second, subscribers->end());

		if (m_subscribers.compare_exchange_strong(subscribers, new_subscribers))
		{
			// We have successfully removed the entries, now we need to disconnect TransmitFrame connections.
			for (auto it = range.first; it != range.second; ++it)
				it->frame_transmit_connection.disconnect();
			break;
		}
	}
}

template <class F>
void VS_StreamsRelay::ForEachCircuit(string_view conf_name, F&& f)
{
	const auto subscribers = m_subscribers.load();
	const auto range = std::equal_range(subscribers->begin(), subscribers->end(), conf_name);
	for (auto it = range.first; it != range.second; ++it)
		if (const auto circuit = it->circuit.lock())
			f(*circuit);
}

void VS_StreamsRelay::CreateConference(const char* conf_name)
{
	ForEachCircuit(conf_name, [&] (VS_CircuitStreamRelayInterface& circuit) {
		circuit.StartConference(conf_name);
	});
}

void VS_StreamsRelay::RemoveConference(const char* conf_name, const stream::ConferenceStatistics& /*cs*/)
{
	ForEachCircuit(conf_name, [&] (VS_CircuitStreamRelayInterface& circuit) {
		circuit.StopConference(conf_name);
	});
	DisconnectFromConference(conf_name);
}

void VS_StreamsRelay::AddParticipant(const char* conf_name, const char* part_name)
{
	ForEachCircuit(conf_name, [&] (VS_CircuitStreamRelayInterface& circuit) {
		circuit.ParticipantConnect(conf_name, part_name);
	});
}

void VS_StreamsRelay::RemoveParticipant(const char* conf_name, const char* part_name, const stream::ParticipantStatistics& /*ps*/)
{
	ForEachCircuit(conf_name, [&] (VS_CircuitStreamRelayInterface& circuit) {
		circuit.ParticipantDisconnect(conf_name, part_name);
	});
}

void VS_StreamsRelay::SetParticipantCaps(const char* conf_name, const char* part_name, const void* caps_buf, size_t buf_sz)
{
	ForEachCircuit(conf_name, [&] (VS_CircuitStreamRelayInterface& circuit) {
		circuit.SetParticipantCaps(conf_name, part_name, caps_buf, buf_sz);
	});
}

int VS_StreamsRelay::RestrictBitrate(const char* /*conf_name*/, const char* /*part_name*/, int /*bitrate*/)
{
	return 0;
}

int VS_StreamsRelay::RestrictBitrateSVC(const char* conf_name, const char* part_name, int v_bitrate, int bitrate, int old_bitrate)
{
	ForEachCircuit(conf_name, [&] (VS_CircuitStreamRelayInterface& circuit) {
		circuit.RestrictBitrateSVC(conf_name, part_name, v_bitrate, bitrate, old_bitrate);
	});
	return 0;
}

void VS_StreamsRelay::RequestKeyFrame(const char* conf_name, const char* part_name)
{
	ForEachCircuit(conf_name, [&] (VS_CircuitStreamRelayInterface& circuit) {
		circuit.RequestKeyFrame(conf_name, part_name);
	});
}

}
