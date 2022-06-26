#include "WsEchoClient_v2.h"

ws::EchoClient::EchoClient(boost::asio::io_service & ios) : Channel(ios)
{
}

bool ws::EchoClient::ProcTextMsg(const char * msg, unsigned long len)
{
	m_is_text = true;
	process_msg(msg, len);
	return false;
}

bool ws::EchoClient::ProcBinaryMsg(const void * msg, unsigned long len)
{
	m_is_text = false;
	process_msg(msg, len);
	return false;
}

bool ws::EchoClient::OnError(const boost::system::error_code & /*ec*/)
{
	return false;
}

bool ws::EchoClient::SendResponse()
{
	if (m_data_len > 0)
	{
		if (m_is_text)
			return SendTextMsg((const char *)&m_data[0], m_data_len);

		return SendBinaryMsg((const void *)&m_data[0], m_data_len);
	}

	return true;
}

bool ws::EchoClient::process_msg(const void * msg, unsigned long len)
{
	m_data_len = len;
	if (len > 0)
	{
		m_data.resize(len);
		memcpy(&(m_data[0]), msg, len);
	}
	return SendResponse();
}
