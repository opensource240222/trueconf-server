/**
 **************************************************************************
 * \file VS_ConnectionPipe.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Named & unNamed pipe`s functions realisation.
 *
 *
 *
 * \b Project
 * \author SlavetskyA
 * \date 01.10.02
 *
 * $Revision: 1 $
 *
 * $History: VS_ConnectionPipe.cpp $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/acs/connection
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connection
 *
 * *****************  Version 13  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/

#include "VS_ConnectionPipe.h"
#include "VS_ConnectionTypes.h"

const char   VS_PipeDir[] = VS_PIPE_DIRECTORY;

VS_ConnectionPipe::VS_ConnectionPipe() :
	imp(new VS_ConnectionPipe_Implementation)
{}
// end VS_ConnectionPipe::VS_ConnectionPipe

VS_ConnectionPipe::~VS_ConnectionPipe()
{	if (imp != NULL)	delete imp;		}
// end VS_ConnectionPipe::~VS_ConnectionPipe

bool VS_ConnectionPipe::SetHeap( const void *handleHeap, const void *cSect )
{	return imp->SetHeap( handleHeap, cSect );	}
// end VS_ConnectionPipe::SetHeap

bool VS_ConnectionPipe::CreateOvWriteEvent( void )
{	return imp->CreateOvWriteEvent();	}
// end VS_ConnectionPipe::CreateOvWriteEvent

bool VS_ConnectionPipe::CreateOvReadEvent( void )
{	return imp->CreateOvReadEvent();	}
// end VS_ConnectionPipe::CreateOvReadEvent

void *VS_ConnectionPipe::OvWriteEvent( void ) {		return imp->writeEvent;		}
// end VS_ConnectionPipe::OvWriteEvent

void *VS_ConnectionPipe::OvReadEvent( void ) {		return imp->readEvent;		}
// end VS_ConnectionPipe::SetOvReadEvent

void VS_ConnectionPipe::CloseOvWriteEvent( void ) {		return imp->CloseOvWriteEvent();	}
// end VS_ConnectionPipe::CloseOvWriteEvent

void VS_ConnectionPipe::CloseOvReadEvent( void ) {		return imp->CloseOvReadEvent();		}
// end VS_ConnectionPipe::CloseOvReadEvent

void VS_ConnectionPipe::SetOvWriteFields( const VS_ACS_Field field1,
											const VS_ACS_Field field2,
											const VS_ACS_Field field3 )
{	imp->SetOvWriteFields( field1, field2, field3 );	}
// end VS_ConnectionPipe::SetOvWriteFields

void VS_ConnectionPipe::SetOvReadFields( const VS_ACS_Field field1,
											const VS_ACS_Field field2,
											const VS_ACS_Field field3 )
{	imp->SetOvReadFields( field1, field2, field3 );	}
// end VS_ConnectionPipe::SetOvReadFields

void VS_ConnectionPipe::SetOvFields( const VS_ACS_Field field1,
											const VS_ACS_Field field2,
											const VS_ACS_Field field3 )
{	imp->SetOvFields( field1, field2, field3 );	}
// end VS_ConnectionPipe::SetOvFields
void VS_ConnectionPipe::SetIOHandler(VS_IOHandler *handler)
{
	imp->SetIOHandler(handler);
}
void VS_ConnectionPipe::SetReadHandler(VS_IOHandler *handler)
{
	imp->SetReadHandler(handler);
}
void VS_ConnectionPipe::SetWriteHandler(VS_IOHandler *handler)
{
	imp->SetWriteHandler(handler);
}
const VS_Overlapped *VS_ConnectionPipe::WriteOv() const
{
	return imp->WriteOv();
}
const VS_Overlapped *VS_ConnectionPipe::ReadOv() const
{
	return imp->ReadOv();
}
void VS_ConnectionPipe::SetOvFildIOCP( const void *handleIOCP )
{	return imp->SetOvFildIOCP( handleIOCP );	}
// end VS_ConnectionPipe::SetOvFildIOCP

bool VS_ConnectionPipe::SetIOCP( void *handleIOCP )
{	return imp->SetIOCP( handleIOCP );	}
// end VS_ConnectionPipe::SetIOCP

bool VS_ConnectionPipe::SetSizeBuffers( const int writeSize, const int readSize )
{	return imp->SetSizeBuffers( writeSize, readSize );	}
// end VS_ConnectionPipe::SetSizeBuffers

bool VS_ConnectionPipe::Create( const char *pipeName,
									const VS_Connection_Type type,
									const VS_Pipe_Type pipeType )
{	return imp->Create( pipeName, type, pipeType );		}
// end VS_ConnectionPipe::Create

bool VS_ConnectionPipe::Open( const char *pipeName,
								const VS_Connection_Type type,
								const VS_Pipe_Type pipeType )
{	return imp->Open( pipeName, type, pipeType );	}
// end VS_ConnectionPipe::Open

bool VS_ConnectionPipe::Create( const VS_Connection_Type type,
									const VS_Pipe_Type pipeType )
{	return imp->Create( type, pipeType );	}
// end VS_ConnectionPipe::Create

bool VS_ConnectionPipe::Open( VS_ConnectionPipe *pipe,
								const VS_Connection_Type type,
								const VS_Pipe_Type pipeType )
{	return imp->Open( pipe->imp, type, pipeType );	}
// end VS_ConnectionPipe::Open

bool VS_ConnectionPipe::Connect( void ) {	return imp->Connect();	}
// end VS_ConnectionPipe::Connect

int VS_ConnectionPipe::GetConnectResult( unsigned long &milliseconds )
{	return imp->GetConnectResult( milliseconds );	}
// end VS_ConnectionPipe::GetConnectResult

bool VS_ConnectionPipe::SetConnectResult( const unsigned long b_trans,
												const struct VS_Overlapped *ov )
{	return imp->SetConnectResult( b_trans, ov );	}
// end VS_ConnectionPipe::SetConnectResult

void *VS_ConnectionPipe::GetHandle( void ) {	return imp->hio;	}
// end VS_ConnectionPipe::GetHandle

VS_Connection_Type VS_ConnectionPipe::Type( void ) const {	return imp->type;	}
// end VS_ConnectionPipe::Type

VS_Pipe_State VS_ConnectionPipe::State( void ) const {	return imp->state;	}
// end VS_ConnectionPipe::State

char *VS_ConnectionPipe::Name( void ) {		return imp->pipeName;	}
// end VS_ConnectionPipe::Name

bool VS_ConnectionPipe::IsWrite( void ) const {		return imp->isWrite;	}
// end VS_ConnectionPipe::IsWrite

bool VS_ConnectionPipe::IsWriteOv( void ) const {	return imp->isWriteOv;	}
// end VS_ConnectionPipe::IsWriteOv

bool VS_ConnectionPipe::IsRead( void ) const {		return imp->isRead;		}
// end VS_ConnectionPipe::IsRead

bool VS_ConnectionPipe::IsReadOv( void ) const {	return imp->isReadOv;	}
// end VS_ConnectionPipe::IsReadOv

bool VS_ConnectionPipe::IsRW( void ) const {	return imp->isWrite || imp->isRead;		}
// end VS_ConnectionPipe::IsRW

bool VS_ConnectionPipe::IsOv( void ) const {	return imp->isWriteOv || imp->isReadOv;	}
// end VS_ConnectionPipe::IsOv

void VS_ConnectionPipe::Free( void *buffer ) {	imp->Free( buffer );	}
// end VS_ConnectionPipe::Free

void VS_ConnectionPipe::Disconnect( void ) {	imp->Disconnect();	}
// end VS_ConnectionPipe::Disconnect

void VS_ConnectionPipe::Close( void ) {		imp->Close();	}
// end VS_ConnectionPipe::Close

const char* VS_ConnectionPipe::GetBindIp() const { return "0.0.0.0"; }
const char* VS_ConnectionPipe::GetBindPort() const { return "0"; }
const char* VS_ConnectionPipe::GetPeerIp() const { return "0.0.0.0"; }
const char* VS_ConnectionPipe::GetPeerPort() const { return "0"; }

const VS_IPPortAddress& VS_ConnectionPipe::GetBindAddress() const { static VS_IPPortAddress addr; return addr; }
const VS_IPPortAddress& VS_ConnectionPipe::GetPeerAddress() const { static VS_IPPortAddress addr; return addr; }

bool VS_ConnectionPipe::SetKeepAliveMode(bool isKeepAlive, unsigned long, unsigned long ) {
	//always enable
	return isKeepAlive;
}

VS_ConnectDirection VS_ConnectionPipe::GetConnectDirection( void ) const {
	return vs_sock_type_accept;
}

unsigned long VS_ConnectionPipe::GetEventTime(){
	return GetTickCount();
}
