
#ifndef VS_TRANSPORT_HANDLER_H
#define VS_TRANSPORT_HANDLER_H

#include "../../acs/AccessConnectionSystem/VS_AccessConnectionHandler.h"

struct VS_TransportRouter_SetConnection
{
	VS_TransportRouter_SetConnection( void ) {}
	// end VS_TransportRouter_SetConnection::VS_TransportRouter_SetConnection
	virtual ~VS_TransportRouter_SetConnection( void ) {}
	// end VS_TransportRouter_SetConnection::~VS_TransportRouter_SetConnection
	virtual inline void   SetConnection( const char *cid,
								const unsigned long version,
								const char *sid,
								class VS_Connection *conn, const bool isAccept,
								const unsigned short maxConnSilenceMs,
								const unsigned char fatalSilenceCoef,
								const unsigned char hop,
								const unsigned long rnd_data_ln,
								const unsigned char *rnd_data,
								const unsigned long sign_ln,
								const unsigned char *sign,
								const bool hs_error = false,
								const bool tcpKeepAliveSupport = false ) = 0;

};
// end VS_TransportRouter_SetConnection struct

class VS_TransportHandler : public VS_AccessConnectionHandler
{
public:
			VS_TransportHandler( const char *endpoint, VS_TransportRouter_SetConnection *tr_sc );
	virtual ~VS_TransportHandler( void );
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
private:
	const char   *endpoint;
	VS_TransportRouter_SetConnection   *tr_sc;
};
// end VS_TransportHandler class

#endif // VS_TRANSPORT_HANDLER_H
