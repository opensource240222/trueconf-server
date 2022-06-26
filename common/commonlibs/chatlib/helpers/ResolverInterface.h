#pragma once
#include "vs_def.h"

#include "std-generic/cpplib/macro_utils.h"
#include <functional>
namespace vs
{
class ResolverInterface
{
public:
	struct ResolveInfo
	{
		VS_FORWARDING_CTOR4(ResolveInfo, callId, type, bsInfo, epList) {}
		ResolveInfo() = default;
		CallID callId;
		CallIDType type;
		BSInfo bsInfo;
		std::vector<CallID> epList;
	};
	using ResolveCallBack = std::function<void(bool/** result */, ResolveInfo&&)>;
	using ResolveListResult = std::vector<std::pair<bool, ResolveInfo>>;
	using ResolveListCallBack = std::function<void(ResolveListResult &&)>;
	using GetCallIdByAliasCallBack = std::function<void(bool /**result*/, CallIDRef)>;
	using ResolveCallIdTypeCallBack = std::function<void(CallIDType)>;

	virtual ~ResolverInterface() = default;
	virtual void Resolve(CallIDRef, const ResolveCallBack&) = 0;
	virtual void ResolveList(std::vector<CallID> &&idx, const ResolveListCallBack &) = 0;
	virtual void GetCallIDByAlias(
		CallIDRef alias, const GetCallIdByAliasCallBack &cb) = 0;
	virtual void ResolveCallIdType(
		CallIDRef callId, const ResolveCallIdTypeCallBack &cb) = 0;
};
}