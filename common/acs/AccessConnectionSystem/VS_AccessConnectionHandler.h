
#ifndef VS_ACCESS_CONNECTION_HANDLER_H
#define VS_ACCESS_CONNECTION_HANDLER_H

#include "../connection/VS_ConnectionTCP.h"
#include "../../acs/VS_AcsDefinitions.h"
#include "Responses.h"

class VS_AccessConnectionHandler
{
public:
			VS_AccessConnectionHandler( void );
	virtual ~VS_AccessConnectionHandler( void );
	struct VS_AccessConnectionHandler_Implementation   *imp;
	virtual bool			IsValid( void ) const {		return imp != 0;	}
	virtual bool			Init( const char *handler_name ) = 0;
	virtual VS_ACS_Response	Connection( unsigned long *in_len ) = 0;
	virtual VS_ACS_Response	Protocol( const void *in_buffer, unsigned long *in_len,
										void **out_buffer, unsigned long *out_len,
										void **context ) = 0;
	virtual void			Accept( VS_ConnectionTCP *conn, const void *in_buffer,
										const unsigned long in_len, const void *context ) = 0;
	virtual void			Destructor( const void *context ) = 0;
	virtual void			Destroy( const char *handler_name ) = 0;
	virtual char			*HandlerName( void ) const;
};
// end VS_AccessConnectionHandler class

#endif // VS_ACCESS_CONNECTION_HANDLER_H
