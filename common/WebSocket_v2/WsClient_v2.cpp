#include "WsClient_v2.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_FAKE_CLIENT

ws::Client::Client(boost::asio::io_service & ios) : Channel(ios)
{
}

bool ws::Client::ProcTextMsg(const char * msg, unsigned long len)
{
	if (m_first_buffer)
	{
		boost::system::error_code ec;
		VS_AbstractJsonConnection::PostConstruct(m_socket.remote_endpoint(ec).address(), VS_AbstractJsonConnection::weak_from_this());
		if (ec) dstream4 << "ws::Client::ProcTextMsg Fail to retrieve remote endpoint!\n";
		m_first_buffer = false;
	}

	VS_AbstractJsonConnection::ProcessRequest(msg, len);
	return true;
}

bool ws::Client::ProcBinaryMsg(const void * msg, unsigned long len)
{
	return false;
}

bool ws::Client::OnError(const boost::system::error_code & ec)
{
	VS_AbstractJsonConnection::onError(ec.value());
	Shutdown();
	return true;
}

bool ws::Client::SendResponse(const char * data)
{
	static const string_view ping("{}");
	if (ping == data)
		return Ping(m_CID);
	else
		return SendTextMsg(data, strlen(data));
}
