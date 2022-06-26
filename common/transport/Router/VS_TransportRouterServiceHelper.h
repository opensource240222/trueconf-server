/**
**************************************************************************************
*
* (c) 2003 Visicron Inc.  http://www.visicron.net/
* Project: Common Services
*
* \file VS_ServiceHelper.h
* \brief
* \note
**************************************************************************************/

#ifndef VS_SERVICE_HELPER_H
#define VS_SERVICE_HELPER_H

#include "VS_TransportRouterServiceBase.h"
#include "VS_RouterMessage.h"
#include "../typedefs.h"
#include "std-generic/cpplib/VS_Container.h"

class VS_TransportRouterServiceHelper : public virtual VS_TransportRouterServiceBase
{
public:
	static const unsigned default_client_timeout;
	static const unsigned default_server_timeout;
	static const unsigned default_timeout;

	virtual bool PostRequest(const char* to_server, const char* to_user, const VS_Container &cnt,
		const char* add_string=0,const char* to_service=0,
		unsigned timeout=default_timeout,
		const char* from_service=0, const char* from_user=0);
	bool PostRequest(const char* to_server, const char* to_user, const void* body, unsigned long bodySize,
		const char* add_string=0,const char* to_service=0,
		unsigned timeout=default_timeout,
		const char* from_service=0, const char* from_user=0);

	bool RequestResponse(const char* to_server, const char* to_user, const VS_Container &cnt,
		ResponseCallBackT &&callback, RequestLifeTimeT &&lifetime,
		const char* add_string = 0, const char* to_service = 0,
		unsigned timeout = default_timeout,
		const char* from_service = 0, const char* from_user = 0);

	bool RequestResponse(const char* to_server, const char* to_user, const void* body, unsigned long bodySize,
		ResponseCallBackT &&callback, RequestLifeTimeT &&lifetime,
		const char* add_string = 0, const char* to_service = 0,
		unsigned timeout = default_timeout,
		const char* from_service = 0, const char* from_user = 0);

	ResponseFutureT RequestResponse(const char* to_server, const char* to_user, const VS_Container &cnt,
		RequestLifeTimeT &&lifetime,
		const char* add_string = 0, const char* to_service = 0,
		unsigned timeout = default_timeout,
		const char* from_service = 0, const char* from_user = 0);

	ResponseFutureT RequestResponse(const char* to_server, const char* to_user, const void* body, unsigned long bodySize,
		RequestLifeTimeT &&lifetime,
		const char* add_string = 0, const char* to_service = 0,
		unsigned timeout = default_timeout,
		const char* from_service = 0, const char* from_user = 0);

	bool PostUnauth(const char* cid, VS_Container &cnt,
		const char* add_string=0,const char* to_service=0,
		unsigned timeout=default_client_timeout,
		const char* from_service=0);
	bool PostUnauth(const char *cid, const void* body, unsigned long bodySize,
		const char* add_string=0,const char* to_service=0,
		unsigned timeout=default_client_timeout,
		const char* from_service=0);
	void ConnectServer(const char* server);
};
// end VS_ServiceHelper class

class VS_TransportRouterServiceReplyHelper : public virtual VS_TransportRouterServiceHelper
{
public:
	VS_TransportRouterServiceReplyHelper(): m_recvMess(nullptr) {}

	const VS_RouterMessage	*m_recvMess;

	bool PostReply(const VS_Container& cnt, const char* add_string = 0,
		unsigned timeout = default_timeout, const VS_RouterMessage* requestMess = nullptr);
};

#endif // VS_SERVICE_HELPER_H
