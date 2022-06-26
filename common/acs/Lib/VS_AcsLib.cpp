
//#include <stdio.h>
#include <errno.h>
#include <cstdio>

#include "../../std/cpplib/VS_MemoryLeak.h"

#include <Winsock2.h>
#include <WS2tcpip.h>

#include "VS_AcsLib.h"
#include "VS_AcsHttpLib.h"
#include "../VS_AcsDefinitions.h"
#include "acs/connection/VS_ConnectionTCP.h"
#include "VS_IPPortAddress.h"
#include "../../net/Handshake.h"
#include "../../net/Lib.h"
#include "std-generic/cpplib/hton.h"
#include "std-generic/cpplib/scope_exit.h"

bool VS_AcsLibInitial( void )
{
	static bool   isWindowsSocketStartUp = false;
	if (!isWindowsSocketStartUp)
	{
		if (!VS_BASE64_Encoding_Init() || !VS_BASE64_Decoding_Init())
			return false;
		WORD wVersionRequested = MAKEWORD( 1, 1 );
		WSADATA wsaData = { 0 };
		if ( WSAStartup( wVersionRequested, &wsaData ) != 0 )
			return false;
		isWindowsSocketStartUp = true;
	}
	return true;
}
// end VS_AcsLibInitial

//////////////////////////////////////////////////////////////////////////////////////////

unsigned VS_NumberOfProcessors( void )
{
	SYSTEM_INFO   info = { 0 };
	GetSystemInfo( &info );
	return (unsigned)info.dwNumberOfProcessors;
}
// end VS_NumberOfProcessors

bool VS_GetIpByHostName( const char *host_name, unsigned long *ip )
{
	struct hostent   far *lpHostEnt;
	char   far *szIPAddr;
	unsigned long   addr = 0;
	if (!host_name || !*host_name)	return false;
	if (!VS_GetIpByHost( host_name, &addr ))
	{
		char   char_m_host[32] = { 0 };
		lpHostEnt = gethostbyname( host_name );
		if (!lpHostEnt)		return false;
		int i = 0;
		for(i = 0; lpHostEnt->h_addr_list[i];i++)
		{
			unsigned char ch = lpHostEnt->h_addr_list[i][0];
			if(ch>1)
				break;
		}
		if(!lpHostEnt->h_addr_list[i])
			return false;
		szIPAddr = inet_ntoa( *((struct in_addr *)(lpHostEnt->h_addr_list[i])) );
		strncpy( char_m_host, szIPAddr, sizeof(char_m_host) );
		addr = inet_addr( char_m_host );
		if (addr == INADDR_NONE)	return false;
		addr = vs_ntohl(addr);
	}
	if (ip)		*ip = addr;
	return true;
}
// end VS_GetInetAddr

bool VS_TestIPv6_IsZero(const in6_addr* ipv6)
{
	const USHORT* part = ipv6->u.Word;
	return part[0] == 0 && part[1] == 0 && part[2] == 0 && part[3] == 0 &&
		part[4] == 0 && part[5] == 0 && part[6] == 0 && part[7] == 0;
}

void VS_GetIPAddressesByHostName(const char* host, unsigned long* ip4, in6_addr* ip6, bool& has_ipv4, bool& has_ipv6)
{
	has_ipv4 = false;
	has_ipv6 = false;

	addrinfo hints;
	addrinfo* result;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	if(getaddrinfo(host, NULL, &hints, &result)) return;
	addrinfo* res = result;

	for( ; result != NULL ; result = result->ai_next)
	{
		if(has_ipv4 && has_ipv6) break; // All done.

		sockaddr* adrs = result->ai_addr;
		if(adrs->sa_family ==  AF_INET && ip4 && !has_ipv4)
		{
			sockaddr_in* addr4 = (sockaddr_in*) adrs;
			*ip4 = addr4->sin_addr.S_un.S_addr;
			has_ipv4 = true;
		}
		else if(adrs->sa_family ==  AF_INET6 && ip6 && !has_ipv6)
		{
			sockaddr_in6* addr6 = (sockaddr_in6*) adrs;
			*ip6 = addr6->sin6_addr;
			has_ipv6 = true;
		}
	}
	freeaddrinfo(res);
}

bool VS_GetIPAddressByHostName(const char* host, int priority, unsigned long* ip4, in6_addr* ip6, bool* isIPv4)
{
	if (!host || !*host) return false;
	// 1) Предполагаем, что host содержит корректный IPv4 адрес, пробуем его преобразовать.
	unsigned long addr4;
	ZeroMemory(&addr4, sizeof(addr4));
	if(VS_GetIpByHost(host, &addr4))
	{
		// Преобразование удалось.
		if(ip4)
		{
			*ip4 = vs_htonl(addr4);
			*isIPv4 = true;
		}
		return true;
	}
	// 2) Предполагаем, что host содержит корректный IPv6 адрес, пробуем его преобразовать.
	in6_addr addr6;
	ZeroMemory(&addr6, sizeof(addr6));
	if(VS_GetIpv6ByHost(host, &addr6))
	{
		// Преобразование удалось.
		if(ip6)
		{
			*ip6 = addr6;
			*isIPv4 = false;
		}
		return true;
	}
	// 3) Предполагаем, что host содержит доменное имя узла, пробуем получить его адрес у DNS службы.
	addrinfo hints;
	addrinfo* result;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	if(getaddrinfo(host, NULL, &hints, &result)) return false;
	addrinfo* res = result;
	// Устанавливаем в true, как только нашли подходящий адрес.
	bool haveAddress = false;
	for( ; result != NULL ; result = result->ai_next)
	{
		sockaddr* adrs = result->ai_addr;
		if(adrs->sa_family ==  AF_INET)
		{
			// Если мы уже нашли адрес IPv6, который в приоритете, нет смысла просматривать IPv4.
			if(haveAddress && priority != AF_INET) continue;
			sockaddr_in* addr4 = (sockaddr_in*) adrs;
			haveAddress = true;
			if(ip4)
			{
				*ip4 = addr4->sin_addr.S_un.S_addr;
				*isIPv4 = true;
			} else break;
		}
		else if(adrs->sa_family ==  AF_INET6)
		{
			// Если мы уже нашли адрес IPv4, который в приоритете, нет смысла просматривать IPv6.
			if(haveAddress && priority != AF_INET6) continue;
			sockaddr_in6* addr6 = (sockaddr_in6*) adrs;
			haveAddress = true;
			if(ip6)
			{
				*ip6 = addr6->sin6_addr;
				*isIPv4 = false;
			} else break;
		}
	}
	freeaddrinfo(res);
	return haveAddress;
}

VS_IPPortAddress VS_GetIPAddressByHostName(const char* host, int priority)
{
	VS_IPPortAddress result;

	unsigned long raw_ipv4;
	in6_addr raw_ipv6;
	bool isipv4;

	if(VS_GetIPAddressByHostName(host, priority, &raw_ipv4, &raw_ipv6, &isipv4))
	{
		if(isipv4)
			result.ipv4_netorder(raw_ipv4);
		else
			result.ipv6(raw_ipv6);
	}

	return result;
}

bool VS_GetInetBroadcastAddr( const char *host, unsigned long *addr )
{
	unsigned long   loc_addr = 0;
	if (!VS_GetIpByHostName( host, &loc_addr ))		return false;
	if (IN_CLASSA(loc_addr))		loc_addr |= IN_CLASSA_HOST;
	else if (IN_CLASSB(loc_addr))	loc_addr |= IN_CLASSB_HOST;
	else if (IN_CLASSC(loc_addr))	loc_addr |= IN_CLASSC_HOST;
	else	return false;
	loc_addr = vs_htonl(loc_addr);
	if (addr)	*addr = loc_addr;
	return true;
}
// end VS_GetInetBroadcastAddr

bool VS_GetInetBroadcastAddr( const char *host, char *broadcastIp, const unsigned long ip_size )
{
	unsigned long   loc_addr = 0;
	if (!VS_GetInetBroadcastAddr( host, &loc_addr ))	return false;
	if (broadcastIp && ip_size)
	{	strncpy( broadcastIp, inet_ntoa( *(in_addr *)&loc_addr ), (size_t)ip_size );
		broadcastIp[ip_size - 1] = 0;	}
	return true;
}
// end VS_GetInetBroadcastAddr

bool VS_GetNameOnAddr( const char *ip, char *host_name,
							const unsigned long host_name_size )
{
	if (!ip || !*ip || !host_name || host_name_size <= 1)	return false;
	addrinfo   *info = 0;
	if (getaddrinfo( ip, 0, 0, &info ) || !info)	return false;
	if (!info->ai_canonname || !*info->ai_canonname)	return false;
	strncpy( host_name, info->ai_canonname, host_name_size - 1 );
	host_name[host_name_size - 1] = 0;
	return true;
}
// end VS_GetNameOnAddr

bool VS_GetIpByHost( const char host[], unsigned long *ip )
{
	const unsigned long   ret = inet_addr( host );
	if (ret == INADDR_NONE)		return false;
	if (ip)
		*ip = vs_ntohl(ret);
	return true;
}
// end VS_GetIpByHost

bool VS_GetIpv6ByHost(const char* host, in6_addr* ip6)
{
	in6_addr res_ip;
	int res = net::inet_pton(AF_INET6, host, &res_ip);
	if(ip6) *ip6 = res_ip;
	return res > 0;
}

bool VS_GetHostByIp( const unsigned long ip, char *host, const unsigned long host_size)
{
	in_addr addr;
	addr.S_un.S_addr = vs_htonl(ip);
	const char   *pch = inet_ntoa(addr);
	if (!pch)	return false;
	if (host && host_size)	strncpy(host, pch, (size_t)host_size);
	return true;
}

bool VS_GetHostByIpv6( const in6_addr ip, char *host, const unsigned long host_size)
{
	char buff[256];
	if (!net::inet_ntop(AF_INET6, &ip, buff, sizeof(buff) - 1))
		return 0;
	if (host && host_size)	strncpy(host, buff, (size_t)host_size);
	return true;
}

//for std::string using
std::string  VS_GetHostByIp_str(const unsigned long ip)
{
	in_addr addr;
	addr.S_un.S_addr = vs_htonl(ip);
	const char   *pch = inet_ntoa(addr);
	if (!pch)	return false;
	return pch;
}

std::string  VS_GetHostByIpv6_str(const in6_addr ip)
{
	char buff[256];
	if (!net::inet_ntop(AF_INET6, &ip, buff, sizeof(buff) - 1))
		return 0;
	return buff;
}

// end VS_GetHostByIp

bool VS_GetHostByName( const char *host_name, char *ip, const unsigned long ip_size )
{
	if (!host_name || !*host_name)	return false;
	char   *ipAddr = 0;
	unsigned long   S_addr = inet_addr( host_name );
	if (S_addr != INADDR_NONE)		ipAddr = (char *)host_name;
	else
	{
		struct hostent   *lpHostEnt = gethostbyname( host_name );
		if (!lpHostEnt)		return false;
		int i = 0;
		for(i = 0; lpHostEnt->h_addr_list[i];i++)
		{
			unsigned char ch = lpHostEnt->h_addr_list[i][0];
			if(ch>1)
				break;
		}
		if(!lpHostEnt->h_addr_list[i])
			return false;
		ipAddr = inet_ntoa( *(struct in_addr *)(lpHostEnt->h_addr_list[i]) );
		if (!ipAddr || !*ipAddr)	return false;
	}
	if (!ip)	return true;
	if (( strlen( ipAddr ) + 1 ) > ip_size)		return false;
	strcpy( ip, ipAddr );	return true;
}
// end VS_GetHostByName

bool VS_GetDefaultHostName( char *host_name, const unsigned long host_name_size )
{
	bool   ret = false;
	if (!host_name || !host_name_size)	return ret;
	char   *tmp_buf = (char *)malloc( 512 );
	VS_SCOPE_EXIT { free(tmp_buf); };
	if (!tmp_buf)	return ret;
	memset( tmp_buf, 0, 512 );
	if (gethostname( tmp_buf, 510 ) == SOCKET_ERROR)
		return ret;
	size_t   sz = strlen( tmp_buf );
	if (!sz || sz >= host_name_size)
		return ret;
	strcpy( host_name, tmp_buf );
	ret = true;
	return ret;
}
// end VS_GetDefaultHostName

bool VS_GetHostByBroadcast( const char *broadcastIp, char *ip, const unsigned long ip_size )
{
	unsigned long   my_addr = 0;
	if (!VS_GetInetBroadcastAddr( broadcastIp, &my_addr ))		return false;
	char   host_name[256] = { 0 };
	if (!VS_GetDefaultHostName( host_name, sizeof(host_name) ))		return false;
	char   host[6][32] = {{ 0 }}, *ips[6] = { host[0], host[1], host[2], host[3], host[4], host[5] };
	const unsigned   nIps = VS_GetHostsByName( host_name, ips, 6, 32 );
	if (!nIps)		return false;
	for (unsigned i = 0; i < nIps; ++i )
	{	unsigned long   addr = 0;
		if (VS_GetInetBroadcastAddr( ips[i], &addr ) && addr == my_addr)
		{	if (ip && ip_size) {	strncpy( ip, ips[i], (size_t)ip_size );
									ip[ip_size - 1] = 0;	}
			return true;
	}	}
	return false;
}
// end VS_GetHostByBroadcast

bool VS_BindPortIsFree( const char *host, const unsigned short port, unsigned test_type )
{
	VS_IPPortAddress addr;
	unsigned ips_count = 0;
	int net_interface = 5;
	char *ips_my[5];

	for(int i = 0; i < net_interface; i++)
		ips_my[i] = new char[46];

	if((!port) || !(ips_count = VS_GetHostsByName(host, ips_my, net_interface, 255)))
	{
		for(int i = 0; i < net_interface; i++)
			delete [] ips_my[i];
		return false;
	}

	bool res = false;
	for(unsigned i = 0; i < ips_count; i++)
	{
		addr.SetIPFromAddressString(ips_my[i]);

		if (addr.getAddressType() == VS_IPPortAddress::ADDR_IPV4) {
			// TODO_MIPv6 why do we need next lines? and what to do with ipv6 in that case?
			if ((addr.ipv4() != 0x7F000001) && //127.0.0.1
				(addr.ipv4() > 0x01000000))
			{
				res = true;
				break;
			}
		} else if (addr.getAddressType() == VS_IPPortAddress::ADDR_IPV6) {
			// for now accept any address
			res = true;
			break;
		}
	}

	for(int i = 0; i < net_interface; i++)
		delete [] ips_my[i];

	if(!res)
		return false;


	SOCKET   s = INVALID_SOCKET;
	// ipv4 and ipv6 are possible
	int af = addr.getAddressType() == VS_IPPortAddress::ADDR_IPV4 ? AF_INET : AF_INET6;
	switch (test_type)
	{
	case VS_TEST_BIND_UDP :		s = socket( af, SOCK_DGRAM, IPPROTO_UDP );		break;
	case VS_TEST_BIND_TCP :		s = socket( af, SOCK_STREAM, IPPROTO_TCP );		break;
	}
	if (s == INVALID_SOCKET)	return false;

	bool ret = false;
	if (addr.getAddressType() == VS_IPPortAddress::ADDR_IPV4) {
		sockaddr_in   addr_in;	ZeroMemory((void *)&addr_in, sizeof(addr_in));
		addr_in.sin_family = AF_INET;
		addr_in.sin_addr.S_un.S_addr = vs_htonl(addr.ipv4());
		addr_in.sin_port = vs_htons(port);
		ret = !bind(s, (sockaddr *)&addr_in, sizeof(addr_in));
		closesocket(s);
	} else if (addr.getAddressType() == VS_IPPortAddress::ADDR_IPV6) {
		sockaddr_in6   addr_in;	ZeroMemory((void *)&addr_in, sizeof(addr_in));
		addr_in.sin6_family = AF_INET6;
		addr_in.sin6_addr = addr.ipv6();
		addr_in.sin6_port = vs_htons(port);
		ret = !bind(s, (sockaddr *)&addr_in, sizeof(addr_in));
		closesocket(s);
	}
	return ret;
}
// end VS_BindPortIsFree

bool VS_CheckTCPPortOnIp(const char *ip, unsigned short port)
{
	if (!ip || !ip[0])
		return false;

	sockaddr_in sin;
	bool libAlreadyInUse = true;
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		if (WSAGetLastError() != WSANOTINITIALISED)
			return false;

		WSAData wsaData;
		if (WSAStartup(0x0101, &wsaData))
			return false;
		if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		{
			WSACleanup();
			return false;
		}

		libAlreadyInUse = false;
	}

	sin.sin_addr.s_addr = inet_addr(ip);
	if (sin.sin_addr.s_addr == INADDR_NONE)
	{
		closesocket(s);
		if (!libAlreadyInUse)
			WSACleanup();
		return false;
	}
	sin.sin_port = vs_htons(port);
	sin.sin_family = AF_INET;

	bool ret = bind(s, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) != SOCKET_ERROR;
	closesocket(s);
	if (!libAlreadyInUse)
		WSACleanup();

	return ret;
}

unsigned VS_GetHostsByName( const char *host_name, char **ip,
											const unsigned long ip_number,
											const unsigned long ip_size )
{
	if (!host_name || !*host_name)	return 0;
	char   *ipAddr = 0;
	unsigned long   S_addr = inet_addr( host_name );
	if (S_addr != INADDR_NONE)
	{
		if (!ip)	return 1;
		ipAddr = (char *)host_name;
		if (( strlen( ipAddr ) + 1 ) > ip_size)		return 0;
		strcpy( *ip, ipAddr );		return 1;
	}
	else
	{
		// Предполагаем, что у нас ipv6 адрес. Пытаемся его извлечь из строки.
		in6_addr addr6;
		if (net::inet_pton(AF_INET6, host_name, &addr6) > 0)
		{
			if (!ip)	return 1;
			ipAddr = (char *)host_name;
			if (( strlen( ipAddr ) + 1 ) > ip_size)		return 0;
			strcpy(*ip, ipAddr);
			return 1;
		}
		else
		{
			struct hostent   *lpHostEnt = gethostbyname( host_name );
			if (!lpHostEnt)		return 0;
			unsigned j(0);
			for (unsigned long i = 0; lpHostEnt->h_addr_list[i] && j < ip_number; ++i)
			{
				if( lpHostEnt->h_addr_list[i][0] == 0)
					continue;
				ipAddr = inet_ntoa( *(struct in_addr *)lpHostEnt->h_addr_list[i] );
				if (!ipAddr || !*ipAddr || ( strlen( ipAddr ) + 1 ) > ip_size)		continue;
				if (ip[j])	strcpy( ip[j], ipAddr );
				++j;
			}
			return j;
		}
}	}
// end VS_GetHostsByName

void WriteTxtTrace( const char title[], const void *buffer, unsigned long size )
{
	static FILE   *file = 0;
	if (!file)
	{
		char   name[MAX_PATH] = "";
		if (!GetTempFileName( "./", "VS_Trace_", 0, name ))		return;
		file = fopen( name, "w+" );
	}
	if (file)
	{
		fputs( "\n", file );	fputs( title, file );	fputs( "\n", file );
		fwrite( buffer, (size_t)size, 1, file );
		fflush( file );
}	}
// end Write

bool VS_ReadZeroHandshake( VS_ConnectionSock *conn, net::HandshakeHeader **hs,
													unsigned long &milliseconds )
{
	if (!conn || !hs || !conn->CreateOvReadEvent())		return false;
	net::HandshakeHeader head = { 0 };
	if (!conn->Read( (void *)&head, sizeof(head) ))				return false;
	if (conn->GetReadResult( milliseconds ) != sizeof(head))	return false;
	if (head.head_cksum != net::GetHandshakeHeaderChecksum(head))
		return false;

	unsigned long   size = sizeof(head) + head.body_length + 1;
	net::HandshakeHeader* h = (net::HandshakeHeader *)malloc( (size_t)size );
	if (!h)		return false;
	*h = head;		size -= sizeof(head);
	if (!conn->Read( (void *)&((char *)h)[sizeof(head)], size )
		|| conn->GetReadResult( milliseconds ) != size
		|| h->body_cksum != net::GetHandshakeBodyChecksum(*h))
	{	free( (void *)h );	return false;	}
	*hs = h;
	return true;
}
// end VS_ReadZeroHandshake

bool VS_WriteZeroHandshake( VS_ConnectionSock *conn, const net::HandshakeHeader *hs,
													unsigned long &milliseconds )
{
	if (!conn || !hs || !conn->CreateOvWriteEvent())	return false;
	const unsigned long size = sizeof(net::HandshakeHeader) + hs->body_length + 1;
	return conn->Write( (const void *)hs, size ) && conn->GetWriteResult( milliseconds ) == (int)size;
}
// end VS_WriteZeroHandshake

// rfc3330.txt : Special-Use IPv4 Addresses
const VS_IP4_CIDR VS_Reserved_IP4List[] =
{
	{ { 0,   0,   0,   0}, 8  }, // "This" Network [RFC1700, page 4]
	{ { 10,  0,   0,   0}, 8  }, // Private-Use Networks [RFC1918]
	{ { 14,  0,   0,   0}, 8  }, // Public-Data Networks [RFC1700, page 181] - IPv4 <-> X.121
	{ { 24,  0,   0,   0}, 8  }, // Cable Television Networks
	{ { 39,  0,   0,   0}, 8  }, // Reserved but subject to allocation [RFC1797]
	{ { 127, 0,   0,   0}, 8  }, // Loopback [RFC1700, page 5]
	{ { 128, 0,   0,   0}, 16 }, // Reserved but subject to allocation --
	{ { 169, 254, 0,   0}, 16 }, // Link Local --
	{ { 172, 16,  0,   0}, 12 }, // Private-Use Networks [RFC1918]
	{ { 191, 255, 0,   0}, 16 }, // Reserved but subject to allocation --
	{ { 192, 0,   0,   0}, 24 }, // Reserved but subject to allocation --
	{ { 192, 0,   2,   0}, 24 }, // Test-Net
	{ { 192, 88,  99,  0}, 24 }, // 6to4 Relay Anycast [RFC3068]
	{ { 192, 168, 0,   0}, 16 }, // Private-Use Networks [RFC1918]
	{ { 198, 18,  0,   0}, 15 }, // Network Interconnect, Device Benchmark Testing [RFC2544]
	{ { 223, 255, 255, 0}, 24 }, // Reserved but subject to allocation --
	{ { 224, 0,   0,   0}, 4  }, // Multicast [RFC3171]
	{ { 240, 0,   0,   0}, 4  }  // Reserved for Future Use [RFC1700, page 4]
};

// convert string in dotted format "x.x.x.x" to ipv4 addr in network or host byte order
unsigned long VS_ConvertStrToAddr(const char *addr, bool ntohl, bool ipv4) {
	if (!ipv4) return 0; // may be add ipv6 support later

	const size_t _len = strlen(addr);
	if (!_len) return 0;
	const unsigned char _dlm = '.'; // ip-address delimiter
	VS_IP4_Addr ip_addr = {};

	// first octet
	char *addr_1 = _strdup(addr);
	if (!addr_1) return 0;

	char *chr_1 = addr_1;
	char *chr_2 = strchr(chr_1, _dlm);
	if (chr_2) {
		*chr_2 = 0; ++chr_2;
		ip_addr._octet[0] = atoi(chr_1);
		chr_1 = chr_2;
	}
	if (!chr_1 || !*chr_1 || (errno == ERANGE)) { free (addr_1); return 0; }

	// second octet
	chr_2 = strchr(chr_1, _dlm);
	if (chr_2) {
		*chr_2 = 0; ++chr_2;
		ip_addr._octet[1] = atoi(chr_1);
		chr_1 = chr_2;
	}
	if (!chr_1 || !*chr_1 || (errno == ERANGE)) { free (addr_1); return 0; }

	// third octet
	chr_2 = strchr(chr_1, _dlm);
	if (chr_2) {
		*chr_2 = 0; ++chr_2;
		ip_addr._octet[2] = atoi(chr_1);
		chr_1 = chr_2;
	}
	if (!chr_1 || !*chr_1 || (errno == ERANGE)) { free (addr_1); return 0; }

	// forth octet
	ip_addr._octet[3] = atoi(chr_1);
	if (errno == ERANGE) { free (addr_1); return 0; }
	free(addr_1);

	if (ntohl)
		ip_addr._addr = vs_ntohl(ip_addr._addr);
	return ip_addr._addr;
};
// End VS_ConvertStrToAddr

bool VS_CheckAddrInCIDR(unsigned long addr, VS_IP4_CIDR c, bool ntohl, bool ipv4) {
	if (!ipv4) return false;
	unsigned long _bitmask = 0;
	for (int i = 0; i < c.mask; ++i) { _bitmask |= (1 << (31 - i)); }
	VS_IP4_Addr ip_min; ip_min._addr = vs_ntohl(c.addr._addr);
	VS_IP4_Addr ip_max; ip_max._addr = ~_bitmask;
	for (int i = 0; i < 4; ++i) {
		ip_max._octet[i] += ip_min._octet[i];
	}
	if (ntohl)
		addr = vs_ntohl(addr);
	return ((addr >= ip_min._addr) && (addr <= ip_max._addr));;
};
// End VS_CheckAddrInCIDR

bool VS_CheckIsInternetAddress(unsigned long addr, bool ntohl, bool override_result, bool result, bool ipv4) {
	if (override_result) return result;
	if (!ipv4) return false;
	const unsigned long _n_count = sizeof(VS_Reserved_IP4List) / sizeof(VS_IP4_CIDR);
	for (unsigned long i = 0; i < _n_count; ++i) {
		if (VS_CheckAddrInCIDR(addr, VS_Reserved_IP4List[i], ntohl)) return false;
	}
	return true;
};
// End VS_CheckIsInternetAddress

bool VS_CheckIsInternetAddress(const char *addr, bool ntohl, bool override_result, bool result, bool ipv4) {
	if (override_result) return result;
	if (!ipv4) return false;
	const unsigned long _n_count = sizeof(VS_Reserved_IP4List) / sizeof(VS_IP4_CIDR);
	const unsigned long _addr = VS_ConvertStrToAddr(addr, ntohl);
	for (unsigned long i = 0; i < _n_count; ++i) {
		if (VS_CheckAddrInCIDR(_addr, VS_Reserved_IP4List[i], !ntohl)) return false;
	}
	return true;
};
// End VS_CheckIsInternetAddress

const char   VS_Servers_PipeDir[] = VS_PIPE_SERVERS_DIRECTORY;
const char   VS_LocHost[] = VS_ACS_LOC_HOST;
