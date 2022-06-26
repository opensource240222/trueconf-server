/**
 **************************************************************************
 * \file VS_ConnectionMsg.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Message regime of communication operation realization.
 *
 *
 *
 * \b Project Message regime
 * \author SlavetskyA
 * \date 01.10.02
 *
 * $Revision: 1 $
 *
 * $History: VS_ConnectionMsg.cpp $
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

#include "VS_ConnectionMsg.h"
#include "VS_ConnectionTypes.h"

bool VS_ConnectionMsg::Create( const char *pipeName, const VS_Pipe_Type pipeType )
{	return imp->Create( pipeName, vs_connection_type_dgram, pipeType );		}
// end VS_ConnectionMsg::Create

bool VS_ConnectionMsg::Open( const char *pipeName, const VS_Pipe_Type pipeType )
{	return imp->Open( pipeName, vs_connection_type_dgram, pipeType );	}
// end VS_ConnectionMsg::Open

bool VS_ConnectionMsg::Create( const VS_Pipe_Type type )
{	return imp->Create( vs_connection_type_dgram, type );	}
// end VS_ConnectionMsg::Create

bool VS_ConnectionMsg::Open( VS_ConnectionMsg *pipe, const VS_Pipe_Type type )
{	return imp->Open( pipe->imp, vs_connection_type_dgram, type );	}
// end VS_ConnectionMsg::Open

bool VS_ConnectionMsg::RWrite( const VS_Buffer *buffers, const unsigned long n_buffers )
{	return imp->RWriteDgram( buffers, n_buffers );	}
// end VS_ConnectionMsg::RWrite

bool VS_ConnectionMsg::Write( const void *buffer, const unsigned long n_bytes )
{	return imp->WriteDgram( buffer, n_bytes );	}
// end VS_ConnectionMsg::Write

int VS_ConnectionMsg::GetWriteResult( unsigned long &milliseconds )
{	return imp->GetWriteDgramResult( milliseconds );	}
// end VS_ConnectionMsg::GetWriteResult

int VS_ConnectionMsg::SetWriteResult( const unsigned long b_trans,
											const struct VS_Overlapped *ov )
{	return imp->SetWriteDgramResult( b_trans, ov );	}
// end VS_ConnectionMsg::SetWriteResult

bool VS_ConnectionMsg::RRead( const unsigned long n_bytes )
{	return imp->RReadDgram( n_bytes );	}
// end VS_ConnectionMsg::RRead

bool VS_ConnectionMsg::Read( void *buffer, const unsigned long n_bytes )
{	return imp->ReadDgram( buffer, n_bytes );	}
// end VS_ConnectionMsg::Read

int VS_ConnectionMsg::GetReadResult( unsigned long &milliseconds,
										void **buffer, const bool portion )
{	return imp->GetReadDgramResult( milliseconds, buffer, portion );	}
// end VS_ConnectionMsg::GetReadResult

int VS_ConnectionMsg::SetReadResult( const unsigned long b_trans,
										const struct VS_Overlapped *ov,
										void **buffer, const bool portion )
{	return imp->SetReadDgramResult( b_trans, ov, buffer, portion );	}
// end VS_ConnectionMsg::SetReadResult
