#include "chatlib/chat_defs.h"
#include "chatlib/helpers/ResolverInterface.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"

#include <string>

class ResolverStub : public vs::ResolverInterface
{
	vs::map<
		vs::CallID,
		vs::ResolverInterface::ResolveInfo,
		vs::less<>> m_cache;
	vs::map<std::string, std::string, vs::str_less> m_alias;
public:
	void Resolve(
		vs::CallIDRef id,
		const vs::ResolverInterface::ResolveCallBack &cb) override;
	void ResolveList(std::vector<chat::CallID> &&idx, const ResolveListCallBack &) override;
	void ResolveCallIdType(
		vs::CallIDRef callId,
		const vs::ResolverInterface::ResolveCallIdTypeCallBack &cb) override;
	void GetCallIDByAlias(
		vs::CallIDRef alias,
		const vs::ResolverInterface::GetCallIdByAliasCallBack &cb) override;
	void Add(vs::CallIDRef callId, vs::CallIDType type, string_view bs,
		std::vector<vs::CallID> &&ep);
	void Add(vs::CallIDRef callId, const vs::AccountInfoPtr &info);
	void AddAlias(std::string callId, std::string alias);
};
