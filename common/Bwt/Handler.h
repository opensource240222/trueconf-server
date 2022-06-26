#pragma once
#include "../acs_v2/Handler.h"

namespace bwt
{
	//Server side handler that initiates send()/recv() test content to client
	class Handler : public acs::Handler
	{

      public:

		virtual ~Handler();
		virtual acs::Response Protocol(const acs::Handler::stream_buffer& buffer, unsigned channel_token = 0) override;
		virtual void Accept(boost::asio::ip::tcp::socket&& sock, acs::Handler::stream_buffer&& buffer) override;

	  private:
	};

}

