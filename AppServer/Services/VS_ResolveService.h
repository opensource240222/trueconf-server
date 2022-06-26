#ifndef VS_RESOLVE_SERVICE_H
#define VS_RESOLVE_SERVICE_H

#include "ServerServices/VS_SubscriptionHub.h"
#include "AppServer/Services/VS_PresenceService.h"
#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "std/cpplib/VS_Map.h"

class VS_ResolveService :
	public VS_TransportRouterServiceReplyHelper,
	public VS_SubscriptionHub::Listener
	, public VS_PresenceServiceMember
{
	VS_Map			m_waiters;
	boost::shared_ptr<VS_ConfRestrictInterface> m_confRestriction;
public:
	VS_ResolveService();
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	void AsyncDestroy() override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	void RegAtPresSRV();

	// VS_SubscriptionHub::Listener
	void OnServerChange(const char* call_id, const char* server_id) override;

	void SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict);
	void AddUsernameAlloc(VS_SimpleStr& bs, VS_RouterMessage* in, VS_RouterMessage* &out);
	void ProcessMessList(bool check_lifetime=true);
};

#endif /*VS_RESOLVE_SERVICE_H*/