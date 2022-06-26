#pragma once
#include "../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../common/std/cpplib/VS_Lock.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../common/std/cpplib/json/reader.h"
#include "../common/ldap_core/common/Common.h"

class VS_ConfRestrictInterface;

class VS_ServerConfiguratorService :
	VS_Lock,
	public VS_TransportRouterServiceHelper
{
private:
	boost::shared_ptr<VS_ConfRestrictInterface> m_confRestrict;

	bool SendConfiguratorCommand(const char *buf, uint32_t buf_len);
	bool SendCommandToService(const std::string &dst_srv, VS_Container &cnt);
	bool JsonToContainer(json::Object &src, VS_Container &dst);
public:
	VS_ServerConfiguratorService();
	virtual ~VS_ServerConfiguratorService();
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	const VS_PushDataSignalSlot GetPushDataSignalSlot();
	void SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict);
};

