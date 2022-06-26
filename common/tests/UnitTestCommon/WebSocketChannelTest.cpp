#include <gtest/gtest.h>
#include "WebSocket/VS_WsChannel.h"

const char http_get []=
R"(GET /websocket/ HTTP/1.1
Host: 127.0.0.1:4307
Pragma: no-cache
Cache-Control: no-cache
Origin: http://192.168.41.119
Sec-WebSocket-Version: 13
User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.99 Safari/537.36
Accept-Encoding: gzip, deflate
Accept-Language: ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7
Cookie: PHPSESSID=session_5f892287f8b04e4db49349788dfe7ab6; __zlcmid=nZhW4tBDBez4ja
Sec-WebSocket-Key: 0G1nTOe5RHjlK5hGelRyuw==
Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits
X-Forwarded-For: 192.168.41.119
X-Forwarded-Host: 192.168.41.119
X-Forwarded-Server: localhost
Upgrade: WebSocket
Connection: Upgrade)"
"\r\n\r\n";

struct FakeWsChannel {
	void Shutdown() {}
};

TEST(WebSocket, CheckHandshake) {
	FakeWsChannel ws_ch;
	bool handshake_done(false);
	EXPECT_TRUE(VS_WsChannel::check_handshake(ws_ch, handshake_done, http_get, http_get + sizeof(http_get) - 1));
	EXPECT_TRUE(handshake_done);
}