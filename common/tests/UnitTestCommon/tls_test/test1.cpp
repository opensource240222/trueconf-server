#if defined(_WIN32) // Not ported yet

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
// win
#include <process.h>

// std
#include <functional>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <random>
#include <list>
#include <vector>
#include <thread>

#include <cstdlib>
#include <cstdarg>

// boost
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>

// custom
#include "../../../acs/connection/VS_ConnectionTLS.h"
#include "../../../acs/connection/VS_ConnectionTypes.h"
#include "../../../std/cpplib/VS_WorkThreadIOCP.h"
#include "std/cpplib/ThreadUtils.h"

#include "common.h"

// certificates and keys
#include "cert_data.h" // temporary solution

// constants
static const unsigned short PORT = 44407;
static const size_t MSG_SIZE = 512 * 1024; // 512 Kb
static const unsigned long SRV_WAIT_TIMEOUT = 5 * 1000; // 20 sec

namespace tls_test1 {
	// imports
	using namespace tls_test_common;

	/* !!! Server Class !!! */
	class Server : public Common {
	public:
		void Run();

		Server(boost::shared_ptr<VS_WorkThread> work_thread, RandomData &data)
			: Common(data, work_thread)
		{
			if ((ev_done = CreateEvent(NULL, 1, 0, NULL)) == INVALID_HANDLE_VALUE)
			{
				//panic(1, "Can\'t create event!");
				throw std::exception("Can\'t create event!");
			}

			conn->SetVerifyMode(VS_ConnectionTLS::VERIFY_NONE);
		}

		virtual ~Server();
	private:
		bool LoadCertAndKey();
		// run handler and stop worker thread;
		void InvokeHandler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size);
		void Handler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size);
	private:
		VS_ConnectionTCP listener;
	};

	Server::~Server()
	{
		listener.Close();
		//worker->Stop();
	}

	void Server::Run()
	{
		if (!LoadCertAndKey())
		{
			SetEvent(ev_done);
			std::cerr << "Can\'t load key and certificate" << std::endl;
			return;
		}

		unsigned long timeout = SRV_WAIT_TIMEOUT;
		listener.Listen(PORT, false);
		conn->SetCallback(boost::bind(&Server::InvokeHandler, this, _1, _2, _3, _4));
		if (conn->Accept(&listener, timeout) <= 0)
		{
			std::cerr << "Can\'t accept connection" << std::endl;
			return;
		}

		WaitForSingleObject(ev_done, INFINITE);
	}

	// run handler and stop worker thead
	void Server::InvokeHandler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size)
	{
		//std::cout << "Invoking server handler..." << std::endl;

		Server::Handler(success, conn, buf, size);
		//std::cout << "Server TLS status: " << conn->GetStateStringLong() << std::endl;
		//self->worker->Stop();
		SetEvent(ev_done);
	}

	void Server::Handler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size)
	{
		std::vector<uint8_t> msg_buf(MSG_SIZE);
		const unsigned long buf_size = MSG_SIZE;
		bool received_ok;

		//std::cout << "Success: " << (success ? "true" : "false") << std::endl;
		if (buf != NULL && size > 0)
		{
			std::cout << "There is some data!" << std::endl;
		}

		if (!success)
		{
			std::cerr << "Error during connection!" << std::endl;
			return;
		}

		// Send data to the client
		if (!SendAll(data.getData(), data.getSize()))
		{
			std::cerr << "Can't send data to the client!" << std::endl;
			return;
		}

		// Receive message back
		if (!ReceiveAll((void *)&msg_buf[0], buf_size))
		{
			std::cerr << "Can't receive meessage from client!" << std::endl;
		}

		received_ok = data.equal(&msg_buf[0], buf_size);

		SetStatus(received_ok);
	}

	// load key and private certificate from memory
	bool Server::LoadCertAndKey()
	{
		if (!conn->UseEndCert(server_cert_data, sizeof(server_cert_data)))
			return false;

		if (!conn->UsePrivateKey(server_key_data, sizeof(server_key_data)))
			return false;

		return true;
	}

	/* !!! END Server Class !!! */

	/* !!! Client Class !!! */
	class Client : public Common {
	public:
		Client(boost::shared_ptr<VS_WorkThread> work_thread, RandomData &data);
		virtual ~Client();

		void Run();
	private:
		bool DoConnect();
		bool LoadCertAndKey();
		// run handler and stop worker thread;
		void InvokeHandler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size);
		void Handler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size);
	};

	Client::Client(boost::shared_ptr<VS_WorkThread> work_thread, RandomData &data)
		: Common(data, work_thread)
	{
		if ((ev_done = CreateEvent(NULL, 1, 0, NULL)) == INVALID_HANDLE_VALUE)
		{
			//panic(1, "Can't create event!");
			throw std::exception("Can\'t create event!");
		}

		conn->SetVerifyMode(VS_ConnectionTLS::VERIFY_NONE);
	}

	Client::~Client()
	{
	}

	bool Client::DoConnect()
	{
		std::unique_ptr<char[]> host_str(new char[128]);
		unsigned long timeout = SRV_WAIT_TIMEOUT;

		if (!VS_GetDefaultHostName(host_str.get(), 128))
			return false;

		if (!conn->Connect(host_str.get(), PORT, timeout))
		{
			return false;
		}

		return true;
	}

	void Client::Run()
	{
		if (!LoadCertAndKey())
		{
			std::cerr << "Can\'t load user certificate and key connection!" << std::endl;

		    InvokeHandler(false, conn.get(), NULL, 0);
			return;
		}

		// seting callback
		conn->SetCallback(boost::bind(&Client::InvokeHandler, this, _1, _2, _3, _4));

		if (!DoConnect())
		{
			std::cerr << "Can't connect to the server" << std::endl;
			return;
		}

		//worker->WaitStopped();
		WaitForSingleObject(ev_done, INFINITE);
	}

	void Client::InvokeHandler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size)
	{
		//std::cout << "Invoking client handler..." << std::endl;

		Client::Handler(success, conn, buf, size);

		//std::cout << "Client TLS status: " << conn->GetStateStringLong() << std::endl;
		//self->worker->Stop();
		SetEvent(ev_done);
	}

	void Client::Handler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size)
	{
		std::vector<uint8_t> msg_buf(MSG_SIZE);
		const unsigned long buf_size = MSG_SIZE;
		bool receive_ok;

		//std::cout << "Success: " << (success ? "true" : "false") << std::endl;

		if (buf != NULL && size > 0)
		{
			std::cout << "There is some data " <<"(" << size << ")!" << std::endl;
			memcpy(&msg_buf[0], buf, size);
		}

		if (!success)
		{
			std::cerr << "Error during connection!" << std::endl;
			return;
		}

		/* get message from server */
		if (!ReceiveAll((void *)&msg_buf[size], buf_size - size))
		{
			std::cerr << "Can\'t receive message from server" << std::endl;
			return;
		}

		receive_ok = data.equal(&msg_buf[0], buf_size);

		/* send it back */
		if (!SendAll((void *)&msg_buf[0], buf_size))
		{
			std::cerr << "Can\'t send message back to server" << std::endl;
			return;
		}

		SetStatus(receive_ok);
		//std::cout << "Received: " << (is_equal ? "OK" : "FAILED") << std::endl;
	}

	bool Client::LoadCertAndKey()
	{
		if (!conn->UseEndCert(client_cert_data, sizeof(client_cert_data)))
			return false;

		if (!conn->UsePrivateKey(client_key_data, sizeof(client_key_data)))
			return false;

		return true;
	}

	/* !!! END Client Class !!! */

	/* test itself */
	TEST_F(TLSTests, Echo)
	{
		try {
			/* ugly random engine seeding */
			std::default_random_engine gen((uint32_t)std::chrono::system_clock::now().time_since_epoch().count());
			RandomData data(MSG_SIZE, gen);
			HANDLE hReady; // server is ready
			boost::shared_ptr<VS_WorkThreadIOCP> srv_work_thread(new VS_WorkThreadIOCP());
			boost::shared_ptr<VS_WorkThreadIOCP> client_work_thread(new VS_WorkThreadIOCP());

			// create event
			hReady = CreateEvent(NULL, FALSE, FALSE, NULL);
			ASSERT_FALSE(hReady == NULL) << "Can\'t create system event!" << std::endl;

			// create server and client
			srv_work_thread->Start("T:SSLServerWrk");
			client_work_thread->Start("T:SSLClientWrk");
			Server srv(srv_work_thread, data);
			Client client(client_work_thread, data);

			std::thread server_thread([hReady, &srv]()
			{
				vs::SetThreadName("T:SSLServer");
				SetEvent(hReady);
				srv.Run();
			});

			std::thread client_thread([hReady, &client]()
			{
				vs::SetThreadName("T:SSLClient");
				WaitForSingleObject(hReady, INFINITE);
				client.Run();
			});

			client_thread.join();
			server_thread.join();

			/*std::cout << "\n\t*** TLS Connection Echo Test results ***" << std::endl;
			std::cout << "Server status: " << (srv.GetStatus() ? "OK" : "FAILED") << std::endl;
			std::cout << "Client status: " << (client.GetStatus() ? "OK" : "FAILED") << std::endl;
			std::cout << "--------------" << std::endl;
			std::cout << "Total: " << (srv.GetStatus() && client.GetStatus() ? "OK" : "FAILED") << std::endl;*/

			EXPECT_TRUE(srv.GetStatus()) << "Server status: FAILED" << std::endl;
			EXPECT_TRUE(client.GetStatus()) << "Client status: FAILED" << std::endl;

			CloseHandle(hReady);

			ASSERT_TRUE(srv.GetStatus() && client.GetStatus());
			//return (srv.GetStatus() && client.GetStatus() ? 0 : 1);
		}
		catch (...)
		{
			FAIL() << "Unexpected exception!" << std::endl;
		}
	}
}

#endif
