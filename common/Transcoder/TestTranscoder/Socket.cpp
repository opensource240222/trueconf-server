#ifdef _WIN32
#include "Socket.h"
#include "std-generic/cpplib/hton.h"

#include <WinSock2.h>

Socket::Socket(void)
{
	this->socket = NULL;
}

Socket::Socket(SOCKET sock)
{
	this->socket = sock;
}

WSADATA Socket::wsaData;

int Socket::Init()
{
	return WSAStartup(WINSOCK_VERSION, &wsaData) == 0 ? 1 : -1;
}

int Socket::Connect(std::string host, int port)
{
	SOCKET s = ::socket( AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) return 0;

	sockaddr_in s_a = { 0 };
	s_a.sin_addr.s_addr = inet_addr( host.c_str() );
	s_a.sin_port = vs_htons(port);
	s_a.sin_family = AF_INET;

	if (connect( s, (struct sockaddr*)&s_a, sizeof( s_a ) ) == SOCKET_ERROR) return -1;

	socket = s;
	return 1;
}

int Socket::Listen(int port, int sock_type)
{
	SOCKET s = ::socket( AF_INET, sock_type, 0);
	if (s == INVALID_SOCKET) return 0;

	sockaddr_in s_a = { 0 };
	s_a.sin_addr.s_addr = vs_htonl(INADDR_ANY);
	s_a.sin_port = vs_htons(port);
	s_a.sin_family = AF_INET;

	if ( ::bind(s, (struct sockaddr*) &s_a, sizeof( s_a ) ) < 0 ) return -1;
	if (sock_type == Socket::TCP) if ( ::listen(s, SOMAXCONN) < 0 ) return -1;

	socket = s;
	return 1;

}
Socket Socket::Accept()
{
	return ::accept(socket, NULL, NULL);
}

int Socket::WaitReadyRead(int sec)
{
	fd_set set;
	set.fd_count = 1;
	set.fd_array[0] = socket;
	return ::select(0, &set, NULL, NULL, 0);
}

int Socket::Read(char *buffer, int count, bool waitForAllData)
{
	if (waitForAllData)
	{
		return ::recv(socket, buffer, count, MSG_WAITALL);
	} else
	{
		fd_set set;
		set.fd_count = 1;
		set.fd_array[0] = socket;
		timeval time;
		time.tv_sec = 0;
		time.tv_usec = 0;
		if (::select(0, &set, NULL, NULL, &time) == 1)
			return ::recv(socket, buffer, count, 0);
		else return 0;
	}
}

int Socket::Write(char *buffer, int count)
{
	return ::send(socket, buffer, count, 0);
}

void Socket::Close()
{
	if (socket) closesocket(socket);
	socket = NULL;
}

Socket::~Socket(void)
{
}
#endif