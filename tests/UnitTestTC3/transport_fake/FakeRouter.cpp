#ifdef _WIN32
#include "FakeRouter.h"
#include "std-generic/cpplib/string_view.h"
#include "../../../common/tools/Server/VS_Server.h"
#include "../../../common/transport/Lib/VS_TransportLib.h"
#include "transport/Router/VS_TransportRouterServiceBase.h"

extern std::string g_tr_endpoint_name;

namespace tc3_test
{
void FakeRouter::RegisterEndpoint(const std::string &id, const ProcessingFuncT& processingFunc, const OnPointDisconnectFuncT &on_point_disc_func)
{
	msg_dispatcher_[id] = std::make_pair(processingFunc, on_point_disc_func);
}
void FakeRouter::UnregisterEndpoint(const std::string &id)
{
	msg_dispatcher_.erase(id);
}
void FakeRouter::SendMsg(VS_RouterMessage* msg)
{
	msg_queue_.push(std::move(msg));
	if (processing_recursive_depth_>0)
		return;
	ProcessQueue();
}
void FakeRouter::ProcessQueue()
{
	if (msg_queue_.empty())
		return;
	++processing_recursive_depth_;
	do
	{
		auto msg = std::move(msg_queue_.front());
		auto dst_user = msg->DstUser();
		auto dst_server = msg->DstServer();
		auto it = msg_dispatcher_.find(!!dst_user&&!!*dst_user ? dst_user : dst_server);
		std::pair<std::string, decltype(it->second)> item;
		if (it != msg_dispatcher_.end())
			item = *it;
		msg_queue_.pop();
		if (!item.first.empty()&& !SkipMsg(msg) && !try_external_processing_(msg))
			item.second.first(std::move(msg));
	} while (!msg_queue_.empty());
	--processing_recursive_depth_;

}
bool FakeRouter::IsEndpointExist(const char *ep_name)
{
	return msg_dispatcher_.find(ep_name) != msg_dispatcher_.end();
}
bool FakeRouter::DisconnectEndpoint(const char *ep)
{
	auto ep_iter = msg_dispatcher_.find(g_tr_endpoint_name);
	if (ep_iter == msg_dispatcher_.end())
		return true;
	VS_PointParams prm;
	prm.uid = ep;
	prm.cid = ep;
	prm.reazon = VS_PointParams::CR_INCOMING;
	prm.type = VS_PointParams::PT_SERVER;
	prm.ver = VS_CURRENT_TRANSPORT_VERSION | VS_SSL_SUPPORT_BITMASK;;
	ep_iter->second.second(&prm);
	return true;
}
bool FakeRouter::SkipMsg(const VS_RouterMessage*msg) const
{
	return check_for_skip_(msg);
}
void FakeRouter::ChangeCurrentEndpoint(const char *current_ep)
{
	if (current_ep) g_tr_endpoint_name = current_ep;
}
}
#endif