#pragma once

#include "net/QoS.h"
#include "VS_IPPortAddress.h"

class VS_QoSEnabledSocketInterface {
public:
	virtual ~VS_QoSEnabledSocketInterface() {};

	virtual void SetQoSFlow(const net::QoSFlowSharedPtr &flow) = 0;
	virtual net::QoSFlowSharedPtr GetQoSFlow(void) = 0;

	// Mostly for UDP
	virtual bool AddExtraFlow(const net::QoSFlowSharedPtr &flow, const VS_IPPortAddress &addr) { return false; };
	virtual bool RemoveExtraFlow(const net::QoSFlowSharedPtr &flow) { return false; };
};

#ifdef _WIN32
class VS_QoSEnabledSocket : public VS_QoSEnabledSocketInterface {
public:
	VS_QoSEnabledSocket();
	~VS_QoSEnabledSocket();

	virtual uintptr_t GetSocketHandle() = 0;
	virtual bool IsConnected(void) = 0;

	// net::QoSEnabledSocketInerface
	void SetQoSFlow(const net::QoSFlowSharedPtr &flow) override;
	net::QoSFlowSharedPtr GetQoSFlow(void) override;

	bool AddExtraFlow(const net::QoSFlowSharedPtr &flow, const VS_IPPortAddress &addr) override;
	bool RemoveExtraFlow(const net::QoSFlowSharedPtr &flow) override;
protected:
	bool AddSocketToFlow(const void *dst_sockaddr);
	bool RemoveSocketFromFlow(void);

	void ClearExtraFlows(void);
private:
	VS_QoSEnabledSocket(const VS_QoSEnabledSocket &) = delete;
	VS_QoSEnabledSocket&operator=(const VS_QoSEnabledSocket &) = delete;
private:
	net::QoSFlowSharedPtr m_flow;
	uintptr_t m_socket;

	std::vector<net::QoSFlowSharedPtr> m_extra_flows;
};

#endif // _WIN32
