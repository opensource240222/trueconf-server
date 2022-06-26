/****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 * $Revision: 19 $
 * $History: VS_HomeForwarderService.cpp $
 *
 * *****************  Version 19  *****************
 * User: Ktrushnikov  Date: 26.10.11   Time: 13:46
 * Updated in $/VSNA/Servers/AppServer/Services
 * - long time for AddToAddressBook in roaming between service
 *
 * *****************  Version 18  *****************
 * User: Ktrushnikov  Date: 10.08.11   Time: 22:23
 * Updated in $/VSNA/Servers/AppServer/Services
 * - send offline chat though AS (not by Resolve)
 *
 * *****************  Version 17  *****************
 * User: Ktrushnikov  Date: 9.08.11    Time: 17:20
 * Updated in $/VSNA/Servers/AppServer/Services
 * - VS_DNSGetASForDomain: AddToAB, PERSON_DETAILS, offline chat
 * - Roaming_GetDN in offline mode (ask AS->BS instead of RS)
 * - AddToAB: try FindUser() to solve alias problem in trueconf.com
 *
 * *****************  Version 16  *****************
 * User: Ktrushnikov  Date: 15.12.10   Time: 16:00
 * Updated in $/VSNA/Servers/AppServer/Services
 * - conf stat to file in AS (was onlyin VCS)
 * - "Save Conf Stat" dword reg key added
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 24.06.10   Time: 16:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - opt disabled when SECUREBEGIN_
 * - locator bs removed
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 5.11.09    Time: 15:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * - leaks removed
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 2.11.09    Time: 14:51
 * Updated in $/VSNA/Servers/AppServer/Services
 * -store ab in vs_map
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 26.10.09   Time: 16:32
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Inverse automate query
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 1.10.09    Time: 15:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * - autoblock users with bad rating (bugfix#6354)
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 16.09.09   Time: 18:48
 * Updated in $/VSNA/Servers/AppServer/Services
 * - merge
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 15.09.09   Time: 16:39
 * Updated in $/VSNA/Servers/AppServer/Services
 * - Rating added
 * - display_name type changed to char* (from wchar_t)
 * - send BS Events to different service (AUTH,FWD)
 *
 * *****************  Version 8  *****************
 * User: Ktrushnikov  Date: 3.09.09    Time: 12:03
 * Updated in $/VSNA/Servers/AppServer/Services
 * - AS abooks moved to VS_Storage
 *
 * *****************  Version 7  *****************
 * User: Ktrushnikov  Date: 3.09.09    Time: 11:38
 * Updated in $/VSNA/Servers/AppServer/Services
 * Save abooks at AS:
 *   - VS_Container: GetLongValueRef() added
 *   - TRANSPORT_SRCUSER_PARAM: pass user from Transport to Container
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 5.06.09    Time: 20:19
 * Updated in $/VSNA/Servers/AppServer/Services
 * - LOG mesasges living time increased
 * - group conference living  time increased to 10 days
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 4.03.09    Time: 21:09
 * Updated in $/VSNA/Servers/AppServer/Services
 * no BS implementation:
 * - AS login user if have previous cached answer from BS
 * - BS add SERVER_PARAM = OurEndpoint() to rCnt
 * - HomeFWDSrv answers by itself if no BS and mess from ABOOK (emulate
 * SearchAB with "no changes" result)
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:20
 * Updated in $/VSNA/Servers/AppServer/Services
 * SeApptProperties realized
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 23.01.08   Time: 17:51
 * Updated in $/VSNA/Servers/AppServer/Services
 * _malloca redefinition warning fixed.
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 29.12.07   Time: 16:51
 * Updated in $/VSNA/Servers/AppServer/Services
 * debug printout
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 12.12.07   Time: 18:11
 * Created in $/VSNA/Servers/AppServer/Services
 * fixes
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 27.11.07   Time: 18:54
 * Created in $/VSNA/Servers/AppServer/Services
 ***********************************************/

#ifdef _WIN32


#include <malloc.h>
#include <string.h>

#include "../../ServerServices/Common.h"
#include "../../common/std/cpplib/VS_Utils.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"
#include "std/debuglog/VS_Debug.h"

#include "VS_HomeForwarderService.h"
#include "transport/Router/VS_RouterMessage.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "VS_AppServerData.h"

#define DEBUG_CURRENT_MODULE VS_DM_USPRS

bool VS_HomeForwarderService::Init(const char* /*our_endpoint*/, const char* /*our_service*/, const bool /*permittedAll*/)
{
	return true;
}

bool VS_HomeForwarderService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)  return true;

	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
	{
		m_recvMess = recvMess.get();
		const auto body = recvMess->Body();
		const auto bodySize = recvMess->BodySize();

		VS_UserData ud;
		const auto user_id = recvMess->SrcUser_sv();
		if (g_storage->FindUser(user_id, ud) ) {
			if ( !_stricmp(recvMess->DstService(), LOG_SRV))
				ProcessConfStat(body, bodySize);

			if ( !_stricmp(recvMess->SrcService(), ADDRESSBOOK_SRV) ) {
				if (!g_appServer->GetState(ud.m_homeServer)) {
					VS_Container cnt;
					if (cnt.Deserialize(body, bodySize))
						ProcessNoBS(cnt, user_id.c_str());
				}
				else {	// BS alive; from User to BS
					if (TryProcessBoooksForAS(body, bodySize, ud.m_homeServer))
					{

						return true;
					}
				}
			}

			VS_RouterMessage* msg = new VS_RouterMessage(
				recvMess->SrcService(),
				recvMess->AddString(),
				recvMess->DstService(),
				recvMess->DstUser(), recvMess->SrcUser(),
				ud.m_homeServer, recvMess->SrcServer(),
				recvMess->DstService() && _stricmp(recvMess->DstService(), LOG_SRV)==0 ? 300000 : 30000, body, bodySize);
			bool result = PostMes(msg);
			if (!result)
				delete msg;

			dstream3 << "FWDSrv " << recvMess->DstService_sv() << ": forward for user " << user_id << " to server " << SimpleStrToStringView(ud.m_homeServer);
		}
		else {
			if (TryForwardToBS(body, bodySize))
				return true;
			// from BS to User
			VS_SimpleStr empty;
			if (TryProcessBoooksForAS(body, bodySize, empty))
				return true;
			dstream0 << "FWDSrv " << recvMess->DstService_sv() << ": could not find user:'" << user_id << "'";
		}

		break;
	}
	case transport::MessageType::Notify:
		break;
	}

	return true;
}

bool VS_HomeForwarderService::TryProcessBoooksForAS(const void* body, unsigned long bodySize, VS_SimpleStr &homeServer)
{
	if (!body || !bodySize)
		return false;

	bool UseAS = false;
	const char *srcuser = m_recvMess->SrcUser();
	bool IsUser = srcuser && *srcuser;

	VS_Container in_cnt;
	if (in_cnt.Deserialize(body, bodySize)) {
		const char* method = 0;
		if ((method = in_cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
			if ((_stricmp(method, ADDTOADDRESSBOOK_METHOD) == 0) ||
				(_stricmp(method, REMOVEFROMADDRESSBOOK_METHOD) == 0) ||
				(_stricmp(method, UPDATEADDRESSBOOK_METHOD) == 0) ||
				(_stricmp(method, SEARCHADDRESSBOOK_METHOD) == 0))
			{
				if (IsUser) {	// send to BS
					bool locate = false;
					if (_stricmp(method, SEARCHADDRESSBOOK_METHOD) == 0) {
						auto ab = AB_NONE;
						in_cnt.GetValueI32(ADDRESSBOOK_PARAM, ab);
						locate = ab==AB_PERSON_DETAILS || ab==AB_USER_PICTURE;
						const char* q = in_cnt.GetStrValueRef(QUERY_PARAM);
						if (!q) q = "";
						dprint3("FWDSrv: from %s q=%s in ab=%d \n", srcuser, q, ab);
					}

					in_cnt.AddValue(TRANSPORT_SRCUSER_PARAM, m_recvMess->SrcUser());
					ProcessCntFromUser(srcuser, in_cnt);
					if (locate)
						PostRequest(homeServer, 0, in_cnt, ADDRESSBOOK_SRV, LOCATE_SRV);
					else
						PostRequest(homeServer, 0, in_cnt, 0, ADDRESSBOOK_SRV);
					return true;
				}
				else {		// send to User
					ProcessCntFromBS(in_cnt);
					const char* to_user = in_cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
					if (to_user)
						PostRequest(OurEndpoint(), to_user, in_cnt, 0, ADDRESSBOOK_SRV);
					return true;
				}
			}
		}
	}
	return false;
}

void VS_HomeForwarderService::ProcessCntFromBS(VS_Container& cnt)
{
	VS_SimpleStr to_user = cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
	if (!to_user)
		return;

	VS_SimpleStr method = cnt.GetStrValueRef(METHOD_PARAM);
	if (!method)
		return;

	auto ab = AB_NONE; cnt.GetValueI32(ADDRESSBOOK_PARAM, ab);
	int32_t hash = 0;     cnt.GetValue(HASH_PARAM, hash);
	int32_t result = 0;   cnt.GetValue(RESULT_PARAM, result);
	if (result <= 0)
		return;

	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);
	const char* display_name = cnt.GetStrValueRef(DISPLAYNAME_PARAM);

	std::lock_guard<std::mutex> lock(g_storage->m_BookLock);
	VS_Storage::VS_UserBook* user_book = g_storage->GetUserBook(to_user, ab, true);
	if (!user_book)
		return;

	if (method == ADDTOADDRESSBOOK_METHOD) {
		if (call_id)
			user_book->book.Insert(call_id, display_name);
	}
	else if (method == REMOVEFROMADDRESSBOOK_METHOD) {
		if (call_id)
			user_book->book.Erase(call_id);
	}
	else if (method == UPDATEADDRESSBOOK_METHOD) {
		if (call_id)
			user_book->book.Assign(call_id, display_name);
	}
	else if (method == SEARCHADDRESSBOOK_METHOD) {
		int32_t cause = 0;
		cnt.GetValue(CAUSE_PARAM, cause);
		if (cause != SEARCH_DONE)
			return;

		char* id = 0;

		if (ab==AB_COMMON) {
			while (cnt.Next()) {
				const char* param_name = cnt.GetName();

				if (strcmp(param_name,USERNAME_PARAM)==0) {
					id = (char*) cnt.GetStrValueRef();
				}
				else if(strcmp(param_name,DISPLAYNAME_PARAM)==0) {
					const char* str = cnt.GetStrValueRef();
					if (str)
						user_book->book.Assign(id, str);
					id = 0;
				}
			}
		}
		else {
			while (cnt.Next()) {
				const char* param_name = cnt.GetName();
				if (strcmp(param_name, USERNAME_PARAM)==0 || strcmp(param_name, CALLID_PARAM)==0) {
					id = (char*) cnt.GetStrValueRef();
					if (id)
						user_book->book.Insert(id, 0);
				}
			}
		}

		// If user has valid HASH and AS doesn't have books then send SEARCH_NOT_MODIFIED to client
		if (hash && user_book->IsRequestedByServer && (hash == user_book->hash))
		{
			VS_SimpleStr query = cnt.GetStrValueRef(QUERY_PARAM);

			cnt.Clear();
			cnt.AddValue(METHOD_PARAM, SEARCHADDRESSBOOK_METHOD);
			cnt.AddValueI32(ADDRESSBOOK_PARAM, ab);
			cnt.AddValueI32(RESULT_PARAM, -1);
			cnt.AddValueI32(CAUSE_PARAM, SEARCH_NOT_MODIFIED);
			cnt.AddValue(QUERY_PARAM, query);

			cnt.AddValue(TRANSPORT_SRCUSER_PARAM, to_user);
		}

		user_book->IsRequestedByServer = false;

	} else {
		return ;	// nothing changed
	}

	user_book->hash = hash;
}

void VS_HomeForwarderService::ProcessCntFromUser(const char* from_user, VS_Container& cnt)
{
	if (!from_user)
		return;

	const char* method = cnt.GetStrValueRef(METHOD_PARAM);
	if (!method)
		return;

	auto ab = AB_NONE; cnt.GetValueI32(ADDRESSBOOK_PARAM, ab);
	int32_t hash = 0;     cnt.GetValue(HASH_PARAM, hash);

	std::lock_guard<std::mutex> lock(g_storage->m_BookLock);
	VS_Storage::VS_UserBook* user_book = g_storage->GetUserBook(from_user, ab, true);
	if (!user_book)
		return;

	// AS doesn't have books, so ask BS for them (passing null HASH)
	if (hash && !user_book->hash)
	{
		user_book->hash = hash;
		user_book->IsRequestedByServer = true;
		int32_t* hash_ptr = cnt.GetLongValueRef(HASH_PARAM);
		*hash_ptr = 0;
	}
}


void VS_HomeForwarderService::ProcessNoBS(VS_Container& in_cnt, const char* callId)
{
	const char* method = in_cnt.GetStrValueRef(METHOD_PARAM);
	if (!method || (_stricmp(method, SEARCHADDRESSBOOK_METHOD)!=0))
		return ;

	auto ab = AB_NONE;
	int32_t client_hash = 0;
	in_cnt.GetValueI32(ADDRESSBOOK_PARAM,ab);
	in_cnt.GetValue(HASH_PARAM,client_hash);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SEARCHADDRESSBOOK_METHOD);
	rCnt.AddValueI32(ADDRESSBOOK_PARAM, ab);
	rCnt.AddValue(QUERY_PARAM, in_cnt.GetStrValueRef(QUERY_PARAM));

	{
		std::lock_guard<std::mutex> lock(g_storage->m_BookLock);
		VS_Storage::VS_UserBook* user_book = g_storage->GetUserBook(callId, ab);
		if (user_book && user_book->book.Size() && user_book->hash && user_book->hash != client_hash)	// Send AS books
		{
			rCnt.AddValueI32(CAUSE_PARAM, SEARCH_DONE);
			rCnt.AddValueI32(RESULT_PARAM, user_book->book.Size());
			rCnt.AddValueI32(HASH_PARAM, user_book->hash);

			VS_Map::Iterator it;
			for(it=user_book->book.Begin(); it != user_book->book.End(); it++)
			{
				rCnt.AddValue(USERNAME_PARAM, (char*)it->key);
				if (it->data) rCnt.AddValue(DISPLAYNAME_PARAM, (char*)it->data);
			}
		}
		else{
			rCnt.AddValueI32(CAUSE_PARAM, SEARCH_NOT_MODIFIED);
		}
	}

	PostReply(rCnt);
}

void VS_HomeForwarderService::ProcessConfStat(const void* body, unsigned long bodySize)
{
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if (!key.IsValid())
		return ;
	int val = 0;
	if (key.GetValue(&val, sizeof(val), VS_REG_INTEGER_VT, SAVE_CONF_STAT_TAG)>0 && !val)
		return ;

	VS_Container in_cnt;
	if (!in_cnt.Deserialize(body, bodySize))
		return ;

	const char* method = in_cnt.GetStrValueRef(METHOD_PARAM);
	if (!method || !*method)
		return ;

	if (_stricmp(method, LOGPARTSTAT_METHOD)!=0)
		return ;

	VS_UserData ud;
	if (!g_storage->FindUser(in_cnt.GetStrValueView(CALLID_PARAM), ud))
		return ;

	VS_FileConfStat::TrySaveToFileConfStat(in_cnt, ud);
}

bool VS_HomeForwarderService::TryForwardToBS(const void* body, unsigned long bodySize)
{
	if (!m_recvMess->DstService())
		return false;

	if (VS_GetServerType(m_recvMess->SrcServer_sv()) == ST_BS)
		return false;

	VS_SimpleStr domain;
	VS_Container cnt;
	if (cnt.Deserialize(body, bodySize))
	{
		VS_SimpleStr user;
		if (!_stricmp(m_recvMess->DstService(), ADDRESSBOOK_SRV))
			user = cnt.GetStrValueRef(QUERY_PARAM);
		else if(!_stricmp(m_recvMess->DstService(), OFFLINECHAT_SRV))
			user = cnt.GetStrValueRef(TO_PARAM);

		if (!!user)
		{
			VS_RealUserLogin r(SimpleStrToStringView(user));
			domain = StringViewToSimpleStr(r.GetDomain());
		}
	}

	if (!domain)
		return false;

	VS_SimpleStr bs;
	g_appServer->GetBSByDomain(domain, bs);

	dprint3("TryForwardToBS: from=%s, srv=%s, domain=%s, bs=%s\n", m_recvMess->SrcServer(), m_recvMess->DstService(), domain.m_str, bs.m_str);
	if (!!bs)
	{
		VS_RouterMessage* msg = new VS_RouterMessage(
			m_recvMess->SrcService(),
			m_recvMess->AddString(),
			m_recvMess->DstService(),
			m_recvMess->DstUser(), m_recvMess->SrcUser(),
			bs, m_recvMess->SrcServer(),
			30000, body, bodySize);
		bool result = PostMes(msg);
		if (!result)
			delete msg;
	}
	return true;
}
#endif