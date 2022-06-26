#pragma once

class VS_ConnectionTCP;
class VS_SetConnectionInterface
{
public:
	virtual ~VS_SetConnectionInterface(){}
	virtual bool SetTCPConnection(VS_ConnectionTCP *conn, const void *in_buf, const unsigned long in_len) = 0;
};