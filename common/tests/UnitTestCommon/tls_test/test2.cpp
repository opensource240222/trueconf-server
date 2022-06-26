#if defined(_WIN32) // Not ported yet

#include "std/cpplib/ThreadUtils.h"
#include "common.h"

// certificates and keys
#include "cert_data.h"

// constants
static const unsigned short PORT = 44408;
static const size_t MSG_SIZE = 512 * 1024; // 512 Kb
static const unsigned long SRV_WAIT_TIMEOUT = 20 * 1000; // 20 sec


namespace tls_test2 {

	// imports
	using namespace tls_test_common;

	/* !!! Server Class !!! */
	class Server : public Common, public VS_IOHandler {
	public:
		enum {
			IO_ACCEPT = 1
		};

		Server(boost::shared_ptr<VS_WorkThread> work_thread, RandomData &data)
			: Common(data, work_thread), work_thread(work_thread)
		{
			if ((ev_done = CreateEvent(NULL, 1, 0, NULL)) == INVALID_HANDLE_VALUE)
			{
				//panic(1, "Can't create event!");
				throw std::exception("Can\'t create event!");
			}

			conn->SetVerifyMode(VS_ConnectionTLS::VERIFY_NONE);
		}
		virtual ~Server();
		void Run();

		// VS_IOHandler
		virtual void Handle(const unsigned long sz, const struct VS_Overlapped *ov);
		virtual void HandleError(const unsigned long err, const struct VS_Overlapped *ov);
	private:
		bool LoadCertAndKey();
		// run handler and stop worker thread;
		void InvokeHandshakeHandler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size);
		void HandshakeHandler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size);
	private:
		VS_ConnectionTCP listener;
		boost::shared_ptr<VS_WorkThread> work_thread;
	};

	Server::~Server()
	{
		WaitForSingleObject(ev_done, INFINITE);

		listener.Close();
	}

	void Server::Run()
	{
		if (!LoadCertAndKey())
		{
			std::cerr << "Can\'t load key and certificate" << std::endl;
			return;
		}

		conn->SetIOHandler(this);
		conn->SetOvReadFields(IO_ACCEPT);
		listener.Listen(PORT,false);
		work_thread->SetHandledConnection(&listener);

		if (!conn->Accept(&listener))
		{
			std::cerr << "Can\'t accept connection" << std::endl;
			return;
		}

		WaitForSingleObject(ev_done, INFINITE);
	}

	// run handler
	void Server::InvokeHandshakeHandler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size)
	{
		//std::cout << "Invoking server handler..." << std::endl;

		Server::HandshakeHandler(success, conn, buf, size);
		//std::cout << "Server TLS status: " << conn->GetStateStringLong() << std::endl;
		SetEvent(ev_done);
	}

	void Server::HandshakeHandler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size)
	{
		std::vector<uint8_t> msg_buf(MSG_SIZE);
		const unsigned long buf_size = MSG_SIZE;
		bool received_ok;

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

	void Server::Handle(const unsigned long sz, const struct VS_Overlapped *ov)
	{
		unsigned long timeout = 5000;// milliseconds
		switch (ov->field1)
		{
		case IO_ACCEPT:
			if (!conn->SetAcceptResult(sz, ov, &listener) ||
				!conn->Handshake(nullptr, 0,
					boost::bind(&Server::InvokeHandshakeHandler, this, _1, _2, _3, _4),
					timeout, true))
			{
				std::cerr << "SetAcceptResult() error!" << std::endl;
				SetEvent(ev_done);
			}
			break;
		default:
				std::cerr << "Illegal I/O opcode!" << std::endl;
			SetEvent(ev_done);
			break;
		}
	}

	void Server::HandleError(const unsigned long err, const struct VS_Overlapped *ov)
	{
		std::cerr << "I/O error" << std::endl;
		SetEvent(ev_done);
	}

	/* !!! END Server Class !!! */

	/* !!! Client Class !!! */
	class Client : public Common, public VS_IOHandler, public std::enable_shared_from_this<Client> {
	public:
		// io completion codes
		enum {
			IO_READ_COMPLETE = 1,
			IO_WRITE_COMPLETE = 2
		};

		Client(boost::shared_ptr<VS_WorkThread> work_thread, RandomData &data);
		virtual ~Client();

		void Run();
	private:
		void Done();
		// VS_IOHandler
		virtual void Handle(const unsigned long sz, const struct VS_Overlapped *ov);
		virtual void HandleError(const unsigned long err, const struct VS_Overlapped *ov);
		bool DoConnect();
		bool LoadCertAndKey();
		void HandshakeHandler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size);
	private:
		uint8_t *readbuf;
		size_t   readen;
		size_t   written;
	};

	Client::Client(boost::shared_ptr<VS_WorkThread> work_thread, RandomData &data)
		: Common(data, work_thread), readen(0), written(0)
	{
		if ((ev_done = CreateEvent(NULL, 1, 0, NULL)) == INVALID_HANDLE_VALUE)
		{
			//panic(1, "Can't create event!");
			throw std::exception("Can\'t create event!");
		}

		conn->SetVerifyMode(VS_ConnectionTLS::VERIFY_NONE);
		readbuf = new uint8_t[data.getSize()];
	}

	Client::~Client()
	{
		WaitForSingleObject(ev_done, INFINITE);
		conn->Close();
		delete[] readbuf;
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
			return;
		}

		// setting callback
		conn->SetCallback(boost::bind(&Client::HandshakeHandler, this, _1, _2, _3, _4));

		if (!DoConnect())
		{
			std::cerr << "Can't connect to the server" << std::endl;
			return;
		}

		//worker->WaitStopped();
		WaitForSingleObject(ev_done, INFINITE);
	}

	void Client::HandshakeHandler(bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size)
	{
		std::vector<uint8_t> msg_buf(MSG_SIZE);
		const unsigned long buf_size = MSG_SIZE;

		if (buf != NULL && size > 0)
		{
			std::cout << "There is some data!" << std::endl;
		}

		if (!success)
		{
			std::cerr << "Error during connection!" << std::endl;
			Done();
			return;
		}

		// set IO handlers
		conn->SetIOHandler(this);
		conn->SetOvReadFields(Client::IO_READ_COMPLETE);
		conn->SetOvWriteFields(Client::IO_WRITE_COMPLETE);


		if (!conn->Read(readbuf, data.getSize()))
		{
			std::cerr << "Can't read data from server" << std::endl;
			Done();
		}
	}

	bool Client::LoadCertAndKey()
	{
		if (!conn->UseEndCert(client_cert_data, sizeof(client_cert_data)))
			return false;

		if (!conn->UsePrivateKey(client_key_data, sizeof(client_key_data)))
			return false;

		return true;
	}

	void Client::Done()
	{
		SetEvent(ev_done);
	}

	// stubs
	void Client::Handle(const unsigned long sz, const struct VS_Overlapped *ov)
	{
		void *indata = nullptr;
		long len = 0;

		switch (ov->field1)
		{
		case IO_READ_COMPLETE:
		{
			len = conn->SetReadResult(sz, ov, nullptr, true);

			if (len == -1)
			{
				std::cerr << "Read error!" << std::endl;
				Done();
				return;
			}

			if (len > 0)
				readen += len;

			if (len == -2)
				return;

			// we have not read enough data
			if (readen < data.getSize())
			{
				if (!conn->Read(readbuf + readen, data.getSize() - readen))
				{
					std::cerr << "Can't read data from server" << std::endl;
					Done();
				}
				return;
			}

			// compare to the data buffer
			SetStatus(data.equal(readbuf, data.getSize()));

			if (!GetStatus())
			{
				std::cerr << "Bad data!" << std::endl;
				Done();
				return;
			}

			if (!conn->Write(data.getData(), data.getSize()))
			{
				std::cerr << "Write error!" << std::endl;
				Done();
				return;
			}

			//Done();
			return;
		}
			break;
		case IO_WRITE_COMPLETE:
		{
			int res = conn->SetWriteResult(sz, ov);

			if (res == -1 )
			{
				std::cerr << "Write error!" << std::endl;
				SetStatus(false);
				Done();
				return;
			}

			if (res == -2)
				return;

			written += res;
			// handle partial write properly
			if (written < data.getSize())
			{
				if (!conn->Write(((uint8_t *)data.getData()) + written, data.getSize() - written))
				{
					std::cerr << "Write error!" << std::endl;
					SetStatus(false);
					Done();
				}
				return;
			}


			SetStatus(true);
			Done();
			return;
		}
			break;
		default:
		{
			switch (ov->field1)
			{
			case IO_READ_COMPLETE:
				conn->SetReadResult(0, ov, nullptr, true);
				break;
			case IO_WRITE_COMPLETE:
				conn->SetWriteResult(0, ov);
				break;
			}
			Done();
		}
			break;
		}
	}

	void Client::HandleError(const unsigned long err, const struct VS_Overlapped *ov)
	{
		switch (ov->field1)
		{
		case IO_READ_COMPLETE:
			conn->SetReadResult(0, ov, nullptr, true);
			break;
		case IO_WRITE_COMPLETE:
			conn->SetWriteResult(0, ov);
			break;
		}

		Done();
	}

	/* !!! END Client Class !!! */

	// test itself
	//bool test_TLS_Read(void)
	TEST_F(TLSTests, Read)
	{
		try {
			/* ugly random engine seeding */
			std::default_random_engine gen((uint32_t)std::chrono::system_clock::now().time_since_epoch().count());
			RandomData data(MSG_SIZE, gen);
			HANDLE hReady; // server is ready
			// worker threads
			boost::shared_ptr<VS_WorkThreadIOCP> srv_work_thread(new VS_WorkThreadIOCP());
			boost::shared_ptr<VS_WorkThreadIOCP> client_work_thread(new VS_WorkThreadIOCP());

			// create event
			hReady = CreateEvent(NULL, FALSE, FALSE, NULL);
			ASSERT_FALSE(hReady == NULL);


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

			/*std::cout << "\n\t*** TLS Connection Read() Test results ***" << std::endl;
			std::cout << "Server status: " << (srv.GetStatus() ? "OK" : "FAILED") << std::endl;
			std::cout << "Client status: " << (client.GetStatus() ? "OK" : "FAILED") << std::endl;
			std::cout << "--------------" << std::endl;
			std::cout << "Total: " << (srv.GetStatus() && client.GetStatus() ? "OK" : "FAILED") << std::endl;
			*/
			EXPECT_TRUE(srv.GetStatus()) << "Server status: FAILED" << std::endl;
			EXPECT_TRUE(client.GetStatus()) << "Client status: FAILED" << std::endl;
			CloseHandle(hReady);

			ASSERT_TRUE(srv.GetStatus() && client.GetStatus());
			//return (srv.GetStatus() && client.GetStatus());
		}
		catch (...)
		{
			FAIL() << "Unexpected exception!" << std::endl;
		}
	}
}

#endif
