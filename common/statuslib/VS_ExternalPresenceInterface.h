#pragma once

#include <string>

class VS_CallIDInfo;
class VS_UserData;

class VS_ExternalPresenceInterface
{
public:
	virtual bool Resolve(std::string& call_id, VS_CallIDInfo& ci, VS_UserData* from_user) = 0;
	virtual void Subscribe(const char *call_id) = 0;
	virtual void Unsubscribe(const char *call_id) = 0;
	virtual bool IsMyCallId(const char *call_id) const = 0;
	virtual bool CanICall(VS_UserData* from_ude, const char* to_call_id, bool IsVCS) = 0;
	virtual bool IsRegisteredTransId(const char* /*trans_id*/) { return false; }
};