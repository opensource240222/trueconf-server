#pragma once

#include "../../acs_v2/Handler.h"

#include <cstdint>
#include <string>

namespace transport
{

struct TransportRouter_SetConnection
{
	TransportRouter_SetConnection(void) {}
	virtual ~TransportRouter_SetConnection(void) {}
	virtual void   SetConnection(const char *cid,
		const uint32_t version,
		const char *sid,
		boost::asio::ip::tcp::socket&& sock, const bool isAccept,
		const uint16_t maxConnSilenceMs,
		const uint8_t fatalSilenceCoef,
		const uint8_t hop,
		const void* rnd_data,
		const size_t rnd_data_ln,
		const void* sign,
		const size_t sign_ln,
		const bool hs_error = false,
		const bool tcp_keep_alive = false) = 0;
};


class Handler : public acs::Handler
{

public:
	explicit Handler(TransportRouter_SetConnection *tr_sc);
	~Handler();
	acs::Response Protocol(const acs::Handler::stream_buffer& buffer, unsigned channel_token = 0) override;
	void Accept(boost::asio::ip::tcp::socket&& sock, acs::Handler::stream_buffer&& buffer) override;

private:
	std::string m_endpoint;
	TransportRouter_SetConnection *m_tr_sc;
};

}
