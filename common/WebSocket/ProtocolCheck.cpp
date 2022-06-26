#include "ProtocolCheck.h"
#include "acs/AccessConnectionSystem/Responses.h"

#include <cstring>

static const char WebSocketSignature[] = "GET /websock";

inline VS_WsRequest CheckMethod(const char *data, const size_t size)
{
	if (data == NULL || size == 0)
		return vs_invalid;

	if (size >= (sizeof(WebSocketSignature) - 1) && strncmp(data, WebSocketSignature, sizeof(WebSocketSignature) - 1) == 0)
		return vs_ws;

	return vs_invalid;
}

VS_ACS_Response ws::ProtocolCheck(const void * in_buffer, size_t in_len)
{
	VS_WsRequest res = CheckMethod(reinterpret_cast<const char *>(in_buffer), in_len);	//GET /websock HTTP/1.1 or 16 03
	switch (res)
	{
	case vs_ws:
		return vs_acs_accept_connections;
	default:
		return vs_acs_connection_is_not_my;
	};
}
