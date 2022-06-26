
/*************************************************
 * $Revision: 44 $
 * $History: VS_AddressBookService.cpp $
 *
 * *****************  Version 44  *****************
 * User: Stass        Date: 19.07.12   Time: 14:08
 * Updated in $/VSNA/Servers/BaseServer/Services
 * userregistration info by alias
 *
 * *****************  Version 43  *****************
 * User: Mushakov     Date: 17.07.12   Time: 23:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - LoginConfigurator() was removed
 * - messages from configurator are handled by SessionID
 * - fix TransportMessage::IsFromServer()
 *
 * *****************  Version 42  *****************
 * User: Mushakov     Date: 16.07.12   Time: 16:29
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - g_dbStorage was wrapped to shared_ptr
 *
 * *****************  Version 41  *****************
 * User: Ktrushnikov  Date: 11.07.12   Time: 12:56
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #12522: if TrueConfID with uppercase, no update of AB_COMMON from
 * Manager
 *
 * *****************  Version 40  *****************
 * User: Mushakov     Date: 10.05.12   Time: 21:55
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - PersonDetailes handled for transcoders
 *
 * *****************  Version 39  *****************
 * User: Ktrushnikov  Date: 9.04.12    Time: 11:30
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #11391: give back to client call_id2 param when VSS_USER_EXISTS
 *
 * *****************  Version 38  *****************
 * User: Ktrushnikov  Date: 24.02.12   Time: 14:27
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - vs_bak_add_user supported (Add & Remove)
 *
 * *****************  Version 37  *****************
 * User: Ktrushnikov  Date: 17.10.11   Time: 17:25
 * Updated in $/VSNA/Servers/BaseServer/Services
 * ManageGroups:
 * - pass GID_PARAM to client as String (not as Integer)
 *
 * *****************  Version 36  *****************
 * User: Ktrushnikov  Date: 29.09.11   Time: 12:56
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - cmd TYPE_PARAM to CAUSE_PARAM (urixx)
 * - dprint fix
 *
 * *****************  Version 35  *****************
 * User: Ktrushnikov  Date: 5.08.11    Time: 23:14
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - AddToAddressBook for BS same like VCS
 *
 * *****************  Version 34  *****************
 * User: Ktrushnikov  Date: 4.08.11    Time: 10:25
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - process unlucky ABStorage::GetServerOfUser
 *
 * *****************  Version 33  *****************
 * User: Ktrushnikov  Date: 3.08.11    Time: 0:30
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - support VCS to BS request of display_name
 * - g_BasePresenceService added
 * - GetDn in roaming small refactoring
 * - declaration of SearchAddressBook_Task class to header file
 *
 * *****************  Version 32  *****************
 * User: Ktrushnikov  Date: 26.07.11   Time: 19:50
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Hash added for AB_GROUPS
 * - ManageGroups moved to base interface class
 *
 * *****************  Version 31  *****************
 * User: Ktrushnikov  Date: 15.07.11   Time: 15:20
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #8535, #8917
 * - user groups supported in Visicron.dll and BS server
 * (VS_AddressBookService, VS_DBStorage)
 *
 * *****************  Version 30  *****************
 * User: Mushakov     Date: 13.07.11   Time: 17:35
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Roaming
 *
 * *****************  Version 29  *****************
 * User: Mushakov     Date: 8.07.11    Time: 17:22
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  post mess to SrcService
 *
 * *****************  Version 28  *****************
 * User: Mushakov     Date: 8.07.11    Time: 16:45
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - poet mess to SrcService
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 14.06.11   Time: 18:05
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - OnAddressBookChange: set server as OurEndpoint()
 *
 * *****************  Version 26  *****************
 * User: Mushakov     Date: 15.03.11   Time: 16:10
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - 8553 automatic updated user info supported
 *
 * *****************  Version 25  *****************
 * User: Ktrushnikov  Date: 2.02.11    Time: 0:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * vcs 3.2:
 * - AddToAB in a PoolThreadTask
 * - AddToAB user with full reallogin in roaming
 * - VS_Container::AttachToCnt()
 *
 * *****************  Version 24  *****************
 * User: Ktrushnikov  Date: 31.01.11   Time: 10:52
 * Updated in $/VSNA/Servers/BaseServer/Services
 * VCS 3.2
 * - request FirstName, LastName, DisplayName at AB_PERSON_DETAILS in
 * SearchAB_Task
 * - use m_abStorage interface to divide VCS and BS code
 *
 * *****************  Version 23  *****************
 * User: Ktrushnikov  Date: 22.09.10   Time: 16:43
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - new search from client by [name, call_id, email]
 * - sp_set_group_endpoint_properties: directX param size fixed for
 * PostgreSQL
 * - VS_WideStr::Trim() function
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 18.12.09   Time: 18:04
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Removed VCS_BUILD somewhere
 * - Add new field to license
 * - Chat service for bsServer renamed
 *
 * *****************  Version 21  *****************
 * User: Mushakov     Date: 26.11.09   Time: 19:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - init g_vcs_storage by getting Licenses
 *
 * *****************  Version 20  *****************
 * User: Mushakov     Date: 4.11.09    Time: 21:13
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - new names
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 2.11.09    Time: 14:30
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - events for new as
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 2.11.09    Time: 14:15
 * Updated in $/VSNA/Servers/BaseServer/Services
 *
 * *****************  Version 17  *****************
 * User: Ktrushnikov  Date: 28.09.09   Time: 18:31
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - support both directions: to clients and new ASs
 *
 * *****************  Version 16  *****************
 * User: Ktrushnikov  Date: 16.09.09   Time: 18:48
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - merge
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 15.09.09   Time: 16:39
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Rating added
 * - display_name type changed to char* (from wchar_t)
 * - send BS Events to different service (AUTH,FWD)
 *
 * *****************  Version 14  *****************
 * User: Ktrushnikov  Date: 3.09.09    Time: 11:38
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Save abooks at AS:
 *   - VS_Container: GetLongValueRef() added
 *   - TRANSPORT_SRCUSER_PARAM: pass user from Transport to Container
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 7.08.09    Time: 14:48
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 12  *****************
 * User: Ktrushnikov  Date: 13.11.08   Time: 18:32
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - BS Events added
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 25.07.08   Time: 14:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - logging app_ID added
 * - logging multi_conf
 * - bug 4602 fixed
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 26.03.08   Time: 18:43
 * Updated in $/VSNA/Servers/BaseServer/Services
 * added report of call_id to remove
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 27.02.08   Time: 21:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * new parameters in addtoAB
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:13
 * Updated in $/VSNA/Servers/BaseServer/Services
 * GetAppProperties method realized
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 11.02.08   Time: 17:43
 * Updated in $/VSNA/Servers/BaseServer/Services
 * changed service
 *
 * *****************  Version 6  *****************
 * User: Dront78      Date: 23.01.08   Time: 17:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * _malloca redefinition warning fixed.
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 29.12.07   Time: 16:58
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixes  in ab srv
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 12.12.07   Time: 20:40
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed init
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 6.12.07    Time: 18:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * base services done
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * BS - new iteration
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 28.11.07   Time: 18:33
 * Created in $/VSNA/Servers/BaseServer/Services
 * first bs ver
 *
 * ***********************************************/

#include "statuslib/VS_ResolveServerFinder.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../../common/std/cpplib/VS_Map.h"
#include "../../common/std/cpplib/VS_SimpleStr.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_Utils.h"
#include "../../BaseServer/Services/storage/VS_DBStorageInterface.h"

#include "../../ServerServices/Common.h"
#include "ServerServices/VS_ConfRestrictInterface.h"

#include <sstream>

#include "VS_AddressBookService.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"

#define DEBUG_CURRENT_MODULE VS_DM_USPRS

////////////////////////////////////////////////////////////////////////////////
// Init
////////////////////////////////////////////////////////////////////////////////

bool VS_AddressBookService::Init(const char *our_endpoint, const char *our_service, const bool permittedAll )
{
	return true;
};
void VS_AddressBookService::SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface> &confRestrict)
{
	m_confRestrict = confRestrict;
}
void VS_AddressBookService::SetABStorage(const boost::shared_ptr<VS_ABStorageInterface>& abStorage)
{
	m_abStorage = abStorage;
}

////////////////////////////////////////////////////////////////////////////////
// Message Processing
////////////////////////////////////////////////////////////////////////////////

bool VS_AddressBookService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)	return true;
	switch (recvMess->Type()) {
	case transport::MessageType::Invalid:	// Skip
		break;
	case transport::MessageType::Request:
		m_recvMess = recvMess.get();
		{
			VS_Container cnt;

			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
				// Process methods
					if (strcasecmp(method, USERREGISTRATIONINFO_METHOD) == 0)
					{
						UserRegistrationInfo_Method(cnt.GetStrValueRef(NAME_PARAM));
					}
					else if (strcasecmp(method, ADDTOADDRESSBOOK_METHOD) == 0)
					{
						auto ab = AB_NONE;
						cnt.GetValueI32(ADDRESSBOOK_PARAM,ab);
						AddToAddressBook_Method((VS_AddressBook)ab, cnt,std::move(recvMess));
					}
					else if (strcasecmp(method, REMOVEFROMADDRESSBOOK_METHOD) == 0)
					{
						auto ab = AB_NONE;
						cnt.GetValueI32(ADDRESSBOOK_PARAM,ab);
						RemoveFromAddressBook_Method((VS_AddressBook)ab,cnt.GetStrValueRef(NAME_PARAM), cnt,std::move(recvMess));
					}
					else if (strcasecmp(method, SEARCHADDRESSBOOK_METHOD) == 0)
					{
						auto ab = AB_NONE;
						int32_t client_hash = 0;
						cnt.GetValueI32(ADDRESSBOOK_PARAM,ab);
						cnt.GetValue(HASH_PARAM,client_hash);

						SearchAddressBook_Method((VS_AddressBook)ab,cnt.GetStrValueRef(QUERY_PARAM),client_hash, cnt,std::move(recvMess));
					}
					else if (strcasecmp(method, UPDATEADDRESSBOOK_METHOD) == 0)
					{
						auto ab = AB_NONE;
						cnt.GetValueI32(ADDRESSBOOK_PARAM,ab);
						UpdateAddressBook_Method((VS_AddressBook)ab, cnt,std::move(recvMess));
					}
					else if (strcasecmp(method, ONADDRESSBOOKCHANGE_METHOD) == 0)
					{
						auto ab = AB_NONE;
						cnt.GetValueI32(ADDRESSBOOK_PARAM,ab);
						OnAddressBookChange_Method(ab, cnt);
					}
					else if (strcasecmp(method, MANAGEGROUPS_METHOD) == 0)
					{
						int32_t seq_id = 0;				cnt.GetValue(SEQUENCE_ID_PARAM, seq_id);
						int32_t cmd = 0;				cnt.GetValue(CMD_PARAM, cmd);
						int32_t gid = 0;				cnt.GetValue(GID_PARAM, gid);
						VS_SimpleStr call_id =			cnt.GetStrValueRef(CALLID_PARAM);
						VS_WideStr gname;
						auto p = cnt.GetStrValueRef(GNAME_PARAM);
						if (p)
							gname = vs::UTF8ToWideCharConvert(p).c_str();

						OnManageGroups_Method((VS_ManageGroupCmd)cmd, gid, call_id, gname, seq_id);
					}
				}
			}
		}
		break;
	case transport::MessageType::Reply:
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return true;
}


void VS_AddressBookService::UserRegistrationInfo_Method(const char *user)
{
	if ( (!user)||(!*user) || (!m_abStorage) ) return;

	VS_UserData ud;
	if (m_abStorage->FindUser(user,ud,false))
	{
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, USERREGISTRATIONINFO_METHOD);
		rCnt.AddValue(USERNAME_PARAM, ud.m_name);
		rCnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
		rCnt.AddValue(CALLID_PARAM, user);

		PostReply(rCnt);
	}
}

AddToAddressBook_Task::AddToAddressBook_Task(VS_AddressBook ab, const char* user, boost::shared_ptr<VS_ABStorageInterface> abStorage,
											 VS_Container& cnt, bool IsTransportSrcUser, std::unique_ptr<VS_RouterMessage>&& recvMess)
											 :	m_ab(ab),m_user(user),m_abStorage(abStorage), m_in_cnt(cnt), m_IsTransportSrcUser(IsTransportSrcUser),m_mess(std::move(recvMess))
{
	m_recvMess = m_mess.get();

	const char* call_id2=cnt.GetStrValueRef(CALLID_PARAM);
	if(!call_id2 || !*call_id2)
		call_id2=cnt.GetStrValueRef(NAME_PARAM);
	if(!call_id2 || !*call_id2)
		return ;				// TODO: ktrushnikov: PostReply(): add failed

	m_call_id2 = call_id2;
};

AddToAddressBook_Task::~AddToAddressBook_Task()
{
}

void AddToAddressBook_Task::Run()
{
	if(!m_abStorage)
		return;

	VS_SimpleStr  add_call_id;
	std::string add_display_name;

	long count=0;
	long hash=0;
	VS_Container rCnt;
	long error_code=m_abStorage->AddToAddressBook(m_ab, m_user, m_in_cnt, hash, rCnt, add_call_id, add_display_name, this);

	if(error_code==0)
		count++;

	rCnt.AddValue(METHOD_PARAM, ADDTOADDRESSBOOK_METHOD);
	rCnt.AddValueI32(ADDRESSBOOK_PARAM, m_ab);
	rCnt.AddValueI32(RESULT_PARAM, count);
	rCnt.AddValueI32(CAUSE_PARAM,	error_code);
	rCnt.AddValueI32(HASH_PARAM, hash);
	rCnt.AddValue(CALLID_PARAM, (!!add_call_id)?add_call_id:m_call_id2);
	rCnt.AddValue(DISPLAYNAME_PARAM, add_display_name);
	if (m_IsTransportSrcUser)	// go to AS
		rCnt.AddValue(TRANSPORT_SRCUSER_PARAM, m_user);

	PostReply(rCnt);
}

void VS_AddressBookService::AddToAddressBook_Method(VS_AddressBook ab, VS_Container& cnt, std::unique_ptr<VS_RouterMessage>&&m)
{
	vs_user_id tmp_id = cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
	vs_user_id user_id = (!!tmp_id)? tmp_id.m_str: m_recvMess->SrcUser();
	if(!user_id) return;

	PutTask(new AddToAddressBook_Task(ab, user_id, m_abStorage, cnt, !!tmp_id, std::move(m)), "AddToAddressBook");
}


UpdateAddressBook_Task::UpdateAddressBook_Task(VS_AddressBook ab, const char* user, boost::shared_ptr<VS_ABStorageInterface> abStorage,
	VS_Container& cnt, bool IsTransportSrcUser, std::unique_ptr<VS_RouterMessage>&& recvMess)
	: m_ab(ab), m_user(user), m_abStorage(abStorage), m_in_cnt(cnt), m_IsTransportSrcUser(IsTransportSrcUser),m_mess(std::move(recvMess))
{
	m_recvMess = m_mess.get();

	const char* call_id2 = cnt.GetStrValueRef(CALLID_PARAM);
	if (!call_id2 || !*call_id2)
		call_id2 = cnt.GetStrValueRef(NAME_PARAM);
	if (!call_id2 || !*call_id2)
		return;				// TODO: ktrushnikov: PostReply(): add failed

	m_call_id2 = call_id2;
};

UpdateAddressBook_Task::~UpdateAddressBook_Task()
{
}

void UpdateAddressBook_Task::Run()
{
	if (!m_abStorage)
		return;

	VS_UserData ud;
	long count = 0;
	long hash = 0;
	long error_code = 0;
	const char* call_id = m_in_cnt.GetStrValueRef(CALLID_PARAM);
	const char* dn = m_in_cnt.GetStrValueRef(DISPLAYNAME_PARAM);

	VS_Container rCnt;
	if (call_id)
	{
		error_code = m_abStorage->UpdateAddressBook(m_ab, m_user, call_id, m_in_cnt, hash, rCnt);
		if (error_code == 0)
			count++;
	}
	else
		error_code = VSS_USER_NOT_VALID;


	rCnt.AddValue(METHOD_PARAM, UPDATEADDRESSBOOK_METHOD);
	rCnt.AddValueI32(ADDRESSBOOK_PARAM, m_ab);
	rCnt.AddValue(CALLID_PARAM, call_id);
	rCnt.AddValueI32(RESULT_PARAM, count);
	rCnt.AddValueI32(CAUSE_PARAM, error_code);
	rCnt.AddValueI32(HASH_PARAM, hash);
	if (m_ab == AB_COMMON && count>0) // update dn
		rCnt.AddValue(DISPLAYNAME_PARAM, dn);
	if (m_IsTransportSrcUser)	// go to AS
		rCnt.AddValue(TRANSPORT_SRCUSER_PARAM, m_user);

	PostReply(rCnt);
}

void VS_AddressBookService::UpdateAddressBook_Method(VS_AddressBook ab, VS_Container& cnt,std::unique_ptr<VS_RouterMessage>&&m)
{
	vs_user_id tmp_id = cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
	vs_user_id user_id = (!!tmp_id) ? tmp_id.m_str : m_recvMess->SrcUser();
	if (!user_id) return;

	PutTask(new UpdateAddressBook_Task(ab, user_id, m_abStorage, cnt, !!tmp_id, std::move(m)), "UpdateAddressBook");
}

RemoveFromAddressBook_Task::RemoveFromAddressBook_Task(VS_AddressBook ab, const char* user, boost::shared_ptr<VS_ABStorageInterface> abStorage,
											 VS_Container& cnt, bool IsTransportSrcUser, std::unique_ptr<VS_RouterMessage> && recvMess)
											 :	m_ab(ab),m_user(user),m_abStorage(abStorage), m_in_cnt(cnt), m_IsTransportSrcUser(IsTransportSrcUser),m_mess(std::move(recvMess))
{
	m_recvMess = m_mess.get();
	const char* call_id2=cnt.GetStrValueRef(CALLID_PARAM);
	if(!call_id2 || !*call_id2)
		call_id2=cnt.GetStrValueRef(NAME_PARAM);
	if(!call_id2 || !*call_id2)
		return ;				// TODO: ktrushnikov: PostReply(): add failed

	m_call_id2 = call_id2;
};

RemoveFromAddressBook_Task::~RemoveFromAddressBook_Task()
{
}

void RemoveFromAddressBook_Task::Run()
{
	vs_user_id tmp_id = m_in_cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
	vs_user_id user_id = (!!tmp_id)? tmp_id.m_str: m_recvMess->SrcUser();
	if(!user_id || !m_abStorage)
		return;
	const char* entry =  m_in_cnt.GetStrValueRef(NAME_PARAM);

	VS_UserData ud;
	long count=0;
	long hash=0;

	VS_Container rCnt;
	long error=m_abStorage->RemoveFromAddressBook(m_ab, m_user, entry, m_in_cnt, hash, rCnt);
	if(error==0)
		count++;

	rCnt.AddValue(METHOD_PARAM, REMOVEFROMADDRESSBOOK_METHOD);
	rCnt.AddValueI32(ADDRESSBOOK_PARAM, m_ab);
	rCnt.AddValueI32(RESULT_PARAM, count);
	rCnt.AddValueI32(CAUSE_PARAM, error);
	rCnt.AddValueI32(HASH_PARAM, hash);
	rCnt.AddValue(CALLID_PARAM, entry);
	if (!!tmp_id)	// go to AS
		rCnt.AddValue(TRANSPORT_SRCUSER_PARAM, user_id);

	PostReply(rCnt);
}

void VS_AddressBookService::RemoveFromAddressBook_Method(VS_AddressBook ab,const vs_user_id& entry, VS_Container& cnt, std::unique_ptr<VS_RouterMessage> &&m)
{
	vs_user_id tmp_id = cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
	vs_user_id user_id = (!!tmp_id)? tmp_id.m_str: m_recvMess->SrcUser();
	if(!user_id || !m_abStorage)
		return;

	PutTask(new RemoveFromAddressBook_Task(ab, user_id, m_abStorage, cnt, !!tmp_id, std::move(m)), "RemoveFromAddressBook");
}


SearchAddressBook_Task::SearchAddressBook_Task(VS_AddressBook ab, const char* query, const char* server,
											   const char* user, unsigned long client_hash, boost::shared_ptr<VS_ABStorageInterface> abStorage, VS_Container& cnt, bool to_user, std::unique_ptr<VS_RouterMessage> &&recvMess, const boost::shared_ptr<VS_ConfRestrictInterface> &confRestrict)
	: m_ab(ab)
	, m_server(server)
	, m_to_user(to_user)
	, m_client_hash(client_hash)
	, m_abStorage(abStorage)
	, m_in_cnt(cnt)
	, m_mess(std::move(recvMess))
	, m_conf_restrict(confRestrict)
{
	if(query) m_query = query;
	m_recvMess = m_mess.get();
	if (user && *user)
		m_user = vs::UTF8ToLower(user).c_str();
};

SearchAddressBook_Task::~SearchAddressBook_Task()
{
}

void SearchAddressBook_Task::Run() {
	if(!m_abStorage)
		return;
	int NumOfUsers = 0;


	VS_Container rCnt;
	int result= m_abStorage->FindUsers(rCnt, NumOfUsers, m_ab, m_user, m_query, m_client_hash, m_in_cnt);
	VS_RealUserLogin query_r(m_query);

	std::string server;
	bool is_our = query_r.IsOurSID();
	if (!is_our) {
		VS_ResolveServerFinder	*resolve_srv = VS_ResolveServerFinder::Instance();
		if (resolve_srv)
			resolve_srv->GetServerForResolve(query_r.GetID(), server, false);
		is_our = server == OurEndpoint() || server == m_server.m_str;
		if (VS_GetServerType(server) != ST_VCS && VS_GetServerType(m_server.m_str) != ST_VCS)		// it is all cloud service, not TCS
			is_our = true;
		dstream4 << "SearchAddressBook_Task: GetServerForResolve(" << query_r.GetID() << ") return " << server << ", is_our=" << is_our << ", m_server=" << m_server.m_str;
	}

	if ((m_ab != AB_PERSON_DETAILS && m_ab != AB_USER_PICTURE) || NumOfUsers != 0 || (NumOfUsers == 0 && is_our)) {
		dstream4 << "SearchAddressBook_Task(ab=" << (long)m_ab << ", client_hash=" << m_client_hash << "): NumOfUsers=" << NumOfUsers << ", m_server=" << m_server.m_str
			<< ", m_to_user=" << m_to_user << ", m_user=" << m_user.m_str << ", m_query=" << m_query << ", result=" << result;
		rCnt.AddValue(METHOD_PARAM, SEARCHADDRESSBOOK_METHOD);
		rCnt.AddValueI32(ADDRESSBOOK_PARAM, m_ab);
		rCnt.AddValueI32(RESULT_PARAM, NumOfUsers);
		rCnt.AddValueI32(CAUSE_PARAM, result);
		rCnt.AddValue(QUERY_PARAM, m_query.c_str());
		if (!m_to_user)		// go to AS
			rCnt.AddValue(TRANSPORT_SRCUSER_PARAM, m_user);

		PostRequest(m_server, (m_to_user) ? m_user : 0, rCnt, 0, !m_recvMess ? ADDRESSBOOK_SRV : m_recvMess->SrcService(), default_timeout, ADDRESSBOOK_SRV);
	} else if (!m_query.empty() && !is_our) {	// not local
		dstream4 << "SearchAddressBook_Task(ab=" << (long)m_ab << ", client_hash=" << m_client_hash << "): resend query to server=" << server
			<< ", m_query=" << m_query << ", m_server=" << m_server.m_str << ", m_to_user=" << m_to_user << ", m_user=" << m_user.m_str;
		VS_Container cnt2;
		cnt2.AddValue(METHOD_PARAM, SEARCHADDRESSBOOK_METHOD);
		cnt2.AddValueI32(ADDRESSBOOK_PARAM, m_ab);
		cnt2.AddValueI32(HASH_PARAM, m_client_hash);
		cnt2.AddValue(QUERY_PARAM, m_query);
		cnt2.AddValue(TRANSPORT_SRCSERVER_PARAM, m_server);
		if (!server.empty())
			PostRequest(server.c_str(), 0, cnt2, ADDRESSBOOK_SRV, LOCATE_SRV, default_timeout, ADDRESSBOOK_SRV, m_user);
	}
	else
	{
		dstream2 << "SearchAddressBook_Task: unexpected result, nothing done. IsVcs() =" << (!!m_conf_restrict ? (m_conf_restrict->IsVCS() ? "true" : "false") : "<none>")
			<< ", m_query = " << m_query << ", is_our = " << is_our << ", ab = " << m_ab << ", NumOfUsers = " << NumOfUsers << '\n';
	}
}


void VS_AddressBookService::SearchAddressBook_Method(VS_AddressBook ab, const char* query, long client_hash, VS_Container& cnt, std::unique_ptr<VS_RouterMessage> &&m)
{

	if (!query && !cnt.GetStrValueRef(CALLID_PARAM) &&
				  !cnt.GetStrValueRef(EMAIL_PARAM) &&
				  !cnt.GetStrValueRef(NAME_PARAM))
		return;

	vs_user_id tmp_id = cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
	vs_user_id user_id = (!!tmp_id)? tmp_id.m_str: m_recvMess->SrcUser();
	if(!user_id) return;

	VS_SimpleStr src_server = cnt.GetStrValueRef(TRANSPORT_SRCSERVER_PARAM);
	if (!src_server)
		src_server = m_recvMess->SrcServer();

	PutTask(new SearchAddressBook_Task(ab, query, src_server, user_id, client_hash, m_abStorage, cnt, (!!tmp_id) ? false : true, std::move(m), m_confRestrict), "SearchAddressBook");
}


////////////////////////////////////////////////////////////////////////////////
// Events
////////////////////////////////////////////////////////////////////////////////

void VS_AddressBookService::OnAddressBookChange_Method(VS_AddressBook ab, VS_Container& cnt)
{
	dprint3("ABSrv: OnABChange from server\n");

	if(!m_recvMess) return;

	if(!m_recvMess->IsFromServer())
	{
		auto dbStorage = g_dbStorage;
		if(!dbStorage || !dbStorage->CheckSessionID( cnt.GetStrValueRef(SESSION_PARAM) ))
			return;
	}
	VS_SimpleStr to_server;
	if (cnt.GetStrValueRef(SERVER_PARAM)!=0) {
		to_server = cnt.GetStrValueRef(SERVER_PARAM);
	}else{
		to_server = OurEndpoint();
	}
	PutTask(new OnAddressBookChange_Task(ab, cnt.GetStrValueRef(QUERY_PARAM), to_server, m_abStorage, cnt), "OnAddressBookChange");
}

bool VS_AddressBookService::Timer(unsigned long tickcount)
{
	if (tickcount - m_BSEventsTime > 10*1000) {	// 10 sec
		m_BSEventsTime = tickcount;

		if(m_abStorage)
		{
			std::vector<BSEvent> v;
			m_abStorage->GetBSEvents(v);

			for(unsigned int i = 0; i < v.size(); i++)
			{
				PostRequest(v[i].broker_id, 0, *(v[i].cnt), 0, v[i].to_service);
				if (v[i].cnt) { delete v[i].cnt; v[i].cnt = 0; }
			}
		}
	}
	return true;
}

void VS_AddressBookService::OnManageGroups_Method(const VS_ManageGroupCmd cmd, const long gid, const VS_SimpleStr& call_id, VS_WideStr gname, const long seq_id)
{
	dprint3("ABSrv: OnManageGroups_Method(%d, %ld, %s)\n", cmd, gid, call_id.m_str);
	if (!!gname)
		dstream3 << "gname=" << gname.m_str << "\n";
	if (!m_abStorage)
		return ;
	bool res = 0;
	long ret_gid = 0;
	long hash = 0;
	switch (cmd)
	{
	case MGC_CREATE_GROUP:
		res = m_abStorage->ManageGroups_CreateGroup(m_recvMess->SrcUser(), gname, ret_gid, hash);
		break;
	case MGC_DELETE_GROUP:
		res = m_abStorage->ManageGroups_DeleteGroup(m_recvMess->SrcUser(), gid, hash);
		break;
	case MGC_RENAME_GROUP:
		res = m_abStorage->ManageGroups_RenameGroup(m_recvMess->SrcUser(), gid, gname, hash);
		break;
	case MGC_ADD_USER:
		res = m_abStorage->ManageGroups_AddUser(m_recvMess->SrcUser(), gid, call_id, hash);
		break;
	case MGC_DELETE_USER:
		res = m_abStorage->ManageGroups_DeleteUser(m_recvMess->SrcUser(), gid, call_id, hash);
		break;
	default:
		res = false;
		break;
	}

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, MANAGEGROUPS_METHOD);
	rCnt.AddValueI32(CAUSE_PARAM, cmd);
	rCnt.AddValueI32(RESULT_PARAM, res);
	rCnt.AddValueI32(HASH_PARAM, hash);
	if (cmd==MGC_CREATE_GROUP && ret_gid)
	{
		rCnt.AddValue(GID_PARAM, std::to_string(ret_gid));
	}
	rCnt.AddValueI32(SEQUENCE_ID_PARAM, seq_id);
	PostReply(rCnt);
}

OnAddressBookChange_Task::OnAddressBookChange_Task(VS_AddressBook ab, const char* query, const char* server,
  boost::shared_ptr<VS_ABStorageInterface> abStorage, VS_Container& cnt)
	: m_in_cnt(cnt)
	, m_ab(ab)
	, m_server(server)
	, m_abStorage(abStorage)
{
	if(query) m_query = query;
}

void OnAddressBookChange_Task::Run()
{
	if(!m_abStorage) return;

	std::vector<VS_SimpleStr> users;
	m_in_cnt.Reset();
	while(m_in_cnt.Next())
	{
		if(strcasecmp(m_in_cnt.GetName(), USERNAME_PARAM) == 0)
			users.emplace_back(m_in_cnt.GetStrValueRef());
	}

	for (std::vector<VS_SimpleStr>::iterator i = users.begin(); i != users.end(); ++i)
	{
		int NumOfUsers = 0;
		VS_Container rCnt;
		int result= m_abStorage->FindUsers(rCnt, NumOfUsers, m_ab, (*i).m_str, m_query, 0, m_in_cnt);
		rCnt.AddValue(METHOD_PARAM, SEARCHADDRESSBOOK_METHOD);
		rCnt.AddValueI32(ADDRESSBOOK_PARAM, m_ab);
		rCnt.AddValueI32(RESULT_PARAM, NumOfUsers);
		rCnt.AddValueI32(CAUSE_PARAM, result);
		rCnt.AddValue(QUERY_PARAM, m_query.c_str());

		PostRequest(m_server, (*i).m_str, rCnt, 0, !m_recvMess?ADDRESSBOOK_SRV:m_recvMess->SrcService(), default_timeout, ADDRESSBOOK_SRV);
	}
}
