#pragma once

#include "IService.h"
#include "Monitor.h"
#include "Router.h"

namespace transport
{
class ServiceBase : public IService
{
protected:
	std::shared_ptr<Router> m_router;

public:
	void SetTransportRouter(const std::shared_ptr<Router>& router)
	{
		m_router = router;
	}

	void FillMonitorStruct(Monitor::TmReply::Service &service) override;

	const std::string& OurEndpoint() const;
	void PostRequest(string_view to_server, string_view to_user, const VS_Container& cnt,
		string_view add_string = {}, string_view to_service = {},
		unsigned timeout=0 /* todo(kt): default_timeout */, string_view from_service = {}, string_view from_user = {});
	void PostUnauth(string_view cid, const VS_Container& cnt,
		string_view add_string = {}, string_view to_service = {},
		unsigned timeout = 0 /* todo(kt): default_timeout */, string_view from_service = {}, string_view from_user = {});
	void PostReply(const Message& msg, const VS_Container& cnt);

protected:
	MessageStats m_send_stats, m_recv_stats;

};
}
