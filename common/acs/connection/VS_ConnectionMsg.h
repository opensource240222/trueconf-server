/**
 **************************************************************************
 * \file VS_ConnectionMsg.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Message regime of communication operation.
 *
 *
 *
 * \b Project Message regime
 * \author SlavetskyA
 * \date 01.10.02
 *
 * $Revision: 1 $
 *
 * $History: VS_ConnectionMsg.h $
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
#ifndef VS_CONNECTION_MSG_H
#define VS_CONNECTION_MSG_H

#include "VS_ConnectionPipe.h"

class VS_ConnectionMsg : public VS_ConnectionPipe
{
public:
			VS_ConnectionMsg( void ) {};
	virtual ~VS_ConnectionMsg( void ) {};
	bool	Create( const char *pipeName, const VS_Pipe_Type pipeType );
	bool	Open( const char *pipeName, const VS_Pipe_Type pipeType );
	bool	Create( const VS_Pipe_Type pipeType );
	bool	Open( VS_ConnectionMsg *pipe, const VS_Pipe_Type pipeType );
	bool	RWrite( const VS_Buffer *buffers, const unsigned long n_buffers );
	bool	Write( const void *buffer, const unsigned long n_bytes );
	int		GetWriteResult( unsigned long &milliseconds );
	int		SetWriteResult( const unsigned long b_trans,
								const struct VS_Overlapped *ov );
	bool	RRead( const unsigned long n_bytes );
	bool	Read( void *buffer, const unsigned long n_bytes );
	int		GetReadResult( unsigned long &milliseconds,
							void **buffer = 0, const bool portion = false );
	int		SetReadResult( const unsigned long b_trans,
							const struct VS_Overlapped *ov,
							void **buffer = 0, const bool portion = false );
};
// end VS_ConnectionMsg class

#endif // VS_CONNECTION_MSG_H
