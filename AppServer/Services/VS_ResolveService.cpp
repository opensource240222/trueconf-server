#include "VS_ResolveService.h"
#include <list>
#include <deque>
#include "../../common/std/cpplib/VS_CallIDUtils.h"
#include "../../common/std/debuglog/VS_Debug.h"
#include "std-generic/cpplib/ThreadUtils.h"

#define DEBUG_CURRENT_MODULE VS_DM_RESOLVE

struct VS_RouterMessWrap
{
	VS_RouterMessWrap();
	VS_RouterMessWrap(VS_RouterMessage * mess) : m_rmess(mess), m_user(mess->DstUser()), m_when_created(std::chrono::steady_clock::now())
	{}
	VS_RouterMessage*	m_rmess;
	const char*			m_user;
	const std::chrono::steady_clock::time_point m_when_created;
};

typedef std::list<VS_RouterMessWrap*> RouterMessList;
typedef std::deque<std::pair<VS_SimpleStr,std::chrono::steady_clock::time_point>> SubscribeQueue;

RouterMessList s_messlist;
SubscribeQueue s_subscribers;

#define SUBSCRIBERS_MAX_NUM		256		// unsubscribe oldest users after achiving this threshold
const std::chrono::seconds MESSAGES_LIVE_TIME(20);		// time in sec to wait dst user going online

VS_ResolveService::VS_ResolveService()
{
	m_TimeInterval = std::chrono::seconds(1);
	m_waiters.SetKeyFactory(VS_SimpleStr::Factory, VS_SimpleStr::Destructor);
	m_waiters.SetPredicate(VS_SimpleStr::Predicate);
}

bool VS_ResolveService::Init(const char* /*our_endpoint*/, const char* /*our_service*/, const bool /*permittedAll*/)
{
	return true;
}

void VS_ResolveService::RegAtPresSRV()
{
	m_presenceService->AddListener(LISTENER_USER, this);
}

void VS_ResolveService::AsyncDestroy()
{
	VS_Container fake;
	PostRequest(OurEndpoint(), 0, fake, "ResolveSRV_AsyncDestroy", OurService());
	vs::SleepFor(std::chrono::milliseconds(100));		// offline chat will be sent to LOCATE_SRV, give some time to save chat in DB
}

bool VS_ResolveService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (!recvMess)
		return true;

	if (strcasecmp(recvMess->AddString(),"ResolveSRV_AsyncDestroy")==0) {
		ProcessMessList(false);
		return true;
	}

	// check if it is from callback
	const char* service = recvMess->DstService();
	if (strcasecmp(service, RESOLVE_SRV)==0 && recvMess->Type() == transport::MessageType::Request) {
		VS_Container cnt;
		if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
			const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);
			const char* server_id = cnt.GetStrValueRef(SERVER_PARAM);
			VS_Map::Iterator it = m_waiters.Find(call_id);

			if (it!=m_waiters.End()) {
				RouterMessList::iterator il = s_messlist.begin();
				while (il!=s_messlist.end()) {
					VS_RouterMessWrap *rmess = *il;
					if (strcasecmp(rmess->m_user, call_id)==0) {
						if (server_id && *server_id)
						{
							rmess->m_rmess->SetDstServer(string_view(server_id));
							dprint4("VS_ResolveService: PostMess to server = %s for call_id = %s\n", server_id, call_id);
							PostMes(rmess->m_rmess);
						}
						il = s_messlist.erase(il);
						delete rmess;
					}
					else
						++il;
				}
				m_waiters.Erase(it);
			}
		}
		return true;
	}

	// message from transport
	VS_SimpleStr call_id = recvMess->DstUser();
	if (!call_id)
		return true;
	auto call_id_orig = call_id;
	string_view dst_srv(recvMess->DstService());
	if (dst_srv == CHAT_SRV && VS_IsSIPCallID(call_id.m_str))
	{
		recvMess->SetDstServer(OurEndpoint());
		recvMess->SetDstService(SIPCALL_SRV);
		recvMess->SetDstUser("");
		PostMes(recvMess.release());
		return true;
	}
	string_view sv(call_id.m_str);
	bool do_ext_resolve = VS_IsRTPCallID(call_id.m_str) && sv.find(TRANSCODER_ID_SEPARATOR) == string_view::npos;
	if (dst_srv == CHAT_SRV && do_ext_resolve)	// bug#47415: chat to rtsp/h323 should not allocate transcoder
		return true;
	VS_CallIDInfo ci;
	if (m_presenceService->Resolve(call_id, ci, true, 0, do_ext_resolve) > USER_LOGOFF) {
		if (ci.m_serverID.empty()) {
			dprint0("RSLV: NULL server in Resolve of user %s\n", call_id.m_str);
		}
		else {
			recvMess->SetDstServer(ci.m_serverID);
			if (call_id_orig != call_id)	// changed during resolve
				recvMess->SetDstUser(call_id.m_str);
			PostMes(recvMess.release());
		}
	}
	else {
		VS_RouterMessWrap* mess = new VS_RouterMessWrap(recvMess.release());
		s_messlist.push_back(mess);
		m_waiters.Insert(call_id, 0);
		m_presenceService->Subscribe(call_id, VS_FullID(OurEndpoint(), LISTENER_USER));
		s_subscribers.emplace_back(call_id,std::chrono::steady_clock::now());
		if (s_subscribers.size() > SUBSCRIBERS_MAX_NUM) {
			VS_SimpleStr &uns = s_subscribers.front().first;
			auto life_time_in_sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - s_subscribers.front().second).count();
			dstream4 << "VS_ResolveService: s_subscribers.size() = " << s_subscribers.size() << ". Listener unsub from " << uns.m_str << "; sub duration = " << life_time_in_sec << " sec\n";
			m_presenceService->Unsubscribe(uns, VS_FullID(OurEndpoint(), LISTENER_USER));
			s_subscribers.pop_front();
		}
	}
	return true;
}

void VS_ResolveService::OnServerChange(const char* call_id, const char* server_id)
{
	if (!call_id || !*call_id || !server_id || !*server_id)
		return;
	dprint4("VS_ResolveService: OnServerChange call_id = %s; server_id = %s\n", call_id, server_id);
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, RESOLVE_METHOD);
	cnt.AddValue(CALLID_PARAM, call_id);
	cnt.AddValue(SERVER_PARAM, server_id);

	PostRequest(OurEndpoint(), 0, cnt, 0, RESOLVE_SRV, 10000);
}

bool VS_ResolveService::Timer(unsigned long tickcount)
{
	ProcessMessList();
	return true;
}

void VS_ResolveService::ProcessMessList(bool check_lifetime)
{
	auto now = std::chrono::steady_clock::now();
	RouterMessList::iterator il = s_messlist.begin();
	while (il!=s_messlist.end()) {
		VS_RouterMessWrap *rmess = (*il);
		if (!check_lifetime || (now - rmess->m_when_created > MESSAGES_LIVE_TIME)) {
			// offline chat message
			if (rmess->m_rmess && !strcasecmp(rmess->m_rmess->DstService(), CHAT_SRV)) {

				VS_SimpleStr bs;
				if (m_confRestriction)
				{
					if (m_confRestriction->IsVCS())
						bs = OurEndpoint();
					else
						bs = m_confRestriction->GetAnyBSbyDomain(rmess->m_rmess->DstUser_sv());
					if (!bs)
						m_confRestriction->GetFirstBS(rmess->m_rmess->DstUser(), OurEndpoint(), bs);
				}

				VS_RouterMessage* mess(0);
				dprint4("VS_ResolveService: offline chat message for user = %s resent to bs = %s\n", rmess->m_rmess->DstUser(), bs.m_str);
				AddUsernameAlloc(bs, rmess->m_rmess, mess);
				if (mess && !PostMes(mess))
					delete mess;
			}

			delete rmess->m_rmess;
			il = s_messlist.erase(il);
			delete rmess;
		}
		else
			++il;
	}
}

void VS_ResolveService::SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict)
{
	m_confRestriction = confRestrict;
}

void VS_ResolveService::AddUsernameAlloc(VS_SimpleStr& bs, VS_RouterMessage* in, VS_RouterMessage* &out)
{
	if (!bs)
		return ;

	VS_Container cnt;

	if (!cnt.Deserialize(in->Body(), in->BodySize()))
		return ;
	if (!cnt.AddValue(USERNAME_PARAM, in->DstUser()))
		return ;
	void* body = nullptr;
	size_t bodySize = 0;
	if (!cnt.SerializeAlloc(body, bodySize))
		return ;

	out = new VS_RouterMessage(in->SrcService(), OFFLINECHAT_SRV, LOCATE_SRV, 0, in->SrcUser(), bs, in->SrcServer(), 30000, body, bodySize);

	free(body);
}
