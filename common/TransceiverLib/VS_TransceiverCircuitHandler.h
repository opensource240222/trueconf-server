#pragma once
#include "acs/AccessConnectionSystem/VS_AccessConnectionHandler.h"
#include <memory>

class VS_SetConnectionInterface;

/**
	TODO: make this handler common for all and use acs->AddHAndler inside this class. Maybe after wrapping acs to shared_ptr
*/

class VS_TransceiverCircuitHandler : public VS_AccessConnectionHandler
{
private:
	std::weak_ptr<VS_SetConnectionInterface>	m_proxiesPool;
public:
	explicit VS_TransceiverCircuitHandler(const std::shared_ptr<VS_SetConnectionInterface> &proxiesPool);
	virtual ~VS_TransceiverCircuitHandler();

	bool Init(const char *handler_name) override;
	VS_ACS_Response	Connection( unsigned long *in_len ) override;
	VS_ACS_Response	Protocol( const void *in_buffer, unsigned long *in_len,
								void **out_buffer, unsigned long *out_len,
								void **context ) override;
	void            Accept( VS_ConnectionTCP *conn, const void *in_buffer,
								const unsigned long in_len,
								const void *context ) override;
	void            Destructor( const void *context ) override {}
	void            Destroy( const char *handler_name ) override{}

};