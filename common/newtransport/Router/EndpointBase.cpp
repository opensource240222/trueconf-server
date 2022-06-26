#include "EndpointBase.h"
#include "Router.h"
#include "transport/Message.h"

namespace transport {

EndpointBase::EndpointBase(std::weak_ptr<Router> router, string_view endpoint_id)
	: m_router(std::move(router))
	, m_id(endpoint_id)
	, m_hops(0)
	, m_authorized(false)
{
}

EndpointBase::~EndpointBase() = default;

string_view EndpointBase::GetId()
{
	return m_id;
}

string_view EndpointBase::GetUserId()
{
	return m_user_id;
}

void EndpointBase::SetUserId(string_view user_id)
{
	m_user_id = std::string(user_id);
}

void EndpointBase::Authorize(string_view id)
{
	SetUserId(id);
	m_authorized = true;
}

void EndpointBase::Unauthorize()
{
	SetUserId({});
	m_authorized = false;
}

bool EndpointBase::IsAuthorized()
{
	return m_authorized;
}

uint8_t EndpointBase::GetHops()
{
	return m_hops;
}

void EndpointBase::PreprocessMessage(Message& message)
{
	auto msg_proxy = Message::Make();
	msg_proxy.Copy(message).DstCID({});
	if (m_authorized)
	{
		msg_proxy.SrcCID({});
		msg_proxy.DstCID({});
		auto r = m_router.lock();
		if (!m_hops && r) {	// from auth user
			msg_proxy.SrcUser(m_user_id);

			auto dst_server = message.DstServer_sv();
			auto dst_user = message.DstUser_sv();

			if (!dst_server.empty() && !dst_user.empty()) {
				if (dst_server != r->EndpointName()) { // !IsOurEndpointName
					msg_proxy.SrcServer(r->EndpointName());
				}
			}
			else {
				msg_proxy.SrcServer(r->EndpointName());
				if (dst_user.empty() && dst_server.empty()) {
					msg_proxy.DstServer(r->EndpointName());
				}
			}
		} else {			// from auth server
			auto src_server = message.SrcServer_sv();
			if (src_server.empty()) // support new transport managed messages from v9
			{
				msg_proxy.SrcServer(m_id);
			}
		}
	} else {
		msg_proxy.SrcCID(m_id);
		msg_proxy.SrcUser({});
		msg_proxy.DstUser({});
		msg_proxy.DstCID({});

		if (!m_hops) {	// from unauth client
			msg_proxy.SrcServer({});
			msg_proxy.DstServer({});
		} else {		// from unauth server
		}
	}
	message = msg_proxy;
}

}
