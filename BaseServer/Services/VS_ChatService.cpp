#ifdef _WIN32	// not ported
#include "VS_ChatService.h"

#include "../../ServerServices/Common.h"
#include "transport/Router/VS_RouterMessage.h"
#include "../../common/std/cpplib/VS_Utils.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"
#include "../../common/statuslib/VS_ResolveServerFinder.h"
#include "VS_BasePresenceService.h"
#include "VS_BSLogService.h"

#define DEBUG_CURRENT_MODULE VS_DM_CHATS

bool VS_BSChatService::Init(const char* /*our_endpoint*/, const char* /*our_service*/, const bool /*permittedAll*/)
{
	return true;
}

bool VS_BSChatService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)  return true;
	VS_Container cnt;

	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
		m_recvMess = recvMess.get();
		if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
			const char* method = cnt.GetStrValueRef(METHOD_PARAM);
			if (method && _stricmp(method, SENDMESSAGE_METHOD) == 0) {
				unsigned long new_id = 0;
				{	// gen uniq id
					VS_AutoLock lock(&m_thread_order_lock);
					unsigned long n = 100;
					do {
						new_id = VS_GenKeyByMD5();
						--n;
					} while (std::find(m_thread_order.begin(), m_thread_order.end(), new_id) != m_thread_order.end() && n);
					m_thread_order.push_back(new_id);
				}

				PutTask(new ProcessChat_Task(shared_from_this(), new_id, cnt, std::move(recvMess)), "ProcessChat");
			}
			else if (method && _stricmp(method, "NewOfflineChat") == 0) {
				PutTask(new NewOfflineChat_Task(cnt.GetStrValueRef(CALLID_PARAM)), "NewOfflineChat");
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

void VS_BSChatService::NotifyWebAboutOfflineChat(){
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, SENDMAIL_METHOD);
	cnt.AddValueI32(TYPE_PARAM, e_notifyviaweb_missed_chat_msg);
	PostRequest(OurEndpoint(), "", cnt, nullptr, LOG_SRV);
}

void VS_BSChatService::WaitMyID(const unsigned long id)
{
	unsigned long n = 6000;		// wait max 60 sec
	m_thread_order_lock.Lock();
	while (!m_thread_order.empty() && m_thread_order[0] != id && n)
	{
		m_thread_order_lock.UnLock();
		Sleep(10);
		--n;
		m_thread_order_lock.Lock();
	}
	m_thread_order_lock.UnLock();
}

void VS_BSChatService::DeleteMyID(const unsigned long id)
{
	VS_AutoLock lock(&m_thread_order_lock);
	m_thread_order.erase(std::remove(m_thread_order.begin(), m_thread_order.end(), id), m_thread_order.end());
}

ProcessChat_Task::ProcessChat_Task(const boost::shared_ptr<VS_BSChatService>& ptr, const unsigned long id, VS_Container& cnt, std::unique_ptr<VS_RouterMessage>&& recvMess) :
m_srv(ptr), m_id(id), m_in_cnt(cnt),m_mess(std::move(recvMess))
{
	m_recvMess = m_mess.get();
}

ProcessChat_Task::~ProcessChat_Task()
{
}

void ProcessChat_Task::Run()
{
	int32_t type = 0;
	m_in_cnt.GetValue(TYPE_PARAM, type);
	if (type == 1)						// if it is Reply from DstServer then delete message in SQLite
	{
		//		dprint3("Roaming offline chat delivery replay from %s\n", m_recvMess->SrcServer());
		//		m_confRestriction->DeleteOfflineChatMessage(m_in_cnt);
		return;
	}

	VS_SimpleStr to_user = m_in_cnt.GetStrValueRef(TO_PARAM);
	bool IsFromVCS = VS_GetServerType(m_recvMess->SrcServer_sv()) == ST_VCS;

	if (IsFromVCS) {
		VS_CallIDInfo ci;
		bool ToUserOnline = g_BasePresenceService->Resolve(to_user, ci, false, this) >= USER_LOGOFF && !ci.m_serverID.empty();
		if (m_srv)
			m_srv->WaitMyID(m_id);
		if (ToUserOnline) {
			dprint3("From VCS %s to User %s at %s\n", m_recvMess->SrcServer(), to_user.m_str, ci.m_serverID.c_str());

			// send to online user
			PostRequest(ci.m_serverID.c_str(), to_user, m_in_cnt, 0, CHAT_SRV);
		}
		else {
			dprint3("SaveMessageInDB(from VCS %s to=%s)\n", m_recvMess->SrcServer(), to_user.m_str);
			SaveMessageInDB(m_in_cnt);
		}

		// send Reply to VCS
		m_in_cnt.AddValueI32(TYPE_PARAM, 1);
		PostReply(m_in_cnt);
	}
	else {
		std::string to_server;
		VS_ResolveServerFinder	*resolve_srv = VS_ResolveServerFinder::Instance();
		resolve_srv->GetASServerForResolve(to_user, to_server, false);
		if (m_srv)
			m_srv->WaitMyID(m_id);
		if (VS_GetServerType(to_server) == ST_VCS) {
			dprint3("Send OfflineChatMsg to %s at %s\n", to_user.m_str, to_server.c_str());
			PostRequest(to_server.c_str(), 0, m_in_cnt, 0, OFFLINECHAT_SRV);
		}
		else {		// between two trueconf.ru users
			dprint3("SaveMessageInDB(to=%s)\n", to_user.m_str);
			SaveMessageInDB(m_in_cnt);
			if (m_srv)	m_srv->NotifyWebAboutOfflineChat();
		}
	}
	if (m_srv)
		m_srv->DeleteMyID(m_id);
}

void ProcessChat_Task::SaveMessageInDB(VS_Container &cnt)
{
	VS_SimpleStr from = cnt.GetStrValueRef(FROM_PARAM);
	std::string from_dn;	if (auto p = cnt.GetStrValueRef(DISPLAYNAME_PARAM)) from_dn = p;
	VS_SimpleStr to = cnt.GetStrValueRef(TO_PARAM);
	VS_SimpleStr body_utf8 = cnt.GetStrValueRef(MESSAGE_PARAM);

	auto dbStorage = g_dbStorage;
	if (!!dbStorage)
		dbStorage->SetOfflineChatMessage(from, to, body_utf8, from_dn,cnt);
}

NewOfflineChat_Task::NewOfflineChat_Task(VS_SimpleStr call_id) : m_call_id(std::move(call_id))
{
}

NewOfflineChat_Task::~NewOfflineChat_Task()
{

}

void NewOfflineChat_Task::Run()
{
	VS_CallIDInfo ci;
	bool is_online = g_BasePresenceService->Resolve(m_call_id, ci, false, this) >= USER_LOGOFF && !ci.m_serverID.empty();

	if (is_online)
	{
		auto dbStorage = g_dbStorage;
		if (!dbStorage)
			return;
			std::vector<VS_Container> vec;
		int n = dbStorage->GetOfflineChatMessages(m_call_id, vec);
		for (int i = 0; i < n; i++)
				PostRequest(ci.m_serverID.c_str(), m_call_id, vec[i], 0, CHAT_SRV);
		vec.clear();
	}
}
#endif