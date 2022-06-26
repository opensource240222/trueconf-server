#ifdef _WIN32 //not ported
#include <windows.h>

#include "VS_WsChannel.h"
#include <fstream>
#include <iostream>
#include "std/debuglog/VS_Debug.h"
#include <functional>

#pragma warning (push)
#pragma warning (disable:4355)
#include <websocketpp/frame.hpp>
#include <websocketpp/connection.hpp>
#pragma warning (pop)

VS_WsChannel::VS_WsChannel(const boost::shared_ptr<VS_WorkThread> &thread)
	: ws::ChannelBase([this]() { VS_BufferedConnectionBase::Shutdown(); },[this](vs::SharedBuffer&& b) { return VS_BufferedConnectionBase::Send(std::move(b)); })
{
	m_thread = thread;
}

void VS_WsChannel::Init()
{
	VS_BufferedConnectionBase::Init();
}

VS_WsChannel::~VS_WsChannel()
{
}

bool VS_WsChannel::SetTCPConnection(VS_ConnectionTCP *conn, const void *in_buf, const unsigned long in_len)
{
	return SetConnection(conn, m_thread, in_buf, in_len);
}

size_t VS_WsChannel::onReceive(const void* data, size_t size)
{
	if (!VS_WsChannel::check_handshake(*this, m_was_handshake, static_cast<const char*>(data), static_cast<const char*>(data) + size))
		return 0;

	return m_cptr->read_some(static_cast<const char*>(data), size);
}

void VS_WsChannel::onSend(void)
{
	if (m_closing)
	{
		Shutdown();
	}
}
#endif