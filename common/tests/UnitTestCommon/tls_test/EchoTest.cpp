#ifdef _WIN32	// win only test for compability with old VS_ConnectionTLS
#include <gtest/gtest.h>

#include "net/tls/Connection.h"
#include "acs/connection/VS_ConnectionTLS.h"
#include "cert_data.h"
#include "std/cpplib/event.h"
#include "std-generic/cpplib/scope_exit.h"
#include "SecureLib/VS_SecureConstants.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/VS_WorkThreadIOCP.h"
#include "tests/common/ASIOEnvironment.h"
#include "transport/Router/VS_TransportRouterServiceTypes.h"

const unsigned short c_srvListenPort = 44408;
std::chrono::milliseconds c_listenTimeout = std::chrono::milliseconds(5000);
static const std::vector<uint8_t> c_testData = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
static vs::event listenBegin {true};
static int verify_cb(int /*preverifyOk*/, X509_STORE_CTX* /*ctx*/) {
	return 1;	// always ok
}

struct Server {
	VS_ConnectionTCP listener;
	std::shared_ptr<VS_ConnectionTLS> tlsConn;
	bool statusOk = false;

	explicit Server(const boost::shared_ptr<VS_WorkThread> &worker)
		: tlsConn(std::make_shared<VS_ConnectionTLS>(nullptr, worker, TLS_method()))
	{
		tlsConn->SetVerifyMode(VS_ConnectionTLS::VERIFY_NONE);

	}
	~Server() {
		listener.Close();
		tlsConn->Disconnect();
		tlsConn->Close();
	}
	void ConnectHandler(bool success, VS_ConnectionTLS *conn, const void* /*buf*/, const unsigned long /*size*/) {
		if (!success){
			std::cerr << "Error during connection!" << std::endl;
			return;
		}

		int sent = tlsConn->Send(c_testData.data(), c_testData.size());
		assert(sent == c_testData.size());
		if (!sent) {
			std::cerr << "Can't send data to the client!" << std::endl;
			return;
		}

		const size_t c_msgSize(1024);
		std::vector<uint8_t> msgBuf(c_msgSize);
		int recv = tlsConn->Receive(msgBuf.data(), msgBuf.size());
		if (recv <= 0) {
			std::cerr << "Can't receive meessage from client!" << std::endl;
			return;
		}

		msgBuf.resize(recv);
		statusOk = msgBuf == c_testData;
	}

	void Run() {
		bool secureDataLoaded =
			tlsConn->UseEndCert(server_cert_data, sizeof(server_cert_data)) &&
			tlsConn->UsePrivateKey(server_key_data, sizeof(server_key_data)) && tlsConn->AddCaCert();
		assert(secureDataLoaded);
		if (!secureDataLoaded)
			return;

		vs::event done {true};
		listener.Listen(c_srvListenPort, false);
		tlsConn->SetCallback([&](bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size) {
			VS_SCOPE_EXIT{ done.set(); };
			ConnectHandler(success, conn, buf, size);
		});

		auto timeout = static_cast<unsigned long>(c_listenTimeout.count());
		if (!tlsConn->Accept(&listener, timeout)) {
			std::cerr << "Can\'t accept connection" << std::endl;
			return;
		}

		done.wait();
	}
};

struct Client {
	tls::socket socket;
	bool statusOk = false;

	Client(boost::asio::io_service& service)
		: socket(service)
	{
		socket.reset_verify_cb(SSL_VERIFY_NONE, verify_cb);
	}
	~Client() {
		boost::system::error_code ec;
		socket.close(ec);
		assert(!ec);
	}

	void Run() {
		VS_GET_PEM_CACERT
		bool resetKeysDone = socket.reset_secure_data(client_key_data, sizeof(client_key_data), "", client_cert_data, sizeof(client_cert_data), PEM_CACERT, strlen(PEM_CACERT) + 1);
		assert(resetKeysDone);
		if (!resetKeysDone) {
			std::cerr << "Can\'t load user certificate and key connection!" << std::endl;
			return;
		}

		vs::event done(true);
		tls::endpoint dstEp(boost::asio::ip::address::from_string("127.0.0.1"), c_srvListenPort);
		socket.async_connect(dstEp, [&](const boost::system::error_code& ec) {
			VS_SCOPE_EXIT{ done.set(); };

			if (ec) {
				std::cerr << "Error during connection!" << std::endl << ec.message() << '\n';
				return;
			}

			vs::event receveDone(true);
			const size_t c_msgSize(1024);
			std::vector<uint8_t> msgBuf(c_msgSize);
			socket.async_receive(boost::asio::buffer(msgBuf.data(), msgBuf.size()),
				[&](const boost::system::error_code& ec, size_t bytes_transferred)
			{
				VS_SCOPE_EXIT{ receveDone.set(); };
				if (ec) {
					std::cerr << "Can\'t receive message from server" << std::endl;
					statusOk = false;
					return;
				}

				msgBuf.resize(bytes_transferred);
				statusOk = msgBuf == c_testData;

			});
			receveDone.wait();

			vs::event sendingDone(true);
			socket.async_send(boost::asio::buffer(c_testData.data(), c_testData.size()),
				[&](const boost::system::error_code& ec, size_t bytes_transferred)
			{
				VS_SCOPE_EXIT{ sendingDone.set(); };
				if (ec) {
					std::cerr << "Can\'t send message back to server" << std::endl << ec.message() << '\n';
					statusOk = false;
					return;
				}
				assert(bytes_transferred == c_testData.size());
			});
			sendingDone.wait();
		});
		done.wait();
	}
};

TEST(TLS, Echo) {
	boost::shared_ptr<VS_WorkThreadIOCP> srv_work_thread(new VS_WorkThreadIOCP());
	srv_work_thread->Start("T:SSLServerWrk");

	Server srv(srv_work_thread);
	Client client(g_asio_environment->IOService());

	std::thread server_thread([&srv]()
	{
		vs::SetThreadName("T:SSLServer");
		srv.Run();
	});

	std::thread client_thread([&client]()
	{
		vs::SetThreadName("T:SSLClient");
		client.Run();
	});

	client_thread.join();
	server_thread.join();

	EXPECT_TRUE(srv.statusOk);
	EXPECT_TRUE(client.statusOk);
}

#endif