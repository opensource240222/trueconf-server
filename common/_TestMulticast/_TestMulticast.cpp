// _TestMulticast.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

int _tmain(int argc, _TCHAR* argv[])
{
	WSAData data = {};
	WSAStartup(MAKEWORD(2,2), &data);
	SOCKET s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, 0, 0);
	unsigned char ttl(8); DWORD ret(0);
	WSAIoctl(s, SIO_MULTICAST_SCOPE , &ttl, sizeof(ttl), 0, 0, &ret, 0, 0);
	if (INVALID_SOCKET == s) return WSAGetLastError();
	addrinfo *addr(0), *addrto(0);
	if (getaddrinfo("0.0.0.0", "4000", 0, &addr)) return WSAGetLastError();
	if (getaddrinfo("224.0.0.1", "4000", 0, &addrto)) return WSAGetLastError();
	if (bind(s, addr->ai_addr, sizeof(sockaddr))) return WSAGetLastError();
	const char *snddata = "Hello World!";
	if (SOCKET_ERROR  == sendto(s, snddata, (int)(strlen(snddata) + 1), 0, addrto->ai_addr, sizeof(sockaddr))) return WSAGetLastError();
	return WSACleanup();
}

