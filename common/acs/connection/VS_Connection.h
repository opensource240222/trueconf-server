
/**
 **************************************************************************
 * \file VS_Connections.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Here there are more sofisticated win32-overlapped functions.
 *
 *
 *
 * \b Project Overlapped Connections
 * \author SlavetskyA
 * \date 08.10.02
 *
 * $Revision: 1 $
 *
 * $History: VS_Connection.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/acs/connection
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connection
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 19.10.06   Time: 17:29
 * Updated in $/VS/acs/connection
 * IsSecureHandshakeInProgress
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 27.05.06   Time: 17:51
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 13  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/
#ifndef VS_CONNECTION_H
#define VS_CONNECTION_H

#include "../Lib/VS_Buffer.h"
#include "../Lib/VS_IPPortAddress.h"
#include <cstdint>

namespace boost
{
	template<class T> class shared_ptr;
};

class VS_WorkThread;
struct VS_TlsContext;

typedef   uint64_t   VS_ACS_Field;
#define   VS_ACS_INVALID_FIELD   ((VS_ACS_Field)~0)

class VS_IOHandler;

enum VS_Connection_Type { vs_connection_type_uninstalled,
							vs_connection_type_stream,
							vs_connection_type_dgram};

enum VS_ConnectDirection { vs_sock_type_not_connected,
							vs_sock_type_connect,
								vs_sock_type_accept };

class VS_Connection
{
public:
			VS_Connection( void ) {};
	virtual ~VS_Connection( void ) {};
	virtual bool	IsValid() const = 0;
	virtual bool	SetHeap( const void *handleHeap, const void *cSect = 0 ) = 0;
	virtual bool	CreateOvWriteEvent( void ) = 0;
	virtual bool	CreateOvReadEvent( void ) = 0;
	virtual void	*OvWriteEvent( void ) = 0;
	virtual void	*OvReadEvent( void ) = 0;

	virtual const struct VS_Overlapped *WriteOv() const = 0;
	virtual const struct VS_Overlapped *ReadOv() const = 0;

	virtual void	CloseOvWriteEvent( void ) = 0;
	virtual void	CloseOvReadEvent( void ) = 0;
	virtual void	SetOvWriteFields( const VS_ACS_Field field1 = VS_ACS_INVALID_FIELD,
										const VS_ACS_Field field2 = VS_ACS_INVALID_FIELD,
										const VS_ACS_Field field3 = VS_ACS_INVALID_FIELD ) = 0;
	virtual void	SetOvReadFields( const VS_ACS_Field field1 = VS_ACS_INVALID_FIELD,
										const VS_ACS_Field field2 = VS_ACS_INVALID_FIELD,
										const VS_ACS_Field field3 = VS_ACS_INVALID_FIELD ) = 0;
	virtual void	SetOvFields( const VS_ACS_Field field1 = VS_ACS_INVALID_FIELD,
										const VS_ACS_Field field2 = VS_ACS_INVALID_FIELD,
										const VS_ACS_Field field3 = VS_ACS_INVALID_FIELD ) = 0;

	virtual void	SetIOHandler(VS_IOHandler *handler)	= 0;
	virtual bool	SetIOThread(const boost::shared_ptr<VS_WorkThread> &io_thread);

	virtual void	SetReadHandler(VS_IOHandler *handler) = 0;
	virtual void	SetWriteHandler(VS_IOHandler *handler) = 0;
	virtual void	SetOvFildIOCP( const void *handleIOCP ) = 0;

	virtual bool	SetIOCP( void *handleIOCP ) = 0;
	virtual bool	SetSizeBuffers( const int writeSize, const int readSize ) = 0;
	virtual void	*GetHandle( void ) = 0;
	virtual VS_Connection_Type	Type( void ) const = 0;
	virtual bool	RWrite( const VS_Buffer *buffers, const unsigned long n_buffers ) = 0;
	virtual bool	Write( const void *buffer, const unsigned long n_bytes ) = 0;
	virtual bool	IsWrite( void ) const = 0;
	virtual bool	IsWriteOv( void ) const = 0;
	virtual int		GetWriteResult( unsigned long &milliseconds ) = 0;
	virtual int		SetWriteResult( const unsigned long b_trans,
										const struct VS_Overlapped *ov ) = 0;
	virtual bool	RRead( const unsigned long n_bytes ) = 0;
	virtual bool	Read( void *buffer, const unsigned long n_bytes ) = 0;
	virtual bool	IsRead( void ) const = 0;
	virtual bool	IsReadOv( void ) const = 0;
	virtual bool	IsRW( void ) const = 0;
	virtual bool	IsOv( void ) const = 0;
	virtual int		GetReadResult( unsigned long &milliseconds,
									void **buffer = 0, const bool portion = false ) = 0;
	virtual int		SetReadResult( const unsigned long b_trans,
									const struct VS_Overlapped *ov,
									void **buffer = 0, const bool portion = false ) = 0;
	virtual void	Disconnect( void ) = 0;
	virtual void	Close( void ) = 0;
	virtual void	Free( void *buffer ) = 0;
	virtual bool	IsSecureHandshakeInProgress(){return false;}

	virtual const char* GetBindIp() const = 0;
	virtual const char* GetBindPort() const = 0;
	virtual const char* GetPeerIp() const = 0;
	virtual const char* GetPeerPort() const = 0;
	virtual const VS_IPPortAddress& GetBindAddress() const = 0;
	virtual const VS_IPPortAddress& GetPeerAddress() const = 0;
	virtual bool SetKeepAliveMode(bool isKeepAlive, unsigned long keepaliveTime, unsigned long keepalliveInterval) = 0;
	virtual VS_ConnectDirection	GetConnectDirection( void ) const  = 0;
	virtual unsigned long GetEventTime()  = 0;

	virtual const VS_TlsContext* GetTlsContext() const = 0;
};
// end VS_Connection class

#endif // VS_CONNECTION_H
