/**
 **************************************************************************
 * \file VS_ConnectionSock.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Win32 socket function declaration.
 *
 * The base of this functions is a microsoft realisation of
 * Berkly sockets. But nonblocked operations released
 * is msdn recomended - by overlapped Api.
 *
 * \b Project
 * \author SlavetskyA
 * \date 01.10.02
 *
 * $Revision: 3 $
 *
 * $History: VS_ConnectionSock.cpp $
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 16.01.10   Time: 18:57
 * Updated in $/VSNA/acs/connection
 * - SSL without EncryptLSP
 * - code for SSL throu EncryptLSP removed
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 9.02.09    Time: 19:43
 * Updated in $/VSNA/acs/connection
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/acs/connection
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connection
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 15.11.06   Time: 11:03
 * Updated in $/VS/acs/connection
 *  - asynchronous ClientSecureHandshake added to TransportRouter
 *
 * *****************  Version 21  *****************
 * User: Mushakov     Date: 27.04.06   Time: 13:10
 * Updated in $/VS/acs/connection
 * updated manage of certificate
 *
 * *****************  Version 20  *****************
 * User: Avlaskin     Date: 26.04.06   Time: 16:36
 * Updated in $/VS/acs/connection
 * added accept timing counter
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 18.04.06   Time: 19:26
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 13.01.06   Time: 16:42
 * Updated in $/VS/acs/connection
 * Added SecureLib, SecureHandshake
 *
 * *****************  Version 17  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/


#include "VS_ConnectionSock.h"
#include "VS_ConnectionTypes.h"

struct sockaddr   zero_addr = { AF_INET, { 0 }};
OVERLAPPED   over_zero = { 0 };
VS_Overlapped   ovZero = {{ 0 }, 0 };

VS_ConnectionSock::VS_ConnectionSock(bool useIPv6) :
	imp(new VS_ConnectionSock_Implementation)
{
	if(useIPv6) imp->SwitchToIPv6();
}
// end VS_ConnectionSock::VS_ConnectionSock

bool VS_ConnectionSock::IsIPv6() const
{
	return imp->GetSocketAddressFamily() == VS_IPPortAddress::ADDR_IPV6;
}

bool VS_ConnectionSock::IsIPv4() const
{
	return imp->GetSocketAddressFamily() == VS_IPPortAddress::ADDR_IPV4;
}

unsigned VS_ConnectionSock::GetConnectionAddressFamily() const
{
	return imp->GetConnectionAddressFamily();
}

unsigned VS_ConnectionSock::GetSocketAddressFamily() const
{
	return imp->GetSocketAddressFamily();
}

VS_ConnectionSock::~VS_ConnectionSock()
{	if (imp != NULL)	delete imp;		}
// end VS_ConnectionSock::~VS_ConnectionSock

bool VS_ConnectionSock::SetHeap( const void *handleHeap, const void *cSect )
{	return imp->SetHeap( handleHeap, cSect );	}
// end VS_ConnectionSock::SetHeap

bool VS_ConnectionSock::CreateOvWriteEvent( void )
{	return imp->CreateOvWriteEvent();	}
// end VS_ConnectionSock::CreateOvWriteEvent

bool VS_ConnectionSock::CreateOvReadEvent( void )
{	return imp->CreateOvReadEvent();	}
// end VS_ConnectionSock::CreateOvReadEvent

void *VS_ConnectionSock::OvWriteEvent( void ) {		return imp->writeEvent;		}
// end VS_ConnectionSock::OvWriteEvent

void *VS_ConnectionSock::OvReadEvent( void ) {		return imp->readEvent;		}
// end VS_ConnectionSock::OvReadEvent

void VS_ConnectionSock::CloseOvWriteEvent( void ) {		return imp->CloseOvWriteEvent();	}
// end VS_ConnectionSock::CloseOvWriteEvent

void VS_ConnectionSock::CloseOvReadEvent( void ) {		return imp->CloseOvReadEvent();		}
// end VS_ConnectionSock::CloseOvReadEvent

void VS_ConnectionSock::SetOvWriteFields( const VS_ACS_Field field1,
											const VS_ACS_Field field2,
											const VS_ACS_Field field3 )
{	imp->SetOvWriteFields( field1, field2, field3 );	}
// end VS_ConnectionSock::SetOvWriteFields
const VS_Overlapped *VS_ConnectionSock::WriteOv() const
{
	return imp->WriteOv();
}
const VS_Overlapped *VS_ConnectionSock::ReadOv() const
{
	return imp->ReadOv();
}

void VS_ConnectionSock::SetOvReadFields( const VS_ACS_Field field1,
											const VS_ACS_Field field2,
											const VS_ACS_Field field3 )
{	imp->SetOvReadFields( field1, field2, field3 );	}
// end VS_ConnectionSock::SetOvReadFields

void VS_ConnectionSock::SetOvFields( const VS_ACS_Field field1,
											const VS_ACS_Field field2,
											const VS_ACS_Field field3 )
{	imp->SetOvFields( field1, field2, field3 );	}
// end VS_ConnectionSock::SetOvFields

void VS_ConnectionSock::SetOvFildIOCP( const void *handleIOCP )
{	return imp->SetOvFildIOCP( handleIOCP );	}
// end VS_ConnectionSock::SetOvFildIOCP

void VS_ConnectionSock::SetIOHandler(VS_IOHandler *handler)
{
	imp->SetIOHandler(handler);
}
void VS_ConnectionSock::SetReadHandler(VS_IOHandler *handler)
{
	imp->SetReadHandler(handler);
}
void VS_ConnectionSock::SetWriteHandler(VS_IOHandler *handler)
{
	imp->SetWriteHandler(handler);
}

bool VS_ConnectionSock::SetIOCP( void *handleIOCP )
{	return imp->SetIOCP( handleIOCP );	}
// end VS_ConnectionSock::SetIOCP

bool VS_ConnectionSock::SetSizeBuffers( const int writeSize, const int readSize )
{	return imp->SetSizeBuffers( writeSize, readSize );	}
// end VS_ConnectionSock::SetSizeBuffers

bool VS_ConnectionSock::SetAsWriter( unsigned long &milliseconds )
{	return imp->SetAsWriter();	}
// end VS_ConnectionSock::SetAsWriter

bool VS_ConnectionSock::SetAsReader( unsigned long &milliseconds )
{	return imp->SetAsReader();	}
// end VS_ConnectionSock::SetAsReader

void *VS_ConnectionSock::GetHandle( void ) {	return imp->hio;	}
// end GetHandle

VS_Connection_Type VS_ConnectionSock::Type( void ) const {	return imp->type;	}
// end VS_ConnectionSock::Type

VS_Sock_State VS_ConnectionSock::State( void ) const {	return imp->state;	}
// end VS_ConnectionSock::State

VS_ConnectDirection	VS_ConnectionSock::GetConnectDirection( void ) const
{	return imp->connectType;	}
// end VS_ConnectionSock::ConnectType

const char* VS_ConnectionSock::GetBindIp() const { return imp->GetBindIp(); }
// end VS_ConnectionSock::GetBindIp

const char* VS_ConnectionSock::GetBindPort() const { return imp->GetBindPort(); }
// end VS_ConnectionSock::GetBindPort

const char* VS_ConnectionSock::GetPeerIp() const { return imp->GetPeerIp(); }
// end VS_ConnectionSock::GetPeerIp

const char* VS_ConnectionSock::GetPeerPort() const { return imp->GetPeerPort(); }
// end VS_ConnectionSock::GetPeerPort

char *VS_ConnectionSock::GetAcceptHost( void ) const {		return imp->GetAcceptHost();	}
// end VS_ConnectionSock::GetAcceptHost

char *VS_ConnectionSock::GetAcceptPort( void ) const {		return imp->GetAcceptPort();	}
// end VS_ConnectionSock::GetAcceptPort

char *VS_ConnectionSock::GetConnectHost( void ) const {		return imp->GetConnectHost();	}
// end VS_ConnectionSock::GetConnectHost

char *VS_ConnectionSock::GetConnectPort( void ) const {		return imp->GetConnectPort();	}
// end VS_ConnectionSock::GetConnectPort

const VS_IPPortAddress& VS_ConnectionSock::GetBindAddress() const
{
	return imp->GetBindAddress();
}

const VS_IPPortAddress& VS_ConnectionSock::GetPeerAddress() const
{
	return imp->GetPeerAddress();
}

bool VS_ConnectionSock::IsWrite( void ) const {		return imp->isWrite;	}
// end VS_ConnectionSock::IsWrite

bool VS_ConnectionSock::IsWriteOv( void ) const {	return imp->isWriteOv;	}
// end VS_ConnectionSock::IsWriteOv

bool VS_ConnectionSock::IsRead( void ) const {		return imp->isRead;		}
// end VS_ConnectionSock::IsRead

bool VS_ConnectionSock::IsReadOv( void ) const {	return imp->isReadOv;	}
// end VS_ConnectionSock::IsReadOv

bool VS_ConnectionSock::IsRW( void ) const {	return imp->isWrite || imp->isRead || imp->isAccept;		}
// end VS_ConnectionSock::IsRW

bool VS_ConnectionSock::IsOv( void ) const {	return imp->isWriteOv || imp->isReadOv;	}
// end VS_ConnectionSock::IsOv

int VS_ConnectionSock::Send( const void *buffer, const unsigned long n_bytes )
{	return imp->Send( buffer, n_bytes );	}
// end VS_ConnectionSock::Send

int VS_ConnectionSock::Receive( void *buffer, const unsigned long n_bytes )
{	return imp->Receive( buffer, n_bytes );		}
// end VS_ConnectionSock::Receive

int VS_ConnectionSock::SelectOut( unsigned long &milliseconds )
{	return imp->SelectOut( milliseconds );	}
// end VS_ConnectionSock::SelectOut

int VS_ConnectionSock::SelectIn( unsigned long &milliseconds )
{	return imp->SelectIn( milliseconds );	}
// end VS_ConnectionSock::SelectIn

void VS_ConnectionSock::Free( void *buffer ) {	imp->Free( buffer );	}
// end VS_ConnectionSock::Free

void VS_ConnectionSock::Disconnect( void ) {	imp->Disconnect();	}
// end VS_ConnectionSock::Disconnect

void VS_ConnectionSock::Terminate( void ) {	imp->Terminate();	}
// end VS_ConnectionSock::Terminate

void VS_ConnectionSock::Close( void ) {		imp->Close();	}
// end VS_ConnectionSock::Close

void VS_ConnectionSock::Break( void *handle )
{	VS_ConnectionSock_Implementation::Break( handle );	}
// end VS_ConnectionSock::Break

void VS_ConnectionSock::Cancel( void *handle )
{	VS_ConnectionSock_Implementation::Cancel( handle );	}
// end VS_ConnectionSock::Cancel

int VS_ConnectionSock::SelectIn( const void **handles, const unsigned n_handles,
										unsigned long &milliseconds )
{	return VS_ConnectionSock_Implementation::SelectIn( handles, n_handles, milliseconds );	}
// end VS_ConnectionSock::SelectIn
unsigned long VS_ConnectionSock::GetEventTime()
{
	return imp->GetEventTime();
}

bool VS_ConnectionSock::SetKeepAliveMode(bool isKeepAlive, unsigned long keepaliveTime, unsigned long keepalliveInterval)
{
	return true;
}
