#pragma once

#include "TransceiverLib/VS_SetConnectionInterface.h"
#include "acs/connection/VS_ConnectionTCP.h"
#include "acs/connection/VS_BufferedConnectionBase.h"
#include "VS_WsChannelBase.h"

class VS_ConnectionTCP;
struct VS_Overlapped;

class VS_WsChannel : public VS_SetConnectionInterface, public VS_BufferedConnectionBase, public ws::ChannelBase
{
public:
	VS_WsChannel(const boost::shared_ptr<VS_WorkThread> &thread);
	void Init();
	virtual ~VS_WsChannel();

	virtual bool SetTCPConnection(VS_ConnectionTCP *conn, const void *in_buf, const unsigned long in_len);
	template <class WsChannel, class BufferIter>
	static bool check_handshake(WsChannel& ch,bool &handshake_done, BufferIter begin, BufferIter end);
	size_t onReceive(const void* data, size_t size) override;
	void onError(unsigned err) override {}
	void onSend() override;
};

template<class WsChannel, class BufferIter>
inline bool VS_WsChannel::check_handshake(WsChannel & ch, bool & handshake_done, BufferIter begin, BufferIter end)
{
	auto buff_size = end - begin;

	// predicates
	auto buffer_contains = [&begin, &end](const char *str) -> bool
	{
		const size_t str_len = strlen(str);
		return std::search(begin, end, &str[0], &str[str_len]) != end;
	};

	auto is_full_request_header = [&buffer_contains]() -> bool
	{
		return buffer_contains("\r\n\r\n");
	};

	auto is_web_socket_handshake = [&buffer_contains]() -> bool
	{
		return buffer_contains("GET") &&
			buffer_contains("Upgrade") &&
			buffer_contains("websocket") &&
			buffer_contains("Sec-WebSocket-Key");
	};

	auto is_GET_request = [&begin, &buff_size]() -> bool
	{
		return buff_size >= 3 && begin[0] == 'G' && begin[1] == 'E' && begin[2] == 'T';
	};

	if (!handshake_done) // check for WebSocket handshake
	{
		if (buff_size < 3) // not enough data to perform initial check
		{
			return false;
		}

		if (!is_GET_request()) // Is it GET request?
		{
			ch.Shutdown(); // Close connection
			return false;
		}

		if (is_full_request_header()) // read full HTTP request header before checking
		{
			if (!is_web_socket_handshake()) // it is not websocket upgrade request
			{
				ch.Shutdown(); // Close connection
				return false;
			}

			handshake_done = true;
		}
		else
		{
			return false; // read more data
		}

	}

	return true;
}
