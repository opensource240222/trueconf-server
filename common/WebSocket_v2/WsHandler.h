#include "acs_v2/Handler.h"
#include "std-generic/asio_fwd.h"

namespace ws {
	class Handler : public acs::Handler {
	public:
		explicit Handler(boost::asio::io_service &ios);
		acs::Response Protocol(const stream_buffer& buffer, unsigned channel_token = 0) override;
		void Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer) override;
	private:
		boost::asio::io_service &m_ios;
	};
}