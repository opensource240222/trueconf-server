#if defined(_WIN32) // Not ported

#include "VS_WsClient.h"
#include "../std/cpplib/json/elements.h"
#include <boost/make_shared.hpp>

#include "FakeClient/VS_FakeClientManager.h"
#include "../tools/Server/VS_Server.h"

#include "../std/cpplib/json/writer.h"
#include "../std/cpplib/json/reader.h"

VS_WsClient::VS_WsClient()
	: VS_WsChannel(VS_FakeClientManager::Instance().GetExternalThread())
{
	m_first_buffer = true;
}

VS_WsClient::~VS_WsClient(void)
{
}

bool VS_WsClient::ProcBinaryMsg( const void* msg, unsigned long len )
{
	return false;
}

bool VS_WsClient::ProcTextMsg( const char* msg, unsigned long len )
{
	if (m_first_buffer)
	{
		VS_AbstractJsonConnection::PostConstruct(m_c->GetPeerIp(), m_c->GetBindAddress(), m_c->GetPeerAddress(), VS_AbstractJsonConnection::weak_from_this());
		m_first_buffer = false;
	}

	VS_AbstractJsonConnection::ProcessRequest(msg, len);
	return true;
}

bool VS_WsClient::SendResponse(const char *data)
{
	static const string_view ping("{}");
	if (ping == data)
		return Ping(m_CID);
	else
		return SendTextMsg(data, strlen(data));
}

void VS_WsClient::onError(unsigned err)
{
	VS_AbstractJsonConnection::onError(err);
}

#endif
