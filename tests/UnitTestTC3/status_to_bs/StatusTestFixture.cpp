#ifdef _WIN32
#include "StatusTestFixture.h"

#include "AppServer/Services/VS_PresenceService.h"
#include "BaseServer/Services/storage/VS_LocatorStorage.h"
#include "BaseServer/Services/storage/VS_LocatorStorageImplMemory.h"
#include "RoutingServer/Services/VS_RoutingService.h"
#include "tests/UnitTestTC3/dummies/ConfRestrictEmpty.h"
#include "tests/UnitTestTC3/transport_fake/PoolThreadsTask_fake.h"
#include "tests/UnitTestTC3/transport_fake/ThreadPoolService_fake.h"
#include "tests/UnitTestTC3/transport_fake/TransportRouterService_Fakes.h"

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

namespace tc3_test
{
class ResolveTask_Test :public PoolThreadsTask_fake
{
	StatusTestFixture::ResolveResult_T &res_ref_;
	boost::shared_ptr<VS_PresenceService> presence_;
	std::string who_;
	bool use_cache_;
	void Run() override
	{
		VS_CallIDInfo ci;
		VS_SimpleStr real_id = who_.c_str();
		FakeRouter::ChangeCurrentEndpoint(OurEndpoint());
		auto st = presence_->Resolve(real_id, ci, use_cache_, nullptr);
		res_ref_ = std::make_tuple(std::string(real_id), std::move(ci));
	}
public:
	ResolveTask_Test(
		const std::string &who,
		const decltype(presence_)& presence,
		const bool use_cache, StatusTestFixture::ResolveResult_T&res)
		: res_ref_(res)
		, presence_(presence)
		, who_(who)
		, use_cache_(use_cache)
	{}
};
class ResolveAllTaskSync_Task : public PoolThreadsTask_fake
{
	boost::shared_ptr<VS_BasePresenceService> presence_;
	std::list<VS_CallIDInfo> &res_ref_;
	std::list<std::string> ids_;
	void Run() override
	{
		res_ref_ = presence_->ResolveAllSync(std::move(ids_), this);
	}
public:
	ResolveAllTaskSync_Task(
		std::list<std::string> &&ids,
		const decltype(presence_)& presence,
		std::list<VS_CallIDInfo> & res)
		: res_ref_(res)
		, presence_(presence)
		, ids_(std::move(ids))
	{
	}
};

std::pair<bool, VS_CallIDInfo>
StatusTestFixture::BasePresenceServiceWrap::GetCallIDInfoFromCache(
	string_view call_id) const
{
	auto info = m_statusCache[std::string(call_id).c_str()];
	if (!info)
		return { false, VS_CallIDInfo() };
	return { true, std::move(info) };
}
StatusTestFixture::StatusTestFixture()
{
	VS_ResolveServerFinder::SetResolverByDomainFunc(
		[this](const std::string&domain, std::string&server_name) -> bool
		{
			return ResolveByDomain(domain, server_name);
		});
	auto impl = std::make_unique<VS_LocatorStorageImplMemory>();
	locator_storage_impl_ = impl.get();
	VS_LocatorStorage::Instance(std::make_unique<VS_LocatorStorageImplMemory>()).ReInit(std::move(impl));
	OfflineStatusCache::SetWorkMode(OfflineStatusCache::WorkMode::sync);
}
void StatusTestFixture::TearDown()
{
	VS_ResolveServerFinder::Release();
	users_.clear();
	AS_.clear();
	BS_.clear();
	RS_.clear();
	router_.reset();
	ASSERT_EQ(TransportRouterCallService_Fake::cout_of_instanses_, 0);
	ASSERT_EQ(TransportRouterService_fake::cout_of_instanses_, 0);
}
bool StatusTestFixture::ResolveByDomain(const std::string &domain, std::string& server_name)
{
	auto it = servers_for_resolve_by_domain_.find(domain);
	if (it == servers_for_resolve_by_domain_.end())
		return false;
	server_name = it->second;
	return true;
}
void StatusTestFixture::SetResolveServerForDomain(const std::string &domain, const std::string& server)
{
	servers_for_resolve_by_domain_[domain] = server;
}
void StatusTestFixture::AddUser(const std::string &call_id, const std::list<std::string> &aliases,
		const std::string& display_name, const std::string& cid,
		const std::string& as_name, const std::string&  homeBs,
		const FakeRouter::ProcessingFuncT &processing_func)
{
	auto& state_user_descr = statuses_by_users_.emplace(
			call_id, StateOfUser()).first->second;
	auto user_processing = [processing_func](VS_RouterMessage*m,
			decltype(state_user_descr.trace_)& trace,
			StatusCacheT& out_status)
	{
		if (processing_func)
			processing_func(m);
		VS_Container cnt;
		cnt.Deserialize(m->Body(), m->BodySize());
		std::string method = !!cnt.GetStrValueRef(METHOD_PARAM) ? cnt.GetStrValueRef(METHOD_PARAM) : "";
		trace.push_back(std::make_tuple(
					m->DstService(), method, cnt,
					m->SrcServer(), m->SrcUser(), m->SrcService()));
		if (method == UPDATESTATUS_METHOD)
		{
			for(auto && i : GetStatusesFromContainer(cnt))
			{
				auto &curr = out_status[i.callID];
				UpdateCallIDInfo(i.info, curr, i.set_server, false);
			}
		}
	};
	VS_UserData ud;
	ud.m_name = call_id.c_str();
	ud.m_type = ud.UT_PERSON;
	for (auto &i : aliases)
		ud.m_aliases.Assign(i.c_str(), 0);
	ud.m_homeServer = homeBs.c_str();
	ud.m_displayName = display_name;
	users_.emplace(call_id, std::make_tuple(call_id, cid, as_name, homeBs, ud));
	locator_storage_impl_->SetServerForLogin(call_id.c_str(), homeBs.c_str());
	locator_storage_impl_->SetServerForCallID(call_id.c_str(), homeBs.c_str());
	for (auto &i : aliases)
	{
		locator_storage_impl_->SetServerForLogin(i.c_str(), homeBs.c_str());
		locator_storage_impl_->SetServerForCallID(i.c_str(), homeBs.c_str());
	}
	router_->RegisterEndpoint(call_id, [&state_user_descr, user_processing](VS_RouterMessage*m)
	{
		user_processing(m, state_user_descr.trace_, state_user_descr.statuses_);
	},
		[](const VS_PointParams*)
	{});
}
VS_CallIDInfo StatusTestFixture::GetStatus(
	string_view owner, string_view whose)
{
	auto own_it = statuses_by_users_.find(owner);
	if (own_it != statuses_by_users_.end())
	{
		auto it = own_it->second.statuses_.find(whose);
		if (it != own_it->second.statuses_.end())
			return it->second;
	}
	return {};
}
void StatusTestFixture::OnPointConnected(const std::string &ep1, const std::string &ep2)
{
	for (auto i = 0; i < 2; i++)
	{
		auto &ep_where = i == 0 ? ep1 : ep2;
		auto &ep_who = i == 0 ? ep2 : ep1;
		auto ep1_i = endpoints_by_name_.find(ep_where);
		if (ep1_i != endpoints_by_name_.end() && !ep1_i->second.expired())
			ep1_i->second.lock()->OnPointConnected(ep_who);
	}
}
void StatusTestFixture::OnPointDisconnected(const std::string &ep1, const std::string &ep2)
{
	for (auto i = 0; i < 2; i++)
	{
		auto &ep_where = i == 0 ? ep1 : ep2;
		auto &ep_who = i == 0 ? ep2 : ep1;
		auto ep1_i = endpoints_by_name_.find(ep_where);
		if (ep1_i != endpoints_by_name_.end() && !ep1_i->second.expired())
			ep1_i->second.lock()->OnPointDisconnected(ep_who);
	}
}

StatusTestFixture::ResolveResult_T StatusTestFixture::Resolve(
		const std::string&as_name, const std::string &user,
		const bool use_cache)
{
	StatusTestFixture::ResolveResult_T res;
	auto as = AS_.find(as_name);
	if (as != AS_.end())
	{
		auto pool_th = std::get<1>(as->second);
		pool_th->PutTask(new ResolveTask_Test(user, std::get<1>(as->second), use_cache, res));
	}
	return res;
}
std::list<VS_CallIDInfo> StatusTestFixture::ResolveAllSync(
		const std::string &bs, std::list<std::string> &&ids)
{
	auto bs_iter = BS_.find(bs);
	EXPECT_NE(bs_iter, BS_.end());
	std::list<VS_CallIDInfo> res;
	auto pool_th = std::get<1>(bs_iter->second);
	pool_th->PutTask(new ResolveAllTaskSync_Task(std::move(ids), std::get<1>(bs_iter->second), res));
	return res;
}
void StatusTestFixture::AddAS(const std::string &as_name)
{
	AddFrontServer(as_name, false);
}
void StatusTestFixture::AddTCS(const std::string &tcs_name)
{
	AddFrontServer(tcs_name, true);
}
void StatusTestFixture::AddFrontServer(const std::string &as_name, bool isVCS)
{
	auto ep_as = std::make_shared<Endpoint>(as_name, router_);
	auto presense = boost::make_shared<VS_PresenceService>(ios_);
	{
		auto conf_restrict = boost::make_shared<ConfRestrictEmpty>(isVCS);
		presense->SetConfRestrict(conf_restrict);
		presense->Init(as_name.c_str(), PRESENCE_SRV);
		auto impl = new TransportRouterService_fake(presense.get(), as_name.c_str(), PRESENCE_SRV, router_);
		static_cast<VS_TransportRouterServiceBase*>(presense.get())->imp.reset(impl);
		ep_as->RegisterService(PRESENCE_SRV, presense, impl);
	}
	{
		auto thread_pool_srv = std::make_unique<ThreadPoolService_fake>(as_name.c_str(), router_, ep_as);
		auto impl = thread_pool_srv->GetImpl();
		const char *srv_name = thread_pool_srv->OurService();
		ep_as->RegisterService(srv_name, std::move(thread_pool_srv), std::move(impl));
	}
	AS_.emplace(as_name, std::make_tuple(ep_as, presense));
	endpoints_by_name_.emplace(as_name, ep_as);
}
void StatusTestFixture::AddBS(const std::string &bs_name)
{
	auto ep_bs = std::make_shared<Endpoint>(bs_name, router_);
	auto bs_presence = boost::make_shared<BasePresenceServiceWrap>();
	{
		bs_presence->Init(bs_name.c_str(), PRESENCE_SRV);
		auto impl = new TransportRouterService_fake(
			bs_presence.get(), bs_name.c_str(),
			PRESENCE_SRV, router_);
		static_cast<VS_TransportRouterServiceBase*>(bs_presence.get())->imp.reset(impl);
		ep_bs->RegisterService(PRESENCE_SRV, bs_presence, impl);
	}
	{
		auto thread_pool_srv = std::make_unique<ThreadPoolService_fake>(
			bs_name.c_str(), router_, ep_bs);
		auto impl = thread_pool_srv->GetImpl();
		const char *srv_name = thread_pool_srv->OurService();
		ep_bs->RegisterService(
			srv_name, std::move(thread_pool_srv),
			std::move(impl));
	}
	BS_.emplace(bs_name, std::make_tuple(ep_bs, bs_presence));
	endpoints_by_name_.emplace(bs_name, ep_bs);
}

void StatusTestFixture::AddRS(const std::string &rs_name)
{
	auto ep_rs = std::make_shared<Endpoint>(rs_name, router_);
	auto rs_presence = boost::make_shared<VS_RoutingService>();
	{
		rs_presence->Init(rs_name.c_str(), PRESENCE_SRV, true);
		auto impl = new TransportRouterService_fake(
			rs_presence.get(), rs_name.c_str(),
			PRESENCE_SRV, router_);
		static_cast<VS_TransportRouterServiceBase*>(rs_presence.get())->imp.reset(impl);
		ep_rs->RegisterService(PRESENCE_SRV, rs_presence, impl);
	}
	RS_.emplace(rs_name, std::make_tuple(ep_rs, rs_presence));
	endpoints_by_name_.emplace(rs_name, ep_rs);
}
std::pair<bool, VS_CallIDInfo>
StatusTestFixture::GetStatusFromBSCache(
		string_view bs_name, string_view call_id) const
{
	auto bs = BS_.find(bs_name);
	if (bs == BS_.end())
		return {false, VS_CallIDInfo()};
	auto presence = std::get<1>(bs->second);
	return presence->GetCallIDInfoFromCache(call_id);
}
std::pair<bool, VS_CallIDInfo>
StatusTestFixture::GetStatusFromAS(string_view as_name, string_view call_id) const
{
	auto iter = AS_.find(as_name);
	if (iter == AS_.end())
		return { false, VS_CallIDInfo() };
	return { true, std::get<1>(iter->second)->GetStatus(std::string(call_id).c_str()) };
}
bool StatusTestFixture::GetMsgTrace(
	const std::string&name, std::vector<Endpoint::MsgTraceType> & trace)
{
	auto ep = endpoints_by_name_.find(name);
	if (ep == endpoints_by_name_.end())
		return false;
	auto ptr = ep->second.lock();
	if (!ptr)
	{
		endpoints_by_name_.erase(ep);
		return false;
	}
	ptr->GetMsgTrace(trace);
	return true;
}
size_t StatusTestFixture::GetMsgByFilter(
		const std::string&name, const std::string &dst_service,
		const std::string&method, const size_t &start_from_index,
		Endpoint::MsgTraceType&out,
		const std::function<bool(const Endpoint::MsgTraceType&)> &filter) // index of msg in vector
{
	std::vector<Endpoint::MsgTraceType> trace;
	auto res = std::numeric_limits<size_t>::max();
	if (!GetMsgTrace(name, trace))
		return res;
	for (size_t i = start_from_index; i < trace.size(); i++)
	{
		auto &item = trace[i];
		if (std::get<0>(item) == dst_service && std::get<1>(item) == method && filter(item))
		{
			out = item;
			res = i;
			break;
		}
	}
	return res;
}
bool StatusTestFixture::RegisterStatus(
		const std::string &user_name, const std::string &src_as,
		const std::string & seq, const std::string &cid)
{
	auto user_it = users_.find(user_name);
	if (user_it == users_.end())
		return false;
	auto as_it = AS_.find(src_as);
	if (as_it == AS_.end())
		return false;
	const VS_UserData &ud = std::get<4>(user_it->second);
	//RegisterStatus
	{
		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, REGISTERSTATUS_METHOD);
		cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
		cnt.AddValue(REALID_PARAM, ud.m_name);
		cnt.AddValue(ENDPOINT_PARAM, cid.c_str());
		cnt.AddValue(SEQUENCE_PARAM, seq.c_str());
		cnt.AddValueI32(TYPE_PARAM, ud.m_type);
		cnt.AddValue(LOCATORBS_PARAM, ud.m_homeServer);
		cnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
		for (VS_StrI_IntMap::ConstIterator i = ud.m_aliases.Begin(); !!i; ++i) {
			const char* alias = i->key;
			cnt.AddValue(ALIAS_PARAM, alias);
		}
		// Send to RS
		void *buf(0);
		size_t sz(0);
		cnt.SerializeAlloc(buf, sz);
		auto msg = new VS_RouterMessage(
			AUTH_SRV, nullptr, PRESENCE_SRV,
			nullptr, nullptr, ud.m_homeServer,
			src_as.c_str(), 0, buf, sz);
		free(buf);
		router_->SendMsg(msg);
	}
	return true;
}
bool StatusTestFixture::UnregisterStatus(
	const std::string &user_name, const std::string &src_as)
{
	auto user_it = users_.find(user_name);
	if (users_.end() == user_it)
		return false;
	{
		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, UNREGISTERSTATUS_METHOD);
		cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
		cnt.AddValue(CALLID_PARAM, user_name.c_str());
		// Send to RS
		void *buf(0);
		size_t sz(0);
		cnt.SerializeAlloc(buf, sz);
		auto msg = new VS_RouterMessage(
			AUTH_SRV, nullptr, PRESENCE_SRV,
			nullptr, nullptr, std::get<4>(user_it->second).m_homeServer,
			src_as.c_str(), 0, buf, sz);
		free(buf);
		router_->SendMsg(msg);
	}
	return true;
}
void StatusTestFixture::LoginUser(const std::string &user_name)
{
	/**
		1. create UserDate
		2. RegisterStatus
		3. presence->OnUserLogin
	*/
	auto user_it = users_.find(user_name);
	if (user_it == users_.end())
		return;
	auto as_it = AS_.find(std::get<2>(user_it->second));
	if (as_it == AS_.end())
		return;
	if (!RegisterStatus(user_name, std::get<2>(user_it->second),
			"seq", std::get<1>(user_it->second)))
	{
		return;
	}
	auto presence = std::get<1>(as_it->second);
	const VS_UserData &ud = std::get<4>(user_it->second);
	FakeRouter::ChangeCurrentEndpoint(as_it->first.c_str());
	presence->OnUserLoginEnd_Event(ud, std::get<1>(user_it->second));
}
void StatusTestFixture::LogOutUser(const std::string& who)
{
	/**
		- UnregisterStatus
		- presence->OnUserLogoff;
	*/
	auto user_it = users_.find(who);
	if (users_.end() == user_it)
		return;

	if (!UnregisterStatus(who, std::get<2>(user_it->second)))
		return;
	auto as_it = AS_.find(std::get<2>(user_it->second));
	auto presence = std::get<1>(as_it->second);
	FakeRouter::ChangeCurrentEndpoint(as_it->first.c_str());
	presence->OnUserLogoff_Event(std::get<4>(user_it->second), std::get<1>(user_it->second));
}
void StatusTestFixture::Subscribe(const std::string & who, const std::string &whom)
{
	auto user_it = users_.find(who);
	if (users_.end() == user_it)
		return;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, SUBSCRIBE_METHOD);
	cnt.AddValue(CALLID_PARAM, whom.c_str());
	void *buf(0);
	size_t sz(0);
	cnt.SerializeAlloc(buf, sz);
	auto msg = new VS_RouterMessage(
		PRESENCE_SRV, nullptr,
		PRESENCE_SRV, nullptr,
		who.c_str(), std::get<2>(user_it->second).c_str(),
		std::get<2>(user_it->second).c_str(), 0, buf, sz);
	free(buf);
	router_->SendMsg(msg);
}
void StatusTestFixture::UnSubscribe(const std::string&who, const std::string &whom)
{
	auto user_it = users_.find(who);
	if (users_.end() == user_it)
		return;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, UNSUBSCRIBE_METHOD);
	cnt.AddValue(CALLID_PARAM, whom.c_str());
	void *buf(0);
	size_t sz(0);
	cnt.SerializeAlloc(buf, sz);
	auto msg = new VS_RouterMessage(
		PRESENCE_SRV, nullptr,
		PRESENCE_SRV, nullptr,
		who.c_str(), std::get<2>(user_it->second).c_str(),
		std::get<2>(user_it->second).c_str(), 0, buf, sz);
	free(buf);
	router_->SendMsg(msg);
}
void StatusTestFixture::PushStatus(
	const std::string& who, const VS_UserPresence_Status&st,
	const VS_ExtendedStatusStorage &ext_st_set,
	bool insert_all_ext_st)
{
	auto user_it = users_.find(who);
	if (users_.end() == user_it)
		return;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
	cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
	cnt.AddValueI32(USERPRESSTATUS_PARAM, st);
	if (!!ext_st_set)
		ext_st_set.ToContainer(cnt, insert_all_ext_st);
	void *buf(0);
	size_t sz(0);
	cnt.SerializeAlloc(buf, sz);
	auto msg = new VS_RouterMessage(
		PRESENCE_SRV, nullptr,
		PRESENCE_SRV, nullptr,
		who.c_str(), std::get<2>(user_it->second).c_str(),
		std::get<2>(user_it->second).c_str(), 0, buf, sz);
	free(buf);
	router_->SendMsg(msg);
}
}
#endif