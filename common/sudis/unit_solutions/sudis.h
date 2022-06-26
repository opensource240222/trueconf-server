#pragma once

#include <vector>

namespace sudis{
class TCciUserLoginV1Request;
class TCciUserLoginV1Response;
class TCciUserRoleListV1Request;
class TCciUserRoleListV1Response;
class TCciUserListV1Request;
class TCciUserListV1Response;
class TCciUserLogoutV1Request;
class TCciUserLogoutV1Response;

typedef std::string ticketId;			// string returned by TicketV3Responce
typedef std::string ticketBody;			// binary returned by TicketV3Responce
typedef std::string signedTicketBody;	// binary, that should be signed by CVC and send to server to UserLoginV3 on sudis
typedef std::string oid;				// returned as user attributes on login at sudis; example "51bb128df27369bea00001cf"


// private methods
	template<class TClient, class TRequest, class TResponce, void (TClient::*ptr_req)(const TRequest&), void (TClient::*ptr_rsp)(TResponce&)>
	bool ProcessRequest(const TRequest& req, TResponce& rsp, const char* request_url);

	template<class TClient, class TRequest, class TResponce, void (TClient::*ptr_req)(const TRequest&), void (TClient::*ptr_rsp)(TResponce&)>
	bool ProcessRequestNoEncrypt(const TRequest& req, TResponce& rsp, const char* request_url);

	template<class TClient, class TRequest, class TResponce, void (TClient::*ptr_req)(const TRequest&), void (TClient::*ptr_rsp)(TResponce&)>
	bool ProcessRequestSPSB(const TRequest& req, TResponce& rsp, const char* request_url);

	template<class T, class Arg, void (T::*ptr)(const Arg&)>	bool Serialize(const Arg& req, std::vector<unsigned char> &v);
	template<class T, class Arg, void (T::*ptr)(Arg&)>			bool Deserialize(const std::vector<unsigned char> &v, Arg& rsp);

	bool SendRecv(unsigned long ip, unsigned short port, char* in_buff, unsigned long in_sz, char* out_buff, unsigned long& out_sz);
	bool CheckHTTPResponceCode(char* buff_rcv, const unsigned long buff_rcv_sz);

}


#ifdef __cplusplus
extern "C" {
#endif

namespace sudis
{
	// internal DLL func
	bool __declspec( dllexport ) InitWSA();
	void generate_nonce(char* str);

	void __declspec( dllexport ) SetSudisAddress(const char* ip, const unsigned short port);
	void __declspec( dllexport ) SetCryptoProAddress(const char* ip, const unsigned short port);
	void __declspec( dllexport ) SetSPSBAddress(const char* ip, const unsigned short port, const char* uuid);


	bool __declspec( dllexport ) CheckAccount(const char* call_id, const char* pwd, char* user_token, char* oid);
	bool __declspec( dllexport ) LogoutUser(const char* userTokenId);
	bool CheckUserRole(const char* call_id, const char* user_token);
	bool __declspec( dllexport ) CheckUserList(const char* call_id);


	bool __declspec( dllexport ) CheckAccountBySmartCard_step1(char* ticketId, char* ticketBody);
	bool __declspec( dllexport ) CheckAccountBySmartCard_step2(const char* ticketId, const char* signedTicketBody, const unsigned long signedTicketBody_sz, char* oid);
	bool __declspec( dllexport ) LogEvent(const char* user_id, const char* status, const char* ip_address);
}

#ifdef __cplusplus
}
#endif
