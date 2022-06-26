#ifdef _WIN32
#ifndef VS_ACS_BWT_HANDLER_H
#define VS_ACS_BWT_HANDLER_H

#include "../acs/AccessConnectionSystem/VS_AccessConnectionHandler.h"

class VS_BwtHandler : public VS_AccessConnectionHandler
{
public:
		VS_BwtHandler( void ) {};
	virtual ~VS_BwtHandler( void ) {};
	bool				IsValid( void ) const override;
	bool				Init( const char *handler_name ) override;
	VS_ACS_Response		Connection( unsigned long *in_len ) override;
	VS_ACS_Response		Protocol( const void *in_buffer, unsigned long *in_len,
									void **out_buffer, unsigned long *out_len,
									void **context ) override;
	void				Accept( VS_ConnectionTCP *conn, const void *in_buffer,
									const unsigned long in_len, const void *context ) override;
	void				Destructor( const void *context ) override;
	void				Destroy( const char *handler_name ) override;
};
// end VS_BwtHandler class

#endif // VS_ACS_BWT_HANDLER_H
#endif
