#ifdef _WIN32 // not ported
#include "../../ServerServices/Common.h"
#include "VS_BasePresenceService.h"
#include "std-generic/cpplib/string_view.h"
#include "storage/VS_LocatorStorage.h"
#include "storage/VS_DBStorageInterface.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_USPRS
boost::shared_ptr<VS_BasePresenceService> g_BasePresenceService;
namespace
{
	static const auto c_wait_unreg_status_timeout = std::chrono::seconds(10);
}

VS_BasePresenceService::VS_BasePresenceService()
	: m_offline_statuses_cache([this](OfflineStatusCache::ExtStatusUpdateInfo&& upd) { OnExtStatusUpdated(std::move(upd)); })
{
	m_TimeInterval = std::chrono::seconds(2);
}
bool VS_BasePresenceService::Init(const char *our_endpoint, const char *our_service, const bool permittedAll)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return false;
	dbStorage->CleanUp();
	return true;
}
bool VS_BasePresenceService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)	return true;
	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		m_recvMess = recvMess.get();
		{
			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
				string_view method;
				const char *m = cnt.GetStrValueRef(METHOD_PARAM);

				if (m &&!(method = m).empty())
				{
					dprint3("BASEPRESENCE: Processing %20s; cid:%s serv:%s user:%s bytes:%zu\n", m, recvMess->SrcCID(), recvMess->SrcServer(), recvMess->SrcUser(), recvMess->BodySize());
					// Process methods

					if (method == RESOLVE_METHOD)
					{
						if (recvMess->IsFromServer())
							Resolve_Method(cnt);
					}
					else if (method == RESOLVEALL_METHOD)
					{
						if (recvMess->IsFromServer())
							ResolveAll_Method(cnt);
					}
					else if ( method == UPDATESTATUS_METHOD)
					{
						UpdateStatus_Method(VS_FullID(recvMess->SrcServer(),recvMess->SrcUser()), cnt);
					}
					else if (method == GETALLUSERSTATUS_METHOD)
					{
						GetAllUserStatus_Method(cnt);
					}
					else if (method == REGISTERSTATUS_METHOD)
					{
						if (recvMess->IsFromServer())
							RegisterStatus(m_recvMess->SrcServer(), cnt);
					}
					else if (method == UNREGISTERSTATUS_METHOD)
					{
						if (recvMess->IsFromServer())
							UnregisterStatus_Method(cnt,recvMess->SrcServer());
					}
					else if (method == SUBSCRIBE_METHOD)
					{
						Subscribe_Method(cnt);

					}
					else if (method == UNSUBSCRIBE_METHOD)
					{
						Unsubscribe_Method(cnt);
					}
					else if (method == GETSUBSCRIPTIONS_METHOD)
					{
						// ?? skip
					}
					else
					{
						assert(false);
					}
				}
			}
		}
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return true;
}
bool VS_BasePresenceService::Timer(unsigned long tickcount)
{
	auto ds = dstream4;
	size_t count = 0;
	for(auto i = m_delay_until_unregister.begin();i!=m_delay_until_unregister.end();)
	{
		if(std::get<2>(i->second) <= std::chrono::steady_clock::now())
		{
			/*
			 * 1. erase from m_delay_unregister
			 * 2. disconnect from old server (reset, force unregister)
			 * 4. retry_login for item i;
			 * */
			auto delayed_register = std::move(*i);
			i =	m_delay_until_unregister.erase(i);
			if(!count)
				ds << "Force UnregisterStatus by timeout for users\n";
			ds << delayed_register.first <<'\n';
			auto cache = m_statusCache.Find(delayed_register.first.c_str());
			if(!!cache && !cache->data->m_serverID.empty())
			{
				ds <<" force disconnect from old server = "<<cache->data->m_serverID << '\n';
				DisconnectEndpoint(cache->data->m_serverID.c_str());
			}
			const char * ep_param = std::get<1>(delayed_register.second).GetStrValueRef(ENDPOINT_PARAM);
			const char * seq = std::get<1>(delayed_register.second).GetStrValueRef(SEQUENCE_PARAM);
			const char *server = std::get<0>(delayed_register.second).c_str();
			ds<<" retry login for server = "<< server <<", ep_param = "<<ep_param<< ", seq = "<<seq<<'\n';
			SendRegResult(RETRY_LOGIN, server, ep_param,seq);
			++count;
		}
		else
			++i;
	}
	return VS_SubscriptionHub::Timer(tickcount);
}
void VS_BasePresenceService::ResolveImpl(const VS_RouterMessage& msg_for_replay, const char * resolve_method, const std::map<std::string, std::set<std::string>> &location_info)
{
	if (!resolve_method || !*resolve_method)
		return;
	VS_Container local_resolve;
	std::map<std::string, VS_Container>	servers_for_resolve;

	for (const auto &i : location_info)
	{
		if (i.first == OurEndpoint() || i.first.empty())
		{
			for (const auto &id : i.second)
			{
				// local; Send status
				local_resolve.AddValue(CALLID_PARAM, id.c_str());
				auto info = !i.first.empty() ? GetStatus(id.c_str()) : VS_CallIDInfo(VS_UserPresence_Status::USER_INVALID);
				info.ToContainer(local_resolve, true, true);
			}
		}
		else
		{
			auto server = servers_for_resolve.find(i.first);
			if (server == servers_for_resolve.end())
			{
				server = servers_for_resolve.emplace(i.first, VS_Container()).first;
				server->second.AddValue(METHOD_PARAM, resolve_method);
			}
			for (const auto &id : i.second)
				server->second.AddValue(CALLID_PARAM, id.c_str());
		}
	}
	if (!local_resolve.IsEmpty())
	{
		/**
		replay for message
		**/
		local_resolve.AddValue(METHOD_PARAM, resolve_method);
		void* body = nullptr;
		size_t bodySize = 0;
		local_resolve.SerializeAlloc(body, bodySize);
		auto replay = new VS_RouterMessage(&msg_for_replay, default_server_timeout, body, bodySize);
		if (!PostMes(replay))
			delete replay;
		free(body);
	}
	for (const auto &i : servers_for_resolve)
	{
		/**
		resend to home servers
		*/
		auto cnt = std::move(i.second);
		void *body(0);
		size_t sz = 0;
		if (cnt.SerializeAlloc(body, sz))
		{
			auto msg = new VS_RouterMessage(msg_for_replay.SrcService(), msg_for_replay.AddString(), msg_for_replay.DstService(), nullptr, msg_for_replay.SrcUser(), i.first.c_str(), msg_for_replay.SrcServer(), msg_for_replay.TimeLimit(), body, sz);
			if (!PostMes(msg))
				delete msg;
			free(body);
		}
	}
}

void VS_BasePresenceService::Resolve_Method(const VS_Container &cnt)
{
	const char * c_id = cnt.GetStrValueRef(CALLID_PARAM);
	if (!c_id || !*c_id)
		return;
	auto cache = m_statusCache[c_id];
	if (!cache)
	{
		VS_RouterMessage m = std::move(*m_recvMess);
		m_offline_statuses_cache.Locate({ c_id }, [this,m](std::map<std::string, std::set<std::string>> &&res)
		{
			auto resolve_server = VS_ResolveServerFinder::Instance();
			if (resolve_server)
			{
				for (const auto&i : res)
				{
					if (!i.first.empty())
					{
						for (const auto &id : i.second)
						{
							if (!id.empty())
								resolve_server->SetServerForUser(id, i.first, true);
						}
					}
				}
			}
			CallInProcessingThread([this,m,res]() mutable
			{
				ResolveImpl(m, RESOLVE_METHOD, std::move(res));
			}
			);
		});
	}
	else
	{
		VS_Container resolve_cnt;
		resolve_cnt.AddValue(METHOD_PARAM, RESOLVE_METHOD);
		cache->data->ToContainer(resolve_cnt, true, true);
		dprint3("RS: resolving '%s' got %d,'%s',type=%d\n", c_id, cache->data->m_status, cache->data->m_serverID.c_str(), cache->data->m_type);
		PostReply(resolve_cnt);
	}
}
void VS_BasePresenceService::ResolveAll_Method(const VS_Container &cnt)
{
	/**
			m_offline_statuses_cache.locate
			resolve only call id from the server;
	*/
	cnt.Reset();
	std::list<std::string> call_ids;
	while (cnt.Next())
	{
		string_view name = cnt.GetName();
		if (name == CALLID_PARAM)
		{
			const char *val(nullptr);
			if (!(val = cnt.GetStrValueRef()) || !*val)
				continue;
			call_ids.emplace_back(val);
		}
	}
	VS_RouterMessage m = std::move(*m_recvMess);
	m_offline_statuses_cache.Locate(std::move(call_ids), [this, m](std::map<std::string, std::set<std::string>> &&res)
	{
		auto resolve_server = VS_ResolveServerFinder::Instance();
		if (resolve_server)
		{
			for (const auto&i : res)
			{
				if (!i.first.empty())
				{
					for (const auto &id : i.second)
					{
						if (!id.empty())
							resolve_server->SetServerForUser(id, i.first, true);
					}
				}
			}
		}
		CallInProcessingThread([this,m,res]() mutable
		{
			ResolveImpl(m, RESOLVEALL_METHOD, std::move(res));
		});
	});
}
std::list<VS_CallIDInfo> VS_BasePresenceService::ResolveAllSync(std::list<std::string> &&ids, VS_TransportRouterServiceBase* caller)
{
	std::list<VS_CallIDInfo> res;
	std::mutex m;
	vs::condition_variable cond_var;
	std::atomic<bool> complete_flag(false);
	std::map<std::string, std::set<std::string>> ids_by_bs;
	m_offline_statuses_cache.Locate(std::move(ids), [this, &res, &complete_flag, &cond_var, &ids_by_bs](std::map<std::string, std::set<std::string>> &&locate_result)
	{
		auto resolve_server = VS_ResolveServerFinder::Instance();
		if (resolve_server)
		{
			for (const auto&i : locate_result)
			{
				if (!i.first.empty())
				{
					for (const auto &id : i.second)
					{
						if (!id.empty())
							resolve_server->SetServerForUser(id, i.first, true);
					}
				}
			}
		}
		ids_by_bs = std::move(locate_result);
		complete_flag = true;
		cond_var.notify_all();
	});
	std::unique_lock<std::mutex> l(m);
	while (!cond_var.wait_for(l, std::chrono::milliseconds(10), [&complete_flag]() {return complete_flag == true; }))
		;
	std::map<string_view, std::set<string_view>> for_remote_resolve;
	for (const auto &i : ids_by_bs)
	{
		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, RESOLVEALL_METHOD);
		for (const auto &j : i.second)
		{
			if (i.first == OurEndpoint())
				res.push_back(GetStatus(j.c_str()));
			else if (!i.first.empty())
			{
				cnt.AddValue(CALLID_PARAM, j.data());
				for_remote_resolve[i.first].insert(j);
			}
		}
		if (i.first != OurEndpoint() && !i.first.empty())
			PostRequest(i.first.data(), 0, cnt, 0, PRESENCE_SRV, 10000, caller->OurService());
	}
	/**
		Recv from all servers
	*/
	std::chrono::steady_clock::duration wait_time = std::chrono::seconds(30);
	while (wait_time != std::chrono::steady_clock::duration::zero() && !for_remote_resolve.empty())
	{
		std::unique_ptr<VS_RouterMessage> mes;
		if (caller->ReceiveMes(mes, wait_time) == 1)
		{
			if (!mes->IsFromServer() || mes->Type() != transport::MessageType::Reply)
				continue;
			for_remote_resolve.erase(mes->SrcServer());
			VS_Container rCnt;
			if (rCnt.Deserialize(mes->Body(), mes->BodySize()))
			{
				auto p = rCnt.GetStrValueRef(METHOD_PARAM);
				if (!p || !*p || string_view(p) != RESOLVEALL_METHOD)
					continue;
				for (auto &&i : GetStatusesFromContainer(rCnt))
					res.emplace_back(std::move(i.info));
			}
		}
	}
	return res;
}

void VS_BasePresenceService::GetAllUserStatus_Method(const VS_Container &cnt)
{
	if (!m_recvMess->IsFromServer())
	{
		dprint1("BASEPRESENCE: GetAllUserStatus_Method from user is forbidden. Skip message. Src.server = %s; src.user = %s\n", m_recvMess->SrcServer(), m_recvMess->SrcUser());
		return;
	}
	VS_FullID id(m_recvMess->SrcServer(), m_recvMess->SrcUser());
	auto type = m_recvMess->Type();

	const char* type_string = type == transport::MessageType::Request ? "request" : "replay";
	dprint3("GetAllUserStatus %s from %s\n", type_string, m_recvMess->SrcServer());
	switch (type)
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	{
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, GETALLUSERSTATUS_METHOD);
		rCnt.AddValueI32(CAUSE_PARAM, 1);
		int users = GetNextSubscriberStatuses(id, rCnt, false, false, 200);
		int32_t seq_id = m_out_sync.Reset(id);
		rCnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);
		PostReply(rCnt);
		dprint3(" sending seq_id=%08x, found %d\n", seq_id, users);
		while (IsCurrentSubscriberStatusesExist())
		{
			VS_Container	cnt;
			cnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
			long hops = VS_SubscriptionHub::DEFAULT_HOPS - 1;		// minus one for myself
			cnt.AddValueI32(HOPS_PARAM, hops);
			users += GetNextSubscriberStatuses(id, cnt, false, false, 200);
			SeqPost(id, cnt);
		}
		dprint3(" found %d users\n", users);
	}
		break;
	case transport::MessageType::Reply:
	{
		dprint3("UpdateStatus by GetAllUserStatus replay\n");
		UpdateStatus_Method(VS_FullID(m_recvMess->SrcServer(),m_recvMess->SrcUser()), cnt);
	}
		break;
	case transport::MessageType::Notify:
		break;
	default:
		break;
	}
}

void VS_BasePresenceService::SendRegResult(VS_UserLoggedin_Result result, const char *server, const char *temp_id, const char *seq)
{
	if (!server || !*server || !temp_id || !*temp_id || !seq || !*seq)
		return;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, USERLOGGEDIN_METHOD);
	cnt.AddValueI32(RESULT_PARAM, result);
	cnt.AddValue(ENDPOINT_PARAM, temp_id);
	cnt.AddValue(SEQUENCE_PARAM, seq);
	PostRequest(server, 0, cnt, 0, AUTH_SRV);
}
void VS_BasePresenceService::RegisterStatus(const char* src_server, const VS_Container& cnt)
{
	const char *call_id = cnt.GetStrValueRef(REALID_PARAM);
	const char *displayName=cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	const char * temp_id = cnt.GetStrValueRef(ENDPOINT_PARAM);
	const char *seq = cnt.GetStrValueRef(SEQUENCE_PARAM);
	const char *home_server = cnt.GetStrValueRef(LOCATORBS_PARAM);
	int32_t user_type = -1;
	cnt.GetValue(TYPE_PARAM, user_type);
	assert(!!home_server && string_view(home_server) == OurEndpoint());
	if (!call_id || !*call_id || !src_server || !*src_server || !temp_id || !*temp_id || !seq || !*seq)
		return;

	dprint4("RegisterStatus server:%s; user:%s\n",src_server,call_id);

	auto reg_record = m_registred_call_id.get<by_call_id>().find(call_id);
	auto time_wait = c_wait_unreg_status_timeout;
	if (reg_record != m_registred_call_id.get<by_call_id>().end() && string_view(src_server) != reg_record->server_)
	{
		auto ds = dstream4;
		ds << "User " << call_id << " already registered on server " << reg_record->server_ << ". ";
		auto prev = m_delay_until_unregister.find(call_id);
		if (prev != m_delay_until_unregister.end()) //reject previous request;
		{
			auto delayed_reg = std::move(*prev);
			m_delay_until_unregister.erase(prev);
			ds << "Hanging register status request for server:" << std::get<0>(delayed_reg.second) << " user:" << call_id << '\n';
			ds << "force disconnect from old server = " << reg_record->server_ << '\n';
			DisconnectEndpoint(reg_record->server_.c_str());
			if (std::get<0>(delayed_reg.second) != src_server)
			{
				const char * ep_param = std::get<1>(delayed_reg.second).GetStrValueRef(ENDPOINT_PARAM);
				const char * seq = std::get<1>(delayed_reg.second).GetStrValueRef(SEQUENCE_PARAM);
				const char *server = std::get<0>(delayed_reg.second).c_str();
				ds << "retry login for server = " << server << ", ep_param = " << ep_param << ", seq = " << seq << '\n';
				SendRegResult(RETRY_LOGIN, server, ep_param, seq);
			}
			const char * ep_param = cnt.GetStrValueRef(ENDPOINT_PARAM);
			const char * seq = cnt.GetStrValueRef(SEQUENCE_PARAM);
			ds << "retry login for server = " << src_server << ", ep_param = " << ep_param << ", seq = " << seq << '\n';
			SendRegResult(RETRY_LOGIN, src_server, ep_param, seq);
			return;
		}
		ds << "Logout user from old server, delay register status;\n";
		m_delay_until_unregister[call_id] = std::make_tuple(src_server, cnt, std::chrono::steady_clock::now() + time_wait);
		VS_Container logout_cnt;
		logout_cnt.AddValue(METHOD_PARAM, LOGOUTUSER_METHOD);
		logout_cnt.AddValue(USERNAME_PARAM, call_id);
		PostRequest(reg_record->server_.c_str(), 0, logout_cnt, 0, AUTH_SRV);
		return;
	}
	else if (reg_record == m_registred_call_id.get<by_call_id>().end())
		m_registred_call_id.emplace(call_id, src_server);
	RegisterAliases(cnt);
	VS_CallIDInfo ci(USER_AVAIL, VS_ExtendedStatusStorage(), !!src_server ? src_server : std::string{}, user_type, cnt.GetStrValueRef(REALID_PARAM), !!home_server ? home_server : std::string{}, !!displayName ? displayName : std::string{});
	VS_MultiLoginCapability ml_cap = VS_MultiLoginCapability::UNKNOWN;
	cnt.GetValueI32(MULTI_LOGIN_CAPABILITY_PARAM, ml_cap);
	ci.m_ml_cap = ml_cap;
	SaveToStorageAndPush(call_id,
			std::move(ci),
			true,
			src_server);
	SendRegResult(USER_LOGGEDIN_OK, src_server,temp_id,seq);
}
void VS_BasePresenceService::UnregisterStatus_Method(const VS_Container&cnt, const char *src_server)
{
	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);
	if (!call_id || !*call_id || !src_server || !*src_server)
		return;
	dprint4("UnregisterStatus for server:%s user:%s\n", src_server, call_id);
	auto reg_record = m_registred_call_id.get<by_call_id>().find(call_id);
	if (reg_record != m_registred_call_id.get<by_call_id>().end() && src_server != reg_record->server_)
	{
		dprint1("BSPRESENCE: server %s is trying to unreg user %s from %s - blocked\n", src_server, call_id, reg_record->server_.c_str());
		return;
	}
	else if (reg_record != m_registred_call_id.get<by_call_id>().end())
		m_registred_call_id.erase(reg_record);
	VS_CallIDInfo ci(USER_LOGOFF, VS_ExtendedStatusStorage(), {}, -1);
	VS_MultiLoginCapability ml_cap = VS_MultiLoginCapability::UNKNOWN;
	cnt.GetValueI32(MULTI_LOGIN_CAPABILITY_PARAM, ml_cap);
	ci.m_ml_cap = ml_cap;
	SaveToStorageAndPush(static_cast<std::string>(call_id),
			std::move(ci),
			true,
			src_server);
	UnregisterAliases(call_id,false);
	RegisterDelayedStatus(call_id);
}
void VS_BasePresenceService::RegisterDelayedStatus(string_view call_id)
{
	if (call_id.empty())
		return;
	auto delay_iter = m_delay_until_unregister.find(static_cast<std::string>(call_id));
	if (delay_iter != m_delay_until_unregister.end())
	{
		dprint4("Process delayed register status request; server:%s user:%s\n", std::get<0>(delay_iter->second).c_str(), delay_iter->first.c_str());
		auto item(std::move(*delay_iter));
		m_delay_until_unregister.erase(delay_iter);
		RegisterStatus(std::get<0>(item.second).c_str(), std::get<1>(item.second));
	}
}
void VS_BasePresenceService::Subscribe_Method(const VS_Container&cnt)
{
	VS_FullID server_id(m_recvMess->SrcServer(), m_recvMess->SrcUser());
	if (!VS_SyncPool::CheckExistenceParamsForSync(cnt))
	{
		dprint0("BASEPRESENCE: required params is absent. Request is silently rejected. srcSrv = %s; srcUsr = %s\n", server_id.m_serverID.m_str, server_id.m_userID.m_str);
		return;
	}
	if (!m_sub_sync.ConsistentCheck(server_id, cnt))
	{
		dprint3("BASEPRESENCE: sync lost in subs. GetAllSubscription from FullID(server=%s,user=%s)\n", server_id.m_serverID.m_str, server_id.m_userID.m_str);
		VS_Container reset_cnt;
		reset_cnt.AddValue(METHOD_PARAM, GETSUBSCRIPTIONS_METHOD);
		PostRequest(server_id.m_serverID, server_id.m_userID, reset_cnt, NULL, PRESENCE_SRV);
	}

	cnt.Reset();
	VS_Container local_sub_cnt;
	std::list<std::string> call_id_for_locate;
	int32_t cause(0);
	if (cnt.GetValue(CAUSE_PARAM, cause))
		local_sub_cnt.AddValue(CAUSE_PARAM, cause);
	int32_t seq_id(-1);
	if (cnt.GetValue(SEQUENCE_ID_PARAM, seq_id))
		local_sub_cnt.AddValue(SEQUENCE_ID_PARAM, seq_id);
	{
		auto ds = dstream4;
		ds << "BASEPRESENCE: Subscribe to:";
		auto cache_by_server_it = m_unsub_cache.find(server_id);
		while (cnt.Next())
		{
			static const string_view call_id_param = CALLID_PARAM;
			if (call_id_param == cnt.GetName() && cnt.GetStrValueRef() != nullptr)
			{
				string_view id = cnt.GetStrValueRef();
				call_id_for_locate.push_back(static_cast<std::string>(id));
				ds << id << '\n';
				if (cache_by_server_it != m_unsub_cache.end())
					cache_by_server_it->second.erase(static_cast<std::string>(id));
			}
		}
		if (cache_by_server_it != m_unsub_cache.end() && cache_by_server_it->second.empty())
			m_unsub_cache.erase(cache_by_server_it);
	}
	m_offline_statuses_cache.Locate(std::move(call_id_for_locate), [this, local = std::move(local_sub_cnt), srcServer = std::string(m_recvMess->SrcServer_sv()), srcUser = std::string(m_recvMess->SrcUser_sv())](std::map<std::string, std::set<std::string>>&& located_id) mutable
	{
		VS_Container set_locator_bs;
		string_view our_ep = OurEndpoint();
		std::list<std::string> callid_for_cache_update;
		auto resolve_server = VS_ResolveServerFinder::Instance();
		for (const auto &i : located_id)
		{
			if (i.first == our_ep)
			{
				for (const auto &j : i.second)
				{
					local.AddValue(CALLID_PARAM, j.c_str());
					callid_for_cache_update.push_back(j);
					if (resolve_server)
						resolve_server->SetServerForUser(j,i.first, true);
				}
			}
			else
			{
				for (const auto &j : i.second)
				{
					set_locator_bs.AddValue(CALLID_PARAM, j.c_str());
					if (i.first.length() > 0)
					{
						set_locator_bs.AddValue(LOCATORBS_PARAM, i.first.c_str());
						if (resolve_server)
							resolve_server->SetServerForUser(j, i.first, true);
					}
					else
						set_locator_bs.AddValueI32(CAUSE_PARAM, 1); // locator was not found
				}
			}
		}
		if (!set_locator_bs.IsEmpty())
		{
			set_locator_bs.AddValue(METHOD_PARAM, SETLOCATORBS_METHOD);
			PostRequest(srcServer.c_str(), srcUser.c_str(), set_locator_bs, NULL, PRESENCE_SRV);
		}
		if (!callid_for_cache_update.empty())
		{
			m_offline_statuses_cache.ReadStickyToCache(std::move(callid_for_cache_update), [this, local, srcServer, srcUser](){
				CallInProcessingThread([this, local, srcServer, srcUser]
				{
					SubscribeLocated(srcServer, srcUser, local);
				});
			});
		}
	});
}
void VS_BasePresenceService::Unsubscribe_Method(const VS_Container &cnt)
{
	VS_FullID server_id(m_recvMess->SrcServer(), m_recvMess->SrcUser());
	if (!VS_SyncPool::CheckExistenceParamsForSync(cnt))
	{
		dprint0("BASEPRESENCE: required params is absent. Request is silently rejected. srcSrv = %s; srcUsr = %s\n", server_id.m_serverID.m_str, server_id.m_userID.m_str);
		return;
	}
	auto seq_test = m_sub_sync.GetCurrentSeqId(server_id);
	if (!m_sub_sync.ConsistentCheck(server_id, cnt))
	{
		dprint1("BASEPRESENCE: sync lost in unsub for %s L%08x!=R%08x\n", m_recvMess->SrcServer(), seq_test, m_sub_sync.GetCurrentSeqId(server_id));
		VS_Container reset_cnt;
		reset_cnt.AddValue(METHOD_PARAM, GETSUBSCRIPTIONS_METHOD);
		PostRequest(server_id.m_serverID, server_id.m_userID, reset_cnt, NULL, PRESENCE_SRV);
	}
	auto cache_by_server_it = m_unsub_cache.emplace(server_id, std::set<std::string>()).first;
	Unsubscribe(cnt, server_id,&cache_by_server_it->second);
}
void VS_BasePresenceService::SubscribeLocated(const std::string &srcServer, const std::string&srcUser, const VS_Container&cnt)
{
	dprint4("BASEPRESENCE: SubscribeLocated(...). srcServer = %s. srcUser = %s\n", srcServer.empty() ? "<nul>" : srcServer.c_str(), srcUser.empty() ? "<null>" : srcUser.c_str());
	VS_FullID server_id(srcServer.c_str(), srcUser.c_str());
	int32_t cause = 0;
	cnt.GetValue(CAUSE_PARAM, cause);
	if (cause == 1)
		UnsubscribeID(server_id);
	auto exclude_idx = m_unsub_cache.find(server_id);
	Subscribe(cnt, server_id, cause == 1, (exclude_idx == m_unsub_cache.end() ? std::set<std::string>() : exclude_idx->second));
}
void VS_BasePresenceService::OnExtStatusUpdated(OfflineStatusCache::ExtStatusUpdateInfo&& upd)
{
	if (IsInProcessingThread())
	{
		for (const auto &i : upd)
		{
			auto status = GetStatus(i.first.c_str());
			status.m_extStatusStorage += i.second;
			PushStatusFull(i.first.c_str(), status);
		}
	}
	else
		CallInProcessingThread([this, upd]() mutable
		{
			OnExtStatusUpdated(std::move(upd));
		});
}
void VS_BasePresenceService::UpdateStatus_Method(const VS_FullID& source, const VS_Container &cnt)
{
	///TODO: check ext statuses
	if (source.m_serverID.IsEmpty())
	{
		dstream1 << "BASEPRESENCE: UpdateStatus_Method from empty source.m_serverID\n";
		return;
	}
	if(!m_recvMess->IsFromServer())
	{
		dprint1("BASEPRESENCE: no update from %s:%s allowed \n",source.m_serverID.m_str, source.m_userID.m_str);
		return;
	}
	if (!VS_SyncPool::CheckExistenceParamsForSync(cnt))
	{
		dprint0("BASEPRESENCE: required params is absent. Request is silently rejected. srcSrv = %s; srcUsr = %s\n", source.m_serverID.m_str, source.m_userID.m_str);
		return;
	}
	auto dbStorage = g_dbStorage;
	int32_t cause = 0;
	cnt.GetValue(CAUSE_PARAM, cause);
	if (cause == 1)
	{
		if (dbStorage)
			dbStorage->ClearStatuses(source.m_serverID);
	}

	if (!m_in_sync.ConsistentCheck(source, cnt))
	{
		VS_Container reset_cnt;
		reset_cnt.AddValue(METHOD_PARAM, GETALLUSERSTATUS_METHOD);
		PostRequest(source.m_serverID, source.m_userID, reset_cnt, NULL, PRESENCE_SRV);
	}
	for (auto &&i :	GetStatusesFromContainer(cnt))
	{
		/**
			1. save to base
			2. Send offline chat
			3. pushstatusFull
		*/
		if (!i.aliases.empty())
			RegisterAliases(i.callID, i.aliases);
		PushWithCheck(i.callID, std::move(i.info), source, cause == 1);
	}
}
void VS_BasePresenceService::PushWithCheck(const std::string& call_id, VS_CallIDInfo&& ci, const VS_FullID &source, bool cause)
{
	auto registred_it = m_registred_call_id.find(call_id);
	string_view current_server;
	if (registred_it != m_registred_call_id.end())
		current_server = registred_it->server_;
	if (current_server.empty())
	{
		if (ci.m_status < USER_AVAIL)
		{
			VS_Container cnt;
			cnt.AddValue(CALLID_PARAM, call_id.c_str());
			cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, ci.m_ml_cap);
			UnregisterStatus_Method(cnt,source.m_serverID);
			return;
		}
	}
	else if (current_server != string_view{ source.m_serverID.m_str,  (size_t)source.m_serverID.Length() })
	{
		dstream4 << "BASEPRESENCE: reject update status from no registered server; call_id = " << call_id << "; source = " << source.m_serverID.m_str << "; current server = " << current_server;
		if (ci.m_status >= USER_AVAIL)
		{
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, LOGOUTUSER_METHOD);
			cnt.AddValue(USERNAME_PARAM, call_id.c_str());
			PostRequest(ci.m_serverID.c_str(), 0, cnt, 0, AUTH_SRV);
			if (string_view{ source.m_serverID.m_str, (size_t)source.m_serverID.Length() } != ci.m_serverID)
			{
				dstream2 << "BASEPRESENCE: PushWithCheck source != serverID from CallIDInfo; call_id = " << call_id << "; source = " << source.m_serverID.m_str << "; ci.m_serverID = " << ci.m_serverID;
			}
		}
		return;
	}
	auto it = m_check_offline_chat.find(call_id);
	if (it!=m_check_offline_chat.end() && ci.m_status >= VS_UserPresence_Status::USER_AVAIL && !source.m_serverID.IsEmpty())
	{
		auto dbStorage = g_dbStorage;
		if (dbStorage)
		{
			//update offline status cac
			std::vector<VS_Container> vec;
			dbStorage->GetOfflineChatMessages(call_id.c_str(), vec);
			for (const auto &cont : vec)
				PostRequest(source.m_serverID.m_str, call_id.c_str(), cont, 0, CHAT_SRV);
		}
		m_check_offline_chat.erase(it);
	}
	if (cause && current_server.empty())
		m_registred_call_id.emplace(call_id, source.m_serverID.m_str);
	SaveToStorageAndPush(call_id, std::move(ci), current_server.empty(), source.m_serverID.m_str);
}
void VS_BasePresenceService::SaveToStorageAndPush(std::string call_id, VS_CallIDInfo&&ci, bool set_server, std::string source_server, std::string source_user)
{
	m_check_offline_chat.insert(call_id);
	m_offline_statuses_cache.UpdateUserStatus(std::move(call_id), ci.m_status,std::move(ci.m_extStatusStorage),set_server,std::move(source_server), [this, call_id,ci,set_server,source_server,source_user] ()
	{
		CallInProcessingThread([this, call_id, ci, set_server, source_server, source_user]() mutable {
			PushStatusFull(call_id.c_str(), ci, VS_CallIDInfo::LOCAL, set_server, source_server.c_str(), source_user.c_str());
		});
	}
	);
}
VS_UserPresence_Status VS_BasePresenceService::NetworkResolve(const char* uplink, const VS_SimpleStr& call_id,VS_CallIDInfo& ci,VS_TransportRouterServiceBase* caller)
{
	///TODO: Check Ext Status
		ci.m_status = USER_INVALID;

		if(!caller)
			return ci.m_status;

		VS_Container rCnt;

		// send req
		rCnt.AddValue(METHOD_PARAM, RESOLVE_METHOD);
		rCnt.AddValue(CALLID_PARAM, call_id);
		PostRequest(uplink, 0, rCnt, 0, PRESENCE_SRV, 10000, caller->OurService());

		//wait for reply
		std::chrono::steady_clock::duration wait_time = std::chrono::seconds(30);
		std::unique_ptr<VS_RouterMessage> mes;
		if (caller->ReceiveMes(mes, wait_time) == 1)
		{
			if (rCnt.Deserialize(mes->Body(), mes->BodySize()))
			{
				int32_t lval;
				if(rCnt.GetValue(USERPRESSTATUS_PARAM, lval))
					ci.m_status=(VS_UserPresence_Status)lval;
				{
					VS_Container extStatusCnt;
					if (rCnt.GetValue(EXTSTATUS_PARAM, extStatusCnt))
						ci.m_extStatusStorage.UpdateStatus(extStatusCnt);
				}
				ci.m_serverID		= rCnt.GetStrValueRef(SERVER_PARAM);
				ci.m_realID			= rCnt.GetStrValueRef(REALID_PARAM);
				ci.m_homeServer		= rCnt.GetStrValueRef(LOCATORBS_PARAM);

				if(rCnt.GetValue(TYPE_PARAM, lval))
					ci.m_type=(VS_UserPresence_Status)lval;
			} //error in container
		} //no reply - return INVALID

		return ci.m_status;
}

VS_UserPresence_Status VS_BasePresenceService::Resolve(VS_SimpleStr& call_id,VS_CallIDInfo& ci, bool use_cache, VS_TransportRouterServiceBase* caller)
{
	ci.Empty();
	bool found = false;

	if (!use_cache) {
		std::string server;
		VS_ResolveServerFinder *resolve_server = VS_ResolveServerFinder::Instance();
		if(!found && resolve_server->GetServerForResolve(call_id,server,false) && !server.empty() && caller!=0)
		{
			NetworkResolve(server.c_str(), call_id, ci, caller);
		}
	}

	if (!ci.m_realID.empty())
		call_id = ci.m_realID.c_str();

	return ci.m_status;
}

bool VS_BasePresenceService::IsRoamingAllowed(const char *server_name)
{
	return VS_SubscriptionHub::IsRoamingAllowed(server_name);
}

VS_CallIDInfo VS_BasePresenceService::CheckOfflineStatus(const VS_SimpleStr& id)
{
	VS_CallIDInfo res;
	auto item = m_offline_statuses_cache.GetOfflineStatus(id.m_str);
	if (item.second)
	{
		res.m_extStatusStorage = item.first.ext_status_;
		res.m_homeServer = item.first.locatorBS_;
	}
	return res;
}
bool VS_BasePresenceService::OnPointConnected_Event(const VS_PointParams* prm)
{
	VS_PointParams prm_val = *prm;
	std::string uid = prm->uid;
	auto reazon = prm->reazon;
	auto type = prm->type;
	CallInProcessingThread([uid,type,reazon,this]()
	{
		if (reazon <= 0) {
			dprint1("BSPRESENCE: Error {%d} while Connect to (%s)\n", reazon, uid.c_str());
			if (type == VS_PointParams::PT_SERVER)
			{
				//clean register items
				dprint4("Clean hanging register status requests for server %s\n", uid.c_str());
				for (auto i = m_delay_until_unregister.begin(); i != m_delay_until_unregister.end();)
				{
					if (std::get<0>(i->second) == uid)
						i = m_delay_until_unregister.erase(i);
					else
						++i;
				}
			}
		}
		else if (type == VS_PointParams::PT_SERVER)
		{
			dprint1("BASEPRESENCE: Server (%s) NEW CONNECT | reason: %2d\n", uid.c_str(), reazon);

			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, GETALLUSERSTATUS_METHOD);
			PostRequest(uid.c_str(), 0, cnt, 0, PRESENCE_SRV);
		}
		else
		{
			dprint1("BASEPRESENCE: NOT SERVER Connect!: uid=%s, reason: %2d\n", uid.c_str(), reazon);
		}
	});
	return true;
}

bool VS_BasePresenceService::OnPointDisconnected_Event(const VS_PointParams* prm)
{
	if (prm->type == VS_PointParams::PT_SERVER)
	{
		dprint1("BASEPRESENCE: Server (%s) DISCONNECT | reason: %2d\n", prm->uid, prm->reazon);
		std::string uid = prm->uid;
		CallInProcessingThread([uid, this]
		{
			CleanServer(uid.c_str());
			VS_FullID subs(uid.c_str(), 0);
			UnsubscribeID(subs);
			//clean register items
			dprint4("Clean hanging register status requests for server %s\n",uid.c_str());
			for (auto i = m_delay_until_unregister.begin(); i != m_delay_until_unregister.end();)
			{
				if (std::get<0>(i->second) == uid)
					i = m_delay_until_unregister.erase(i);
				else
					++i;
			}
		});
	}
	return true;
}
void VS_BasePresenceService::CleanServer(const char *server)
{
	/*
	 *  make UnregisterStatus for all call_ids
	 * */
	if (!server || !*server)
		return;
	dprint3("@CleanServer %s\n", server);
	std::map<VS_FullID, VS_Container> ids_for_update_status;;
	std::vector<VS_Container> unregister_method_cnt;
	{
		auto ds = dstream3;
		auto range = m_registred_call_id.get<by_server>().equal_range(server);
		for (auto i = range.first; i != range.second; ++i)
		{
			if (unregister_method_cnt.empty())
				ds << "@Force UnregisterStatus for users (\n";
			ds << i->call_id_ << '\n';
			VS_Container unreg_cnt;
			unreg_cnt.AddValue(METHOD_PARAM, UNREGISTERSTATUS_METHOD);
			unreg_cnt.AddValue(CALLID_PARAM, i->call_id_.c_str());
			unreg_cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
			unregister_method_cnt.push_back(std::move(unreg_cnt));
		}
		if(!unregister_method_cnt.empty())
			ds <<")\n";
	}
	for (const auto &cnt : unregister_method_cnt)
		UnregisterStatus_Method(cnt, server);
}

void VS_BasePresenceService::ListOfOnlineUsers(UsersList &users)
{
	users = std::move(VS_SubscriptionHub::ListOfOnlineUsers());
}

bool VS_BasePresenceService::UsersStatuses(UsersList &users)
{
	return VS_SubscriptionHub::FindStatuses(users, false);
}

#endif