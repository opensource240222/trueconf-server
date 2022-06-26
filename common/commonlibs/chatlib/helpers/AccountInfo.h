#pragma once
#include "vs_def.h"
#include "std-generic/cpplib/macro_utils.h"
namespace vs {
class AccountInfo
{
	using GetRelatedCallIDS = std::function<std::vector<CallID>(CallIDRef)>;

	CallID m_callId;
	CallID m_exactCallId;
	CallIDType m_type;
	BSInfo m_bs;
	GetRelatedCallIDS m_getRelatedCallids;
public:
	VS_FORWARDING_CTOR5(
		AccountInfo,
		m_callId,
		m_exactCallId,
		m_type,
		m_bs,
		m_getRelatedCallids) {}

	const CallID& GetCallID() const;
	const CallID& GetExactCallID() const;
	CallIDType GetCallIDType() const;
	const BSInfo& GetBS() const;
	bool IsMyCallId(CallIDRef call_id) const;
	// FIXME: remove this method, use resolver for getting all endpoints (after subscribing implementing)
	std::vector<CallID> GetAllEndpoints() const;
};
}