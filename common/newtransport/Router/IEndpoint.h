#pragma once

#include "Monitor.h"
#include "std-generic/cpplib/string_view.h"

#include <cstdint>
#include <string>

namespace transport {

class Message;

class IEndpoint
{
public:
	virtual ~IEndpoint() {};

	virtual void Close() = 0;
	virtual void Shutdown() = 0;

	virtual void ProcessMessage(const Message& message) = 0;
	virtual void SendToPeer(const Message& message) = 0;
	virtual void SendPing() = 0;
	virtual void SendDisconnect() = 0;

	virtual string_view GetId() = 0;
	virtual string_view GetUserId() = 0;
	virtual void SetUserId(string_view user_id) = 0;
	virtual void Authorize(string_view id = {}) = 0;
	virtual void Unauthorize() = 0;
	virtual bool IsAuthorized() = 0;
	virtual uint8_t GetHops() = 0;
	virtual std::string GetRemoteIp() = 0;

	virtual void FillMonitorStruct(Monitor::TmReply::Endpoint &tmreply_endpoint) = 0;
};

}
