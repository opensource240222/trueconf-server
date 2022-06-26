#pragma once

#include "BufferedConnectionInterface.h"

namespace net
{
template<class Socket>
BufferedConnectionInterface_impl<Socket>::BufferedConnectionInterface_impl(
	boost::asio::io_service& ios,
	size_t write_buffer_size,
	size_t read_buffer_size):
	BufferedConnection<Socket>(ios, write_buffer_size, read_buffer_size)
//	m_connection(std::make_shared<BufferedConnectionInterface_impl<Socket>>(ios, write_buffer_size, read_buffer_size))
	{}

template<class Socket>
BufferedConnectionState BufferedConnectionInterface_impl<Socket>::GetState() const
{
	return BufferedConnection<Socket>::GetState();
}

template<class Socket>
void BufferedConnectionInterface_impl<Socket>::Close()
{
	BufferedConnection<Socket>::Close();
}
template<class Socket>
void BufferedConnectionInterface_impl<Socket>::Shutdown()
{
	BufferedConnection<Socket>::Shutdown();
}

template<class Socket>
void BufferedConnectionInterface_impl<Socket>::Send(vs::SharedBuffer&& buffer)
{
	BufferedConnection<Socket>::Send(std::move(buffer));
}

template<class Socket>
void BufferedConnectionInterface_impl<Socket>::SetIOHandler(std::weak_ptr<IOHandler> handler)
{
	m_handler = std::move(handler);
}

template<class Socket>
boost::asio::io_service::strand& BufferedConnectionInterface_impl<Socket>::GetStrand()
{
	return BufferedConnection<Socket>::m_strand;
}

template<class Socket>
size_t BufferedConnectionInterface_impl<Socket>::OnReceive(const void* data, size_t size)
{
	if (auto handler = m_handler.lock())
		return handler->OnReceive(data, size);
	else
		return BufferedConnection<Socket>::OnReceive(data, size);
}

template<class Socket>
void BufferedConnectionInterface_impl<Socket>::OnSend(size_t size)
{
	if (auto handler = m_handler.lock())
		handler->OnSend(size);
	else
		BufferedConnection<Socket>::OnSend(size);
}

template<class Socket>
bool BufferedConnectionInterface_impl<Socket>::OnError(const boost::system::error_code& ec)
{
	if (auto handler = m_handler.lock())
		return handler->OnError(ec);
	else
		return BufferedConnection<Socket>::OnError(ec);
}

template<class Socket>
void BufferedConnectionInterface_impl<Socket>::OnClose()
{
	if (auto handler = m_handler.lock())
		handler->OnClose();
	else
		BufferedConnection<Socket>::OnClose();
}

}// namespace net
