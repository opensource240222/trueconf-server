#pragma once

#include "FakeClient/VS_FakeEndpoint.h"

#include <boost/shared_ptr.hpp>

class VS_WorkThread;

class VS_FakeEndpointV1 : public VS_FakeEndpoint
{
public:
	~VS_FakeEndpointV1();

	bool Init(const boost::shared_ptr<VS_WorkThread>& thread, VS_TransportRouter_SetConnection* router, const char* our_endpoint, const VS_IPPortAddress& bind_addr, const VS_IPPortAddress& peer_addr);

	void SetReceiver(std::weak_ptr<Receiver> receiver) override;
	void Stop() override;
	const std::string& CID() const override;
	bool Send(transport::Message&& message) override;

private:
	class Connection;
	std::shared_ptr<Connection> m_connection;
};

class VS_FakeEndpointFactoryV1 : public VS_FakeEndpointFactory
{
public:
	VS_FakeEndpointFactoryV1(VS_TransportRouter_SetConnection* router, string_view our_endpoint);
	~VS_FakeEndpointFactoryV1();

	void Stop() override;
	std::unique_ptr<VS_FakeEndpoint> Create() override;
	std::unique_ptr<VS_FakeEndpoint> Create(const VS_IPPortAddress& bind_addr, const VS_IPPortAddress& peer_addr)	override;


private:
	VS_TransportRouter_SetConnection* m_router;
	std::string m_our_endpoint;
	boost::shared_ptr<VS_WorkThread> m_thread;
};
