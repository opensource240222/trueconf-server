/**
 **************************************************************************
 * \file VS_ConnectionPipe.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Named & unNamed pipe`s functions declaration.
 *
 *
 *
 * \b Project
 * \author SlavetskyA
 * \date 01.10.02
 *
 * $Revision: 1 $
 *
 * $History: VS_ConnectionPipe.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/acs/connection
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connection
 *
 * *****************  Version 14  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/

#ifndef VS_CONNECTION_PIPE_H
#define VS_CONNECTION_PIPE_H

#include "VS_Connection.h"

enum VS_Pipe_State { vs_pipe_state_not_created,
						vs_pipe_state_created,
						vs_pipe_state_connected };
enum VS_Pipe_Type { vs_pipe_type_uninstalled,
						vs_pipe_type_duplex,
						vs_pipe_type_inbound,
						vs_pipe_type_outbound };

class VS_ConnectionPipe : public VS_Connection
{
public:
			VS_ConnectionPipe( void );
	virtual ~VS_ConnectionPipe( void );
	bool	IsValid( void ) const override{ return &imp != 0; }
	bool	SetHeap( const void *handleHeap, const void *cSect = 0 ) override;
	bool	CreateOvWriteEvent( void ) override;
	bool	CreateOvReadEvent( void ) override;
	void	*OvWriteEvent( void ) override;
	void	*OvReadEvent( void ) override;
	const	VS_Overlapped *WriteOv() const override;
	const	VS_Overlapped *ReadOv() const override;
	void	CloseOvWriteEvent( void ) override;
	void	CloseOvReadEvent( void ) override;
	void	SetOvWriteFields( const VS_ACS_Field field1 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field2 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field3 = VS_ACS_INVALID_FIELD ) override;
	void	SetOvReadFields( const VS_ACS_Field field1 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field2 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field3 = VS_ACS_INVALID_FIELD ) override;
	void	SetOvFields( const VS_ACS_Field field1 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field2 = VS_ACS_INVALID_FIELD,
								const VS_ACS_Field field3 = VS_ACS_INVALID_FIELD ) override;

	void	SetIOHandler(VS_IOHandler *handler) override;
	void	SetReadHandler(VS_IOHandler *handler) override;
	void	SetWriteHandler(VS_IOHandler *handler) override;

	void	SetOvFildIOCP( const void *handleIOCP ) override;
	bool	SetIOCP( void *handleIOCP ) override;
	bool	SetSizeBuffers( const int writeSize, const int readSize ) override;
	bool	Create( const char *pipeName,
						const VS_Connection_Type type,
						const VS_Pipe_Type pipeType );
	bool	Open( const char *pipeName,
					const VS_Connection_Type type,
					const VS_Pipe_Type pipeType );
	bool	Create( const VS_Connection_Type type,
						const VS_Pipe_Type pipeType );
	bool	Open( VS_ConnectionPipe *pipe,
					const VS_Connection_Type type,
					const VS_Pipe_Type pipeType );
	bool	Connect( void );
	int		GetConnectResult( unsigned long &milliseconds );
	bool	SetConnectResult( const unsigned long b_trans,
									const struct VS_Overlapped *ov );
	void	*GetHandle( void ) override;
	VS_Connection_Type	Type( void ) const override;
	VS_Pipe_State		State( void ) const;
	char	*Name( void );
	bool	IsWrite( void ) const override;
	bool	IsWriteOv( void ) const override;
	bool	IsRead( void ) const override;
	bool	IsReadOv( void ) const override;
	bool	IsRW( void ) const override;
	bool	IsOv( void ) const override;
	void	Free( void *buffer ) override;
	void	Disconnect( void ) override;
	void	Close( void ) override;

	const char* GetBindIp() const override;
	const char* GetBindPort() const override;
	const char* GetPeerIp() const override;
	const char* GetPeerPort() const override;
	const VS_IPPortAddress& GetBindAddress() const override;
	const VS_IPPortAddress& GetPeerAddress() const override;
	bool SetKeepAliveMode(bool isKeepAlive, unsigned long keepaliveTime, unsigned long keepalliveInterval) override;
	VS_ConnectDirection	GetConnectDirection( void ) const override;
	unsigned long GetEventTime() override;
	const VS_TlsContext* GetTlsContext() const override {return nullptr;}

	struct VS_ConnectionPipe_Implementation	  *imp;
};
// end VS_ConnectionPipe class

#define   VS_PIPE_DIRECTORY   "\\\\.\\pipe\\"
extern const char   VS_PipeDir[];	// = VS_PIPE_DIRECTORY

#endif // VS_CONNECTION_PIPE_H
