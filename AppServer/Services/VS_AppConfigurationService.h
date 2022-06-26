#ifndef VS_APPCONFIGURATION_SERVICE_H
#define VS_APPCONFIGURATION_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "std-generic/cpplib/VS_Container.h"
#include "transport/VS_SystemMessage.h"
#include "../../ServerServices/Common.h"
#include "VS_AppServerData.h"
#include "ldap_core/common/Common.h"

class VS_AppConfigurationService :
	public VS_TransportRouterServiceReplyHelper,
	public VS_SystemMessage
{
	VS_AppPropertiesMap	m_prop_map;

	// Service implementation
	void UpdateConfiguration_Method(VS_Container &cnt);
	void GetAppProperties_Method(VS_Container &cnt);
	void SetEpProperties_Method(VS_Container &cnt, std::unique_ptr<VS_RouterMessage> &&m);
	void SetProperties_Method(VS_Container &cnt);
public:
	VS_AppConfigurationService(void) { }
	virtual ~VS_AppConfigurationService(void);
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;
};

#endif
