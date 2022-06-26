#pragma once
#ifdef _WIN32

#include "BaseServer/Services/VS_BasePresenceService.h"
#include "statuslib/VS_CallIDInfo.h"
#include "tests/UnitTestTC3/transport_fake/Endpoint.h"
#include "tests/UnitTestTC3/transport_fake/FakeRouter.h"
#include "std-generic/compat/memory.h"
#include "std-generic/compat/functional.h"
#include "std/cpplib/VS_CallIDUtils.h"

#include <boost/asio/io_service.hpp>
#include <gtest/gtest.h>

class VS_LocatorStorageImplMemory;
class VS_PresenceService;
class VS_RoutingService;
namespace tc3_test
{
class StatusTestFixture : public ::testing::Test
{
	class BasePresenceServiceWrap : public VS_BasePresenceService
	{
	public:
		std::pair<bool, VS_CallIDInfo>
			GetCallIDInfoFromCache(string_view call_id) const;
	};
	using UserInfoT = std::tuple<
		std::string, std::string,
		std::string, std::string,
		VS_UserData>; //0-callid, 1-cid, 2-current as, 3-homeBs, 4-userdata
	using AS_T = std::tuple<
		std::shared_ptr<Endpoint>,
		boost::shared_ptr<VS_PresenceService>>; //0 - Endpoint, 1-PresenceService;
	using BS_T = std::tuple<
		std::shared_ptr<Endpoint>,
		boost::shared_ptr<BasePresenceServiceWrap>>;
	using RS_T = std::tuple<
		std::shared_ptr<Endpoint>,
		boost::shared_ptr<VS_RoutingService>>;

	boost::asio::io_service ios_;
	vs::map<std::string, UserInfoT, std::less<>> users_;
	vs::map<std::string, AS_T, vs::less<>> AS_;
	vs::map<std::string, BS_T, vs::less<>> BS_;
	vs::map<std::string, RS_T, vs::less<>> RS_;
	vs::map<std::string, std::string,vs::less<>> servers_for_resolve_by_domain_; //domain -> server;
	vs::map<std::string, std::weak_ptr<Endpoint>, vs::less<>> endpoints_by_name_;
	const std::string RS_NAME = "rs.trueconf.com#rs";
	using UserTraceT = std::vector<Endpoint::MsgTraceType>;
	using StatusCacheT = vs::map<std::string, VS_CallIDInfo, vs::less<>>;
	struct StateOfUser
	{
		UserTraceT trace_;
		StatusCacheT statuses_;
	};
	vs::map<std::string, StateOfUser, std::less<>> statuses_by_users_; //  user -> [user->status]
	VS_LocatorStorageImplMemory *locator_storage_impl_;
protected:
	std::shared_ptr<FakeRouter>	 router_ = std::make_shared<FakeRouter>();
public:
	StatusTestFixture();
	void TearDown() override;
	bool ResolveByDomain(const std::string &domain, std::string& server_name);
	void SetResolveServerForDomain(const std::string &domain, const std::string& server);
	void AddUser(const std::string &call_id, const std::list<std::string> &aliases,
			const std::string& display_name, const std::string& cid,
			const std::string& as_name, const std::string&  homeBs,
			const FakeRouter::ProcessingFuncT &processing_func = FakeRouter::ProcessingFuncT());
	VS_CallIDInfo GetStatus(string_view owner, string_view whose);
	void OnPointConnected(const std::string &ep1, const std::string &ep2);
	void OnPointDisconnected(const std::string &ep1, const std::string &ep2);
	using ResolveResult_T = std::tuple<std::string/**real name*/, VS_CallIDInfo/**call id*/>;
	ResolveResult_T	Resolve(const std::string&as_name,
			const std::string &user, const bool use_cache);
	std::list<VS_CallIDInfo> ResolveAllSync(const std::string &bs, std::list<std::string> &&ids);
	// AS or TCS
	void AddFrontServer(const std::string &as_name, bool isVCS);
	void AddTCS(const std::string &tcs_name);
	void AddAS(const std::string &as_name);
	void AddBS(const std::string &bs_name);
	auto offline_status_cache(const std::string &name) -> decltype(std::get<1>(BS_[name])->offline_status_cache())
	{
		EXPECT_EQ(BS_.count(name), 1);
		return std::get<1>(BS_[name])->offline_status_cache();
	}
	void AddRS(const std::string &rs_name);
	std::pair<bool, VS_CallIDInfo>
		GetStatusFromBSCache(string_view bs_name, string_view call_id) const;
	std::pair<bool, VS_CallIDInfo>
		GetStatusFromAS(string_view as_name, const string_view call_id) const;
	bool GetMsgTrace(const std::string&name, std::vector<Endpoint::MsgTraceType> & trace);
	size_t GetMsgByFilter(
		const std::string&name, const std::string &dst_service,
		const std::string&method, const size_t &start_from_index,
		Endpoint::MsgTraceType &out,
		const std::function<bool(const Endpoint::MsgTraceType&)> &filter); // index of msg in vector
	bool RegisterStatus(
		const std::string &user_name, const std::string &src_as,
		const std::string & seq = "seq", const std::string &cid = "cid");
	bool UnregisterStatus(const std::string &user_name, const std::string &src_as);
	void LoginUser(const std::string &user_name);
	void LogOutUser(const std::string& who);
	void Subscribe(const std::string & who, const std::string &whom);
	void UnSubscribe(const std::string&who, const std::string &whom);
	void PushStatus(
		const std::string& who, const VS_UserPresence_Status&st,
		const VS_ExtendedStatusStorage &ext_st_set = VS_ExtendedStatusStorage(),
		bool insert_all_ext_st = false);
};
}
#endif