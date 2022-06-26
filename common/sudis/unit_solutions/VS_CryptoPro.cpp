#include "VS_CryptoPro.h"
#include "VS_Sudis.h"
#include "../gen-cpp/TCciUserLoginV1.h"

#include "../acs/connection/VS_ConnectionTCP.h"
#include "../acs/Lib/VS_AcsLib.h"



VS_CryptoProConnection::VS_CryptoProConnection(long id, boost::asio::io_service& io_service, boost::asio::ip::tcp::resolver::iterator endpoint_iterator, boost::function<void (long, std::vector<unsigned char>&)> cb_onContent):
m_id(id), m_io_service(io_service), m_socket(io_service), m_fireOnContent(cb_onContent)
{
	printf("async_connect to %s, %s\n", endpoint_iterator->host_name().c_str(), endpoint_iterator->endpoint().address().to_string().c_str());
	boost::asio::async_connect(m_socket, endpoint_iterator,
		boost::bind(&VS_CryptoProConnection::handle_connect, this,
		boost::asio::placeholders::error));
}
VS_CryptoProConnection::~VS_CryptoProConnection()
{

}

void VS_CryptoProConnection::handle_connect(const boost::system::error_code& error)
{
	std::cout << "VS_CryptoProConnection::handle_connect() error = " << error.value() << "\n";
	if (!error)
	{
		//std::vector<unsigned char> v;
		//m_read_queue.push_back(v);
		//std::deque<std::vector<unsigned char>>::reverse_iterator it = m_read_queue.rbegin();
		//it->reserve(10000);
		//it->resize(10000);
		//boost::asio::async_read(m_socket,
		//	boost::asio::buffer(*it, it->size()),
		//	boost::bind(&VS_CryptoProConnection::handle_read, this,
		//	boost::asio::placeholders::error));
		read_some();

		if (!m_write_queue.empty())
		{
			write_first_msg();
		}
	}else{
		// todo(kt): handle no connect
	}
}

void VS_CryptoProConnection::write_first_msg()
{
	boost::asio::async_write(m_socket,
		boost::asio::buffer(m_write_queue.front(),
		m_write_queue.front().size()),
		boost::bind(&VS_CryptoProConnection::handle_write, this,
		boost::asio::placeholders::error));
}

void VS_CryptoProConnection::write(const std::vector<unsigned char>& msg)
{
	m_io_service.post(boost::bind(&VS_CryptoProConnection::write_in_tread, this, msg));
}

void VS_CryptoProConnection::write_in_tread(std::vector<unsigned char> msg)
{
	// todo(kt): store in send queue
	std::cout << "VS_CryptoProConnection::do_write()\n";
	bool write_in_progress = !m_write_queue.empty();
	m_write_queue.push_back(msg);
	if (!write_in_progress)		// todo(kt): check connected here?
	{
		write_first_msg();
	}
}

void VS_CryptoProConnection::read_some()
{
	//std::vector<unsigned char> v;
	//m_read_queue.push_back(v);
	//std::deque<std::vector<unsigned char>>::reverse_iterator it = m_read_queue.rbegin();
	//it->reserve(100);
	//it->resize(100);
	m_read_buff.reserve(100);
	m_read_buff.resize(100);
	m_socket.async_read_some(
		//boost::asio::buffer(*it,it->size()),
		boost::asio::buffer(m_read_buff,m_read_buff.size()),
		boost::bind(&VS_CryptoProConnection::handle_read, this, boost::asio::placeholders::error,  boost::asio::placeholders::bytes_transferred));
}

void VS_CryptoProConnection::handle_read(const boost::system::error_code& error,  size_t bytes_transferred)
{
	std::cout << "VS_CryptoProConnection::handle_read() error = " << error.value() << "\n";
	if (!error)
	{
		m_read_buff.resize(bytes_transferred);
//		m_fireOnReceive(m_id, *it, error);
		m_read_queue.push_back(m_read_buff);

		read_some();

		// todo(kt): check that we received content-length

		//std::iostream response_stream(&m_read_buff);
		//long content_length = 0;
		//std::string header;
		//while (std::getline(response_stream, header) && header != "\r")
		//{
		//	size_t pos = header.find("Content-Length:");
		//	if (pos!= std::string::npos)
		//	{
		//		char* ptr_value = (char*)header.c_str() + pos + strlen("Content-Length:");
		//		content_length = atoi(ptr_value);
		//	}
		//}

		//if (!content_length)
		//{
		//	// todo(kt): close connection
		//}

		//response_stream.seekg (0, response_stream.end);
		//int length = response_stream.tellg();
		//response_stream.seekg (0, response_stream.beg);

		//response_stream.read((char*)&(*it)[0],it->size());

//		read_new_msg();
		//boost::asio::async_read(m_socket,
		//	//boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
		//	m_read_buff,
		//	boost::bind(&VS_CryptoProConnection::handle_read_body, this,
		//	boost::asio::placeholders::error));
		int x = 3;
		x++;
	}
	else
	{
//		if (error == 2)
		{
			std::vector<unsigned char> v;
			for (std::deque<std::vector<unsigned char>>::iterator it=m_read_queue.begin(); it!=m_read_queue.end(); ++it)
			{
				v.insert(v.end(), it->begin(), it->end());
			}

			m_fireOnContent(m_id, v);
		}


		int x = 3;
		x++;
		//do_close();
	}
}

void VS_CryptoProConnection::handle_write(const boost::system::error_code& error)
{
	std::cout << "VS_CryptoProConnection::handle_write() error = " << error.value() << "\n";
	if (!error)
	{
		m_write_queue.pop_front();
		if (!m_write_queue.empty())
		{
			write_first_msg();
		}
	}
	else
	{
		//do_close();
	}
}

VS_CryptoPro::VS_CryptoPro(): m_io_service_work(m_io_service)//: m_resolver(0)
{
	m_ip = "192.168.62.234";
	m_port = "8001";

	//m_resolver = new boost::asio::ip::tcp::resolver(m_io_service);
	//m_query = new boost::asio::ip::tcp::resolver::query(m_ip, m_port);
	//m_endpoint_iterator = m_resolver.resolve(m_query);

	boost::asio::ip::tcp::resolver resolver(m_io_service);
	boost::asio::ip::tcp::resolver::query query(m_ip, m_port);
	//boost::asio::ip::tcp::resolver::iterator iterator = m_resolver.resolve(query);
	m_endpoint_iterator = resolver.resolve(query);

//	m_thread = boost::thread(boost::bind(&boost::asio::io_service::run, &m_io_service));
	m_thread.reset(new VS_SimpleWorkThread);
	m_thread->Start();
}


VS_CryptoPro::~VS_CryptoPro()
{
	//delete m_resolver;
	//delete m_query;

	// m_thread.join() -- finish boost thread?
}

//void VS_CryptoPro::InitConnection()
//{
//	unsigned long ip = 0xC0A83EEA;	// 192.168.62.234
//	unsigned short port = 8001;
//	unsigned long mills = 30000;
//	if (!m_tcp.Connect(ip, port, mills))
//		return;
//}

void VS_CryptoPro::HandleMessage(const boost::shared_ptr<VS_MessageData> &message)
{
	printf("VS_CryptoPro::HandleMessage start\n");
	boost::asio::io_service::work w(m_io_service);
	m_io_service.run();
	printf("VS_CryptoPro::HandleMessage finished\n");

	//if(!message)
	//	return;
	//int event_type(0);
	//unsigned long sz(0);
	//unsigned long type(0);
	//message->GetMessPointer(type,sz);
	//switch(type)
	//{
	//case e_notifyviaweb_missed_call:
	//	{
	//		NotifyViaWeb_MissedCall_Mess *mess = reinterpret_cast<NotifyViaWeb_MissedCall_Mess*>(message.get());
	//		g_dbStorage->NotifyWeb_MissedCall(mess->m_cnt);

	//		if (!!m_notify_calls_url)
	//		{
	//			// libcURL
	//			CURL *curl;
	//			CURLcode res;
	//			curl = curl_easy_init();
	//			if(curl) {
	//				curl_easy_setopt(curl, CURLOPT_URL, m_notify_calls_url.m_str);
	//				curl_easy_setopt(curl, CURLOPT_NOBODY, 1);						// to use HEAD instead of GET (not to get contents)
	//				curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);					// timeout in seconds
	//				res = curl_easy_perform(curl);
	//				/* always cleanup */
	//				curl_easy_cleanup(curl);
	//			}
	//		}
	//	}
	//	break;
	//}
}

bool VS_CryptoPro::SendRecv(unsigned long ip, unsigned short port, char* in_buff, unsigned long in_sz, char* out_buff, unsigned long& out_sz)
{
	unsigned long mills = 30000;
	VS_ConnectionTCP tcp;
	if (!tcp.Connect(ip, port, mills))
		return false;
	if (!tcp.Send(in_buff, in_sz, mills))
		return false;
	out_sz=tcp.Receive(out_buff, out_sz);
	return true;
}

void VS_CryptoPro::Test()
{
	// todo(kt): do it at Init (not ctor)
	boost::shared_ptr<VS_MessageData> mess;
	boost::shared_ptr<VS_MessageHandler> handler = m_this.lock();
	if(!handler)
		return ;
	m_thread->Post(handler, mess);


////////////////////////////////////////
//	const long id_0 = 111;
//	std::vector<unsigned char> ttt;
//	{
//		boost::asio::ip::tcp::resolver resolver(m_io_service);
//		boost::asio::ip::tcp::resolver::query query("ya.ru.ru", "http");
//		//boost::asio::ip::tcp::resolver::iterator iterator = m_resolver.resolve(query);
//		boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
//
//		//	std::vector<unsigned char> http;
//		char* http_str = "GET / HTTP/1.0\r\n"
//			"Host: ya.ru\r\n"
////			"Content-Type: application/octet-stream\r\n"
////			"Accept: application/octet-stream\r\n"
////			"Content-Length: %d\r\n\r\n";
//"\r\n";
//		unsigned char buff[10000] = {0};
//		int n_bytes = _snprintf((char*) &buff[0], 10000, http_str/*, v.size()*/);
//		std::vector<unsigned char> http;
//		http.assign(&buff[0], &buff[0] + n_bytes);
//
//		VS_CryptoProConnection c(id_0, m_io_service, endpoint_iterator, boost::bind(&VS_CryptoPro::cb_onContent, this, _1, _2));
//
//		std::cout << "wait for connect...\n";
//		Sleep(2000);
//		std::cout << "try write...\n";
//		c.write(http);
//	//	c.write(tmp);
//
//		printf("start wait for id0\n");
//		if (!WaitResponce(id_0, ttt))
//			return ;
//		printf("got responce for id0\n");
//
//		printf("ttt %d!\n", ttt.size());
//		if (ttt.size())
//		{
//			printf("ttt %d = %s\n", ttt.size(), (char*) &ttt[0]);
//		}
//
//	}
////////////////////////////////////////




	sudis::TCciUserLoginV1Request req;
	req.requestDateTime = "2014-07-08T10:54:27.643+04:00";
	req.requestNonce = "nonsa-nonsa";
	req.login = "ttest001";
	req.password = "0P4lrr7E";
	req.spCode = "svks-m";

	VS_Sudis sudis;
	std::vector<unsigned char> v;
	sudis.Serialize(req, v);

// try Synchronus

//InitConnection();
	unsigned long ip_cryptopro = 0xC0A83EEA;	// 192.168.62.234
	unsigned short port_cryptopro = 8001;
	unsigned long mills = 30000;

	char* http_str = "POST /encode HTTP/1.0\r\n"
		"Host: 192.168.62.234\r\n"
		"X-senderkey: cert22\r\n"
		"X-recipientkey: cert22\r\n"
		"Content-Type: application/octet-stream\r\n"
		"Accept: application/octet-stream\r\n"
		"Content-Length: %d\r\n\r\n";
	char buff[10000] = {0};
	int n_bytes = _snprintf((char*) &buff[0], 10000, http_str, v.size());
	memcpy_s(buff+n_bytes, 10000-n_bytes, &v[0], v.size());

	//std::vector<unsigned char> http;
	//http.assign(&buff[0], &buff[0] + n_bytes);
	//std::copy(v.begin(), v.end(), std::back_inserter(http));

	char buff_rcv[8192] = {0};
	unsigned long buff_rcv_sz = 5;
//	SendRecv(ip_cryptopro, port_cryptopro, &http[0], http.size(), buff_rcv, buff_rcv_sz);
	SendRecv(ip_cryptopro, port_cryptopro, buff, n_bytes + v.size(), buff_rcv, buff_rcv_sz);

	printf("received %d bytes, %s\n", buff_rcv_sz, buff_rcv);

//
//	// Can send this to SUDIS
//
//
//	printf("Can send this to SUDIS %d\n%s", n_bytes, buff);
//
//	return ;
//	*/
//	const long id_1 = 777;
//	std::vector<unsigned char> encrypted;
//	{
//		//	std::vector<unsigned char> http;
//		char* http_str = "POST /encode HTTP/1.0\r\n"
//			"Host: 192.168.62.234\r\n"
//			"X-senderkey: cert22\r\n"
//			"X-recipientkey: cert22\r\n"
//			"Content-Type: application/octet-stream\r\n"
//			"Accept: application/octet-stream\r\n"
//			"Content-Length: %d\r\n\r\n";
//		//std::string str = http_str;
//		//char bbb[16] = {0};
//		//str += _itoa(v.size(), bbb, 10);
//		//str += "\r\n\r\n";
//		unsigned char buff[10000] = {0};
//		int n_bytes = _snprintf((char*) &buff[0], 10000, http_str, v.size());
//		std::vector<unsigned char> http;
//		http.assign(&buff[0], &buff[0] + n_bytes);
//
//		VS_CryptoProConnection c(id_1, m_io_service, m_endpoint_iterator, boost::bind(&VS_CryptoPro::cb_onContent, this, _1, _2));
//
//		std::cout << "wait for connect...\n";
//		Sleep(2000);
//		std::cout << "try write...\n";
//		c.write(http);
//		c.write(v);
//
//		printf("start wait for id1\n");
//		if (!WaitResponce(id_1, encrypted))
//			return ;
//		printf("got responce for id1\n");
//	}
//
//
//
////	boost::asio::ip::tcp::endpoint endp_sudis(boost::asio::ip::address::from_string("int.sudis.at-consulting.com"), 80001);
//	boost::asio::ip::tcp::resolver resolver(m_io_service);
//	boost::asio::ip::tcp::resolver::query query("idmapi01.int.sudis.at-consulting.ru", "http");
//	//boost::asio::ip::tcp::resolver::iterator iterator = m_resolver.resolve(query);
//	boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
//
//	const long id_2 = 888;
//	std::vector<unsigned char> v2;
//	{
//		//	std::vector<unsigned char> http;
//		char* http_str = "POST /tbr/cciUserLoginV1 HTTP/1.0\r\n"
//			"Host: idmapi01.int.sudis.at-consulting.ru\r\n"
//			"Content-Type: application/octet-stream\r\n"
//			"Accept: application/octet-stream\r\n"
//			"Content-Length: %d\r\n\r\n";
//		//std::string str = http_str;
//		//char bbb[16] = {0};
//		//str += _itoa(v.size(), bbb, 10);
//		//str += "\r\n\r\n";
//		unsigned char buff[10000] = {0};
//		int n_bytes = _snprintf((char*) &buff[0], 10000, http_str, encrypted.size());
//		std::vector<unsigned char> http;
//		http.assign(&buff[0], &buff[0] + n_bytes);
//
//		VS_CryptoProConnection c(id_2, m_io_service, endpoint_iterator, boost::bind(&VS_CryptoPro::cb_onContent, this, _1, _2));
//
//		std::cout << "wait for connect...\n";
//		Sleep(2000);
//		std::cout << "try write...\n";
//		c.write(http);
//		c.write(encrypted);
//
//		printf("start wait for id2\n");
//		if (!WaitResponce(id_2, v2))
//			return ;
//		printf("got responce for id2\n");
//	}
//
//
//
//
//	const long id_3 = 999;
//	std::vector<unsigned char> v3;
//	{
//
//		boost::asio::ip::tcp::resolver resolver(m_io_service);
//		boost::asio::ip::tcp::resolver::query query(m_ip, m_port);
//		//boost::asio::ip::tcp::resolver::iterator iterator = m_resolver.resolve(query);
//		m_endpoint_iterator = resolver.resolve(query);
//
//
//		//	std::vector<unsigned char> http;
//		char* http_str = "POST /decode HTTP/1.0\r\n"
//			"Host: 192.168.62.234\r\n"
//			"X-senderkey: cert22\r\n"
//			"X-recipientkey: cert22\r\n"
//			"Content-Type: application/octet-stream\r\n"
//			"Accept: application/octet-stream\r\n"
//			"Content-Length: %d\r\n\r\n";
//		//std::string str = http_str;
//		//char bbb[16] = {0};
//		//str += _itoa(v.size(), bbb, 10);
//		//str += "\r\n\r\n";
//		unsigned char buff[10000] = {0};
//		int n_bytes = _snprintf((char*) &buff[0], 10000, http_str, v2.size());
//		std::vector<unsigned char> http;
//		http.assign(&buff[0], &buff[0] + n_bytes);
//
//		VS_CryptoProConnection c(id_3, m_io_service, m_endpoint_iterator, boost::bind(&VS_CryptoPro::cb_onContent, this, _1, _2));
//
//		std::cout << "wait for connect...\n";
//		Sleep(2000);
//		std::cout << "try write...\n";
//		c.write(http);
//		c.write(v2);
//
//		printf("start wait for id3\n");
//		if (!WaitResponce(id_3, v3))
//			return ;
//		printf("got responce for id3\n");
//	}
//
//
//
//	sudis::TCciUserLoginV1Response rsp;
//	bool r3 = sudis.Deserialize(v3, rsp);
//
//	printf("Deserialize %d , code=%s\n", r3, rsp.resultMessage.code.c_str());
//
//	// todo(kt): send to sudis ?

	Sleep(30000);

	//if (m_resolver)
	//	delete resolver;
	//m_resolver = new boost::asio::ip::tcp::resolver(m_io_service);
	////boost::asio::ip::tcp::resolver m_resolver(m_io_service);
	//boost::asio::ip::tcp::resolver::query query(m_ip, m_port);
	//boost::asio::ip::tcp::resolver::iterator iterator = m_resolver.resolve(query);

	////chat_client c(io_service, iterator);

	//boost::thread t(boost::bind(&boost::asio::io_service::run, &m_io_service));

}

bool VS_CryptoPro::WaitResponce(long id, std::vector<unsigned char>& encoded)
{
	bool do_exit(false);
	bool found = false;
	unsigned long t1 = GetTickCount();
	while(!found && !do_exit)
	{
		{
			VS_AutoLock lock(&m_responces_lock);
			std::map<long, std::vector<unsigned char>>::iterator it = m_responces.find(id);
			if (it!=m_responces.end())
			{
				found = true;
				encoded.assign(it->second.begin(), it->second.end());
			}
		}

		unsigned long t2 = GetTickCount();
		if (t2-t1 >= 30000)
			do_exit = true;
		Sleep(0);
	}

	printf("got responce found:%d id:%d bytes:%d\n", found, id, encoded.size());
	return found;
};

void VS_CryptoPro::cb_onContent(long id, std::vector<unsigned char> &data)
{
	printf("VS_CryptoPro::cb_onContent: id = %d, sz = %d\n", id, data.size());

	if (!data.size())
		return ;

	char* ptr = (char*) &data[0];
	if (!ptr)
		return ;

	char* content = strstr(ptr, "\r\n\r\n");
	if (!content)
	{
		// todo(kt): read more data?
		return;
	}

	content+=4;
	if (!content)
	{
		// todo(kt): read more data?
		return;
	}

	printf("before erase %d = %s\n", data.size(), (char*) &data[0]);
	data.erase(data.begin(), data.begin() + (content-ptr));		// remove http headers
	if(data.size())
	{
		printf("after erase %d = %s\n", data.size(), (char*) &data[0]);
	}

	VS_AutoLock lock(&m_responces_lock);
	m_responces[id].assign(data.begin(), data.end());
}
//
//void VS_CryptoPro::cb_onReceive(long id, std::vector<unsigned char> &data, const boost::system::error_code& error)
//{
//	printf("VS_CryptoPro::cb_onReceive: id = %d, sz = %d\n", id, data.size());
//
//	if (!data.size())
//	{
//		////TEMP!!!
//		//char* ptr = (char*) &data[0];
//		//if (!ptr)
//		//	return ;
//
//		//char* content = strstr(ptr, "\r\n\r\n");
//		//if (!content)
//		//{
//		//	// todo(kt): read more data?
//		//	return;
//		//}
//
//		//content+=4;
//		//if (!content)
//		//{
//		//	// todo(kt): read more data?
//		//	return;
//		//}
//
//		//unsigned long h_len = content - ptr;
//
////		VS_AutoLock lock(&m_responces_lock);
////		m_responces[id].assign(content, content+(.size() - h_len));
//		//-----------------------------
//		// todo(kt): read more data?
//		return;
//	}
//
//	char* ptr = (char*) &data[0];
//	if (!ptr)
//		return ;
//
//	char* ptr_content_length = strstr(ptr, "Content-Length:");
//	if (!ptr_content_length)
//	{
//		// TEMP!!!
//		char* ptr = (char*) &data[0];
//		if (!ptr)
//			return ;
//
//		char* content = strstr(ptr, "\r\n\r\n");
//		if (!content)
//		{
//			// todo(kt): read more data?
//			return;
//		}
//
//		content+=4;
//		if (!content)
//		{
//			// todo(kt): read more data?
//			return;
//		}
//
//		unsigned long h_len = content - ptr;
//		VS_AutoLock lock(&m_responces_lock);
//		m_responces[id].assign(content, content+(data.size() - h_len));
//		//////////
//
//		// todo(kt): read more data?
//		return;
//	}
//	ptr_content_length += strlen("Content-Length:");
//	if (!ptr_content_length)
//	{
//		// todo(kt): read more data?
//		return;
//	}
//	long content_length = atoi(ptr_content_length);
//	if (!content_length)
//	{
//		// todo(kt): read more data?
//		return;
//	}
//
//	char* content = strstr(ptr, "\r\n\r\n");
//	if (!content)
//	{
//		// todo(kt): read more data?
//		return;
//	}
//
//	content+=4;
//	if (!content)
//	{
//		// todo(kt): read more data?
//		return;
//	}
//
//	unsigned long headers_size = content-ptr;
//	if (data.size() > headers_size+content_length)
//	{
//		// todo(kt): read more data?
//		return;
//	}else if (data.size() == headers_size+content_length) {
//		// todo(kt) callback for content
//		printf("CONTENT (%d)\n", content_length);
//
//		// todo(kt): AutoLock ? can do it in IO thread callback?
//		VS_AutoLock lock(&m_responces_lock);
//		m_responces[id].assign(content, content+content_length);
//		// todo(kt): add timestamp for responce
//	}else {
//		// todo(kt): read too much, may be two messages at one time
//	}
//
//	printf("sz1 = %d, sz2 = %d\n", data.size(), headers_size+content_length);
//}