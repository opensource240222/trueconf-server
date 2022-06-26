/**
 **************************************************************************
 * \file VS_ConnectionTCP.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Wrapper around Tcp connection.
 *
 * Realization see in VS_ConnectionTypes.h
 *
 * \b Project
 * \author SlavetskyA
 * \date 08.10.02
 *
 * $Revision: 3 $
 *
 * $History: VS_ConnectionTCP.cpp $
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 7.08.12    Time: 21:42
 * Updated in $/VSNA/acs/Connection
 * - static var isDisableNagle  removed
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 14.12.09   Time: 17:09
 * Updated in $/VSNA/acs/connection
 * - bugfix 6848
 * - bugfix 6849
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/acs/connection
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 6.04.07    Time: 18:06
 * Updated in $/VS2005/acs/connection
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 20.03.07   Time: 19:44
 * Updated in $/VS2005/acs/connection
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 7.02.07    Time: 12:56
 * Updated in $/VS2005/acs/connection
 * fixed warning
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:30
 * Updated in $/VS2005/acs/connection
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connection
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 13.10.06   Time: 10:47
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 21  *****************
 * User: Avlaskin     Date: 12.09.06   Time: 16:14
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 20  *****************
 * User: Avlaskin     Date: 22.06.06   Time: 14:12
 * Updated in $/VS/acs/connection
 * Bind method added to TCP
 *
 * *****************  Version 19  *****************
 * User: Avlaskin     Date: 9.06.06    Time: 17:38
 * Updated in $/VS/acs/connection
 * SIP add ons
 *
 * *****************  Version 18  *****************
 * User: Avlaskin     Date: 15.11.05   Time: 20:17
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 16  *****************
 * User: Avlaskin     Date: 18.01.05   Time: 16:24
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 15  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/


#include <stdio.h>
#include "VS_ConnectionTCP.h"
#include "VS_ConnectionTypes.h"
#include "../Lib/VS_AcsHttpLib.h"
#include "std-generic/cpplib/hton.h"
//#include "../../std/cpplib/VS_MemoryLeak.h"


bool VS_ConnectionTCP::Socket( void )
{
	return imp->Socket( vs_connection_type_stream ); ///&& SetKeepAliveMode(true);
}
// end of VS_ConnectionTCP::Socket

bool VS_ConnectionTCP::Bind(const char *host, const unsigned short port, const bool exclusiveUseAddr)
{
	return imp->Bind(host, port, vs_connection_type_stream, exclusiveUseAddr);
}
// end VS_ConnectionTCP::Bind

bool VS_ConnectionTCP::Bind( const unsigned long ip, const unsigned short port, const bool exclusiveUseAddr )
{
	return imp->Bind(ip, port, vs_connection_type_stream, exclusiveUseAddr);
}
// end VS_ConnectionTCP::Bind

bool VS_ConnectionTCP::Bind( const in6_addr ip, const unsigned short port, const bool exclusiveUseAddr )
{
	return imp->BindV6(ip, port, vs_connection_type_stream, exclusiveUseAddr);
}
// end VS_ConnectionTCP::Bind

bool VS_ConnectionTCP::Bind( const VS_IPPortAddress& ipport, const bool exclusiveUseAddr )
{
	switch(ipport.getAddressType())
	{
	case VS_IPPortAddress::ADDR_IPV4:
		return imp->Bind(ipport.ipv4(), ipport.port(), vs_connection_type_stream, exclusiveUseAddr);
	case VS_IPPortAddress::ADDR_IPV6:
		return imp->BindV6(ipport.ipv6(), ipport.port(), vs_connection_type_stream, exclusiveUseAddr);
	default:
		return false;
	}
}
// end VS_ConnectionTCP::Bind

bool VS_ConnectionTCP::Listen( const char *host, const unsigned short port, const bool exclusiveUseAddr,
								const int backlog, const bool waitAccept )
{
	return Bind(host, port, exclusiveUseAddr) && imp->Listen(backlog, waitAccept);
}
// end of VS_ConnectionTCP::Listen

bool VS_ConnectionTCP::Listen( const unsigned long ip, const unsigned short port, const bool exclusiveUseAddr,
								const int backlog, const bool waitAccept )
{
	return Bind(ip, port, exclusiveUseAddr) && imp->Listen(backlog, waitAccept);
}
// end of VS_ConnectionTCP::Listen

bool VS_ConnectionTCP::Listen( const in6_addr ip, const unsigned short port, const bool exclusiveUseAddr,
								const int backlog, const bool waitAccept )
{
	return Bind(ip, port, exclusiveUseAddr) && imp->Listen(backlog, waitAccept);
}
// end of VS_ConnectionTCP::Listen

bool VS_ConnectionTCP::Listen( const VS_IPPortAddress& ipport, const bool exclusiveUseAddr,
								const int backlog, const bool waitAccept )
{
	return Bind(ipport, exclusiveUseAddr) && imp->Listen(backlog, waitAccept);
}
// end of VS_ConnectionTCP::Listen

bool VS_ConnectionTCP::Listen(const unsigned short port, const bool exclusiveUseAddr, const int backlog, const bool waitAccept)
{
	return imp->Listen(port, backlog, waitAccept, exclusiveUseAddr);
}

bool VS_ConnectionTCP::ListenInRange( const char *host,
							const unsigned short min_port, const unsigned short max_port,
							const bool exclusiveUseAddr,
							unsigned short *fixed_port,
							const int backlog, const bool waitAccept )
{
	if (!host || !*host || !min_port || !max_port)	return false;
	unsigned short   port = min_port, port_end = max_port;
	if (port > port_end) {		port = max_port;	port_end = min_port;	}
	for (; port <= port_end; ++port)
	{
		if (imp->Bind(host, port, vs_connection_type_stream, exclusiveUseAddr) && imp->Listen(backlog, waitAccept))
		{	if (fixed_port)		*fixed_port = port;
			return true;
	}	}
	return false;
}
// end of VS_ConnectionTCP::ListenInRange
bool VS_ConnectionTCP::ConnectAsynch(const unsigned long ip, const unsigned short port,void *& event	)
{
	return imp->ConnectAsynch( ip, port, vs_connection_type_stream, event );
}
// end of VS_ConnectionTCP::ConnectAsynch
bool VS_ConnectionTCP::ConnectAsynchV6(in6_addr ip6, const unsigned short port, void *& event	)
{
	return imp->ConnectAsynchV6( ip6, port, vs_connection_type_stream, event );
}
// end of VS_ConnectionTCP::ConnectAsynch
bool VS_ConnectionTCP::ConnectAsynch(const char* host, const unsigned short port, void *& event)
{
	return imp->ConnectAsynch(host, port, vs_connection_type_stream, event);
}
// end of VS_ConnectionTCP::ConnectAsynchV6

bool VS_ConnectionTCP::ConnectAsynch(const VS_IPPortAddress& ipport, void *& event)
{
	switch(ipport.getAddressType())
	{
	case VS_IPPortAddress::ADDR_IPV4:
		return imp->ConnectAsynch( ipport.ipv4(), ipport.port(), vs_connection_type_stream, event);
	case VS_IPPortAddress::ADDR_IPV6:
		return imp->ConnectAsynchV6( ipport.ipv6(), ipport.port(), vs_connection_type_stream, event);
	default:
		return false;
	}
}

bool VS_ConnectionTCP::GetConnectResult(unsigned long mills ,bool noWait ,bool isFastSocket, bool qos, _QualityOfService * qos_params )
{
	return imp->GetConnectResult(mills,noWait,isFastSocket, qos, qos_params);
}
// end of VS_ConnectionTCP::GetConnectResult
bool VS_ConnectionTCP::Connect( const char *host, const unsigned short port,
									unsigned long &mills,
									bool isFastSocket , const bool qos, _QualityOfService *qos_params)
{	return imp->Connect( host, port, vs_connection_type_stream, mills , isFastSocket , qos , qos_params );		}
// end of VS_ConnectionTCP::Connect

bool VS_ConnectionTCP::Connect( const unsigned long ip, const unsigned short port,
									unsigned long &mills,
									bool isFastSocket , const bool qos, _QualityOfService * qos_params)
{	return imp->Connect( ip, port, vs_connection_type_stream, mills, isFastSocket , qos , qos_params);		}
// end of VS_ConnectionTCP::Connect

bool VS_ConnectionTCP::ConnectV6( const in6_addr ip, const unsigned short port,
									unsigned long &mills,
									bool isFastSocket , const bool qos, _QualityOfService * qos_params)
{	return imp->ConnectV6( ip, port, vs_connection_type_stream, mills, isFastSocket , qos , qos_params);		}
// end of VS_ConnectionTCP::Connect

bool VS_ConnectionTCP::Connect( const VS_IPPortAddress& ipport, unsigned long &milliseconds,
							bool isFastSocket, const bool isQoSSocket , _QualityOfService * QoSdata)
{
	switch(ipport.getAddressType())
	{
	case VS_IPPortAddress::ADDR_IPV4:
		return imp->Connect( ipport.ipv4(), ipport.port(), vs_connection_type_stream, milliseconds, isFastSocket , isQoSSocket , QoSdata);
	case VS_IPPortAddress::ADDR_IPV6:
		return imp->ConnectV6( ipport.ipv6(), ipport.port(), vs_connection_type_stream, milliseconds, isFastSocket , isQoSSocket
			, QoSdata);
	default:
		return false;
	}
}

bool VS_ConnectionTCP::PassSocks4( const char *host, const unsigned short port,
										unsigned long &mills )
{
	if (!host || !*host || !port)
		return false;
	unsigned char cmd[9] = { 4, 0 };
	cmd[1] = 1;
	*reinterpret_cast<unsigned short *>(&cmd[2]) = vs_htons(port);
	unsigned long ip = 0;
	if (!VS_GetIpByHostName( host, &ip ))
		return false;
	*reinterpret_cast<unsigned long *>(&cmd[4]) = vs_htonl(ip);
	return imp->Send(cmd, 9, mills, true) == 9 &&
	       imp->Receive(cmd, 8, mills) == 8 &&
	       cmd[1] == 90;
}
// end of VS_ConnectionTCP::PassSocks4

unsigned VS_ConnectionTCP::PassSocks5( const char *host, const unsigned short port,
								const char *user, const char *passwd, unsigned long &mills )
{
	if (!host || !*host || !port)
		return 0;
	const char *usr = !user ? "" : user,
	           *psw = !passwd ? "" : passwd;
	const bool flLgn = user && *user;
	unsigned char   cmd[513] { 5, 1, static_cast<unsigned char>(flLgn ? 2 : 0), 0 };
	unsigned long   ind;

	if (imp->Send(cmd, 3, mills, true) != 3 ||
		imp->Receive(cmd, 2, mills) != 2)
		return 0;
	if (cmd[1])
	{
		if (cmd[1] != 2)
			return cmd[0] != 5 ? 0 : 2;
		const size_t usr_len = strlen( usr ),
		             psw_len = strlen( psw );
		if (usr_len > 255 || psw_len > 255)
			return 0;
		cmd[0] = 1;
		cmd[1] = static_cast<unsigned char>(usr_len);
		memcpy(&cmd[2], usr, usr_len );
		ind = 2 + usr_len;
		cmd[ind] = static_cast<unsigned char>(psw_len);
		++ind;
		memcpy(&cmd[ind], psw, psw_len );
		ind += psw_len;
		if (imp->Send(cmd, ind, mills, true ) != static_cast<int>(ind) ||
			imp->Receive(cmd, 2, mills ) != 2)
			return 0;
		if (cmd[1])
			return cmd[1] != 1 ? 0 : 2;
	}
	const size_t host_len = strlen( host );
	if (host_len > 255)
		return 0;
	cmd[0] = 5;
	cmd[1] = 1;
	cmd[2] = 0;
	cmd[3] = 3;
	cmd[4] = static_cast<unsigned char>(host_len);
	memcpy(&cmd[5], host, host_len );
	ind = 5 + host_len;
	*reinterpret_cast<unsigned short *>(&cmd[ind]) = vs_htons(port);
	ind += 2;
	if (imp->Send(cmd, ind, mills, true ) != static_cast<int>(ind) ||
		imp->Receive(cmd, 10, mills ) != 10 || cmd[1])
		return 0;
	return 1;
}
// end of VS_ConnectionTCP::PassSocks5

unsigned VS_ConnectionTCP::PassHttpTnl( const char *host, const unsigned short port,
								const char *user, const char *passwd, unsigned long &mills )
{
	if (!host || !*host || !port)	return 0;
	const bool   flLgn = user && *user;
	const unsigned long   host_len = (unsigned long)strlen( host );
	char   host_port[128], usr_psw[128] = { 0 };
	if (host_len > ( sizeof(host_port) - 6 ))	return 0;
	sprintf( host_port, "%s:%u", host, port );
	const char   *start_line[] = { "CONNECT", host_port, "HTTP/1.0", 0 },
					*header[4] = { 0 }, **headers[2] = { 0 }, ***header_line = 0;
	if (flLgn) {	char   src_usr_psw[128];	sprintf( src_usr_psw, "%s:%s", user, passwd ? passwd : "" );
					unsigned long   sz = VS_BASE64_Encode( (unsigned char *)src_usr_psw, (unsigned long)strlen( src_usr_psw ), (unsigned char *)usr_psw );
					header[0] = "Proxy-authorization: Basic";	header[1] = usr_psw;
					headers[0] = header;	header_line = headers;		}
	VS_HttpMessage   wrMs;		memset( (void *)&wrMs, 0, sizeof(wrMs) );
	char   *buf = (char *)malloc( 8192 );		if (!buf)	return 0;
	memset( (void *)buf, 0, 8192 );		wrMs.buf = buf;		wrMs.size_buf = 8192;
	wrMs.size_mes = VS_SizeHeader( start_line, header_line );
	unsigned   ret = 0;			if (!wrMs.size_mes)		goto go_ret;
	wrMs.size_mes = VS_GenerateHeader( wrMs.buf, start_line, header_line );
	if (imp->Send( (void *)wrMs.buf, wrMs.size_mes, mills, true ) != (int)wrMs.size_mes)	goto go_ret;
	wrMs.index = wrMs.size_mes = 0;
	if (VS_ReadHttpHead( wrMs, imp, mills ) && !_strnicmp( buf, "HTTP/", 5 ))
	{	if (!strncmp( &buf[8], " 200", 4 ))			ret = 1;
		else if (!strncmp( &buf[8], " 407", 4 ))	ret = 2;	}
go_ret:		free( (void *)buf );
	return ret;
}
// end of VS_ConnectionTCP::PassHttpTnl

bool VS_ConnectionTCP::Accept( const char *host, const unsigned short port, unsigned long &mills,
								const bool exclusiveUseAddr, bool isFastSocket, bool qos, _QualityOfService * qos_params)
{
	return imp->Accept(host, port, vs_connection_type_stream, mills, exclusiveUseAddr, isFastSocket, qos, qos_params);
}
// end of VS_ConnectionTCP::Accept

int VS_ConnectionTCP::Accept( VS_ConnectionTCP *listener, unsigned long &mills,
								 bool isFastSocket, bool qos, _QualityOfService * qos_params)
{
	return imp->Accept( listener->imp, mills , isFastSocket, qos, qos_params );
}
// end of VS_ConnectionTCP::Accept

bool VS_ConnectionTCP::Accept( VS_ConnectionTCP *listener,
								 bool isFastSocket, bool qos, _QualityOfService * qos_params)
{	return imp->Accept( listener->imp , ( bool )isFastSocket , qos, qos_params);	}
// end of VS_ConnectionTCP::Accept

bool VS_ConnectionTCP::SetAcceptResult( const unsigned long b_trans,
									   const struct VS_Overlapped *ov,
									   VS_ConnectionTCP *listener,
										bool isFastSocket, bool qos, _QualityOfService * qos_params )
{	return imp->SetAcceptResult( b_trans, ov, !listener ? 0 : listener->imp , isFastSocket , qos , qos_params);	}
// end of VS_ConnectionTCP::SetAcceptResult

bool VS_ConnectionTCP::IsAccept( void ) const {		return imp->isAccept;	}
// end of VS_ConnectionTCP::IsAccept

bool VS_ConnectionTCP::RWrite( const VS_Buffer *buffers, const unsigned long n_buffers )
{	return imp->RWriteStream( buffers, n_buffers );		}
// end of VS_ConnectionTCP::RWrite

bool VS_ConnectionTCP::Write( const void *buffer, const unsigned long n_bytes )
{	return imp->WriteStream( buffer, n_bytes );		}
// end of VS_ConnectionTCP::Write

int VS_ConnectionTCP::GetWriteResult( unsigned long &mills )
{	return imp->GetWriteStreamResult( mills );	}
// end of VS_ConnectionTCP::GetWriteResult

int VS_ConnectionTCP::SetWriteResult( const unsigned long b_trans, const struct VS_Overlapped *ov )
{	return imp->SetWriteStreamResult( b_trans, ov );	}
// end of VS_ConnectionTCP::SetWriteResult

bool VS_ConnectionTCP::RRead( const unsigned long n_bytes )
{	return imp->RReadStream( n_bytes );		}
// end of VS_ConnectionTCP::RRead

bool VS_ConnectionTCP::Read( void *buffer, const unsigned long n_bytes )
{	return imp->ReadStream( buffer, n_bytes );	}
// end of VS_ConnectionTCP::Read

int VS_ConnectionTCP::GetReadResult( unsigned long &mills, void **buffer, const bool portion )
{	return imp->GetReadStreamResult( mills, buffer, portion );	}
// end of VS_ConnectionTCP::GetReadResult

int VS_ConnectionTCP::SetReadResult( const unsigned long b_trans, const struct VS_Overlapped *ov, void **buffer, const bool portion )
{	return imp->SetReadStreamResult( b_trans, ov, buffer, portion );	}
// end of VS_ConnectionTCP::SetReadResult

int VS_ConnectionTCP::Send( const void *buffer, const unsigned long n_bytes, unsigned long &mills, const bool keep_blocked )
{	return imp->Send( buffer, n_bytes, mills, keep_blocked );	}
// end of VS_ConnectionTCP::Send

int VS_ConnectionTCP::Send(const void *buffer, const unsigned long n_bytes)
{   return imp->Send(buffer, n_bytes);  }
// end of VS_ConnectionTCP::Send

int VS_ConnectionTCP::Receive( void *buffer, const unsigned long n_bytes, unsigned long &mills, bool portion)
{	return imp->Receive( buffer, n_bytes, mills, portion);	}

int VS_ConnectionTCP::Receive( void *buffer, const unsigned long n_bytes)
{	return imp->Receive( buffer, n_bytes);	}
// end of VS_ConnectionTCP::Receive

bool VS_ConnectionTCP::SetMulticastTTL(unsigned long ttl)
{	return false;	}
// end of VS_ConnectionTCP::SetMulticastTTL

bool VS_ConnectionTCP::SetQOSSocket(_QualityOfService *qos_params)
{	return imp->SetQOSSocket(qos_params);	}
// end of VS_ConnectionTCP::SetQOSSocket
void VS_ConnectionTCP::SetFastSocket(bool isFast)
{
	imp->SetFastSocket(isFast);
}
bool VS_ConnectionTCP::SetKeepAliveMode(bool isKeepAlive, unsigned long keepaliveTime, unsigned long keepalliveInterval)
{
	return imp->SetKeepAliveMode(isKeepAlive,keepaliveTime,keepalliveInterval);
}

void VS_ConnectionTCP::SetQoSFlow(const net::QoSFlowSharedPtr &flow)
{
	imp->SetQoSFlow(flow);
}

net::QoSFlowSharedPtr VS_ConnectionTCP::GetQoSFlow(void)
{
	return imp->GetQoSFlow();
}

bool VS_ConnectionTCP::AddExtraFlow(const net::QoSFlowSharedPtr &flow, const VS_IPPortAddress &addr)
{
	if (GetQoSFlow() != nullptr)
	{
		return false;
	}

	SetQoSFlow(flow);
	return true;
}

bool VS_ConnectionTCP::RemoveExtraFlow(const net::QoSFlowSharedPtr &flow)
{
	if (GetQoSFlow() != flow)
	{
		return false;
	}

	net::QoSFlowSharedPtr ptr = nullptr;
	SetQoSFlow(ptr);
	return true;
}

// For TLS mostly
VS_Overlapped &VS_ConnectionTCP::GetReadOv()
{
	return imp->readOv;
}

VS_Overlapped &VS_ConnectionTCP::GetWriteOv()
{
	return imp->writeOv;
}

void VS_ConnectionTCP::Close(void)
{
	VS_ConnectionSock::Close();
}
