#include "Handler.h"
#include "Conference.h"
#include "RouterInternalInterface.h"
#include "../../streams/Handshake.h"
#include "../../net/Handshake.h"

namespace stream {

Handler::Handler(RouterInternalInterface* router)
	: m_router(router)
{
}

Handler::~Handler() = default;

acs::Response Handler::Protocol(const acs::Handler::stream_buffer& buffer, unsigned /*channel_token*/)
{
	if (buffer.size() < sizeof(net::HandshakeHeader))
		return acs::Response::next_step;
	auto hs = reinterpret_cast<const net::HandshakeHeader*>(buffer.data());
	if (hs->head_cksum != GetHandshakeHeaderChecksum(*hs)
		|| hs->version < 1
		|| strncmp(hs->primary_field, PrimaryField, sizeof(hs->primary_field)) != 0)
	{
		return acs::Response::not_my_connection;
	}
	const size_t hs_size = sizeof(net::HandshakeHeader) + hs->body_length + 1;
	if (buffer.size() < hs_size)
		return acs::Response::my_connection;
	if (buffer.size() > hs_size)
		return acs::Response::not_my_connection;
	if (hs->body_cksum != GetHandshakeBodyChecksum(*hs))
		return acs::Response::not_my_connection;
	return acs::Response::accept_connection;
}

void Handler::Accept(boost::asio::ip::tcp::socket&& socket, acs::Handler::stream_buffer&& buffer)
{
	ClientType type;
	const char* conference_name;
	const char* participant_name;
	const char* connected_participant_name;
	const char* connected_endpoint_name;
	const uint8_t* mtracks;

	auto hs = reinterpret_cast<const net::HandshakeHeader*>(buffer.data());
	if (!GetHandshakeFields(hs, type, conference_name, participant_name, connected_participant_name, connected_endpoint_name, mtracks))
		return;
	if (type != ClientType::sender && type != ClientType::receiver)
		return;

	auto conf = m_router->GetConference(conference_name);
	if (conf)
		conf->SetParticipantConnection(participant_name, std::move(socket), type, mtracks);
}

}
