#pragma once
#include "chatlib/chat_defs.h"
#include "chatlib/helpers/ResolverInterface.h"

#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"
#include "std-generic/cpplib/synchronized.h"
#include "std/cpplib/VS_Utils.h"

#include "boost/asio/io_service.hpp"
#include "boost/asio/strand.hpp"

#include <memory>

class ResolverForChatImpl
	: public vs::ResolverInterface
	, public vs::enable_shared_from_this<ResolverForChatImpl>
	, public VS_PresenceServiceMember
{
	using cache_type = vs::map<chat::CallID, ResolveInfo, vs::less<>>;
	vs::Synchronized<cache_type> resolve_cache_;

	template<class CallBack>
	void do_resolve(
		chat::CallIDRef call_id,
		CallBack&& cb)
	{
		auto resolve_res = resolve_cache_.withLock([&](cache_type &cache)
		{
			auto it = cache.find(call_id);
			if (cache.end() == it)
				return ResolveInfo();
			return it->second;
		});
		if (!resolve_res.callId.empty())
		{
			cb(true, std::move(resolve_res));
			return;
		}
		if (VS_IsBrokerFormat(call_id))
		{
			auto result = ResolveInfo(call_id,
				vs::CallIDType::server, call_id,
				std::vector<chat::CallID>{ {chat::CallID(call_id)} });
			resolve_cache_->emplace(call_id, result);
			cb(true, std::move(result));
			return;
		}
		m_strand.dispatch(
			[self = shared_from_this(),
			call_id = chat::CallID(call_id),
			cb = std::move(cb),
			this]() mutable
		{
			VS_CallIDInfo ci;
			VS_SimpleStr res_callid = std::string(call_id).c_str();
			m_presenceService->Resolve(res_callid, ci);		// todo(kt): sip is occupied here now
			if (ci.m_homeServer.empty())
				cb(false, {});
			else
			{
				auto result = ResolveInfo(res_callid.m_str,
					vs::CallIDType::client,
					ci.m_homeServer,
					std::vector < chat::CallID> { {res_callid.m_str} });
				resolve_cache_->emplace(call_id, result);
				cb(true, std::move(result));
			}
		});
	}
protected:
	ResolverForChatImpl(boost::asio::io_service&ios)
		: m_strand(ios)
	{}
private:
	void Resolve(chat::CallIDRef call_id, const ResolveCallBack &cb) override
	{
		do_resolve(call_id, [cb](bool res, ResolveInfo &&info)
		{
			cb(res, std::move(info));
		});
	}
	void ResolveList(std::vector<chat::CallID> &&idx, const ResolveListCallBack &cb) override
	{
		if (idx.empty())
		{
			cb({});
			return;
		}
		auto sz = idx.size();
		auto cb_ref = std::make_shared<ResolveListCallBack>(cb);
		auto result_ptr = std::make_shared<vs::Synchronized<ResolveListResult>>();
		for (const auto &id : idx)
		{
			do_resolve(id,
				[sz, result_ptr, cb_ref](bool res, ResolveInfo &&info)
			{
				ResolveListResult cnt_for_ret;
				result_ptr->withLock([&](ResolveListResult &cnt)
				{
					cnt.emplace_back(res, std::move(info));
					if (cnt.size() == sz)
						cnt_for_ret = std::move(cnt);
				});
				if (!cnt_for_ret.empty())
					(*cb_ref)(std::move(cnt_for_ret));
			});
		}
	}
	void GetCallIDByAlias(
		chat::CallIDRef alias, const GetCallIdByAliasCallBack &cb) override
	{
		do_resolve(alias, [cb](bool res, ResolveInfo &&info)
		{
			cb(res, info.callId);
		});
	}
	void ResolveCallIdType(
		chat::CallIDRef call_id, const ResolveCallIdTypeCallBack &cb) override
	{
		cb(VS_IsBrokerFormat(call_id)
			? vs::CallIDType::server
			: vs::CallIDType::client);
	}
	boost::asio::io_service::strand m_strand;
};