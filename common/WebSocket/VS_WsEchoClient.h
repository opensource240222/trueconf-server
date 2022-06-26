#pragma once
#include <vector>

#include <cstdint>
#include <cstdlib>

#include "../WebSocket/VS_WsChannel.h"

class VS_WsEchoClient :
	public VS_WsChannel
{

	virtual bool ProcTextMsg(const char* msg, unsigned long len);
	virtual bool ProcBinaryMsg(const void* msg, unsigned long len);

	virtual void onError(unsigned err){};


	virtual bool SendResponse();
public:
	VS_WsEchoClient(void);
	virtual ~VS_WsEchoClient(void);
private:
	bool m_is_text;
	std::vector<uint8_t> m_data;
	size_t m_data_len;
};
