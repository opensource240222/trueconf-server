/**
 **************************************************************************************
 *
 * (c) 2003 Visicron Inc.  http://www.visicron.net/
 * Project: Common Services
 *
 * \file VS_ServiceHelper.cpp
 * \brief
 * \note
 **************************************************************************************/

#include "VS_TransportRouterServiceHelper.h"

#include <stdlib.h>

const unsigned VS_TransportRouterServiceHelper::default_client_timeout = 30000;
const unsigned  VS_TransportRouterServiceHelper::default_server_timeout = 120000;
const unsigned  VS_TransportRouterServiceHelper::default_timeout = ~0;

bool VS_TransportRouterServiceHelper::PostRequest(const char* to_server, const char *to_user, const VS_Container& cnt,
	const char* add_string,const char* to_service,
	unsigned timeout, const char* from_service, const char* from_user)
{
	void* body;
	size_t bodySize;
	cnt.SerializeAlloc(body, bodySize);
	bool result = PostRequest(to_server, to_user, body, bodySize, add_string, to_service, timeout, from_service, from_user);
	free(body);
	return result;
}
bool VS_TransportRouterServiceHelper::PostRequest(const char *to_server, const char *to_user, const void* body, unsigned long bodySize,
	const char* add_string,const char* to_service,
	unsigned timeout, const char* from_service, const char* from_user)
{
	if (timeout==default_timeout) {
		if (to_user==0 || *to_user==0)
			timeout=default_server_timeout;
		else
			timeout=default_client_timeout;
	}
	VS_RouterMessage* msg = new VS_RouterMessage(
		from_service?from_service:OurService(),
		add_string,
		to_service?to_service:OurService(),
		to_user, from_user,
		to_server, OurEndpoint(),
		timeout, body, bodySize);
	bool result = PostMes(msg);
	if (!result)
		delete msg;
	return result;
}
bool VS_TransportRouterServiceHelper::RequestResponse(const char* to_server, const char* to_user, const VS_Container &cnt,
	ResponseCallBackT &&callback, RequestLifeTimeT &&lifetime,
	const char* add_string, const char* to_service,
	unsigned timeout,
	const char* from_service, const char* from_user)
{
	void* body;
	size_t bodySize;
	cnt.SerializeAlloc(body, bodySize);
	bool result = RequestResponse(to_server, to_user, body, bodySize, std::move(callback),std::move(lifetime), add_string, to_service, timeout, from_service, from_user);
	free(body);
	return result;
}
bool VS_TransportRouterServiceHelper::RequestResponse(const char* to_server, const char* to_user, const void* body, unsigned long bodySize,
	ResponseCallBackT &&callback, RequestLifeTimeT &&lifetime,
	const char* add_string, const char* to_service,
	unsigned timeout,
	const char* from_service, const char* from_user)
{
	if (timeout == default_timeout) {
		if (to_user == 0 || *to_user == 0)
			timeout = default_server_timeout;
		else
			timeout = default_client_timeout;
	}
	VS_RouterMessage* msg = new VS_RouterMessage(
		from_service ? from_service : OurService(),
		add_string,
		to_service ? to_service : OurService(),
		to_user, from_user,
		to_server, OurEndpoint(),
		timeout, body, bodySize);
	bool result = ReqResp(msg,std::move(callback),std::move(lifetime));
	if (!result)
		delete msg;
	return result;
}
ResponseFutureT VS_TransportRouterServiceHelper::RequestResponse(const char* to_server, const char* to_user, const VS_Container &cnt,
	RequestLifeTimeT &&lifetime,
	const char* add_string, const char* to_service,
	unsigned timeout, const char* from_service, const char* from_user)
{
	void* body;
	size_t bodySize;
	cnt.SerializeAlloc(body, bodySize);
	auto result = RequestResponse(to_server, to_user, body, bodySize, std::move(lifetime), add_string, to_service, timeout, from_service, from_user);
	free(body);
	return result;
}
ResponseFutureT VS_TransportRouterServiceHelper::RequestResponse(const char* to_server, const char* to_user, const void* body, unsigned long bodySize,
	RequestLifeTimeT &&lifetime,
	const char* add_string, const char* to_service,
	unsigned timeout,
	const char* from_service, const char* from_user)
{
	if (timeout == default_timeout) {
		if (to_user == 0 || *to_user == 0)
			timeout = default_server_timeout;
		else
			timeout = default_client_timeout;
	}
	VS_RouterMessage* msg = new VS_RouterMessage(
		from_service ? from_service : OurService(),
		add_string,
		to_service ? to_service : OurService(),
		to_user, from_user,
		to_server, OurEndpoint(),
		timeout, body, bodySize);
	auto result = ReqResp(msg, std::move(lifetime));
	if (!result.valid())
		delete msg;
	return result;
}

bool VS_TransportRouterServiceHelper::PostUnauth(const char* cid, VS_Container& cnt,
	const char* add_string,const char* to_service,
	unsigned timeout, const char* from_service)
{
	void* body;
	size_t bodySize;
	cnt.SerializeAlloc(body, bodySize);
	bool result = PostUnauth(cid, body, bodySize, add_string, to_service, timeout, from_service);
	free(body);
	return result;
}

bool VS_TransportRouterServiceHelper::PostUnauth(const char *cid, const void* body, unsigned long bodySize,
	const char* add_string,const char* to_service,
	unsigned timeout, const char* from_service)
{

	VS_RouterMessage* msg = new VS_RouterMessage(
		0,
		from_service?from_service:OurService(),
		add_string,
		cid,
		to_service?to_service:OurService(),
		timeout, body, bodySize, 0, 0, 0, OurEndpoint());
	bool result = PostMes(msg);
	if (!result)
		delete msg;
	return result;
}

void VS_TransportRouterServiceHelper::ConnectServer(const char* server)
{
	VS_Container cnt;
	cnt.AddValue("Connect", static_cast<int32_t>(1));
	PostRequest(server, 0, cnt, 0, 0, 20000);
}

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////
bool VS_TransportRouterServiceReplyHelper::PostReply(const VS_Container& cnt, const char* add_string, unsigned timeout, const VS_RouterMessage* requestMess)
{
	auto recvMess = (requestMess)? requestMess: m_recvMess;
	if (!recvMess)
		return false;
	void* body;
	size_t bodySize;
	cnt.SerializeAlloc(body, bodySize);

	if (timeout==default_timeout) {
		if (recvMess->SrcUser_sv().empty())
			timeout=default_server_timeout;
		else
			timeout=default_client_timeout;
	}
	VS_RouterMessage *reply = new VS_RouterMessage(recvMess, timeout, body, bodySize,add_string);
	bool result = PostMes(reply);
	free(body);

	if(!result)
		delete reply;
	return result;
}