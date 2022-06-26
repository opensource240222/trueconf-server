#include "ResolverStub.h"
#include "chatlib/helpers/AccountInfo.h"
#include "std/cpplib/VS_Utils.h"

void ResolverStub::Resolve(vs::CallIDRef id,
	const vs::ResolverInterface::ResolveCallBack &cb)
{
	GetCallIDByAlias(id, [this, cb](bool res, vs::CallIDRef callId)
	{
		if (!res)
			cb(false, {});
		else
		{
			const auto& iter = m_cache.find(callId);
			if (iter == m_cache.end())
				cb(false, {});
			else
				cb(true, ResolveInfo(iter->second));
		}
	});
}
void ResolverStub::ResolveList(std::vector<chat::CallID> &&idx, const ResolveListCallBack &cb)
{
	ResolveListResult list;
	for (const auto &i : idx)
	{
		auto callId = m_alias.find(i);
		if (callId != m_alias.end())
		{
			auto iter = m_cache.find(callId->second);
			if (iter != m_cache.end())
				list.emplace_back(true, iter->second);
			else
				list.emplace_back(false, ResolveInfo());
		}
	}
	cb(std::move(list));
}
void ResolverStub::ResolveCallIdType(vs::CallIDRef callId,
	const vs::ResolverInterface::ResolveCallIdTypeCallBack &cb)
{
	auto id = std::string(callId);
	if (VS_IsBrokerFormat(id))
		cb(vs::CallIDType::server);
	else
		cb(vs::CallIDType::client);
}
void ResolverStub::GetCallIDByAlias(vs::CallIDRef alias,
	const vs::ResolverInterface::GetCallIdByAliasCallBack &cb)
{
	auto callId = m_alias.find(alias);
	if (callId != m_alias.end())
		cb(true, callId->second);
	else
		cb(false, std::string());
}
void ResolverStub::Add(vs::CallIDRef callId, vs::CallIDType type, string_view bs,
	std::vector<vs::CallID> &&ep)
{
	vs::ResolverInterface::ResolveInfo res(callId, type, bs, std::move(ep));
	m_cache.emplace(callId, std::move(res));
}
void ResolverStub::Add(vs::CallIDRef callId, const vs::AccountInfoPtr &info)
{
	vs::ResolverInterface::ResolveInfo res(
		info->GetCallID(),
		info->GetCallIDType(),
		info->GetBS(),
		info->GetAllEndpoints());
	m_cache.emplace(callId, std::move(res));
}
void ResolverStub::AddAlias(std::string callId, std::string alias)
{
	m_alias.insert_or_assign(std::move(alias), std::move(callId));
}