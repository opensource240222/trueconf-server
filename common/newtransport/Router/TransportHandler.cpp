#include "TransportHandler.h"
#include "../Const.h"
#include "../Handshake.h"
#include "../../net/Handshake.h"

namespace transport
{

	Handler::Handler(TransportRouter_SetConnection *tr_sc) :m_tr_sc(tr_sc)
	{
	}

	Handler::~Handler() = default;

	acs::Response Handler::Protocol(const acs::Handler::stream_buffer& buffer, unsigned channel_token)
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

	void Handler::Accept(boost::asio::ip::tcp::socket&& sock, acs::Handler::stream_buffer&& in_buffer)
	{
		net::HandshakeHeader   *hs = reinterpret_cast<net::HandshakeHeader*>(in_buffer.data());
		// say error to old architecture and prevent flood from clients
		if ((hs->version & ~c_ssl_support_mask) <= c_version_old)
		{
			m_tr_sc->SetConnection("", 1, "", std::move(sock), true, 0, 0, 0, 0, 0, 0, 0, false, false);
			return;
		}
		const char* cid = nullptr;
		const char* sid = nullptr;
		uint8_t hops = 0;
		const void* rnd_data = nullptr;
		size_t rnd_data_ln = 0;
		const void* sign = nullptr;
		size_t sign_ln = 0;
		bool tcpKeepAliveSupport = false;
		if (!ParseHandshake(hs, cid, sid, hops, rnd_data, rnd_data_ln, sign, sign_ln, tcpKeepAliveSupport))
		{
			sock.close();
			return;
		}

		m_tr_sc->SetConnection(cid, hs->version, sid, std::move(sock), true, 0, 0, hops, rnd_data, rnd_data_ln, sign, sign_ln, false, tcpKeepAliveSupport);
	}

}
