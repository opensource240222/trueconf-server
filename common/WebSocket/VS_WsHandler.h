#ifndef VS_WS_HANDLER_H
#define VS_WS_HANDLER_H

#include "acs/AccessConnectionSystem/VS_AccessConnectionHandler.h"
#include "acs/AccessConnectionSystem/VS_AccessConnectionHandlerTypes.h"
#include "std/cpplib/VS_Lock.h"
#include "std/cpplib/VS_SimpleStr.h"

class VS_SetConnectionInterface;

class VS_WsHandler :
	public VS_AccessConnectionHandler,
	VS_Lock
{
	VS_SimpleStr	m_handlerName;
public:
							VS_WsHandler( void );
	virtual					~VS_WsHandler( void );
	bool					IsValid( void ) const override;
	bool					Init( const char *handler_name ) override;
	VS_ACS_Response			Connection( unsigned long *in_len ) override;
	VS_ACS_Response			Protocol( const void *in_buffer, unsigned long *in_len,
									void **out_buffer, unsigned long *out_len,
									void **context ) override;
	void					Accept( VS_ConnectionTCP *conn, const void *in_buffer,
									const unsigned long in_len, const void *context ) override;
	void					Destructor( const void *context ) override;
	void					Destroy( const char *handler_name ) override;
	char					*HandlerName( void ) const override{
		return m_handlerName.m_str;
	}

	static VS_ACS_Response	Protocol( const void *in_buffer, unsigned long *in_len);

};

extern VS_ACS_Response IsWsHandlerProtocol(const void *in_buffer, unsigned long *in_len);

#endif
