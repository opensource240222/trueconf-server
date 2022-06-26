#ifdef _WIN32
#include "Endpoint.h"
#include "FakeRouter.h"
#include "TransportRouterService_Fakes.h"
#include "../std/cpplib/VS_Protocol.h"

namespace tc3_test
{
Endpoint::Endpoint(const std::string &name, const std::shared_ptr<FakeRouter>&router) :endpoint_name_(name), router_(router)
{
	auto processing_func = [&](VS_RouterMessage*m)
	{
		Processing(m);
	};
	auto on_point_disconnect_func = [&](const VS_PointParams *p)
	{
		for (auto &i : services_)
		{
			auto point_cond = std::get<0>(i.second);
			if (point_cond)
				point_cond->OnPointDisconnected_Event(p);
		}
	};
	router_->RegisterEndpoint(endpoint_name_,processing_func,on_point_disconnect_func);
}
void Endpoint::Processing(VS_RouterMessage*msg)
{
	std::unique_ptr<VS_RouterMessage> m(msg);

	VS_Container cnt;
	cnt.Deserialize(msg->Body(), msg->BodySize());
	std::string method = !!cnt.GetStrValueRef(METHOD_PARAM) ? cnt.GetStrValueRef(METHOD_PARAM) : "";
	msg_trace_.emplace_back(m->DstService(), method, cnt, m->SrcServer(), m->SrcUser(), m->SrcService());

	auto service = services_.find(m->DstService());
	if (services_.end() == service)
		return;
	FakeRouter::ChangeCurrentEndpoint(endpoint_name_.c_str());
	std::get<1>(service->second)->ForwardMsg(m.release());
	std::get<0>(service->second)->Thread();
}
void Endpoint::RegisterService(const std::string &name, const boost::shared_ptr<VS_TransportRouterServiceBase> &srv, TransportRouterService_fake *impl)
{
	services_.insert({ name, std::make_tuple( srv, impl )});
}
void Endpoint::GetMsgTrace(std::vector<MsgTraceType>&out)
{
	out = msg_trace_;
}

void Endpoint::UnregisterService(const std::string &name)
{
	services_.erase(name);
}
void Endpoint::OnPointConnected(const std::string&ep_name)
{
	if (ep_name == endpoint_name_)
		return;
	FakeRouter::ChangeCurrentEndpoint(endpoint_name_.c_str());
	VS_PointParams prm;
	prm.uid = ep_name.c_str();
	prm.type = VS_PointParams::Point_Type::PT_SERVER;
	prm.reazon = VS_PointParams::Condition_Reason::CR_INCOMING;
	for (auto &i : services_)
	{
		auto ep_cond = std::get<0>(i.second);
		if (ep_cond)
			ep_cond->OnPointConnected_Event(&prm);
	}
}
void Endpoint::OnPointDisconnected(const std::string&ep_name)
{
	if (ep_name == endpoint_name_)
		return;
	FakeRouter::ChangeCurrentEndpoint(endpoint_name_.c_str());
	VS_PointParams prm;
	prm.uid = ep_name.c_str();
	prm.type = VS_PointParams::Point_Type::PT_SERVER;
	prm.reazon = VS_PointParams::Condition_Reason::CR_UNKNOWN;
	for (auto &i : services_)
	{
		auto ep_cond = std::get<0>(i.second);
		if (ep_cond)
			ep_cond->OnPointDisconnected_Event(&prm);
	}
}

ClientEndpoint::ClientEndpoint(const char *user_name, const std::shared_ptr<FakeRouter> &r) :router_(r), name_(user_name)
{
	router_->RegisterEndpoint(name_, [this](VS_RouterMessage*m)
	{
		Processing(m);
	},
	[](const VS_PointParams*)
	{});
}
void ClientEndpoint::Processing(VS_RouterMessage*m)
{

}
}
#endif