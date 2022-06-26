#include "VS_GatewayABStorage.h"
//#include "../../common/std/cpplib/VS_CallIDUtils.h"
#include "../../common/TrueGateway/VS_GatewayStarter.h"
#include "../../common/TrueGateway/CallConfig/VS_Indentifier.h"
#include "AppServer/Services/VS_Storage.h"
#include "std/cpplib/VS_CallIDUtils.h"

int VS_GatewayABStorage::FindUsers(VS_Container &cnt, int &entries, VS_AddressBook ab, const vs_user_id &owner, const std::string &query, long client_hash, VS_Container &in_cnt)
{
	if (IsMyCallId(query.c_str()))
		return FindGatewayUsers(cnt, entries, ab, owner, query, client_hash, in_cnt);
	else
		return VS_VCSABStorage::FindUsers(cnt, entries, ab, owner, query, client_hash, in_cnt);
	entries = 1;
	return SEARCH_DONE;
}
bool VS_GatewayABStorage::IsMyCallId(string_view call_id) const {
	if (call_id.empty())
		return false;

	return VS_IsRTPCallID(call_id);
}
int VS_GatewayABStorage::FindGatewayUsers(VS_Container &cnt, int &entries, VS_AddressBook ab, const vs_user_id &owner, const std::string &query, long client_hash, VS_Container &in_cnt)
{
	std::lock_guard<std::mutex> _(m_map_lock);
	if (ab==AB_PERSON_DETAILS && !query.empty())
	{
		string_view user_id{ query };
		if(!IsMyCallId(user_id))
			return SEARCH_DONE;
		auto h_it = m_users_hashes.find(user_id);
		if(h_it == m_users_hashes.end())
		{
			h_it = m_users_hashes.emplace(user_id, VS_MakeHash(std::chrono::system_clock::now())).first;
		}
		cnt.AddValue(USERNAME_PARAM, user_id);
		cnt.AddValue(CALLID_PARAM,   user_id);

		VS_UserData ud;
		if (g_storage->FindUser(user_id, ud) && !ud.m_displayName.empty()) {
			cnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
		}
		else if (VS_IsRTPCallID(user_id)) {
			cnt.AddValue(DISPLAYNAME_PARAM, vs::PrettyRTPName(user_id));
		}
		/*cnt.AddValue(FIRSTNAME_PARAM, ud.m_FirstName);
		cnt.AddValue(LASTNAME_PARAM, ud.m_LastName);
		cnt.AddValue(USERCOMPANY_PARAM,ud.m_Company);*/
		cnt.AddValueI32(HASH_PARAM, h_it->second);
		entries=1;
		return SEARCH_DONE;
	}
	else
		return SEARCH_FAILED;
}
