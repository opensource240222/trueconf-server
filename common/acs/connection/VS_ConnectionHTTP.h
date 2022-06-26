/**
 **************************************************************************
 * \file VS_ConnectionHTTP.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief HTTP function declaration.
 *
 *
 *
 * \b Project HTTP tunneling.
 * \author SlavetskyA
 * \date 01.10.02
 *
 * $Revision: 1 $
 *
 * $History: VS_ConnectionHTTP.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/acs/connection
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connection
 *
 * *****************  Version 9  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/
#ifndef VS_CONNECTION_HTTP_H
#define VS_CONNECTION_HTTP_H

#include "VS_ConnectionTCP.h"

enum VS_ConnectionHTTP_EncapsulationType {
		vs_connection_http_type_uninstalled, vs_connection_http_type_raw,
		vs_connection_http_type_base64, vs_connection_http_type_tunneling };

class VS_ConnectionHTTP : public VS_ConnectionSock
{
public:
			VS_ConnectionHTTP( void );
	virtual ~VS_ConnectionHTTP( void );
	bool	IsValid( void ) const { return VS_ConnectionSock::IsValid() && (&imp != 0); }
	bool	Connect( const char *host, const unsigned short port,
				unsigned long &milliseconds, const VS_ConnectionHTTP_EncapsulationType type,
				const char *http_proxy_host = 0, const unsigned short http_proxy_port = 0,
				const char *http_proxy_user = 0, const char *http_proxy_passwd = 0 );
	virtual bool	Accept( const char *host, const unsigned short port,
								unsigned long &milliseconds );
	virtual int		Accept( VS_ConnectionTCP *listener,
								unsigned long &milliseconds );
	virtual bool	SetAsWriter( unsigned long &milliseconds );
	virtual bool	SetAsReader( unsigned long &milliseconds );
	VS_ConnectionHTTP_EncapsulationType   HTTP_Type( void ) const;
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
	struct VS_ConnectionHTTP_Implementation	  *imp;
};
// end VS_ConnectionHTTP class

#endif // VS_CONNECTION_HTTP_H
