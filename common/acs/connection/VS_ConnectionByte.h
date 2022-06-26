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
 * $History: VS_ConnectionByte.h $
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

#ifndef VS_CONNECTION_BYTE_H
#define VS_CONNECTION_BYTE_H

#include "VS_ConnectionPipe.h"
#include "../Lib/VS_IPPortAddress.h"

class VS_ConnectionByte : public VS_ConnectionPipe
{
protected:
	VS_IPPortAddress bindAddr, peerAddr;
	char *peerIp, *peerPort, *bindIp, *bindPort;

	void ResetPeerIpPort(void);
	void ResetBindIpPort(void);
	void UpdatePeerAddrStr(void);
	void UpdateBindAddrStr(void);
public:
	VS_ConnectionByte();
	VS_ConnectionByte(const VS_IPPortAddress &bindAddr, const VS_IPPortAddress &peerAddr);
	VS_ConnectionByte(const VS_ConnectionByte &rhs);
	virtual ~VS_ConnectionByte(void) { ResetBindIpPort(); ResetPeerIpPort(); };
	bool	Create( const char *pipeName, const VS_Pipe_Type pipeType );
	bool	Open( const char *pipeName, const VS_Pipe_Type pipeType );
	bool	Create( const VS_Pipe_Type pipeType );
	bool	Open( VS_ConnectionByte *pipe, const VS_Pipe_Type pipeType );
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

	const char* GetBindIp() const override;
	const char* GetBindPort() const override;
	const char* GetPeerIp() const override;
	const char* GetPeerPort() const override;
	const VS_IPPortAddress& GetBindAddress() const;
	const VS_IPPortAddress& GetPeerAddress() const;
};
// end VS_ConnectionByte class

#endif // VS_CONNECTION_BYTE_H
