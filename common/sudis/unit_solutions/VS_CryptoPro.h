#pragma once

#include <vector>
#include <deque>
#include <string>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/function.hpp>

#include "../acs/connection/VS_ConnectionTCP.h"
#include "../../common/std/cpplib/VS_Lock.h"
#include "../../common/std/cpplib/VS_SimpleWorkThread.h"
#include "../../common/std/cpplib/VS_MessageHandler.h"

class VS_CryptoProConnection
{
	long	m_id;
	boost::asio::io_service& m_io_service;
	boost::asio::ip::tcp::socket m_socket;
	std::vector<unsigned char> m_read_buff;
	std::deque<std::vector<unsigned char>> m_read_queue;
	std::deque<std::vector<unsigned char>> m_write_queue;
	//boost::asio::streambuf m_read_buff;
	boost::function<void (long, std::vector<unsigned char>&r)> m_fireOnContent;
public:
	VS_CryptoProConnection(long id, boost::asio::io_service& io_service, boost::asio::ip::tcp::resolver::iterator endpoint_iterator, boost::function<void (long, std::vector<unsigned char>&)> cb_onReceive);
	virtual ~VS_CryptoProConnection();

	//void Test();

	void write(const std::vector<unsigned char>& msg);

	//void close()
	//{
	//	io_service_.post(boost::bind(&VS_CryptoPro::do_close, this));
	//}

private:

	void handle_connect(const boost::system::error_code& error);
	void write_in_tread(std::vector<unsigned char> msg);
	void write_first_msg();
	void read_some();
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
//	void handle_read_body(const boost::system::error_code& error);
	void handle_write(const boost::system::error_code& error);
	//void handle_write(const boost::system::error_code& error)
	//{
	//	if (!error)
	//	{
	//		write_msgs_.pop_front();
	//		if (!write_msgs_.empty())
	//		{
	//			boost::asio::async_write(socket_,
	//				boost::asio::buffer(write_msgs_.front().data(),
	//				write_msgs_.front().length()),
	//				boost::bind(&VS_CryptoPro::handle_write, this,
	//				boost::asio::placeholders::error));
	//		}
	//	}
	//	else
	//	{
	//		do_close();
	//	}
	//}

	//void do_close()
	//{
	//	socket_.close();
	//}

private:


//	chat_message read_msg_;
//	chat_message_queue write_msgs_;
};

class VS_CryptoPro: public VS_MessageHandler
{
	boost::asio::io_service m_io_service;
	boost::asio::io_service::work m_io_service_work;

	//boost::thread m_thread;
	boost::shared_ptr<VS_SimpleWorkThread> m_thread;
	boost::weak_ptr<VS_CryptoPro>	m_this;
	friend class boost::signals2::deconstruct_access;
	template<typename T> friend
		void adl_postconstruct(const boost::shared_ptr<VS_CryptoPro> &p, T *instance)
	{
		p->m_this = p;
		//p->Init();
	}

	std::string m_ip;
	std::string m_port;


	////
//	boost::asio::ip::tcp::resolver* m_resolver;
//	boost::asio::ip::tcp::resolver::query m_query;
	boost::asio::ip::tcp::resolver::iterator m_endpoint_iterator;


	VS_Lock	m_responces_lock;
	std::map<long, std::vector<unsigned char>> m_responces;

//	VS_ConnectionTCP m_tcp;
public:
	VS_CryptoPro();
	virtual ~VS_CryptoPro();

	void Test();
//	void cb_onReceive(long id, std::vector<unsigned char> &data, const boost::system::error_code& error);
	void cb_onContent(long id, std::vector<unsigned char> &data);
	bool WaitResponce(long id, std::vector<unsigned char>& encoded);
	void HandleMessage(const boost::shared_ptr<VS_MessageData> &message);
	bool SendRecv(unsigned long ip, unsigned short port, char* in_buff, unsigned long in_sz, char* out_buff, unsigned long& out_sz);
	//	void InitConnection();
};