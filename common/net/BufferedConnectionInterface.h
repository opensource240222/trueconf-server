#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "BufferedConnection.h"

namespace net
{

class BufferedConnectionInterface
{
public:
//	If you want to override methods like you did when you derived from BufferedConnection,
//	you instead derive from IOHandler and call SetIOHandler with 'this' as an argument
	class IOHandler
	{
	public:
		virtual size_t OnReceive(const void* /*data*/, size_t size) {return size;}
		virtual void OnSend(size_t /*bytes_transferred*/) {};
		virtual bool OnError(const boost::system::error_code& /*ec*/) {return false;}
		virtual void OnClose() {};

		virtual ~IOHandler() = default;
	};
public:
	virtual BufferedConnectionState GetState() const = 0;
	virtual void Close() = 0;
	virtual void Shutdown() = 0;
	virtual void Send(vs::SharedBuffer&& buffer) = 0;
	virtual void SetIOHandler(std::weak_ptr<IOHandler> handler) = 0;
	virtual boost::asio::io_service::strand& GetStrand() = 0;

	virtual ~BufferedConnectionInterface() = default;
};// class BufferedConnectionInterface

// Implements BufferedConnection functionality
template<class Socket = boost::asio::ip::tcp::socket>
class BufferedConnectionInterface_impl
	: public BufferedConnectionInterface
	, public BufferedConnection<Socket>
{
public:
	using socket_type = Socket;

	BufferedConnectionInterface_impl(boost::asio::io_service& ios,
		size_t wirte_buffer_size = 16 * 1024,
		size_t read_buffer_size = 16 * 1024);
	BufferedConnectionState GetState() const override;
	void Close() override;
	void Shutdown() override;
	void Send(vs::SharedBuffer&& buffer) override;
	void SetIOHandler(std::weak_ptr<IOHandler> handler) override;
	boost::asio::io_service::strand& GetStrand() override;
	using BufferedConnection<Socket>::SetSocket;
	size_t OnReceive(const void* data, size_t size) override;
	void OnSend(size_t bytes_transferred) override;
	bool OnError(const boost::system::error_code& ec) override;
	void OnClose() override;
private:
	std::weak_ptr<BufferedConnectionInterface::IOHandler> m_handler;

};// class BufferedConnectionInterface_impl

}// namespace net
