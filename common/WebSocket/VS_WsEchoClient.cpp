#if defined(_WIN32) // Not ported

#include "VS_WsEchoClient.h"
#include <boost/make_shared.hpp>

#include "FakeClient/VS_FakeClientManager.h"
#include "../tools/Server/VS_Server.h"

VS_WsEchoClient::VS_WsEchoClient(void)
	: VS_WsChannel(VS_FakeClientManager::Instance().GetExternalThread())
	, m_is_text(true)
	, m_data_len(0)
{}

VS_WsEchoClient::~VS_WsEchoClient(void)
{}

bool VS_WsEchoClient::ProcBinaryMsg(const void* msg, unsigned long len)
{
	m_is_text = false;

	m_data_len = len;
	if (len > 0)
	{
		m_data.resize(len);
		memcpy(&(m_data[0]), msg, len);
	}
	return SendResponse();
}

bool VS_WsEchoClient::ProcTextMsg(const char* msg, unsigned long len)
{
	m_is_text = true;

	m_data_len = len;
	if (len > 0)
	{
		m_data.resize(len);
		memcpy(&(m_data[0]), msg, len);
	}

	return SendResponse();
}

bool VS_WsEchoClient::SendResponse()
{
	if (m_data_len > 0)
	{
		if (m_is_text)
			return SendTextMsg((const char *)&m_data[0], m_data_len);

		return SendBinaryMsg((const void *)&m_data[0], m_data_len);
	}

	return true;
}

#endif
