#pragma once

#include "FakeClient/VS_AbstractJsonConnection.h"
#include "WebSocket_v2/WsChannel.h"

namespace ws {

class Client : public ws::Channel, public VS_AbstractJsonConnection {
public:
	explicit Client(boost::asio::io_service &ios);

	bool ProcTextMsg(const char* msg, unsigned long len) override;
	bool ProcBinaryMsg(const void* msg, unsigned long len) override;
	bool OnError(const boost::system::error_code &ec) override;
	bool SendResponse(const char *data) override;
private:
	bool m_first_buffer = true;
};

}