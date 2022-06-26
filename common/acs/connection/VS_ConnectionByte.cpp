/**
 **************************************************************************
 * \file VS_ConnectionByte.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Byte stream oriented functions.
 *
 * For e.g. - for named pipe Byte regime.
 *
 * \b Project
 * \author SlavetskyA
 * \date 01.10.02
 *
 * $Revision: 1 $
 *
 * $History: VS_ConnectionByte.cpp $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/acs/connection
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connection
 *
 * *****************  Version 6  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/

#include "VS_ConnectionByte.h"
#include "VS_ConnectionTypes.h"
#include "std/cpplib/VS_SimpleStr.h"

VS_ConnectionByte::VS_ConnectionByte() : peerIp(NULL), peerPort(NULL), bindIp(NULL), bindPort(NULL) {
}

VS_ConnectionByte::VS_ConnectionByte(const VS_IPPortAddress &bindAddr, const VS_IPPortAddress &peerAddr)
		: peerIp(NULL), peerPort(NULL), bindIp(NULL), bindPort(NULL), bindAddr(bindAddr), peerAddr(peerAddr) {
	UpdatePeerAddrStr();
	UpdateBindAddrStr();
}

VS_ConnectionByte::VS_ConnectionByte(const VS_ConnectionByte &rhs) {
	bindIp = _strdup(rhs.bindIp);
	bindPort = _strdup(rhs.bindPort);
	peerIp = _strdup(rhs.peerIp);
	peerPort = _strdup(rhs.peerPort);

	bindAddr = rhs.bindAddr;
	peerAddr = rhs.peerAddr;
}

void VS_ConnectionByte::ResetPeerIpPort(void) {
	if (peerIp) { free((void *)peerIp);			peerIp = 0; }
	if (peerPort) { free((void *)peerPort);		peerPort = 0; }
	peerAddr = VS_IPPortAddress();
}

void VS_ConnectionByte::ResetBindIpPort(void) {
	if (bindIp) { free((void *)bindIp);			bindIp = 0; }
	if (bindPort) { free((void *)bindPort);		bindPort = 0; }
	bindAddr = VS_IPPortAddress();
}

void VS_ConnectionByte::UpdatePeerAddrStr(void) {
	if (peerIp) { free((void *)peerIp);			peerIp = 0; }
	if (peerPort) { free((void *)peerPort);		peerPort = 0; }

	char host[256];
	if (peerAddr.getAddressType() == VS_IPPortAddress::ADDR_IPV4) {
		VS_GetHostByIp(peerAddr.ipv4(), host, 256);
		peerIp = _strdup(host);
	} else if (peerAddr.getAddressType() == VS_IPPortAddress::ADDR_IPV6) {
		VS_GetHostByIpv6(peerAddr.ipv6(), host, 256);
		peerIp = _strdup(host);
	}
	_itoa(peerAddr.port(), host, 10);
	peerPort = _strdup(host);
}

void VS_ConnectionByte::UpdateBindAddrStr(void) {
	if (bindIp) { free((void *)bindIp);			bindIp = 0; }
	if (bindPort) { free((void *)bindPort);		bindPort = 0; }

	char host[256];
	if (bindAddr.getAddressType() == VS_IPPortAddress::ADDR_IPV4) {
		VS_GetHostByIp(bindAddr.ipv4(), host, 256);
		bindIp = _strdup(host);
	} else if (bindAddr.getAddressType() == VS_IPPortAddress::ADDR_IPV6) {
		VS_GetHostByIpv6(bindAddr.ipv6(), host, 256);
		bindIp = _strdup(host);
	}
	_itoa(bindAddr.port(), host, 10);
	bindPort = _strdup(host);
}

bool VS_ConnectionByte::Create( const char *pipeName, const VS_Pipe_Type pipeType )
{	return imp->Create( pipeName, vs_connection_type_stream, pipeType );	}
// end VS_ConnectionByte::Create

bool VS_ConnectionByte::Open( const char *pipeName, const VS_Pipe_Type pipeType )
{	return imp->Open( pipeName, vs_connection_type_stream, pipeType );	}
// end VS_ConnectionByte::Open

bool VS_ConnectionByte::Create( const VS_Pipe_Type type )
{	return imp->Create( vs_connection_type_stream, type );	}
// end VS_ConnectionByte::Create

bool VS_ConnectionByte::Open( VS_ConnectionByte *pipe, const VS_Pipe_Type type )
{	return imp->Open( pipe->imp, vs_connection_type_stream, type );	}
// end VS_ConnectionByte::Open

bool VS_ConnectionByte::RWrite( const VS_Buffer *buffers,
									const unsigned long n_buffers )
{	return imp->RWriteStream( buffers, n_buffers );		}
// end VS_ConnectionByte::RWrite

bool VS_ConnectionByte::Write( const void *buffer, const unsigned long n_bytes )
{	return imp->WriteStream( buffer, n_bytes );		}
// end VS_ConnectionByte::Write

int VS_ConnectionByte::GetWriteResult( unsigned long &milliseconds )
{	return imp->GetWriteStreamResult( milliseconds );	}
// end VS_ConnectionByte::GetWriteResult

int VS_ConnectionByte::SetWriteResult( const unsigned long b_trans,
											const struct VS_Overlapped *ov )
{	return imp->SetWriteStreamResult( b_trans, ov );	}
// end VS_ConnectionByte::SetWriteResult

bool VS_ConnectionByte::RRead( const unsigned long n_bytes )
{	return imp->RReadStream( n_bytes );		}
// end VS_ConnectionByte::RRead

bool VS_ConnectionByte::Read( void *buffer, const unsigned long n_bytes )
{	return imp->ReadStream( buffer, n_bytes );	}
// end VS_ConnectionByte::Read

int VS_ConnectionByte::GetReadResult( unsigned long &milliseconds,
										void **buffer, const bool portion )
{	return imp->GetReadStreamResult( milliseconds, buffer, portion );	}
// end VS_ConnectionByte::GetReadResult

int VS_ConnectionByte::SetReadResult( const unsigned long b_trans,
										const struct VS_Overlapped *ov,
										void **buffer, const bool portion )
{	return imp->SetReadStreamResult( b_trans, ov, buffer, portion );		}
// end VS_ConnectionByte::SetReadResult

const char* VS_ConnectionByte::GetBindIp() const {
	return bindIp;
}

const char* VS_ConnectionByte::GetBindPort() const {
	return bindPort;
}

const char* VS_ConnectionByte::GetPeerIp() const {
	return peerIp;
}

const char* VS_ConnectionByte::GetPeerPort() const {
	return peerPort;
}

const VS_IPPortAddress& VS_ConnectionByte::GetBindAddress() const {
	return peerAddr;
}

const VS_IPPortAddress& VS_ConnectionByte::GetPeerAddress() const {
	return peerAddr;
}