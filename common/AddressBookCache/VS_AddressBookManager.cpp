
#include "VS_AddressBookManager.h"
#include "../transport/client/VS_TransportClient.h"
#include "../vsclient/VS_Dmodule.h"
#include "../vsclient/VSTrClientProc.h"
#include <direct.h>

namespace { // TODO: remove this dup
	time_t _SystemTimeToUnixTime(LPSYSTEMTIME pst)
	{
		union x64 { time_t t; struct { unsigned long lDword; unsigned long hDword; }; };
		const time_t offset = 116444736000000000;
		x64 value;

		FILETIME ft = {};
		SystemTimeToFileTime(pst, &ft);

		value.lDword = ft.dwLowDateTime;
		value.hDword = ft.dwHighDateTime;
		return (value.t - offset) / 10000000;
	}
}

VS_AddressBookCallBackInterface* VS_AddressBookManager::m_call_back = 0;

VS_AddressBookManager::VS_AddressBookManager()
{
	m_isValid = false;
	_mkdir(VS_CACHE_PATH);
}

VS_AddressBookManager::~VS_AddressBookManager()
{
}

bool VS_AddressBookManager::Init(const char *call_id, const char *server)
{
	if (!call_id || !*call_id || !server || !*server)
		return false;

	m_call_id = call_id;
	m_server = server;
	VS_SimpleStr path(VS_CACHE_PATH);
	path +="\\";
	path += m_call_id;
	_mkdir(path);

	m_subs.Clear();
	m_isValid = m_addr_book.Init(call_id) && m_ban_list.Init(call_id);
	m_isValid = m_isValid && m_users.Init(call_id) && m_phone_book.Init(call_id);
	return m_isValid;
}

bool VS_AddressBookManager::IsValid() const
{
	return m_isValid;
}


void VS_AddressBookManager::NotifyAddressBook(const VS_AddressBook ab, long hash)
{
	if (!IsValid() || !m_call_back)
		return;

	VS_ABUser *users(0);
	unsigned  long size(0);
	if (ab==AB_COMMON) {
		m_addr_book.GetBook(users, size);
		Subscribe(users, size, true);
	}
	else if (ab==AB_BAN_LIST)
		m_ban_list.GetBook(users, size);
	else
		return;

	if (users) {
		for (unsigned int i = 0; i<size; i++) {
			VS_UserPicCache* pic = m_users.GetPicItem(users[i].m_callId);
			if (pic)
				pic->GetPic(users[i].m_avatar);
			users[i].m_status = GetStatus(users[i].m_callId);
		}
	}
	m_call_back->AddressBook(users, size, ABA_LIST, ab, hash);
	if (users)
		delete [] users;
}

void VS_AddressBookManager::NotifyAddressBookHash(const VS_AddressBook ab)
{
	if (!IsValid() || !m_call_back) return;
	m_call_back->AddressBookHash(0, 0, ABA_LIST, ab);
}

void VS_AddressBookManager::NotifyAddressBookError(const long error, const VS_AddressBook ab)
{
}

void VS_AddressBookManager::NotifyAddressBookUser(const char *call_id, const wchar_t *dn, const VS_AB_Action action, const long ab, long hash, bool update)
{
	if (!IsValid() || !m_call_back)
		return;

	VS_ABUser new_record;
	new_record.m_callId = call_id;
	if (dn)
		new_record.m_displayName = dn;
	new_record.m_status = GetStatus(call_id);
	if (ab==AB_COMMON)
		if (action==ABA_ADD)
			Subscribe(&new_record, 1, true);
		else if (action==ABA_REMOVE)
			Subscribe(&new_record, 1, false);
	m_call_back->AddressBook(&new_record, 1, action, (VS_AddressBook)ab, hash, update);
}

void VS_AddressBookManager::NotifyUserDetailes(const char *call_id)
{
	if (!IsValid() || !m_call_back)
		return;
	VS_ABUserInfo user_detailes;
	VS_UserDetaileCache* dui = m_users.GetUserItem(call_id);
	if (!dui) return;
	dui->GetDUI(user_detailes);
	m_call_back->UserDetailes(user_detailes);
}

void VS_AddressBookManager::NotifyUserPicture(const char *call_id, long hash)
{
	if (!IsValid() || !m_call_back)
		return;
	VS_ABUser new_record;
	new_record.m_callId = call_id;
	VS_UserPicCache* pic = m_users.GetPicItem(call_id);
	if (!pic) return;
	pic->GetPic(new_record.m_avatar);
	new_record.m_status = GetStatus(call_id);
	m_call_back->AddressBook(&new_record, 1, ABA_LIST, AB_USER_PICTURE, hash);
}

void VS_AddressBookManager::NotifyUserPictureHash(const char *call_id)
{
	if (!IsValid() || !m_call_back)
		return;
	VS_ABUser new_record;
	new_record.m_callId = call_id;
	VS_UserPicCache* pic = m_users.GetPicItem(call_id);
	if (!pic) return;
	pic->GetPic(new_record.m_avatar);
	new_record.m_status = GetStatus(call_id);
	m_call_back->AddressBookHash(&new_record, 1, ABA_LIST, AB_USER_PICTURE);
}

void VS_AddressBookManager::ParseIncomimgMessage(VS_Container &cnt)
{
	if(!cnt.IsValid())
		return;

	long ab;
	const char *call_id = 0;
	VS_SimpleStr method = cnt.GetStrValueRef(METHOD_PARAM);

	if(method == SEARCHADDRESSBOOK_METHOD) {
		if (!cnt.GetValue(ADDRESSBOOK_PARAM, ab))
			return;

		long hash(0); cnt.GetValue(HASH_PARAM, hash);
		VS_CacheInterface *cache = 0;

		if (ab==AB_COMMON) {
			cache = &m_addr_book;
		}
		else if (ab==AB_BAN_LIST) {
			cache = &m_ban_list;

			cnt.Reset();
			VS_MapS found_pers;
			VS_SimpleStr call_id;
			VS_WideStr dn;
			long res(0);
			cnt.GetValue(RESULT_PARAM,res);
			int i(0);
			if(res>0)
			{
				VS_ABUser * users = new VS_ABUser[res];
				while(cnt.Next())
				{
					if (_stricmp(cnt.GetName(), USERNAME_PARAM) == 0) {
						users[i++].m_callId = cnt.GetStrValueRef();
					}
				}
				m_call_back->AddressBook(users,res,ABA_LIST,AB_BAN_LIST, hash);
				delete [] users;
			}

		}
		else if (ab==AB_MISSED_CALLS) {
			cnt.Reset();
			VS_MapS found_pers;
			VS_SimpleStr call_id;
			VS_WideStr dn;
			long res(0);
			cnt.GetValue(RESULT_PARAM,res);
			int i(0);
			if(res>0)
			{
				VS_ABUser * users = new VS_ABUser[res];
				while(cnt.Next())
				{
					if (i>=res) break;

					if (_stricmp(cnt.GetName(), USERNAME_PARAM) == 0) {
						users[i].m_callId = cnt.GetStrValueRef();
					} else if (_stricmp(cnt.GetName(), TIME_PARAM) == 0) {
						unsigned long size = 0; SYSTEMTIME s = {};
						const void* time = cnt.GetBinValueRef(size);
						if (time && size == sizeof(SYSTEMTIME)) memcpy(&s, time, size);
						users[i].m_lastEventTime = _SystemTimeToUnixTime(&s);
						++i;
					}
				}

				m_call_back->AddressBook(users,res,ABA_LIST,AB_MISSED_CALLS, hash);
				delete [] users;
			}

		}
		else if (ab==AB_PERSON_DETAILS)
		{
			call_id = cnt.GetStrValueRef(CALLID_PARAM);
			cache = m_users.GetUserItem(call_id);
		}
		else if (ab==AB_USER_PICTURE) {
			call_id = cnt.GetStrValueRef(USERNAME_PARAM);
			if (!call_id)
				call_id = cnt.GetStrValueRef(QUERY_PARAM);
			cache = m_users.GetPicItem(call_id);
		}
		else if(ab == AB_PERSONS)
		{
			cnt.Reset();
			VS_MapS found_pers;
			VS_SimpleStr call_id;
			VS_WideStr dn;
			long res(0);
			cnt.GetValue(RESULT_PARAM,res);
			int i(0);
			if(res>0)
			{
				VS_ABUser * users = new VS_ABUser[res];
				while(cnt.Next())
				{
					if (_stricmp(cnt.GetName(), USERNAME_PARAM) == 0) {
						users[i].m_callId = cnt.GetStrValueRef();
					}
					else if(_stricmp(cnt.GetName(), DISPLAYNAME_PARAM) == 0) {
						users[i].m_displayName.AssignUTF8(cnt.GetStrValueRef());
					}
					else if(_stricmp(cnt.GetName(), USERPRESSTATUS_PARAM) == 0)
					{
						long st(0);
						cnt.GetValue(st);
						users[i].m_status = st;
						i++;
					}
				}
				m_call_back->AddressBook(users,res,ABA_LIST,AB_PERSONS, hash);
				delete [] users;
			}
		}
		else if(ab == AB_PHONES)
		{
			cache = &m_phone_book;
		}

		if (cache) {
			//cnt.PrintF();
			long cause(SEARCH_FAILED);
			long result(SEARCH_FAILED);
			//Yulian: we check here both result and cause
			//if we don't compare cause == SEARCH_DONE
			//addressbook does not load at all (bug?)
			cnt.GetValue(CAUSE_PARAM, cause);
			cnt.GetValue(RESULT_PARAM, result);
			if (cause == SEARCH_DONE || result == SEARCH_DONE) {
				if (m_call_back) m_call_back->ClearBookHash((VS_AddressBook)ab);
				cache->DeSerialize(cnt);
				cache->UpdateCache(hash);
				NotifyAddressBook((VS_AddressBook)ab, hash);
				if (ab==AB_PERSON_DETAILS)
					NotifyUserDetailes(call_id);
				else if (ab==AB_USER_PICTURE)
					NotifyUserPicture(call_id, hash);
				else if (ab==AB_PHONES)
				{
					VS_ABPhoneNumber * phoneNumbers = NULL;
					long count = 0;
					m_phone_book.GetPhones(phoneNumbers,count);
					m_call_back->PhoneBook(phoneNumbers,count);
				}
			}
			else {
				cache->UpdateTime();
				switch (ab)
				{
				case AB_USER_PICTURE:
					{
						NotifyUserPictureHash(call_id);
					}
					break;

				case AB_COMMON:
				case AB_MISSED_CALLS:
				case AB_BAN_LIST:
					{
						NotifyAddressBookHash((VS_AddressBook)ab);
					}
					break;
				}
			}
		}
	}
	else if (method == ADDTOADDRESSBOOK_METHOD || method == REMOVEFROMADDRESSBOOK_METHOD || method == UPDATEADDRESSBOOK_METHOD) {
		long error(0);
		if (!cnt.GetValue(ADDRESSBOOK_PARAM, ab))
			return;
		if(method != REMOVEFROMADDRESSBOOK_METHOD)
		{
			cnt.GetValue(CAUSE_PARAM, error);
			if (error) {
				NotifyAddressBookError(error, (VS_AddressBook)ab);
				return;
			}
		}

		long hash(0); cnt.GetValue(HASH_PARAM, hash);
		const char *CallId = cnt.GetStrValueRef(CALLID_PARAM);
		const char *dn8 = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
		VS_WideStr	DisplayName;
		if (dn8)
			DisplayName.AssignUTF8(dn8);

		if (method == ADDTOADDRESSBOOK_METHOD) {
			if (ab==AB_COMMON)
				m_addr_book.UpdateAddressBookRecord(CallId, DisplayName, hash);
			else if (ab==AB_BAN_LIST)
				m_ban_list.AddToBanList(CallId, hash);
			NotifyAddressBookUser(CallId, DisplayName, ABA_ADD, ab, hash);
		}
		else if (method == REMOVEFROMADDRESSBOOK_METHOD) {
			if (ab==AB_COMMON)
				m_addr_book.RemoveFromAddressBook(CallId, hash);
			else if (ab==AB_BAN_LIST)
				m_ban_list.RemoveFromBanList(CallId, hash);
			NotifyAddressBookUser(CallId, DisplayName, ABA_REMOVE, ab, hash);
		}
		else if (method == UPDATEADDRESSBOOK_METHOD) {
			if (ab==AB_COMMON)
				m_addr_book.UpdateAddressBookRecord(CallId, DisplayName, hash);
			NotifyAddressBookUser(CallId, DisplayName, ABA_LIST, ab, hash, true);
		}
	}
	else if(method == UPDATESTATUS_METHOD) {
		long cause = 0; cnt.GetValue(CAUSE_PARAM,cause);
		if(cause==1)
			m_statuses.Clear();

		VS_SimpleStr call_id;
		cnt.Reset();
		VS_MapS m_st;

		while(cnt.Next()) {
			if (_stricmp(cnt.GetName(), CALLID_PARAM)==0 || _stricmp(cnt.GetName(), USERNAME_PARAM)==0) {
				call_id = cnt.GetStrValueRef();
			}
			else if(_stricmp(cnt.GetName(),USERPRESSTATUS_PARAM)==0) {
				long lval(0);
				cnt.GetValue(lval);
				m_statuses.Assign(call_id, (const void*)lval);
				m_st.Insert(call_id, (const void*)lval);
			}
		}
		int size = m_st.Size();
		if (size > 0) {
			VS_ABUser *users = new VS_ABUser[size];
			int i = 0;
			for (VS_MapS::Iterator it = m_st.Begin(); it != m_st.End(); ++it, ++i) {
				users[i].m_callId = (char*)it->key;
				users[i].m_status = (int)it->data;
			}
			if (m_call_back)
				m_call_back->AddressBook(users, size, ABA_LIST, AB_STATUS, 0);
			delete[] users;
		}
	}
}



bool VS_AddressBookManager::GetAddressBook()
{
	if (!IsValid())
		return false;
	NotifyAddressBook(AB_COMMON, 0);
	if (!m_addr_book.IsActual()) {
		long hash(0);
		m_addr_book.Clear(); // BUG: relogin condition here
		hash = m_addr_book.GetHash();
		SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_COMMON, "", hash);
	}
	return true;
}

bool VS_AddressBookManager::GetAddressBook(int hash)
{
	return IsValid() ? (0 != SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_COMMON, "", hash)) : false;
}


bool VS_AddressBookManager::GetBanList()
{
	if (!IsValid())
		return false;
	NotifyAddressBook(AB_BAN_LIST, 0);
	if (!m_ban_list.IsActual()) {
		long hash(0);
		hash = m_ban_list.GetHash();
		SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_BAN_LIST, "", hash);
	}
	return true;
}

bool VS_AddressBookManager::GetBanList(int hash)
{
	return IsValid() ? (0 != SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_BAN_LIST, "", hash)) : false;
}

bool VS_AddressBookManager::GetMissedCalls()
{
	if (!IsValid())
		return false;
	NotifyAddressBook(AB_BAN_LIST, 0);
	if (!m_ban_list.IsActual()) {
		long hash(0);
		hash = m_ban_list.GetHash();
		SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_MISSED_CALLS, "", hash);
	}
	return true;
}

bool VS_AddressBookManager::GetMissedCalls(int hash)
{
	return IsValid() ? (0 != SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_MISSED_CALLS, "", hash)) : false;
}

bool VS_AddressBookManager::GetUserDetailes(const char *call_id)
{
	if (!IsValid())
		return false;

	if (!call_id || !*call_id)
		return false;

	NotifyUserDetailes(call_id);
	VS_UserDetaileCache* dui = m_users.GetUserItem(call_id);
	if (!dui || !dui->IsActual()) {
		long hash(0);
		hash = dui->GetHash();
		SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_PERSON_DETAILS, call_id, hash);
	}
	return true;
}


bool VS_AddressBookManager::GetUserPicture(const char *call_id)
{
	if (!IsValid())
		return false;

	if (!call_id || !*call_id)
		return false;

	NotifyUserPicture(call_id, 0);
	VS_UserPicCache* pic = m_users.GetPicItem(call_id);
	if (!pic || !pic->IsActual()) {
		long hash(0);
		hash = pic->GetHash();
		SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_USER_PICTURE, call_id, hash);
	}
	return true;
}

bool VS_AddressBookManager::GetPhones()
{
	if (!IsValid())
		return false;

	/*NotifyUserDetailes(call_id);
	VS_UserDetaileCache* dui = m_users.GetUserItem(call_id);
	if (!dui || !dui->IsActual()) {
		long hash(0);
		hash = dui->GetHash();
		SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_PERSON_DETAILS, call_id, hash);
	}*/
	SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_PHONES, "", 0);
	return true;
}

bool VS_AddressBookManager::GetUserPicture(const char *call_id, int hash)
{
	return (call_id && *call_id && IsValid()) ? (0 != SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_USER_PICTURE, call_id, hash)) : false;
}

long VS_AddressBookManager::GetStatus(const char *call_id)
{
	VS_MapS::Iterator st = m_statuses.Find(call_id);
	long status = st!=m_statuses.End() ? (long)st->data : USER_LOGOFF;
	return status;
}


void VS_AddressBookManager::Subscribe(VS_ABUser *users, int num, bool plus)
{
	if (!users || !num)
		return;

	const char * method = plus ? SUBSCRIBE_METHOD : UNSUBSCRIBE_METHOD;
	DTRACE(VSTM_PRTCL, "%s of: %s..., num=%d", method, users[0].m_callId, num);

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, method);
	if (m_subs.Empty() && plus)
		cnt.AddValue(CAUSE_PARAM,  (long)1);
	for (int i = 0; i<num; i++) {
		if (plus) {
			if (m_subs.Find(users[i].m_callId)==m_subs.End()) {
				m_subs.Insert(users[i].m_callId, 0);
				cnt.AddValue(CALLID_PARAM, users[i].m_callId);
			}
		}
		else {
			if (m_subs.Find(users[i].m_callId)!=m_subs.End()) {
				m_subs.Erase(users[i].m_callId);
				cnt.AddValue(CALLID_PARAM, users[i].m_callId);
			}
		}
	}
	ComposeSend(cnt, PRESENCE_SRV);
}


bool VS_AddressBookManager::SetDisplayName(const char *call_id, const wchar_t *dm)
{
	DTRACE(VSTM_PRTCL, "SetDisplayName of %s = <%S>", call_id, dm);
	if (!dm)
		return false;
	VS_Container cnt;
	m_addr_book.UpdateAddressBookRecord(call_id, dm, 0);
	cnt.AddValue(METHOD_PARAM, UPDATEADDRESSBOOK_METHOD);
	cnt.AddValue(ADDRESSBOOK_PARAM, (long)AB_COMMON);
	cnt.AddValue(CALLID_PARAM, call_id);
	cnt.AddValue(DISPLAYNAME_PARAM, dm);
	ComposeSend(cnt);
	return true;
}

bool VS_AddressBookManager::SetAvatar(const void *pic_buf, const unsigned long sz)
{
	DTRACE(VSTM_PRTCL, "SetAvatar of %s, size = %d", m_call_id, sz);
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, UPDATEADDRESSBOOK_METHOD);
	cnt.AddValue(ADDRESSBOOK_PARAM, (long)AB_USER_PICTURE);
	cnt.AddValue(CALLID_PARAM, m_call_id);
	cnt.AddValue("avatar_type", "image/jpeg");
	cnt.AddValue("avatar", pic_buf, sz);
	ComposeSend(cnt);
	return true;
}

bool VS_AddressBookManager::AddToAddressBook(const char * call_id)
{
	SendUserQuery(call_id, ADDTOADDRESSBOOK_METHOD, AB_COMMON, 0);
	return true;
}

bool VS_AddressBookManager::AddToBanList(const char * call_id)
{
	SendUserQuery(call_id, ADDTOADDRESSBOOK_METHOD, AB_BAN_LIST, 0);
	return true;
}
bool VS_AddressBookManager::RemoveFromAddressBook(const char *call_id)
{
	SendUserQuery(call_id, REMOVEFROMADDRESSBOOK_METHOD, AB_COMMON, 0);
	return true;
}
bool VS_AddressBookManager::RemoveFromBanList(const char *call_id)
{
	SendUserQuery(call_id, REMOVEFROMADDRESSBOOK_METHOD, AB_BAN_LIST, 0);
	return true;
}

bool VS_AddressBookManager::UserSearch(const char *query)
{
	if (!IsValid())
		return false;

	SendUserQuery(0, SEARCHADDRESSBOOK_METHOD, AB_PERSONS, query, 0);
	return true;
}

bool VS_AddressBookManager::Clear()
{
	m_statuses.Clear(); // BUG: relogin condition here
	m_subs.Clear();
	m_addr_book.Clear();
	m_ban_list.Clear();
	m_users.Clear();
	return true;
}

unsigned long VS_AddressBookManager::ComposeSend(VS_Container &cnt, const char* service, const char *user, const char *server)
{
	unsigned long bodySize;	void *body;
	if (cnt.SerializeAlloc(body, bodySize)) {
		VS_ClientMessage tMsg(service, VSTR_PROTOCOL_VER, 0, service, 20000, body, bodySize, user, 0, server ? server : m_server);
		free(body);
		return tMsg.Send();
	}
	else
		return 0;
}

unsigned long VS_AddressBookManager::SendUserQuery(const char* user, const char* method, long addressBook, const char* query, long hash)
{
	DTRACE(VSTM_PRTCL, "SendUserQuery = %s, user=%s, query='%s', ab=%d, hash=%d", method, user, query, addressBook, hash);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, method);
	rCnt.AddValue(ADDRESSBOOK_PARAM, addressBook);
	if (user) rCnt.AddValue(NAME_PARAM, user);
	if (query) rCnt.AddValue(QUERY_PARAM, query);
	if (hash) rCnt.AddValue(HASH_PARAM, hash);
	return ComposeSend(rCnt);
}

bool VS_AddressBookManager::ChatSend(wchar_t *Message, char * To)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SENDMESSAGE_METHOD);
	rCnt.AddValue(FROM_PARAM, m_call_id);
	//rCnt.AddValue(DISPLAYNAME_PARAM, "DN");
	rCnt.AddValue(MESSAGE_PARAM, Message);
	rCnt.AddValue(TO_PARAM, To);
	ComposeSend(rCnt, CHAT_SRV, To, "");
	return true;
}
