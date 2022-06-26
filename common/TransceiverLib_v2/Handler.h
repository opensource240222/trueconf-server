#pragma once
#include "acs_v2/ISetConnection.h"

#include <memory>

/* transceiver */
namespace ts {
	class Handler : public acs::Handler
	{
		std::weak_ptr<net::ISetConnection<> > m_proxiesPool;
	public:
		explicit Handler(const std::shared_ptr<net::ISetConnection<> > &proxiesPool);
		virtual ~Handler();
		virtual acs::Response Protocol(const acs::Handler::stream_buffer& buffer, unsigned channel_token = 0) override;
		void Accept(boost::asio::ip::tcp::socket&& sock, acs::Handler::stream_buffer&& buffer) override;
	};
}