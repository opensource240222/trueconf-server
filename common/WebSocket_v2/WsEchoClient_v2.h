#pragma once

#include "WebSocket_v2/WsChannel.h"

namespace ws {

class EchoClient : public ws::Channel {
public:
	explicit EchoClient(boost::asio::io_service &ios);

	bool ProcTextMsg(const char* msg, unsigned long len) override;
	bool ProcBinaryMsg(const void* msg, unsigned long len) override;
	bool OnError(const boost::system::error_code &ec) override;
	bool SendResponse();
private:
	bool process_msg(const void* msg, unsigned long len);
	bool m_is_text = true;
	std::vector<uint8_t> m_data;
	size_t m_data_len = 0;
};

}
