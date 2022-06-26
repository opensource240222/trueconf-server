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
 * $History: VS_ConnectionSock.h $
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
 * User: Mushakov     Date: 27.04.06   Time: 13:10
 * Updated in $/VS/acs/connection
 * updated manage of certificate
 *
 * *****************  Version 21  *****************
 * User: Avlaskin     Date: 26.04.06   Time: 16:36
 * Updated in $/VS/acs/connection
 * added accept timing counter
 *
 * *****************  Version 20  *****************
 * User: Mushakov     Date: 18.04.06   Time: 19:26
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 13.01.06   Time: 16:42
 * Updated in $/VS/acs/connection
 * Added SecureLib, SecureHandshake
 *
 * *****************  Version 18  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/
#ifndef VS_CONNECTION_SOCK_H
#define VS_CONNECTION_SOCK_H

#include "../Lib/VS_IPPortAddress.h"

#include "VS_Connection.h"

enum VS_Sock_State { vs_sock_state_not_created,
								vs_sock_state_created,
								vs_sock_state_bind,
								vs_sock_state_listen,
								vs_sock_state_connected };

class VS_ConnectionSock : public VS_Connection
{
public:
			VS_ConnectionSock( bool useIPv6 = false );
	// returns true, if this contains ipv6 socket
	bool	IsIPv6() const;
	// returns true, if this contains ipv4 socket
	bool	IsIPv4() const;
	// returns address family of the current connection
	// if connection exists, returns VS_IPPortAddress::ADDR_IPV4 or VS_IPPortAddress::ADDR_IPV6
	// else returns VS_IPPortAddress::ADDR_UNDEF
	unsigned GetConnectionAddressFamily() const;
	// returns address family of the socket that will be created by default
	// this is only recomüendation
	// real address family can be taken by GetConnectionAddressType() when
	// the connection will exists (bind ip or peer ip will be established).
	unsigned GetSocketAddressFamily() const;
	virtual ~VS_ConnectionSock( void );
	bool	IsValid( void ) const { return &imp != 0; }
	bool	SetHeap( const void *handleHeap, const void *cSect = 0 );
	bool	CreateOvWriteEvent( void );
	bool	CreateOvReadEvent( void );
	void	*OvWriteEvent( void );
	void	*OvReadEvent( void );
	const	struct VS_Overlapped *WriteOv() const override;
	const	struct VS_Overlapped *ReadOv() const override;
	void	CloseOvWriteEvent( void );
	void	CloseOvReadEvent( void );
	void	SetOvWriteFields( const VS_ACS_Field field1 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field2 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field3 = VS_ACS_INVALID_FIELD );
	void	SetOvReadFields( const VS_ACS_Field field1 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field2 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field3 = VS_ACS_INVALID_FIELD );
	void	SetOvFields( const VS_ACS_Field field1 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field2 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field3 = VS_ACS_INVALID_FIELD );
	void	SetIOHandler(VS_IOHandler *handler);
	void	SetReadHandler(VS_IOHandler *handler);
	void	SetWriteHandler(VS_IOHandler *handler);

	void	SetOvFildIOCP( const void *handleIOCP );
	bool	SetIOCP( void *handleIOCP );
	bool	SetSizeBuffers( const int writeSize, const int readSize );
	virtual bool	SetAsWriter( unsigned long &milliseconds );
	virtual bool	SetAsReader( unsigned long &milliseconds );
	void	*GetHandle( void );
	VS_Connection_Type	Type( void ) const;
	VS_Sock_State		State( void ) const;
	VS_ConnectDirection	GetConnectDirection( void ) const;
	const char* GetBindIp() const override;
	const char* GetBindPort() const override;
	const char* GetPeerIp() const override;
	const char* GetPeerPort() const override;
	char	*GetAcceptHost( void ) const;
	char	*GetAcceptPort( void ) const;
	char	*GetConnectHost( void ) const;
	char	*GetConnectPort( void ) const;
	const VS_IPPortAddress& GetBindAddress() const;
	const VS_IPPortAddress& GetPeerAddress() const;
	bool	IsWrite( void ) const;
	bool	IsWriteOv( void ) const;
	bool	IsRead( void ) const;
	bool	IsReadOv( void ) const;
	bool	IsRW( void ) const;
	bool	IsOv( void ) const;
	int		Send( const void *buffer, const unsigned long n_bytes );
	int		Receive( void *buffer, const unsigned long n_bytes );
	int		SelectOut( unsigned long &milliseconds );
	int		SelectIn( unsigned long &milliseconds );
	void	Free( void *buffer );
	void	Disconnect( void );
	void	Terminate( void );
	void	Close( void );
	unsigned long GetEventTime();
	static void		Break( void *handle );
	static void		Cancel( void *handle );
	static int		SelectIn( const void **handles, const unsigned n_handles,
									unsigned long &milliseconds );

	virtual bool SetKeepAliveMode(bool isKeepAlive, unsigned long keepaliveTime, unsigned long keepalliveInterval);
	virtual void SetFastSocket(bool isFast){}
	struct VS_ConnectionSock_Implementation	  *imp;
	virtual const VS_TlsContext* GetTlsContext() const override {return nullptr;}
};
// end VS_ConnectionSock class

#endif // VS_CONNECTION_SOCK_H
