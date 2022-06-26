
#include <malloc.h>
#include <string.h>

#include "VS_OfflineChatService.h"

#include "../../ServerServices/Common.h"
#include "statuslib/VS_ResolveServerFinder.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"
#include "../../common/transport/Router/VS_RouterMessage.h"
#include "../../ServerServices/VS_ReadLicense.h"

#define DEBUG_CURRENT_MODULE VS_DM_CHATS


class SendRoamingMessages_Task : public VS_PoolThreadsTask
{
	VS_OfflineChatService *m_offline_chat_srv;
public:
	SendRoamingMessages_Task(VS_OfflineChatService *p):m_offline_chat_srv(p)
	{}

	void Run()
	{
		if(!m_offline_chat_srv)
			return;
		m_offline_chat_srv->SendRoamingMessages();
	}
};

bool VS_OfflineChatService::Init( const char* /*our_endpoint*/, const char* /*our_service*/, const bool /*permittedAll*/ )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if (key.IsValid())
	{
		long tmp = 0;
		if (key.GetValue(&tmp, sizeof(tmp), VS_REG_INTEGER_VT, "Roaming Offline Message Send Retry Minutes" ) != 0)
		{
			if ((tmp > ROAMING_MSG_PERIOD_MINIMUM) || tmp==0)
				ROAMING_MSG_PERIOD = tmp;
		}
	}
	if (!ROAMING_MSG_PERIOD) {
		dprint1("Roaming offline chat messages are not processed\n");
	} else {
		dprint3("Roaming offline chat messages period is %ld minutes\n", ROAMING_MSG_PERIOD);
	}
	return true;
}

bool VS_OfflineChatService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)  return true;
	VS_Container cnt;

	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		m_recvMess = recvMess.get();
		if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
			const char* method = cnt.GetStrValueRef(METHOD_PARAM);
			if (method && strcasecmp(method, SENDMESSAGE_METHOD) == 0) {
				const char* conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
				if (!conf || !*conf)
				{
					if(VS_CheckLicense(LE_ROAMING_ON))
						ProcessChatInRoaming(cnt);
					else
						ProcessChatLocal(cnt);
				}
			}
		}
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return true;
}

void VS_OfflineChatService::SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict)
{
	m_confRestriction = confRestrict;
}

void VS_OfflineChatService::ProcessChatLocal(VS_Container& cnt)
{
	if (!m_recvMess->SrcServer())
		return ;

	if ( strcasecmp(m_recvMess->SrcServer(), OurEndpoint())!=0 )
		return;

	m_confRestriction->SetOfflineChatMessage(cnt);
}

void VS_OfflineChatService::ProcessChatInRoaming(VS_Container& cnt)
{
	if (!m_recvMess->SrcServer() || !m_recvMess->DstServer())
		return ;

	int32_t type = 0;
	cnt.GetValue(TYPE_PARAM, type);
	if (type == 1)						// if it is Reply from DstServer then delete message in SQLite
	{
		dprint3("Roaming offline chat delivery replay from %s\n", m_recvMess->SrcServer());
		m_confRestriction->DeleteOfflineChatMessage(cnt);
		return ;
	}

	const char* from = cnt.GetStrValueRef(FROM_PARAM);
	const char* to = cnt.GetStrValueRef(TO_PARAM);
	if (!to || !*to)
		return;

	VS_RealUserLogin r(to);
	if (r.IsOurSID()) {				// We are DstServer: send replay to SrcServer
		bool IsOurMess = !strcasecmp(m_recvMess->SrcServer(), m_recvMess->DstServer());
		if (!IsOurMess) {
			dprint3("Recv OfflineChatMessage from %s remote server: %s->%s\n", m_recvMess->SrcServer(), from, to);
			PostRequest(OurEndpoint(), 0, cnt, 0, CHAT_SRV, default_timeout, 0, from);

			cnt.AddValueI32(TYPE_PARAM, 1); // replay to dst server
			PostRequest(m_recvMess->SrcServer(), 0, cnt, 0, OFFLINECHAT_SRV);
		} else {
			dprint3("Local OfflineChatMessage: %s->%s\n", from, to);
			m_confRestriction->SetOfflineChatMessage(cnt);
		}
	} else {
		dprint3("Save OfflineChatMessage for remote server: %s->%s\n", from, to);
		m_confRestriction->SetOfflineChatMessage(cnt);
	}
}

bool VS_OfflineChatService::Timer(unsigned long ticks)
{
	if (ROAMING_MSG_PERIOD && ((ticks-m_last_roaming_msg_tick > ROAMING_MSG_PERIOD*60*1000) || !m_last_roaming_msg_tick))
	{
		PutTask(new SendRoamingMessages_Task(this), "SendRoamingMessages", 30);
		m_last_roaming_msg_tick = ticks;
	}

	return true;
}

void VS_OfflineChatService::SendRoamingMessages()
{
	if(!VS_CheckLicense(LE_ROAMING_ON))
		return ;

	VS_ChatMsgs msgs;
	std::string our_sid = OurEndpoint();
	our_sid = our_sid.substr(0, our_sid.find("#"));

	m_confRestriction->GetRoamingOfflineMessages(our_sid.c_str(), msgs);
	dprint3("GetRoamingOfflineMessages(%zu)\n", msgs.size());
	for(unsigned int i=0; i < msgs.size(); i++)
	{
		VS_Container* c = msgs[i].cnt;
		if (!c)
			continue;

		dprint4("\tSendRoamingMessage(%s)\n", msgs[i].to_callId.m_str);
		if (c->IsValid())
		{
			std::string server;
			VS_ResolveServerFinder	*resolve_srv = VS_ResolveServerFinder::Instance();
			resolve_srv->GetASServerForResolve(msgs[i].to_callId,server,false);
			dprint4("GetASServerForResolve(%s):%s\n", msgs[i].to_callId.m_str, server.c_str());
			if (!server.empty() && server!=OurEndpoint())
			{
				dprint4("\tServer for roaming mess = %s\n", server.c_str());
				PostRequest(server.c_str(), 0, *c, 0, OFFLINECHAT_SRV);
			}
		}

		if (c) { delete c; c = 0; }
	}
}