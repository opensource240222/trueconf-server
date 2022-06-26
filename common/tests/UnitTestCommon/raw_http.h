char raw_s4[] = "GET /s4/1525893763/8455DF5E77E05BC2562E1D833A07C60A?callback=vp_status&_1525890163807= HTTP/1.1\r\n"
"Host: 127.0.0.1:4323\r\n"
"User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/63.0.3239.132 YaBrowser/18.2.1.196 Yowser/2.5 Safari/537.36\r\n"
"Save-Data: on\r\n"
"Accept: */*\r\n"
"Referer: http://localhost/admin/users/list/\r\n"
"Accept-Encoding: gzip, deflate, br\r\n"
"Accept-Language: ru,en;q=0.9\r\n"
"Cookie: vc=3; lang=ru-RU; PHPSESSID=session_33698305f8754c058e2d56db3a1d2493\r\n"
"X-Forwarded-For: ::1\r\n"
"X-Forwarded-Host: localhost\r\n"
"X-Forwarded-Server: localhost\r\n"
"Connection: close\r\n"
"\r\n";

char raw_s2[] = "GET /s2/%23guest%3Ae257cf92/1527702299/E087F840D74D9619A8005A327509187B HTTP/1.1\r\n"
"Host: 127.0.0.1:4308\r\n"
"User-Agent: PHP-Curl-Class/7.4.0 (+https://github.com/php-curl-class/php-curl-class) PHP/5.6.34 curl/7.57.0\r\n"
"Accept: */*\r\n"
"\r\n";

char raw_torrent_announce[] = "GET /announce?info_hash=n%22z%8c%84%3d%ed%7du%a2D%ef%fa%bb%b8iB%0c%abu&peer_id=-LT1100-afI0yUjU.Msm&port=6883&uploaded=0&downloaded=0&left=16384&corrupt=0&key=9EA18C38&event=started&numwant=200&compact=1&no_peer_id=1&redundant=0 HTTP/1.1\r\n"
"Host: 169.254.88.92:4307\r\n"
"User-Agent: libtorrent/1.1.0.0\r\n"
"Accept-Encoding: gzip\r\n"
"Connection: close\r\n"
"\r\n";

char raw_from_web_config[] = "POST / HTTP/1.1\r\n"
"Host: 127.0.0.1:4308\r\n"
"User-Agent: PHP-Curl-Class/7.4.0 (+https://github.com/php-curl-class/php-curl-class) PHP/5.6.34 curl/7.57.0\r\n"
"Accept: */*\r\n"
"ContentType: text/html\r\n"
"Content-Type: application/x-www-form-urlencoded\r\n"
"X-Forwarded-For: ::1\r\n"
"X-Forwarded-Host: localhost\r\n"
"X-Forwarded-Server: localhost\r\n"
"Connection: close\r\n"
"Content-Length: 68\r\n"
"\r\n"
"{\"Service\":\"NET_CONFIG\",\"OnAppPropsChange\":{\"Session\":\"3691001470\"}}";

// acs: acsTestAttemptStr
// acs_v2: acs::c_selftest_data
char raw_AcsTestAttemptStr[] = "AcsTestAttemptStr";

// when join to webrtc conf; need to pass to VS_WsHandler
uint8_t raw_start_websocket[] = "GET /websocket/ HTTP";		// this should be handled by VS_WsHandler, not HttpHandler

uint8_t raw_CONNECT[] = "CONNECT 192.168.56.1:4307 HTTP/1.0\r\n\r\n";
//"CONNECT as1.kt.trueconf.loc:4307 HTTP/1.0";