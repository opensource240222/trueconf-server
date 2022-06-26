#pragma once

#include <functional>

#include "std/cpplib/json/elements.h"
#include "std/cpplib/enable_shared_from_this_virtual_std.h"
#include "net/Address.h"

class VS_JsonClient;

#include "../acs/Lib/VS_IPPortAddress.h"

class VS_AbstractJsonConnection : public enable_shared_from_this_virtual<VS_AbstractJsonConnection>
{
public:
	VS_AbstractJsonConnection();
	virtual ~VS_AbstractJsonConnection(void);

protected:
	std::string m_CID;
#ifdef _WIN32
	void PostConstruct(std::string remote_ip,
		const VS_IPPortAddress &bindAddr,
		const VS_IPPortAddress &peerAddr,
		std::weak_ptr<void> _this);
#endif
	void PostConstruct(const net::address& remote_ip, std::weak_ptr<void> _this);

	void ProcessRequest(const std::string& message);
	void ProcessRequest(const char* message, unsigned int size)
	{
		ProcessRequest(std::string(message, size));
	}

	virtual bool SendResponse(const char *data) = 0;
	void onError(unsigned err);

private:
	bool ProcessResponse(const json::Object& resp, std::shared_ptr<VS_JsonClient> c);
	std::function<bool(const json::Object &resp)> MakeJSONClientCallBack(void);

	std::string m_remote_ip;
#ifdef _WIN32
	VS_IPPortAddress m_bind_addr, m_peer_addr;
#endif
	std::weak_ptr<void> m_this;
	std::shared_ptr<VS_JsonClient> m_json_client;
};
