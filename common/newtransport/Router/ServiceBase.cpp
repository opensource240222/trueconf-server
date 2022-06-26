#include "ServiceBase.h"
#include "std-generic/cpplib/VS_Container.h"

namespace transport {

void ServiceBase::FillMonitorStruct(Monitor::TmReply::Service &service)
{
	//service.service_type = TM_TYPE_PERIODIC_SERVICE;
	service.service_name = std::string(GetName());
	service.send_stats = m_send_stats;
	service.recv_stats = m_recv_stats;
}

const std::string& ServiceBase::OurEndpoint() const
{
	assert(m_router);
	return m_router->EndpointName();
}

void ServiceBase::PostRequest(string_view to_server, string_view to_user, const VS_Container& cnt,
		string_view add_string, string_view to_service,
		unsigned timeout, string_view from_service, string_view from_user)
{
	assert(m_router);
	m_router->PostMessage(Message::Make()
		.DstServer(to_server)
		.DstUser(to_user)
		.DstService(!to_service.empty() ? to_service : GetName())
		.SrcServer(OurEndpoint())
		.SrcUser(from_user)
		.SrcService(!from_service.empty() ? from_service : GetName())
		.Body(cnt)
		.AddString(add_string));
}

void ServiceBase::PostUnauth(string_view cid, const VS_Container& cnt,
	string_view add_string, string_view to_service,
	unsigned timeout, string_view from_service, string_view from_user)
{
	assert(m_router);
	m_router->PostMessage(Message::Make()
		.DstServer(OurEndpoint())
		.DstCID(cid)
		.DstService(!to_service.empty() ? to_service : GetName())
		.SrcServer(OurEndpoint())
		.SrcService(!from_service.empty() ? from_service : GetName())
		.Body(cnt)
		.AddString(add_string));
}

void ServiceBase::PostReply(const Message& msg, const VS_Container& cnt)
{
	assert(m_router);
	m_router->PostMessage(Message::Make()
		.ReplyTo(msg)
		.SrcServer(OurEndpoint())
		.Body(cnt));
}

}
