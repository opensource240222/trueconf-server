#include "sudis.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TTransport.h>
#include <thrift/transport/TBufferTransports.h>

#include "../gen-cpp/TCciUserLoginV1.h"
#include "../gen-cpp/TCciUserRoleListV1.h"
#include "../gen-cpp/TCciUserListV1.h"
#include "../gen-cpp/TCciUserListV2.h"
#include "../gen-cpp/TCciUserLogoutV1.h"
#include "../gen-cpp/TicketV3.h"
#include "../gen-cpp/UserLoginV3.h"

#include "../gen-cpp/TServiceExtSpsb.h"

#include "SecureLib/VS_UtilsLib.h"
#include "../../std/cpplib/base64.h"
#include "std-generic/clib/sha1.h"
#include "../../std/debuglog/VS_Debug.h"

#include "std-generic/compat/memory.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <cctype>

#include <winsock.h>

#define ISO8601_Z_TIME_FMT "%04d-%02d-%02dT%02d:%02d:%02dZ"

namespace sudis{
	std::string host_cryptopro;				// 192.168.62.234
	unsigned long ip_cryptopro;				// 0xC0A83EEA (192.168.62.234)
	unsigned short port_cryptopro;			// 8001

	std::string host_sudis;					// idmapi01.int.sudis.at-consulting.ru
	unsigned long ip_sudis;					// 0xC0A80E9E (192.168.14.158)
	unsigned short port_sudis;				// 80

	std::string host_spsb;					// idmapi01.int.sudis.at-consulting.ru
	unsigned long ip_spsb;					// 0xC0A80E9E (192.168.14.158)
	unsigned short port_spsb;				// 80
	std::string uuid_spsb;					// 6dec3f80-6171-2754-fd9c-58b6d582f365
}

#define DEBUG_CURRENT_MODULE VS_DM_LOGINS

bool sudis::InitWSA()
{
	//Start up Winsock…
    WSADATA wsadata;

    int error = WSAStartup(0x0202, &wsadata);

    //Did something happen?
    if (error)
        return false;

    //Did we get the right Winsock version?
    if (wsadata.wVersion != 0x0202)
    {
        WSACleanup(); //Clean up Winsock
        return false;
    }

	return true;
}
void sudis::SetSudisAddress(const char* host, const unsigned short port)
{
	unsigned long old_ip = sudis::ip_sudis;

	host_sudis = host;
	port_sudis = port;
	ip_sudis = inet_addr(host);
	if (ip_sudis==INADDR_NONE || ip_sudis==INADDR_ANY)
	{
		struct hostent* lpHostEnt = gethostbyname(host);
		if (!lpHostEnt)
		{
			dprint4("sudis: can't resolve %s\n", host);
			return ;
		}
		std::vector<unsigned long> ips;
		int i = 0;
		for(i = 0; lpHostEnt->h_addr_list[i];i++)
		{
			ips.push_back( inet_addr(inet_ntoa( *(struct in_addr *)(lpHostEnt->h_addr_list[i]) )) );
		}
		if(ips.empty())
		{
			dprint4("sudis: resolve %s is empty\n", host);
			return ;
		}

		for(std::vector<unsigned long>::iterator it=ips.begin(); it!=ips.end(); ++it)
		{
			dprint4("sudis: candidate sudis IP: %d.%d.%d.%d\n",
				((unsigned char*)&(*it))[0],
				((unsigned char*)&(*it))[1],
				((unsigned char*)&(*it))[2],
				((unsigned char*)&(*it))[3]);
		}

		if (ips.size() > 1) // skip current
		{
			std::vector<unsigned long>::iterator it = std::find(ips.begin(), ips.end(), old_ip);
			if (it!=ips.end())
				ips.erase(it);
		}

		if (!ips.empty())
			sudis::ip_sudis = ips[rand()%ips.size()];
	}
	dprint4("sudis: use sudis IP: %d.%d.%d.%d\n",
		((unsigned char*)&sudis::ip_sudis)[0],
		((unsigned char*)&sudis::ip_sudis)[1],
		((unsigned char*)&sudis::ip_sudis)[2],
		((unsigned char*)&sudis::ip_sudis)[3]);
}

void sudis::SetCryptoProAddress(const char* host, const unsigned short port)
{
	host_cryptopro = host;
	port_cryptopro = port;
	ip_cryptopro = inet_addr(host);
	if (ip_cryptopro==INADDR_NONE || ip_cryptopro==INADDR_ANY)
	{
		struct hostent* lpHostEnt = gethostbyname(host);
		if (!lpHostEnt)
			return ;
		int i = 0;
		for(i = 0; lpHostEnt->h_addr_list[i];i++)
		{
			unsigned char ch = lpHostEnt->h_addr_list[i][0];
			if(ch>1)
				break;
		}
		if(!lpHostEnt->h_addr_list[i])
			return ;
		ip_cryptopro = inet_addr(inet_ntoa( *(struct in_addr *)(lpHostEnt->h_addr_list[i]) ));
	}
}

void sudis::SetSPSBAddress(const char* host, const unsigned short port, const char* uuid)
{
	host_spsb = host;
	port_spsb = port;
	uuid_spsb = uuid;
	ip_spsb = inet_addr(host);
	if (ip_spsb==INADDR_NONE || ip_spsb==INADDR_ANY)
	{
		struct hostent* lpHostEnt = gethostbyname(host);
		if (!lpHostEnt)
			return ;
		int i = 0;
		for(i = 0; lpHostEnt->h_addr_list[i];i++)
		{
			unsigned char ch = lpHostEnt->h_addr_list[i][0];
			if(ch>1)
				break;
		}
		if(!lpHostEnt->h_addr_list[i])
			return ;
		ip_spsb = inet_addr(inet_ntoa( *(struct in_addr *)(lpHostEnt->h_addr_list[i]) ));
	}
}
//bool sudis::Init()
//{
//	dprint4("sudis: Init\n");
//	VS_RegistryKey key(false, CONFIGURATION_KEY);
//	if (!key.IsValid())
//		return false;
//	char host[1024] = {0};
//	unsigned long sz = 1024;
//	if (key.GetValue(host, sz, VS_REG_STRING_VT, "Sudis Host")>0)
//	{
//		unsigned long ip = 0;
//		if (!VS_GetIpByHostName(host, &ip))
//		{
//			dprint4("sudis: cant resolve %s for sudis server\n", host);
//			return false;
//		}
//		ip_sudis.set(ip);
//		host_sudis.set(host);
//	}else{
//		dprint4("sudis: sudis server is not set in registry\n");
//		return false;
//	}
//
//	unsigned long port = 0;
//	if (key.GetValue(&port, sizeof(unsigned long), VS_REG_INTEGER_VT, "Sudis Port")>0)
//	{
//		port_sudis.set((unsigned short)port);
//	}else{
//		port_sudis.set(80);
//	}
//
//	memset(host, 0, sizeof(host));
//	if (key.GetValue(host, sz, VS_REG_STRING_VT, "CryptoPro Host")>0)
//	{
//		unsigned long ip = 0;
//		if (!VS_GetIpByHostName(host, &ip))
//		{
//			dprint4("sudis: cant resolve %s for CryptoPro server\n", host);
//			return false;
//		}
//		ip_cryptopro.set(ip);
//		host_cryptopro.set(host);
//	}else{
//		dprint4("sudis: CrypoPro server is not set in registry\n");
//		return false;
//	}
//
//	port = 0;
//	if (key.GetValue(&port, sizeof(unsigned long), VS_REG_INTEGER_VT, "CryptoPro Port")>0)
//	{
//		port_cryptopro.set((unsigned short)port);
//	}else{
//		port_cryptopro.set(8001);
//	}
//
//	memset(host, 0, sizeof(host));
//	if (key.GetValue(host, sz, VS_REG_STRING_VT, "Sudis SPSB Host")>0)
//	{
//		unsigned long ip = 0;
//		if (!VS_GetIpByHostName(host, &ip))
//		{
//			dprint4("sudis: cant resolve %s for sudis spsb server\n", host);
//			return false;
//		}
//		ip_sudis_spsb.set(ip);
//		host_sudis_spsb.set(host);
//	}else{
//		dprint4("sudis: sudis spsb server is not set in registry\n");
//		return false;
//	}
//
//	port = 0;
//	if (key.GetValue(&port, sizeof(unsigned long), VS_REG_INTEGER_VT, "Sudis SPSB Port")>0)
//	{
//		port_sudis_spsb.set((unsigned short)port);
//	}else{
//		port_sudis_spsb.set(80);
//	}
//
//	memset(host, 0, sizeof(host));
//	if (key.GetValue(host, sz, VS_REG_STRING_VT, "Sudis SPSB UUID")>0)
//	{
//		spsb_uuid.set(host);
//	}else{
//		dprint4("sudis: sudis spsb uuid is not set in registry\n");
//	}
//
//	dprint4("sudis: using CryptoPro 0x%X:%d, sudis 0x%X:%d spsb 0x%X:%d\n", ip_cryptopro, port_cryptopro, ip_sudis, port_sudis, ip_sudis_spsb.get(), port_sudis_spsb.get());
//	return true;
//}

void sudis::generate_nonce(char* str)
{
	for(unsigned int i=0; i<32; ++i)
		str[i] = 'A' + rand()%26;
}

template<class T, class Arg, void (T::*ptr)(const Arg&)>
bool sudis::Serialize(const Arg& req, std::vector<unsigned char> &v)
{
	bool result(false);
	try {
		boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> mem_buff(new apache::thrift::transport::TMemoryBuffer());		// output data stored here
		boost::shared_ptr<apache::thrift::transport::TTransport> transport(new apache::thrift::transport::TBufferedTransport(mem_buff));
		boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol(new apache::thrift::protocol::TBinaryProtocol(transport));

		T client(protocol);
		(client.*ptr)(req);

		uint8_t* bufPtr(0);
		unsigned long bufSz(0);
		mem_buff->getBuffer(&bufPtr, (uint32_t*) &bufSz);
		v.assign(bufPtr, bufPtr+bufSz);
		result = true;
	}catch (apache::thrift::TException &tx) {
		dprint4("sudis: Serialize ERROR: %s\n", tx.what());
	}
	return result;
}

template<class T, class Arg, void (T::*ptr)(Arg&)>
bool sudis::Deserialize(const std::vector<unsigned char> &v, Arg& rsp) {
	bool result(false);
	try {
		boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> mem_buff(new apache::thrift::transport::TMemoryBuffer((uint8_t*)&v[0],(uint32_t)v.size()));		// input data stored here
		boost::shared_ptr<apache::thrift::transport::TTransport> transport(new apache::thrift::transport::TBufferedTransport(mem_buff));
		boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol(new apache::thrift::protocol::TBinaryProtocol(transport));

		T client(protocol);
		(client.*ptr)(rsp);

		result = true;
	} catch (apache::thrift::TException &tx) {
		dprint4("sudis: Deserialize ERROR: %s\n", tx.what());
	}
	return result;
}

template<class TClient, class TRequest, class TResponce, void (TClient::*ptr_req)(const TRequest&), void (TClient::*ptr_rsp)(TResponce&)>
bool sudis::ProcessRequest(const TRequest& req, TResponce& rsp, const char* request_url)
{
	std::vector<unsigned char> v;
	if (!Serialize<TClient, TRequest, ptr_req>(req, v))
	{
		dprint4("sudis: serialize error\n");
		return false;
	}


	const char* CryptoProHeaders = 	"X-senderkey: cert22\r\n"
									"X-recipientkey: cert22\r\n";

	const char* SudisHeaders	 = 	""; //"Trusted-Sp-Code: sudis-sp-mvd\r\n";

	const char* http_str = "POST %s HTTP/1.0\r\n"
					"Host: %s\r\n"
					"%s"
					"Content-Type: application/octet-stream\r\n"
					"Accept: application/octet-stream\r\n"
					"Content-Length: %d\r\n\r\n";
	char buff[10000] = {0};
	int n_bytes = _snprintf((char*) &buff[0], 10000, http_str, "/encode", host_cryptopro.c_str(), CryptoProHeaders, v.size());
	memcpy_s(buff+n_bytes, 10000-n_bytes, &v[0], v.size());

	char buff_rcv[8192] = {0};
	unsigned long buff_rcv_sz = 8192;

	if (!SendRecv(ip_cryptopro, port_cryptopro, buff, n_bytes + v.size(), buff_rcv, buff_rcv_sz))
	{
		dprint4("sudis: CryptoPro encode error\n");
		return false;
	}
	dprint4("sudis: CryptoPro encode: received %ld bytes, %s\n", buff_rcv_sz, buff_rcv);
	if (!CheckHTTPResponceCode(buff_rcv, buff_rcv_sz))
		return false;

	char* content = strstr(buff_rcv, "\r\n\r\n");
	if (!content)
	{
		dprint4("sudis: no content at CryptoPro encode\n");
		return false;
	}
	content+=4;
	if (!content)
	{
		dprint4("sudis: cant find content at CryptoPro encode\n");
		return false;
	}

	int content_length = buff_rcv_sz-(content-buff_rcv);

	n_bytes = _snprintf((char*) &buff[0], 10000, http_str, request_url, host_sudis.c_str(), SudisHeaders, content_length);
	memcpy_s(buff+n_bytes, 10000-n_bytes, content, content_length);

	const int N_retries = 3;
	int i=1;
	bool res(false);
	do{
		buff_rcv_sz = 8192;
		res = SendRecv(ip_sudis, port_sudis, buff, n_bytes + content_length, buff_rcv, buff_rcv_sz);
		if (!res)
		{
			dprint4("sudis: sudis http server error, host:%s, ip:%d.%d.%d.%d port:%d retries:%d/%d\n", sudis::host_sudis.c_str(),
				((unsigned char*)&sudis::ip_sudis)[0],
				((unsigned char*)&sudis::ip_sudis)[1],
				((unsigned char*)&sudis::ip_sudis)[2],
				((unsigned char*)&sudis::ip_sudis)[3],
				sudis::port_sudis,
				i, N_retries);
			sudis::SetSudisAddress(sudis::host_sudis.c_str(), sudis::port_sudis);
			++i;
		}
	} while(!res && i<=N_retries);
	if (!res)
	{
		dprint4("sudis: sudis http server failed retry\n");
		return false;
	}
	dprint4("sudis: from sudis server received %ld bytes, %s\n", buff_rcv_sz, buff_rcv);
	if (!CheckHTTPResponceCode(buff_rcv, buff_rcv_sz))
		return false;


	content = strstr(buff_rcv, "\r\n\r\n");
	if (!content)
	{
		dprint4("sudis: no content at sudis responce\n");
		return false;
	}
	content+=4;
	if (!content)
	{
		dprint4("sudis: cant find content at sudis responce\n");
		return false;
	}

	content_length = buff_rcv_sz-(content-buff_rcv);

	n_bytes = _snprintf((char*) &buff[0], 10000, http_str, "/decode", host_cryptopro.c_str(), CryptoProHeaders, content_length);
	memcpy_s(buff+n_bytes, 10000-n_bytes, content, content_length);

	buff_rcv_sz = 8192;
	if (!SendRecv(ip_cryptopro, port_cryptopro, buff, n_bytes + content_length, buff_rcv, buff_rcv_sz))
	{
		dprint4("sudis: CryptoPro decode error\n");
		return false;
	}
	dprint4("sudis: CryptoPro decode: received %ld bytes, %s\n", buff_rcv_sz, buff_rcv);
	if (!CheckHTTPResponceCode(buff_rcv, buff_rcv_sz))
		return false;

	content = strstr(buff_rcv, "\r\n\r\n");
	if (!content)
	{
		dprint4("sudis: no content at CryptoPro decode\n");
		return false;
	}
	content+=4;
	if (!content)
	{
		dprint4("sudis: cant find content at CryptoPro decode\n");
		return false;
	}

	content_length = buff_rcv_sz-(content-buff_rcv);


	std::vector<unsigned char> v2((unsigned char*)content, (unsigned char*)content + content_length);
	if (!Deserialize<TClient, TResponce, ptr_rsp>(v2, rsp))
	{
		dprint4("sudis: deserialize error\n");
		return false;
	}

	dprint4("sudis: deserialize ok\n");
	return true;
}

template<class TClient, class TRequest, class TResponce, void (TClient::*ptr_req)(const TRequest&), void (TClient::*ptr_rsp)(TResponce&)>
bool sudis::ProcessRequestNoEncrypt(const TRequest& req, TResponce& rsp, const char* request_url)
{
	std::vector<unsigned char> v;
	if (!Serialize<TClient, TRequest, ptr_req>(req, v))
	{
		dprint4("sudis: serialize error\n");
		return false;
	}

	const char* CryptoProHeaders = 	"X-senderkey: cert22\r\n"
									"X-recipientkey: cert22\r\n";

	const char* SudisHeaders	 = 	""; //"Trusted-Sp-Code: sudis-sp-mvd\r\n";

	const char* http_str = "POST %s HTTP/1.0\r\n"
					"Host: %s\r\n"
					"%s"
					"Content-Type: application/octet-stream\r\n"
					"Accept: application/octet-stream\r\n"
					"Content-Length: %d\r\n\r\n";
	char buff[10000] = {0};
	int n_bytes = _snprintf((char*) &buff[0], 10000, http_str, "/encode", host_cryptopro.c_str(), CryptoProHeaders, v.size());
	memcpy_s(buff+n_bytes, 10000-n_bytes, &v[0], v.size());

	char buff_rcv[8192] = {0};
	unsigned long buff_rcv_sz = 8192;
/*
	if (!SendRecv(ip_cryptopro, port_cryptopro, buff, n_bytes + v.size(), buff_rcv, buff_rcv_sz))
	{
		dprint4("sudis: CryptoPro encode error\n");
		return false;
	}
	dprint4("sudis: CryptoPro encode: received %d bytes, %s\n", buff_rcv_sz, buff_rcv);
	if (!CheckHTTPResponceCode(buff_rcv, buff_rcv_sz))
		return false;

	char* content = strstr(buff_rcv, "\r\n\r\n");
	if (!content)
	{
		dprint4("sudis: no content at CryptoPro encode\n");
		return false;
	}
	content+=4;
	if (!content)
	{
		dprint4("sudis: cant find content at CryptoPro encode\n");
		return false;
	}

	int content_length = buff_rcv_sz-(content-buff_rcv);
*/

	// //////////////////
	char* content = (char*) &(v[0]);
	int content_length = v.size();
	////////////////////////

	n_bytes = _snprintf((char*) &buff[0], 10000, http_str, request_url, host_sudis.c_str(), SudisHeaders, content_length);
	memcpy_s(buff+n_bytes, 10000-n_bytes, content, content_length);

	buff_rcv_sz = 8192;
	if (!SendRecv(ip_sudis, port_sudis, buff, n_bytes + content_length, buff_rcv, buff_rcv_sz))
	{
		dprint4("sudis: sudis http server error\n");
		return false;
	}
	dprint4("sudis: from sudis server received %ld bytes, %s\n", buff_rcv_sz, buff_rcv);
	if (!CheckHTTPResponceCode(buff_rcv, buff_rcv_sz))
		return false;


	content = strstr(buff_rcv, "\r\n\r\n");
	if (!content)
	{
		dprint4("sudis: no content at sudis responce\n");
		return false;
	}
	content+=4;
	if (!content)
	{
		dprint4("sudis: cant find content at sudis responce\n");
		return false;
	}

	content_length = buff_rcv_sz-(content-buff_rcv);

/*
	n_bytes = _snprintf((char*) &buff[0], 10000, http_str, "/decode", host_cryptopro.c_str(), CryptoProHeaders, content_length);
	memcpy_s(buff+n_bytes, 10000-n_bytes, content, content_length);

	buff_rcv_sz = 8192;
	if (!SendRecv(ip_cryptopro, port_cryptopro, buff, n_bytes + content_length, buff_rcv, buff_rcv_sz))
	{
		dprint4("sudis: CryptoPro decode error\n");
		return false;
	}
	dprint4("sudis: CryptoPro decode: received %d bytes, %s\n", buff_rcv_sz, buff_rcv);
	if (!CheckHTTPResponceCode(buff_rcv, buff_rcv_sz))
		return false;

	content = strstr(buff_rcv, "\r\n\r\n");
	if (!content)
	{
		dprint4("sudis: no content at CryptoPro decode\n");
		return false;
	}
	content+=4;
	if (!content)
	{
		dprint4("sudis: cant find content at CryptoPro decode\n");
		return false;
	}

	content_length = buff_rcv_sz-(content-buff_rcv);
*/

	std::vector<unsigned char> v2((unsigned char*)content, (unsigned char*)content + content_length);
	if (!Deserialize<TClient, TResponce, ptr_rsp>(v2, rsp))
	{
		dprint4("sudis: deserialize error\n");
		return false;
	}

	dprint4("sudis: deserialize ok\n");
	return true;
}

template<class TClient, class TRequest, class TResponce, void (TClient::*ptr_req)(const TRequest&), void (TClient::*ptr_rsp)(TResponce&)>
bool sudis::ProcessRequestSPSB(const TRequest& req, TResponce& rsp, const char* request_url)
{
	if (uuid_spsb.empty())
	{
		dprint4("sudis: spsb uuid not found\n");
		return false;
	}

	std::vector<unsigned char> v;
	if (!Serialize<TClient, TRequest, ptr_req>(req, v))
	{
		dprint4("sudis: serialize error\n");
		return false;
	}

	std::string senderKey = uuid_spsb;
	std::string senderKeyBytes_hex = senderKey;

	// remove all "-" chars
	senderKeyBytes_hex.erase(std::remove_if(senderKeyBytes_hex.begin(), senderKeyBytes_hex.end(), [](char c){
		return !isxdigit(c);
	}), senderKeyBytes_hex.end());

	// StrHexToBytes
	std::string senderKeyBytes;
	{
		std::string in = senderKeyBytes_hex;
		if ((in.length() % 2) != 0) {
			return false;
		}
		size_t cnt = in.length() / 2;
		for (size_t i = 0; cnt > i; ++i) {
			long s = 0;
			std::stringstream ss;
			ss << std::hex << in.substr(i * 2, 2);
			ss >> s;

			senderKeyBytes.push_back(static_cast<unsigned char>(s));
		}
	}

	// make sha1
	SHA1 sha1;
	sha1.Update(senderKeyBytes);
	sha1.Update(v.data(), v.size());
	sha1.Final();
	unsigned char hash[20];
	sha1.GetBytes(hash);

	// make base64
	size_t b64_sz = 0;
	base64_encode(hash, 20, nullptr, b64_sz);
	auto b64 = vs::make_unique_default_init<char[]>(b64_sz + 1);
	base64_encode(hash, 20, b64.get(), b64_sz);
	b64[b64_sz] = '\0';

	// store at X-MAC
	const char* SpsbHeadersTemplate = 	"X-senderSpCode: svks-m\r\n"
										"X-MAC: %s\r\n";
	char SpsbHeaders[1024] = {0};
	_snprintf((char*)&SpsbHeaders[0], 1024, SpsbHeadersTemplate, b64.get());

	const char* http_str = "POST %s HTTP/1.0\r\n"
		"Host: %s\r\n"
		"%s"
		"Content-Type: application/x-thrift\r\n"
		"Accept: application/x-thrift\r\n"
		"Content-Length: %d\r\n\r\n";

	char buff[10000] = {0};
	char buff_rcv[8192] = {0};
	unsigned long buff_rcv_sz = 8192;

	// //////////////////
	char* content = (char*) &(v[0]);
	int content_length = v.size();
	////////////////////////

	long n_bytes = _snprintf((char*) &buff[0], 10000, http_str, request_url, host_spsb.c_str(), SpsbHeaders, content_length);
	memcpy_s(buff+n_bytes, 10000-n_bytes, content, content_length);

	buff_rcv_sz = 8192;
	if (!SendRecv(ip_spsb, port_spsb, buff, n_bytes + content_length, buff_rcv, buff_rcv_sz))
	{
		dprint4("sudis: spsb http server error\n");
		return false;
	}
	dprint4("sudis: from spsb server received %ld bytes, %s\n", buff_rcv_sz, buff_rcv);
	if (!CheckHTTPResponceCode(buff_rcv, buff_rcv_sz))
		return false;


	content = strstr(buff_rcv, "\r\n\r\n");
	if (!content)
	{
		dprint4("sudis: no content at spsb responce\n");
		return false;
	}
	content+=4;
	if (!content)
	{
		dprint4("sudis: cant find content at spsb responce\n");
		return false;
	}

	content_length = buff_rcv_sz-(content-buff_rcv);

	std::vector<unsigned char> v2((unsigned char*)content, (unsigned char*)content + content_length);
	if (!Deserialize<TClient, TResponce, ptr_rsp>(v2, rsp))
	{
		dprint4("sudis: deserialize error\n");
		return false;
	}

	dprint4("sudis: deserialize ok\n");
	return true;
}

bool sudis::CheckAccount(const char* call_id, const char* pwd, char* user_token, char* oid)
{
	bool result(false);
	if (!call_id||!*call_id || !pwd||!*pwd)
		return result;
	if (!ip_sudis || !ip_cryptopro)
		return result;
	dprint4("sudis: cciUserLoginV1 %s start\n", call_id);
	char time_buff[128] = {0};
	SYSTEMTIME st;
	GetSystemTime(&st);
	sprintf(time_buff, ISO8601_Z_TIME_FMT, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	sudis::TCciUserLoginV1Request req;
	req.requestDateTime = time_buff;
	char nonce[33] = {0};
	generate_nonce((char*) &nonce[0]);
	req.requestNonce = "nonsa-";	req.requestNonce += nonce;
	req.login = call_id;
	req.password = pwd;
	req.__set_spCode("svks-m");

	sudis::TCciUserLoginV1Response rsp;
	if (!sudis::ProcessRequest<	sudis::TCciUserLoginV1Client,
		sudis::TCciUserLoginV1Request,
		sudis::TCciUserLoginV1Response,
		&sudis::TCciUserLoginV1Client::send_cciUserLoginV1,
		&sudis::TCciUserLoginV1Client::recv_cciUserLoginV1>
		(req,rsp,"/dsapi/tbr/cciUserLoginV1"))
	{
		return result;
	}

	if (rsp.userTokenId.length())		// dont check:  rsp.resultMessage.code == "SUCCESS" || "USER_PASSWORD_SOON_EXPIRED"
	{
		result = true;
		strcpy(user_token, rsp.userTokenId.c_str());

		dprint4("Found %zu attributes\n", rsp.attributes.size());
		for(std::vector< ::sudis::TCciUserAttributeV1>::iterator it=rsp.attributes.begin(); it!=rsp.attributes.end(); ++it)
		{
			dprint4("Name %s, vals %zu\n",it->name.c_str(), it->values.size());
			for(std::vector<std::string>::iterator it2=it->values.begin(); it2!=it->values.end(); ++it2)
			{
				dprint4("\tval: %s\n",it2->c_str());
				if (it->name == "oid")
				{
					strcpy(oid, it2->c_str());
				}
			}
		}
	}

	dprint4("sudis: cciUserLoginV1 %s result:%s, userTokenId:%s, oid=%s\n", call_id, rsp.resultMessage.code.c_str(), user_token, oid);
	return result;
}


bool sudis::CheckAccountBySmartCard_step1(char* ticketId, char* ticketBody)
{
	if (!ip_sudis || !ip_cryptopro)
		return false;
	dprint4("sudis: TicketV3 start\n");

	char time_buff[128] = {0};
	SYSTEMTIME st;
	GetSystemTime(&st);
	sprintf(time_buff, ISO8601_Z_TIME_FMT, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	char nonce[33] = {0};
	generate_nonce((char*) &nonce[0]);

	MessageIdentifier mi;
	mi.__set_timestamp(time_buff);
	mi.__set_nonce(std::string("nonsa-") + nonce);

	TicketV3Request req;
	req.__set_messageIdentifier(mi);

	TicketV3Response rsp;
	if (!sudis::ProcessRequest<	TicketV3Client,
		TicketV3Request,
		TicketV3Response,
		&TicketV3Client::send_getTicket,
		&TicketV3Client::recv_getTicket>
		(req,rsp,"/dsapi/tbr/getTicket"))
	{
		return false;
	}

	//if (rsp.userTokenId.length())		// dont check:  rsp.resultMessage.code == "SUCCESS" || "USER_PASSWORD_SOON_EXPIRED"
	//{
	//	result = USER_LOGGEDIN_OK;
	//	user_token = rsp.userTokenId;

	//	dprint4("Found %d attributes\n", rsp.attributes.size());
	//	for(std::vector< ::sudis::TCciUserAttributeV1>::iterator it=rsp.attributes.begin(); it!=rsp.attributes.end(); ++it)
	//	{
	//		dprint4("Name %s, vals %d\n",it->name.c_str(), it->values.size());
	//		for(std::vector<std::string>::iterator it2=it->values.begin(); it2!=it->values.end(); ++it2)
	//		{
	//			dprint4("\tval: %s\n",it2->c_str());
	//		}
	//	}
	//}

	// todo(kt): check responce_messIdentifier == request_messIdentifier

	strcpy(ticketId, rsp.ticketId.c_str());

	size_t b64_sz = 0;
	base64_encode(rsp.ticketBody.c_str(), rsp.ticketBody.length(), nullptr, b64_sz);
	auto b64 = vs::make_unique_default_init<char[]>(b64_sz + 1);
	base64_encode(rsp.ticketBody.c_str(), rsp.ticketBody.length(), b64.get(), b64_sz);
	b64[b64_sz] = '\0';
	strcpy(ticketBody, b64.get());

	dprint4("sudis: TicketV3 result:%d, tiketId:%s, base64(ticketBody):%s\n", rsp.result.successful, ticketId, ticketBody);

	return true;
}

bool sudis::CheckAccountBySmartCard_step2(const char* ticketId, const char* signedTicketBody, const unsigned long signedTicketBody_sz, char* oid)
{
	bool result(false);
	if (!ip_sudis || !ip_cryptopro)
		return result;
	dprint4("sudis: cciUserLoginV3 start\n");

	size_t sz = 0;
	base64_decode(signedTicketBody, signedTicketBody_sz, nullptr, sz);
	auto decoded_signedTicketBody = vs::make_unique_default_init<char[]>(sz + 1);
	base64_decode(signedTicketBody, signedTicketBody_sz, decoded_signedTicketBody.get(), sz);
	decoded_signedTicketBody[sz] = '\0';

	char time_buff[128] = {0};
	SYSTEMTIME st;
	GetSystemTime(&st);
	sprintf(time_buff, ISO8601_Z_TIME_FMT, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	char nonce[33] = {0};
	generate_nonce((char*) &nonce[0]);

	MessageIdentifier mi;
	mi.__set_timestamp(time_buff);
	mi.__set_nonce(std::string("nonsa-") + nonce);

	UserLoginV3Request req;
	req.__set_messageIdentifier(mi);
	req.__set_ticketId(ticketId);
	req.__set_signedTicketBody(decoded_signedTicketBody.get());
	req.__set_spCode("svks-m");

	UserLoginV3Response rsp;
	if (!sudis::ProcessRequestNoEncrypt<	UserLoginV3Client,
		UserLoginV3Request,
		UserLoginV3Response,
		&UserLoginV3Client::send_userLoginV3,
		&UserLoginV3Client::recv_userLoginV3>
		(req,rsp,"/dsapi/tbr/cciUserLoginV3"))
	{
		return result;
	}
// todo(kt): temporary commented... uncomment this!!!!!!!!!!!
//	if (rsp.userTokenId.length())		// dont check:  rsp.resultMessage.code == "SUCCESS" || "USER_PASSWORD_SOON_EXPIRED"
	{
//		result = USER_LOGGEDIN_OK;
//		user_token = rsp.userTokenId;

		dprint4("Found %zu attributes\n", rsp.attributes.size());
		for(std::vector<TCciUserAttributeV3>::iterator it=rsp.attributes.begin(); it!=rsp.attributes.end(); ++it)
		{
			dprint4("Name %s, vals %zu\n",it->name.c_str(), it->values.size());
			for(std::vector<std::string>::iterator it2=it->values.begin(); it2!=it->values.end(); ++it2)
			{
				dprint4("\tval: %s\n",it2->c_str());

				if (it->name == "oid")
				{
					strcpy(oid,it2->c_str());
				}
			}
		}
	}

	// todo(kt): check responce_messIdentifier == request_messIdentifier

	dprint4("sudis: UserLoginV3 result:%d, userTokenId:%s, oid:%s\n", rsp.result.successful, rsp.userTokenId.c_str(), oid);
	return result;
}

bool sudis::LogoutUser(const char* userTokenId)
{
	if (!userTokenId||!*userTokenId)
		return false;
	if (!ip_sudis || !ip_cryptopro)
		return false;
	dprint4("sudis: cciUserLogoutV1 start (userTokenId: %s)\n", userTokenId);
	char time_buff[128] = {0};
	SYSTEMTIME st;
	GetSystemTime(&st);
	sprintf(time_buff, ISO8601_Z_TIME_FMT, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	sudis::TCciUserLogoutV1Request req;
	req.requestDateTime = time_buff;
	char nonce[33] = {0};
	generate_nonce((char*) &nonce[0]);
	req.requestNonce = "nonsa-";	req.requestNonce += nonce;
	req.userTokenId = userTokenId;

	sudis::TCciUserLogoutV1Response rsp;
	if (!sudis::ProcessRequest<	sudis::TCciUserLogoutV1Client,
		sudis::TCciUserLogoutV1Request,
		sudis::TCciUserLogoutV1Response,
		&sudis::TCciUserLogoutV1Client::send_cciUserLogoutV1,
		&sudis::TCciUserLogoutV1Client::recv_cciUserLogoutV1>
		(req,rsp,"/dsapi/tbr/cciUserLogoutV1"))
	{
		return false;
	}

	bool result = (rsp.resultMessage.code == "SUCCESS")? true: false;
	dprint4("sudis: cciUserLogoutV1 result:%s, resp_nonce:%s\n", rsp.resultMessage.code.c_str(), rsp.responseNonce.c_str());
	return result;
}

bool sudis::CheckUserRole(const char* call_id, const char* user_token)
{
	if (!call_id||!*call_id || !user_token||!*user_token)
		return false;
	if (!ip_sudis || !ip_cryptopro)
		return false;
	dprint4("sudis: CheckUserRole %s\n", call_id);
	char time_buff[128] = {0};
	SYSTEMTIME st;
	GetSystemTime(&st);
	sprintf(time_buff, ISO8601_Z_TIME_FMT, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	sudis::TCciUserRoleListV1Request req;
	req.userTokenId = user_token;
	req.requestDateTime = time_buff;
	char nonce[33] = {0};
	generate_nonce((char*) &nonce[0]);
	req.requestNonce = "nonsa-";	req.requestNonce += nonce;
	req.__set_spCode("svks-m");


	sudis::TCciUserRoleListV1Response rsp;
	if (!sudis::ProcessRequest<	sudis::TCciUserRoleListV1Client,
		sudis::TCciUserRoleListV1Request,
		sudis::TCciUserRoleListV1Response,
		&sudis::TCciUserRoleListV1Client::send_cciUserRoleListV1,
		&sudis::TCciUserRoleListV1Client::recv_cciUserRoleListV1>
		(req,rsp,"/dsapi/tbr/cciUserRoleListV1"))
	{
		return false;
	}

	dprint4("user role result code %s\n", rsp.resultMessage.code.c_str());
	bool result = (rsp.resultMessage.code == "SUCCESS")? true: false;
	if (result)
	{
		//dprint4("user role size %d\n", rsp.roleList.size());
		//for(std::vector<sudis::TCciRoleV1>::iterator it=rsp.roleList.begin(); it!=rsp.roleList.end(); ++it)
		//{
		//	dprint4("Role spCode:%s, mnem:%s, enabled:%d\n", it->spCode.c_str(), it->mnemonic.c_str(), it->enabled);
		//}
	}
	return result;
}

bool sudis::CheckUserList(const char* call_id)
{
	if (!call_id||!*call_id)
		return false;
	if (!ip_sudis || !ip_cryptopro)
		return false;
	dprint4("sudis: cciUserListV2 %s start\n", call_id);
	char time_buff[128] = {0};
	SYSTEMTIME st;
	GetSystemTime(&st);
	sprintf(time_buff, ISO8601_Z_TIME_FMT, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	sudis::TCciUserListV2Request req;
	req.requestDateTime = time_buff;
	char nonce[33] = {0};
	generate_nonce((char*) &nonce[0]);
	req.requestNonce = "nonsa-";	req.requestNonce += nonce;
	req.__set_login(call_id);


	sudis::TCciUserListV2Response rsp;
	if (!sudis::ProcessRequest<	sudis::TCciUserListV2Client,
		sudis::TCciUserListV2Request,
		sudis::TCciUserListV2Response,
		&sudis::TCciUserListV2Client::send_cciUserListV2,
		&sudis::TCciUserListV2Client::recv_cciUserListV2>
		(req,rsp,"/dsapi/tbr/cciUserListV2"))
	{
		return false;
	}

	dprint4("sudis: cciUserListV2 %s result:%s UserListSize:%zu\n", call_id, rsp.resultMessage.code.c_str(), rsp.userList.size());
	bool result(false);
	if (rsp.resultMessage.code == "SUCCESS" && rsp.userList.size()==1)
	{
		dprint4("sudis: call_id:%s isset:%d isBlocked:%d\n", call_id, rsp.userList[0].__isset.isBlocked, rsp.userList[0].isBlocked);
		if (!rsp.userList[0].__isset.isBlocked ||
			(rsp.userList[0].__isset.isBlocked && rsp.userList[0].isBlocked==false))
			result = true;
	}
	return result;
}

bool sudis::SendRecv(unsigned long ip, unsigned short port, char* in_buff, unsigned long in_sz, char* out_buff, unsigned long& out_sz)
{
	struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = ip; //inet_addr(ip);

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
    {
        return false;
    }
    if (connect(s, (SOCKADDR *)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
		if (s)
			closesocket(s);
        return false;
    }

	int res = send(s, in_buff, in_sz, 0);
	if (res<=0 || res==SOCKET_ERROR)
	{
		if (s)
			closesocket(s);
		return false;
	}
	res = recv(s, out_buff, out_sz, 0);
	if (res>0 && res != SOCKET_ERROR)
		out_sz = res;

	if (s)
		closesocket(s);
	return true;
}

bool sudis::CheckHTTPResponceCode(char* buff_rcv, const unsigned long buff_rcv_sz)
{
	char* ptr = strstr(buff_rcv, " ");
	if (!ptr || !*ptr || (unsigned long)(ptr-buff_rcv)>=buff_rcv_sz)
		return false;
	int ret_code = atoi(ptr);
	if (ret_code!=200)
	{
		dprint4("sudis: http responce code %d\n", ret_code);
		return false;
	}
	return true;
}

bool sudis::LogEvent(const char* user_id, const char* status, const char* ip_address)
{
	bool result(false);
	if (!user_id||!*user_id || !status||!*status)
		return result;
	if (!ip_spsb)
		return result;
	dprint4("sudis: SpsbLogEvent %s\n", user_id);

	spsb::TSpsbEventAttributeValue attr;
	attr.__set_code("status");
	attr.__set_value(status);
	attr.__set_type(spsb::TSpSbAttributeType::TEXT);

	std::vector<spsb::TSpsbEventAttributeValue> attrs;
	attrs.push_back(attr);

	char nonce[33] = {0};
	generate_nonce((char*) &nonce[0]);

	int64_t now = time(0);
	now *= 1000;

	spsb::TSpsbEvent ev;
	ev.__set_eventMillis(now);
	ev.__set_spCode("svks-m");
	ev.__set_userLogin(user_id);
	ev.__set_typeCode("RES_ACCESS");
	ev.__set_resultCode("SUCCESS");
	ev.__set_attributesValues(attrs);
	ev.__set_ipAddress(ip_address);

	std::vector<spsb::TSpsbEvent> v;
	v.push_back(ev);

	spsb::TSpsbLogEventArgs req;
	req.__set_version(spsb::TSpsbLogEventVersion::V1);
	req.__set_requestMillis(now);
	req.__set_requestNonce(nonce);
	req.__set_events(v);

	spsb::TSpsbLogEventResult rsp;
	if (!sudis::ProcessRequestSPSB<	spsb::TServiceExtSpsbClient,
		spsb::TSpsbLogEventArgs,
		spsb::TSpsbLogEventResult,
		&spsb::TServiceExtSpsbClient::send_logEvent,
		&spsb::TServiceExtSpsbClient::recv_logEvent>
		(req,rsp,"/spsb-core/api/ext"))
	{
		return result;
	}

	result = true;
	return result;
}