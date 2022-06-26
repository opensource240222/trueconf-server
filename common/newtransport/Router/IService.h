#pragma once

#include "Monitor.h"
#include "std-generic/cpplib/string_view.h"

#include <memory>

namespace transport
{
class Message;

enum class EndpointConnectReason : int
{
	hs_error = -2,
	timeout = -1,
	unknown = 0,
	requested = 1,
	incoming = 2,
};

class IService
{
public:
	virtual ~IService() {};

	/*
				virtual void Post(const Message& message) = 0;
				virtual void Send(const Message& message) = 0;
	*/
	virtual bool ProcessMessage(Message&& message) = 0;
	virtual string_view GetName() = 0;
	virtual void FillMonitorStruct(Monitor::TmReply::Service &tmreply_service) = 0;

	virtual void OnEndpointConnect(bool /*is_client_ep*/, EndpointConnectReason /*reason*/, string_view /*cid*/, string_view /*uid*/) {}
	virtual void OnEndpointDisconnect(bool /*is_client_ep*/, EndpointConnectReason /*reason*/, string_view /*cid*/, string_view /*uid*/) {}
	virtual void OnEndpointIP(string_view /*uid*/, string_view /*ip*/) {}
};
}

