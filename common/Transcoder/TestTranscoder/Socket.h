#pragma once
#include <WinSock2.h>
#include <string>
#include <vector>

class Socket
{
	static WSADATA wsaData;
	SOCKET socket;
public:
	static int const TCP = SOCK_STREAM;
	static int const UDP = SOCK_DGRAM;

	Socket(void);
	Socket(SOCKET sock);

	static int Init();

	bool isValid()
	{return socket != INVALID_SOCKET && socket != NULL; }
	SOCKET getSystemSocketHandle()
	{ return socket; }

	int Connect(std::string host, int port);
	int Listen(int port, int sock_type = Socket::TCP);
	Socket Accept();
	int Read(char *buffer, int count, bool waitForAllData = true);
	int Write(char *buffer, int count);
	int WaitReadyRead(int sec = 0);
	void Close();

	~Socket(void);
};

