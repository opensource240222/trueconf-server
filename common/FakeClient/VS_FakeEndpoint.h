#pragma once

#include "transport/fwd.h"
#include "std-generic/cpplib/string_view.h"

#include <memory>
#include <string>

class VS_IPPortAddress;
namespace transport { class Router; }
struct VS_TransportRouter_SetConnection;

class VS_FakeEndpoint
{
public:
	class Receiver
	{
	public:
		// These functions will be called by VS_FakeEndpoint implementaions in a serialized manner.
		virtual void OnReceive(const transport::Message& message) = 0;
		virtual void OnError(unsigned error) = 0;
		virtual void Timeout() = 0;
	};

	virtual ~VS_FakeEndpoint() {}
	virtual void SetReceiver(std::weak_ptr<Receiver> receiver) = 0;
	virtual void Stop() = 0;
	virtual const std::string& CID() const = 0;
	virtual bool Send(transport::Message&& message) = 0;
};

class VS_FakeEndpointFactory
{
public:
	static void InitV1(VS_TransportRouter_SetConnection* router, string_view our_endpoint);
	static void InitV2(const std::shared_ptr<transport::Router>& router);
	static void DeInit();
	static VS_FakeEndpointFactory& Instance()
	{
		return *s_instance;
	}

public:
	virtual ~VS_FakeEndpointFactory() {}
	virtual void Stop() = 0;
	virtual std::unique_ptr<VS_FakeEndpoint> Create() = 0;
#if defined(_WIN32)
	virtual std::unique_ptr<VS_FakeEndpoint> Create(const VS_IPPortAddress& bind_addr, const VS_IPPortAddress& peer_addr) = 0;
#endif

private:
	static std::unique_ptr<VS_FakeEndpointFactory> s_instance;
};
