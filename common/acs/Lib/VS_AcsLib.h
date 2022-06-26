
#ifndef VS_ACS_LIB_H
#define VS_ACS_LIB_H

#include "VS_AcsLibDefinitions.h"
#include "VS_IPPortAddress.h"
#include "../../net/Handshake_fwd.h"

struct in6_addr;

bool VS_AcsLibInitial( void );
bool VS_GetDefaultHostName( char *host_name, const unsigned long host_name_size );
bool VS_GetHostByBroadcast( const char *broadcastIp, char *ip,
											const unsigned long ip_size );
unsigned VS_NumberOfProcessors( void );

// ���������, �������� �� IPv6 ����� ������� (����� ::)?
bool VS_TestIPv6_IsZero(const in6_addr* ipv6);

// Resolve host name into ipv4 and ipv6 address.
// Works only with host names (not string representations of ip address)!
// If we have found an ipv4 address, has_ipv4 sets to true;
// If we have found an ipv6 address, has_ipv6 sets to true;
// If we found nothing, both has_ipv4 and has_ipv6 variables are false.
// You can pass zero in ip4 or ip6 variable to prevent searching that type of address.
// WARNING: ip4 and ip6 variables are in network-order representantion.
void VS_GetIPAddressesByHostName(const char* host, unsigned long* ip4, in6_addr* ip6, bool& has_ipv4, bool& has_ipv6);
// ����� ��� ��������� ip-������ �� ������, ��������� � ��������� ����.
// ���������� true, ���� �������������� �������.
// �������� isIPv4 ����������, ������� �� ���������� (ip4 ��� ip6) ������ �����.
// ���� isIPv4 == true, ����� �������� � ip4;
// ���� isIPv4 == false, ����� �������� � ip6.
// ���� �������������� �� �������, ��������� false, � ����� ������ ����������
// ip4, ip6, isIPv4 �� ����������.
// � ������ ���� host - �������� ���, � �������� ��� IPv4 � IPv6 ������,
// ������������ ����� ������ ���� ���������, ������� ������ � ��������� priority.
// ��������� �������� ��������� priority: AF_INET (��� IPv4) � AF_INET6 (��� IPv6).
// =======================
// �������������� ����������� ��������� �������:
//  1) ������������, ��� host �������� ���������� IPv4 �����, ������� ��� �������������;
//  2) ������������, ��� host �������� ���������� IPv6 �����, ������� ��� �������������;
//  3) ������������, ��� host �������� �������� ��� ����, ������� �������� ��� ����� � DNS ������.
// =======================
// ��������: �������� ip4 � ip6 ������������ � ������� �������� ����!
bool VS_GetIPAddressByHostName(const char* host, int priority, unsigned long* ip4, in6_addr* ip6, bool* isIPv4);

// Do the same as another VS_GetIPAddressByHostName(), but use VS_IPPortAddress object instead of raw ip addresses.
// Returns result of transformation. If result.getAddressFamily() == VS_IPPortAddress::ADDR_UNDEF, than
// transformation was failed.
VS_IPPortAddress VS_GetIPAddressByHostName(const char* host, int priority);

bool VS_GetIpByHost( const char *host, unsigned long *ip = 0 );
// ����������� ��������� ������������� ipv6 ������ � ��� ��������� �������������.
// ���������� false � ������, ���� ����� ����� �����������.
bool VS_GetIpv6ByHost( const char *host, in6_addr *ip = 0);
bool VS_GetIpByHostName( const char *host_name, unsigned long *ip = 0 );
bool VS_GetInetBroadcastAddr( const char *host, unsigned long *addr = 0 );
bool VS_GetInetBroadcastAddr( const char *host, char *broadcastIp,
											const unsigned long ip_size );
bool VS_GetNameOnAddr( const char *ip, char *host_name,
											const unsigned long host_name_size );
bool VS_GetHostByIp( const unsigned long ip, char *host = 0,
											const unsigned long host_size = 0 );
bool VS_GetHostByIpv6( const in6_addr ip, char *host = 0,
										    const unsigned long host_size = 0 );
std::string VS_GetHostByIp_str(const unsigned long ip);
std::string VS_GetHostByIpv6_str(const in6_addr ip);
bool VS_GetHostByName( const char *host_name, char *ip = 0,
											const unsigned long ip_size = 0 );
unsigned VS_GetHostsByName( const char *host_name, char **ip = 0,
											const unsigned long ip_number = 0,
											const unsigned long ip_size = 0 );

#define   VS_TEST_BIND_UDP   0
#define   VS_TEST_BIND_TCP   1
bool VS_BindPortIsFree( const char *host, const unsigned short port,
											unsigned test_type = VS_TEST_BIND_UDP );
bool VS_CheckTCPPortOnIp(const char *ip, unsigned short port);

void WriteTxtTrace( const char title[], const void *buffer, unsigned long size );

#define   VS_PIPE_SERVERS_DIRECTORY   "vs_servers\\"
extern const char   VS_Servers_PipeDir[];	// = VS_PIPE_SERVERS_DIRECTORY

#define   VS_ACS_LOC_HOST   "127.0.0.1"
extern const char   VS_LocHost[];	// = VS_ACS_LOC_HOST

// ip v4 address in network format
union VS_IP4_Addr {
	unsigned char _octet[4];
	unsigned long _addr;
};

// cidr strucrure for ip v4
struct VS_IP4_CIDR {
	union VS_IP4_Addr addr;
	unsigned char mask;
};

// rfc3330.txt : Special-Use IPv4 Addresses
extern const VS_IP4_CIDR VS_Reserved_IP4List[];

bool VS_ReadZeroHandshake( class VS_ConnectionSock *conn, net::HandshakeHeader **hs,
												unsigned long &milliseconds );
bool VS_WriteZeroHandshake( class VS_ConnectionSock *conn, const net::HandshakeHeader *hs,
												unsigned long &milliseconds );

unsigned long VS_ConvertStrToAddr(const char *addr, bool ntohl = false, bool ipv4 = true);
bool VS_CheckAddrInCIDR(unsigned long addr, VS_IP4_CIDR c, bool ntohl = false, bool ipv4 = true);
bool VS_CheckIsInternetAddress(unsigned long addr, bool ntohl = false, bool override_result = false, bool result = true, bool ipv4 = true);
bool VS_CheckIsInternetAddress(const char *addr, bool ntohl = false, bool override_result = false, bool result = true, bool ipv4 = true);

#endif // VS_ACS_LIB_H
