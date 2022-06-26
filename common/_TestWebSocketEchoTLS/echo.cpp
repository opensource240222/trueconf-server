#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// windows
#include <winsock2.h>
#include <windows.h>
#include <process.h>

// C++
#include <fstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <thread>
// boost
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>

// C
#include <cstdlib>
#include <cstdarg>

//custom
#include "../acs/lib/VS_AcsLib.h"
#include "../WebSocket/VS_WsChannel.h"
#include "../FakeClient/VS_WsClient.h"
#include "../FakeClient/VS_WsEchoClient.h"
#include "../SecureLib/VS_CryptoInit.h"
#include "../acs/connection/VS_ConnectionTLS.h"
#include "../acs/connection/VS_ConnectionTypes.h"
#include "../std/cpplib/VS_WorkThreadIOCP.h"

// common constants
static const uint16_t PORT = 43007;
static const size_t EBUFSZ = 1000;
static const size_t CERT_BUFSZ = 64 * 1024; // 64 Kb

// key and sertificate path
static const char *CERT_PATH = "certs/chain.pem";
static const char *KEY_PATH  = "certs/chain_certs/end.key";

// show message and die
static void panic(int exit_status, const char *fmt, ...)
{
	char ebuf[EBUFSZ] = { 0 };
	va_list ap;

	if (fmt != NULL)
	{
		va_start(ap, fmt);
		vsnprintf(ebuf, EBUFSZ, fmt, ap);
		va_end(ap);
	}

	cerr << ebuf << endl;
	exit(exit_status);
}

class WsServer;

/* Test Echo Client */
class WsEchoClientTest : public VS_WsEchoClient {
public:
	WsEchoClientTest(HANDLE on_done, boost::shared_ptr<VS_WorkThread> worker)
		: ev_done(on_done), worker(worker)
	{}

	virtual ~WsEchoClientTest()
	{
		Shutdown();
		worker->Stop();
		worker->WaitStopped();
	}

	virtual void onError(unsigned err)
	{
		SetEvent(ev_done);
	}
private:
	HANDLE ev_done;
	boost::shared_ptr<VS_WorkThread> worker;
};

/* !!! Main Class !!! */
class WsServer : public VS_IOHandler, public enable_shared_from_this<WsServer> {
public:
	enum {
		IO_ACCEPT = 1
	};
	static shared_ptr<WsServer> Make();
	virtual ~WsServer();
	bool LoadCertAndKey(const char *cert_path, const char *key_path);
	bool Run(const uint16_t port);
	void Handle(const unsigned long sz, const struct VS_Overlapped *ov);
	void HandleError(const unsigned long sz, const struct VS_Overlapped *ov);

	WsServer();
private:
	void Handler(shared_ptr<WsServer> self, bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size);
private:
	VS_ConnectionTLS *conn;
	boost::shared_ptr<VS_WorkThread> worker;
	HANDLE ev_done;
	shared_ptr<VS_ConnectionTCP> listener;
	boost::shared_ptr<WsEchoClientTest> client;
};

shared_ptr<WsServer> WsServer::Make()
{
	return make_shared<WsServer>();
}

WsServer::WsServer()
: worker(new VS_WorkThreadIOCP()), listener(make_shared<VS_ConnectionTCP>()), ev_done(INVALID_HANDLE_VALUE)
{
	worker->Start();
	conn = new VS_ConnectionTLS(worker);
	conn->SetVerifyMode(VS_ConnectionTLS::VERIFY_NONE);

	ev_done = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (ev_done == INVALID_HANDLE_VALUE)
	{
		panic(EXIT_FAILURE, "Can\'t create event!");
	}
	boost::shared_ptr<WsEchoClientTest> client(new WsEchoClientTest(ev_done, worker));
	this->client = client;
}

WsServer::~WsServer()
{
	if (ev_done != INVALID_HANDLE_VALUE)
		WaitForSingleObject(ev_done, INFINITE);
	listener->Close();
	if (ev_done != INVALID_HANDLE_VALUE)
		CloseHandle(ev_done);
}

void WsServer::Handler(shared_ptr<WsServer> self, bool success, VS_ConnectionTLS *conn, const void* buf, const unsigned long size)
{
	cout << "Conected!" << endl;
	cout << "TLS connection state: " << (success ? "success" : "failure") << endl;
	cout << "TLS status: " << conn->GetStateStringLong() << endl;
	if (!success)
	{
		SetEvent(ev_done);
		return;
	}

	client->SetTCPConnection(conn, buf, size);
}

bool WsServer::Run(const uint16_t port)
{
	// set handshake callback
	conn->SetCallback(boost::bind(&WsServer::Handler, this, shared_from_this(), _1, _2, _3, _4));
	conn->SetIOHandler(this);
	conn->SetOvReadFields(IO_ACCEPT);
	// start listen on PORT
	listener->Listen(PORT, true);
	worker->SetHandledConnection(listener.get());
	// async Accept()
	cout << "Waiting for browser connection on port: " << port << "..." << endl;
	if (!conn->Accept(listener.get()))
		return false;

	WaitForSingleObject(ev_done, INFINITE);
	cout << "Done" << endl;
	return true;
}

bool WsServer::LoadCertAndKey(const char *cert_path, const char *key_path)
{
	uint8_t buf[CERT_BUFSZ];
	size_t readen;

	ifstream fin_cert(cert_path, ios_base::in | ios_base::binary);
	ifstream fin_key(key_path, ios_base::in | ios_base::binary);

	fin_cert.read((char *)buf, sizeof(buf));
	readen = (size_t)fin_cert.gcount();
	fin_cert.close();
	if (!conn->UseEndCert(buf, readen))
	{
		return false;
	}

	fin_key.read((char *)buf, sizeof(buf));
	readen = (size_t)fin_key.gcount();
	fin_key.close();
	if (!conn->UsePrivateKey(buf, readen))
	{
		return false;
	}

	return true;
}

void WsServer::Handle(const unsigned long sz, const struct VS_Overlapped *ov)
{
	switch (ov->field1)
	{
	case IO_ACCEPT:
		if (!conn->SetAcceptResult(sz, ov, listener.get()))
		{
			cerr << "SetAcceptResult() error!" << endl;
			SetEvent(ev_done);
		}
		break;
	default:
		cerr << "Illegal I/O opcode!" << endl;
		SetEvent(ev_done);
		break;
	}
}

void WsServer::HandleError(const unsigned long err, const struct VS_Overlapped *ov)
{
	cerr << "I/O error" << endl;
	SetEvent(ev_done);
}

/* !!! end Main Class !!! */
void ws_echo_test(void)
{
	auto srv = WsServer::Make();
	if (!srv->LoadCertAndKey(CERT_PATH, KEY_PATH))
	{
		panic(EXIT_FAILURE, "Illegal certificate or key!");
	}
	srv->Run(PORT);
}

int main(int argc, char **argv)
{
	WSADATA wsa_data;

	if(!VS_AcsLibInitial())
	{
		cerr << "Can\'t initialize ACS!" << endl;
		return EXIT_FAILURE;
	}

	// OpenSSL
	if (!VS_CryptoInit::Init(prov_OPENSSL))
		return EXIT_FAILURE;

	WSAStartup(MAKEWORD(2, 2), &wsa_data);

	ws_echo_test();

	WSACleanup();

	return EXIT_SUCCESS;
}
