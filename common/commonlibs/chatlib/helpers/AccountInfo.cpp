#include "AccountInfo.h"

namespace vs
{
const CallID& AccountInfo::GetCallID() const
{
	return m_callId;
}
const CallID & AccountInfo::GetExactCallID() const
{
	return m_exactCallId;
}
CallIDType AccountInfo::GetCallIDType() const
{
	return m_type;
}
const BSInfo & AccountInfo::GetBS() const
{
	return m_bs;
}
bool AccountInfo::IsMyCallId(CallIDRef call_id) const
{
	return call_id == m_callId;
}
std::vector<CallID> AccountInfo::GetAllEndpoints() const
{
	return !m_getRelatedCallids
		? std::vector<CallID>()
		: m_getRelatedCallids(m_callId);
}
}