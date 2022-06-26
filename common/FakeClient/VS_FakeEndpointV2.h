#pragma once

#include "FakeClient/VS_FakeEndpoint.h"

class VS_FakeEndpointV2 : public VS_FakeEndpoint
{
public:
	explicit VS_FakeEndpointV2(const std::shared_ptr<transport::Router>& router);
	~VS_FakeEndpointV2();

	void SetReceiver(std::weak_ptr<Receiver> receiver) override;
	void Stop() override;
	const std::string& CID() const override;
	bool Send(transport::Message&& message) override;

private:
	class RouterEndpoint;
	std::shared_ptr<RouterEndpoint> m_ep;
};

class VS_FakeEndpointFactoryV2 : public VS_FakeEndpointFactory
{
public:
	explicit VS_FakeEndpointFactoryV2(const std::shared_ptr<transport::Router>& router);

	void Stop() override;
	std::unique_ptr<VS_FakeEndpoint> Create() override;
#if defined(_WIN32)
	std::unique_ptr<VS_FakeEndpoint> Create(const VS_IPPortAddress& bind_addr, const VS_IPPortAddress& peer_addr) override;
#endif

private:
	std::weak_ptr<transport::Router> m_router;
};
