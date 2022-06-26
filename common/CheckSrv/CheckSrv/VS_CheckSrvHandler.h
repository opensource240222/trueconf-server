#pragma once
#include "acs/AccessConnectionSystem/VS_AccessConnectionHandler.h"
#include "VS_CheckSrvPack.h"
class VS_TransportRouter;
class VS_CheckSrvHandler : public VS_AccessConnectionHandler
{
private:
	VS_CheckSrvPack	m_check_pack;
	VS_TransportRouter*		m_tr;
public:
	VS_CheckSrvHandler();
	~VS_CheckSrvHandler();
	bool			IsValid() const override;
	bool			Init(const char *handler_name) override;
	VS_ACS_Response	Connection(unsigned long *in_len) override;
	VS_ACS_Response	Protocol( const void *in_buffer, unsigned long *in_len, void ** out_buffer,
								unsigned long *out_len, void **context) override;
	void			Accept(VS_ConnectionTCP *conn, const void *in_buffer, const unsigned long in_len, const void *context) override;
	void			Destructor(const void *context) override;
	void			Destroy(const char *handler_name) override;
	void			SetTransportRouter(VS_TransportRouter *tr);
};