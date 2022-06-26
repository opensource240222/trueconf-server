
#ifndef VS_STREAMS_HANDLER_H
#define VS_STREAMS_HANDLER_H

#include "../../acs/AccessConnectionSystem/VS_AccessConnectionHandler.h"

struct VS_StreamsRouter_SetConnection
{
	virtual inline void   SetConnection( const char *conferenceName, const char *participantName, const char *connectedParticipantName, const unsigned type, class VS_ConnectionSock *conn, const unsigned char *mtracks ) = 0;
};
// end VS_StreamsRouter_SetConnection struct

class VS_StreamsHandler : public VS_AccessConnectionHandler
{
public:
			VS_StreamsHandler( const char *endpoint, VS_StreamsRouter_SetConnection *sr_sc );
	virtual ~VS_StreamsHandler( void );
	bool				IsValid( void ) const override{ return endpoint && sr_sc; }
	bool				Init( const char *handler_name ) override;
	VS_ACS_Response		Connection( unsigned long *in_len ) override;
	VS_ACS_Response		Protocol( const void *in_buffer, unsigned long *in_len,
									void **out_buffer, unsigned long *out_len,
									void **context ) override;
	void				Accept( VS_ConnectionTCP *conn, const void *in_buffer,
									const unsigned long in_len, const void *context ) override;
	void				Destructor( const void *context ) override;
	void				Destroy( const char *handler_name ) override;
private:
	const char   *endpoint;
	VS_StreamsRouter_SetConnection   *sr_sc;
};
// end VS_StreamsHandler class

#endif // VS_STREAMS_HANDLER_H
