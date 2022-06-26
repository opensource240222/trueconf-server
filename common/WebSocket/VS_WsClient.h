#pragma once
#include "../WebSocket/VS_WsChannel.h"
#include "FakeClient/VS_AbstractJsonConnection.h"

class VS_WsClient :
	public VS_WsChannel, public VS_AbstractJsonConnection
{

	virtual bool ProcTextMsg(const char* msg, unsigned long len);
	virtual bool ProcBinaryMsg(const void* msg, unsigned long len);

	virtual void onError(unsigned err);


	virtual bool SendResponse(const char *data);
public:
	VS_WsClient(void);
	~VS_WsClient(void);

	bool m_first_buffer;
};
