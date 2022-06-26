#include "statuslib/VS_CallIDInfo.h"
#include "ServerServices/Common.h"
#include "statuslib/VS_ResolveServerFinder.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_Utils.h"
#include "std-generic/cpplib/VS_Container_io.h"
#include <std/cpplib/escape_unsafe_html_tags.h>
#include "transport/Router/VS_RouterMessage.h"
#include "transport/Router/VS_RouterMessage_io.h"
#include "TransceiverLib/TransceiversPoolInterface.h"
#include "TransceiverLib/VS_TransceiverProxy.h"
#include "TransceiverLib/VS_ConfControlModule.h"

#include "VS_ChatService.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_CHATS

bool IsRtpTrancoder(int32_t type) {
	return type == CT_TRANSCODER || type == CT_TRANSCODER_CLIENT;
}

bool VS_ChatService::Init( const char* /*our_endpoint*/, const char* /*our_service*/, const bool /*permittedAll*/ )
{
	if (m_confRestrict && m_confRestrict->IsVCS()) // VS_ChatService might be used in the AppServer too.
	{
		auto chatDB = ChatDBInterface::Create();
		if (!chatDB)
			return false;

		const auto db_conn_str = VS_GetDBConnectionString();
		if (!db_conn_str.empty())
		{
			if (!chatDB->Init(db_conn_str))
			{
				dstream1 << "ChatDB init failed, connection string: " << db_conn_str;
				return false;
			}
			m_chatDB = std::move(chatDB);
		}
		else
			dstream0 << "ChatDB not initialized: no connection string";

		uint32_t URChatAllowed(1);
		VS_RegistryKey key(false, CONFIGURATION_KEY);
		key.GetValue(&URChatAllowed, sizeof(URChatAllowed), VS_REG_INTEGER_VT, "URChatAllowed");
		m_ChatAlowed = URChatAllowed != 0;
	}
	return true;
}

bool VS_ChatService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
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
				SendMessage_Method(recvMess->SrcUser(), cnt);
			}
			else if (method && strcasecmp(method, SENDCOMMAND_METHOD) == 0) {
				SendCommand_Method(recvMess->SrcUser(), cnt);
			}
			else if (method && strcasecmp(method, SENDSLIDESTOUSER_METHOD) == 0) {
				SendSlidesToUser_Method(cnt);
			}
			else if (method && strcasecmp(method, CONFERENCEDELETED_METHOD) == 0) {
				ConferenceDeleted_Method(cnt);
			}
			else if (method && strcasecmp(method, DELETEPARTICIPANT_METHOD) == 0) {
				DeleteParticipant_Method(cnt);
			}
			else { // for test clients
				VS_Container cnt2;
				cnt2.AddValue(METHOD_PARAM, PING_METHOD);
				PostUnauth(recvMess->SrcCID(),cnt2);
			}
		}
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return true;
}

void VS_ChatService::SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict)
{
	m_confRestrict = confRestrict;
}

void VS_ChatService::SetTransceiversPool(const std::shared_ptr<ts::IPool>& pool)
{
	m_transceiversPool = pool;
}


void VS_ChatService::SendToUserOrService(const char* from_id, int64_t message_id, const VS_Container& cnt, const char* to, const char* to_server, const bool is_offline)
{
	if (m_chatDB != nullptr)		// at TCS only
	{
		VS_RealUserLogin r2(nullptr == to ? "" : to);
		if (r2.IsOurSID() || (to && VS_IsNotTrueConfCallID(to))) {		// send directly to user (or #sip, #h323)
			PostRequest(0, to, cnt, 0, CHAT_SRV, default_timeout, 0, from_id);
		}
		else {					// send to remote_server
			VS_SimpleStr remote_server(to_server);
			VS_ResolveServerFinder	*resolve_srv = nullptr;
			if (!remote_server && (resolve_srv = VS_ResolveServerFinder::Instance()))
			{
				std::string server;
				// Use caching as it is relatively fast path in the server.
				if (resolve_srv->GetASServerForResolve(to, server, true))
				{
					remote_server = server.c_str();
				}
			}

			if (!!remote_server && remote_server != OurEndpoint())
			{
				m_chatDB->ChangeMessage(message_id, 0, 0, 0, &is_offline);
				PostRequest(remote_server, 0, cnt, 0, CHAT_SRV, default_timeout, 0, from_id);
			}
		}
	} else {	// AS server (or TCS without DB)
		PostRequest(0, to, cnt, 0, CHAT_SRV, default_timeout, 0, from_id);
	}
}

void VS_ChatService::UpdateDBMsgID(VS_Container& cnt, const int64_t mID)
{
	auto p_val = cnt.GetInt64ValueRef(DB_MESSAGE_ID_PARAM);
	if (p_val)
		*p_val = mID;
	else
		cnt.AddValueI64(DB_MESSAGE_ID_PARAM, mID);
}

////////////////////////////////////////////////////////////////////////////////
// SENDMESSAGE_METHOD(FROM_PARAM, TO_PARAM, MESSAGE_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_ChatService::SendMessage_Method(const char* from_id, VS_Container &a_cont)
{
	if (m_recvMess->SrcServer_sv() != OurEndpoint())		// replay to src server
	{
		int32_t type(0);
		if (!a_cont.GetValueI32(TYPE_PARAM, type))
		{
			VS_Container reply(a_cont);
			reply.AddValueI32(TYPE_PARAM, 1);
			PostRequest(m_recvMess->SrcServer(), 0, reply, 0, OFFLINECHAT_SRV);
		}
	}
	if (!m_ChatAlowed)
		return;

	// escape unsafe HTML tags
	VS_Container cont;
	a_cont.Reset();
	while (a_cont.Next())
	{
		if (strcasecmp(a_cont.GetName(), MESSAGE_PARAM) == 0)
		{
			std::string escaped_message;
			const char* message = a_cont.GetStrValueRef();
			if (message && *message)
			{
				auto res = vs::escape_unsafe_html_tags(message, escaped_message);
				assert(res == true);
				assert(escaped_message.length() >= strlen(message));
				cont.AddValue(MESSAGE_PARAM, escaped_message);
			}
		}
		else
		{
			a_cont.AttachCurrentTo(cont);
		}
	}

	VS_Container cnt(cont); // to avoid pointers invalidation
	const char* conf = cont.GetStrValueRef(CONFERENCE_PARAM);
	const char* from = cont.GetStrValueRef(FROM_PARAM);
	const char* display_name = cont.GetStrValueRef(DISPLAYNAME_PARAM);
	const char* message = cont.GetStrValueRef(MESSAGE_PARAM);
	std::time_t timestamp = 0;
	std::time_t *ptimestamp = nullptr;
	std::chrono::system_clock::time_point ts;

	if (!CheckHops(cnt))
	{
		dstream3 << "chat message drop, hops expired. Mess: " << *m_recvMess << ", cnt:\n" << cnt;
		return;
	}

	if (cont.GetValue(FILETIME_PARAM, ts))
	{
		timestamp = std::chrono::system_clock::to_time_t(ts);
		ptimestamp = &timestamp;
	}

	if (!conf)
	{
		const char *to = cont.GetStrValueRef(TO_PARAM);
		if (!to || *to == '\0') // transmit message to the user
		{
			return;
		}

		int64_t mID = 0;
		if (m_chatDB != nullptr)
		{

			if (m_chatDB->AddNewMessage(mID, message, ptimestamp, from, display_name, to))
			{
				UpdateDBMsgID(cnt, mID);
				if (!m_chatDB->ChangeMessage(mID, &cnt, 0, 0))
				{
					dstream3 << "VS_ChatService::SendMessage_Method(to = " << to << ", from = " << from <<", dn = " << display_name << "): cannot change data in DB!";
				}
			}
			else
			{
				dstream3 << "VS_ChatService::SendMessage_Method(to = " << to << ", from = " << from << ", dn = " << display_name << "): cannot create a new message in DB!";
			}
		}
		SendToUserOrService(from_id, mID, cnt, to);
	}
	else
	{
		bool from_user = cont.GetInt64ValueRef(DB_MESSAGE_ID_PARAM) == nullptr;
		auto dst = cont.GetStrValueRef(TRANSPORT_DSTUSER_PARAM);
		int64_t mID = 0;
		if (m_chatDB != nullptr)
		{
			if (m_chatDB->AddNewMessage(mID, message, ptimestamp, from, display_name, dst, conf))
			{
				UpdateDBMsgID(cnt, mID);
				if (!m_chatDB->ChangeMessage(mID, &cnt))
				{
					dprint3("VS_ChatService::SendMessage_Method(from = %s, dn = %s): cannot change data in DB!\n", from, display_name);
				}
			}
			else
			{
				dprint3("VS_ChatService::SendMessage_Method(from = %s, dn = %s): cannot create a new message in DB!\n", from, display_name);
			}
		}

		auto broker_id = strstr(conf, "@");
		if (broker_id) {
			broker_id++;
			if (strcasecmp(OurEndpoint(), broker_id) == 0)
			{
				// Check if chat for guests is allowed on VCS conference.
				if (m_confRestrict && m_confRestrict->IsVCS()) // VS_ChatService might be used in the AppServer too.
				{
					VS_ParticipantDescription pd;
					if (!g_storage->FindParticipant(from_id, pd) || (pd.m_CMRFlags&CMRFlags::CHAT_SEND) == CMRFlags::NONE)
						return;
				}

				if (g_storage->IsParticipantIgnored(conf, from_id))
					return;

				if (g_storage->IsConferenceRecording(conf) && m_confRestrict)
					m_confRestrict->LogGroupChat(cnt);

				std::vector<tPartServer> part_serv;
				g_storage->GetParticipants(conf, part_serv);
				for (auto &i : part_serv) {
					if (i.first != from_id) {
						VS_Container c(cnt);
						c.AddValue(TRANSPORT_DSTUSER_PARAM, i.first);
						SendToUserOrService(from_id, mID, c, i.first, i.second, false);
					}
				}
			}
			else
			{
				if (from_user)		// from our user to remote conf (make sure it is from user, not from roaming server)
					PostRequest(broker_id, 0, cnt, 0, CHAT_SRV, default_timeout, 0, from_id);
				else {		// from remote conf to our user
					if (dst && *dst)
						PostRequest(0, dst, cnt, 0, CHAT_SRV, default_timeout, 0, from_id);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// SENDCOMMAND_METHOD(FROM_PARAM, TO_PARAM, MESSAGE_PARAM)
////////////////////////////////////////////////////////////////////////////////
void VS_ChatService::SendCommand_Method(const char* from_id, VS_Container &cnt)
{
	if (ProcessSlideShowCommand(cnt))
		return;
	if (ProcessExtraVideoFlowCommand(cnt))
		return;

	const char* conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (!conf)
		return;

	VS_BinBuff buff; buff.Set(cnt);
	std::vector<tPartServer> part_serv;
	g_storage->GetParticipants(conf, part_serv);
	if (buff.Buffer())
		for (auto &i : part_serv)
			PostRequest(i.second, i.first, buff.Buffer(), buff.Size());

}

static const boost::regex command_param_re("([^\r\n=]+)=([^\r\n=]+)", boost::regex::optimize);

bool VS_ChatService::ProcessSlideShowCommand(VS_Container& cnt)
{
	const char* conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (!conf||!*conf)
		return false;
	const char *from = cnt.GetStrValueRef(FROM_PARAM);
	if (!from||!*from)
		return false;
	const char *msg = cnt.GetStrValueRef(MESSAGE_PARAM);
	if (!msg||!*msg)
		return false;

	bool bNewSlide = strncasecmp(msg,SHOW_SLIDE_COMMAND,strlen(SHOW_SLIDE_COMMAND))==0;
	bool bEndSlideShow = strncasecmp(msg,END_SLIDESHOW_COMMAND,strlen(END_SLIDESHOW_COMMAND))==0;
	if (!bNewSlide && !bEndSlideShow)
		return false;

	// logging
	if (g_storage->IsConferenceRecording(conf) && m_confRestrict)
		m_confRestrict->LogSlideShow(cnt);

	UpdateSlideShowState(conf, from, bNewSlide, msg);
	SendSlideCommandToMixer(conf, from, msg);
	return true;
}

void VS_ChatService::UpdateSlideShowState(const char* conf, const char* user, bool state, const char* cmd)
{
	auto& ci = m_confs[conf];
	auto& seq_idx = ci.slide_senders.get<seq_tag>();
	auto& val_idx = ci.slide_senders.get<val_tag>();

	int32_t client_type(CT_SIMPLE_CLIENT);

	if (strcasecmp(cmd, END_SLIDESHOW_COMMAND) == 0) {
		auto si = val_idx.find(user);
		if (si == val_idx.end() || si->cmd == END_SLIDESHOW_COMMAND)
			return; // nothing to end;
		client_type = si->client_type;
	}
	else {
		VS_ParticipantDescription pd;
		if (!g_storage->FindParticipant(user, pd) || pd.m_conf_id != conf)
			return; // not participant of conference in command
		client_type = pd.m_ClientType;
	}

	auto last_tcc_slide_it = std::find_if(seq_idx.begin(), seq_idx.end(), [&](const slide_info& x) {
		return !IsRtpTrancoder(x.client_type);
	});
	const bool active_tcc_slide_changed = state
		? !IsRtpTrancoder(client_type)
		: last_tcc_slide_it != seq_idx.end() && last_tcc_slide_it->user == user;

	if (state)
	{
		// Delete all slides from this user and save only new slide.
		val_idx.erase(user);
		seq_idx.emplace_front(user, cmd, client_type);
	}
	else
	{
		if (val_idx.erase(user) == 0)
			return; // This command didn't change the slideshow state, we can drop it.
	}

	if (active_tcc_slide_changed)
	{
		last_tcc_slide_it = std::find_if(seq_idx.begin(), seq_idx.end(), [&](const slide_info& x) {
			return !IsRtpTrancoder(x.client_type);
		});
		if (last_tcc_slide_it != seq_idx.end())
			ci.rtp_content_source = ContentSource::Slideshow;
		else
			ci.rtp_content_source = ContentSource::None;
	}

	VS_BinBuff buffcmd;
	{
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
		rCnt.AddValue(FROM_PARAM, user);
		rCnt.AddValue(MESSAGE_PARAM, cmd);
		buffcmd.Set(rCnt);
	}

	VS_ParticipantDescription* parts;
	int NumOfPart = 0;
	if ((NumOfPart = g_storage->GetParticipants(conf, parts)) > 0)
	{
		for (int j = 0; j < NumOfPart; ++j)
		{
			if (parts[j].m_user_id == user)
				continue;
			if (IsRtpTrancoder(parts[j].m_ClientType)) {
				// RTP participant
				if (active_tcc_slide_changed)
				{
					VS_Container rCnt;
					rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
					if (last_tcc_slide_it != seq_idx.end())
					{
						rCnt.AddValue(FROM_PARAM, last_tcc_slide_it->user.c_str());
						rCnt.AddValue(MESSAGE_PARAM, last_tcc_slide_it->cmd.c_str());
					}
					else
					{
						// Only way we can get there is when the last TCC slide was removed by this command
						rCnt.AddValue(FROM_PARAM, user);
						rCnt.AddValue(MESSAGE_PARAM, END_SLIDESHOW_COMMAND);
					}
					PostRequest(parts[j].m_server_id, parts[j].m_user_id, rCnt);
				}
			}
			else {
				PostRequest(parts[j].m_server_id, parts[j].m_user_id, buffcmd.Buffer(), buffcmd.Size());
			}
		}
		delete[] parts;
	}
}

void VS_ChatService::SendSlideCommandToMixer(const char * conf, const char * from, const char * msg)
{
	auto transceiversPool = m_transceiversPool.lock();
	if (!transceiversPool) {
		return;
	}
	auto streamsCircuit = transceiversPool->GetTransceiverProxy(conf);
	if (!streamsCircuit) {
		return;
	}
	auto confCtrl = streamsCircuit->ConfControl();
	if (!confCtrl) {
		return;
	}

	auto& ci = m_confs[conf];
	auto& seq_idx = ci.slide_senders.get<seq_tag>();

	confCtrl->SlideCommand(conf, from, msg);
	if (strcasecmp(msg, END_SLIDESHOW_COMMAND) == 0) {
		auto sl = seq_idx.begin();
		if (sl != seq_idx.end()) {
			confCtrl->SlideCommand(conf, sl->user, sl->cmd);
		}
	}
}

bool VS_ChatService::ProcessExtraVideoFlowCommand(VS_Container& cnt)
{
	const char* conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (!conf||!*conf)
		return false;
	const char* from = cnt.GetStrValueRef(FROM_PARAM);
	if (!from||!*from)
		return false;
	const char* msg = cnt.GetStrValueRef(MESSAGE_PARAM);
	if (!msg || !*msg)
		return false;

	string_view msg_v(msg);
	if (!boost::starts_with(msg_v, string_view(EXTRAVIDEOFLOW_NOTIFY)))
		return false;

	bool content_available;
	bool valid = false;
	for (boost::cregex_iterator param_it(msg_v.begin() + strlen(EXTRAVIDEOFLOW_NOTIFY), msg_v.end() - strlen(EXTRAVIDEOFLOW_NOTIFY), command_param_re); param_it != boost::cregex_iterator(); ++param_it)
	{
		if (param_it->empty())
			continue;
		if ((*param_it)[1] == CONTENTAVAILABLE_PARAM)
		{
			if ((*param_it)[2] == "true")
			{
				content_available = true;
				valid = true;
			}
			if ((*param_it)[2] == "false")
			{
				content_available = false;
				valid = true;
			}
		}
	}
	if (!valid)
		return false;

	UpdateContentState(conf, from, content_available);
	return false; // ExtraVideoFlow command itself should be forwarded to others in any case
}

void VS_ChatService::UpdateContentState(const char* conf, const char* user, bool state)
{
	auto& ci = m_confs[conf];
	auto& seq_idx = ci.content_senders.get<seq_tag>();
	auto& val_idx = ci.content_senders.get<val_tag>();

	bool active_content_changed = false;

	auto it = val_idx.find(user);
	if (state && it == val_idx.end())
	{
		// Available now, wasn't available before.
		// Start forwarding content from this sender to others.
		active_content_changed = true;
		seq_idx.emplace_front(user);
	}
	else if (!state && it != val_idx.end())
	{
		// Not available now, was available before.
		// If content from this sender was active then start forwarding content from previuos sender if any.
		active_content_changed = ci.content_senders.project<seq_tag>(it) == seq_idx.begin();
		val_idx.erase(it);
	}

	if (!active_content_changed)
		return;

	auto last_tcc_slide_it = std::find_if(ci.slide_senders.get<seq_tag>().begin(), ci.slide_senders.get<seq_tag>().end(), [&](const slide_info& x) {
		return !IsRtpTrancoder(x.client_type);
	});

	if (ci.content_senders.empty() && last_tcc_slide_it == ci.slide_senders.get<seq_tag>().end())
	{
		// Send CONTENTFORWARD_STOP to everybody.
		ci.rtp_content_source = ContentSource::None;

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
		rCnt.AddValue(FROM_PARAM, user);
		rCnt.AddValue(MESSAGE_PARAM, CONTENTFORWARD_STOP);
		VS_BinBuff buff; buff.Set(rCnt);

		std::vector<tPartServer> part_serv;
		g_storage->GetParticipants(conf, part_serv);
		if (buff.Buffer())
			for (auto &i : part_serv)
				PostRequest(i.second, i.first, buff.Buffer(), buff.Size());
	}
	else if (ci.content_senders.empty() && last_tcc_slide_it != ci.slide_senders.get<seq_tag>().end())
	{
		// Resend last SHOW_SLIDE_COMMAND that came from non-RTP participant to other RTP participants.
		ci.rtp_content_source = ContentSource::Slideshow;

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
		rCnt.AddValue(FROM_PARAM, last_tcc_slide_it->user.c_str());
		rCnt.AddValue(MESSAGE_PARAM, last_tcc_slide_it->cmd.c_str());
		VS_BinBuff buff; buff.Set(rCnt);

		VS_ParticipantDescription* parts;
		int NumOfPart = 0;
		if ((NumOfPart = g_storage->GetParticipants(conf, parts)) > 0)
		{
			for (int j = 0; j < NumOfPart; ++j)
			{
				if (parts[j].m_user_id == last_tcc_slide_it->user.c_str())
					continue;
				if (!IsRtpTrancoder(parts[j].m_ClientType))
					continue;
				PostRequest(parts[j].m_server_id, parts[j].m_user_id, buff.Buffer(), buff.Size());
			}
			delete[] parts;
		}
	}
	else
	{
		// Send CONTENTFORWARD_PUSH to active sender, send CONTENTFORWARD_PULL to everybody else.
		ci.rtp_content_source = ContentSource::RTPExtraVideo;
		const auto& active_sender = seq_idx.front();

		VS_BinBuff buff;
		{
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
			rCnt.AddValue(FROM_PARAM, active_sender.c_str());
			rCnt.AddValue(MESSAGE_PARAM, CONTENTFORWARD_PULL);
			buff.Set(rCnt);
		}

		std::vector<tPartServer> part_serv;
		g_storage->GetParticipants(conf, part_serv);
		for (auto &i : part_serv) {
			if (i.first == active_sender.c_str()) {
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
				rCnt.AddValue(FROM_PARAM, active_sender.c_str());
				rCnt.AddValue(MESSAGE_PARAM, CONTENTFORWARD_PUSH);
				PostRequest(i.second, i.first, rCnt);
			}
			else {
				PostRequest(i.second, i.first, buff.Buffer(), buff.Size());
			}
		}
	}
}

void VS_ChatService::SendSlidesToUser_Method(VS_Container &cnt)
{
	const char* conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (!conf||!*conf)
		return;
	const char* user = cnt.GetStrValueRef(CALLID_PARAM);
	if (!user||!*user)
		return;

	VS_ParticipantDescription pd;
	if (!g_storage->FindParticipant(user, pd))
		return;

	auto& ci = m_confs[conf];
	if (!IsRtpTrancoder(pd.m_ClientType))
	{
		// Send all slides to non-RTP participant
		for (auto it = ci.slide_senders.get<seq_tag>().rbegin(); it != ci.slide_senders.get<seq_tag>().rend(); ++it)
		{
			if (it->user == user)
				continue;
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
			rCnt.AddValue(FROM_PARAM, it->user.c_str());
			rCnt.AddValue(MESSAGE_PARAM, it->cmd.c_str());
			PostRequest(pd.m_server_id, user, rCnt);
		}
	}
	else if (ci.rtp_content_source == ContentSource::Slideshow)
	{
		// Send last TCC slide to RTP participant
		auto last_tcc_slide_it = std::find_if(ci.slide_senders.get<seq_tag>().begin(), ci.slide_senders.get<seq_tag>().end(), [&](const slide_info& x) {
			return !IsRtpTrancoder(x.client_type);
		});
		assert(last_tcc_slide_it != ci.slide_senders.get<seq_tag>().end());

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
		rCnt.AddValue(FROM_PARAM, last_tcc_slide_it->user.c_str());
		rCnt.AddValue(MESSAGE_PARAM, last_tcc_slide_it->cmd.c_str());
		PostRequest(pd.m_server_id, user, rCnt);
	}
	else if (ci.rtp_content_source == ContentSource::RTPExtraVideo)
	{
		// Send CONTENTFORWARD_PULL to RTP participant
		assert(!ci.content_senders.empty());
		const auto& active_sender = ci.content_senders.get<seq_tag>().front();

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
		rCnt.AddValue(FROM_PARAM, active_sender.c_str());
		rCnt.AddValue(MESSAGE_PARAM, user == active_sender ? CONTENTFORWARD_PUSH : CONTENTFORWARD_PULL);
		PostRequest(pd.m_server_id, user, rCnt);
	}
	if (pd.m_ClientType == CT_TRANSCODER || pd.m_ClientType == CT_TRANSCODER_CLIENT || pd.m_ClientType == CT_WEB_CLIENT) {
		// Send last slide to mixer
		auto it = ci.slide_senders.get<seq_tag>().begin();
		if (it != ci.slide_senders.get<seq_tag>().end()) {
			SendSlideCommandToMixer(conf, it->user.c_str(), it->cmd.c_str());
		}
	}
}

void VS_ChatService::ConferenceDeleted_Method(VS_Container &cnt)
{
	const char* stream_id = cnt.GetStrValueRef(NAME_PARAM);
	if (!stream_id||!*stream_id)
		return;
	m_confs.erase(stream_id);
}

void VS_ChatService::DeleteParticipant_Method(VS_Container &cnt)
{
	const char* conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (!conf || !*conf)
		return;
	const char* user = cnt.GetStrValueRef(USERNAME_PARAM);
	if (!user || !*user)
		return;

	UpdateSlideShowState(conf, user, false, END_SLIDESHOW_COMMAND);
	UpdateContentState(conf, user, false);
}

bool VS_ChatService::CheckHops(VS_Container& cnt)
{
	auto ptr = cnt.GetLongValueRef(HOPS_PARAM);
	if (!ptr)
	{
		cnt.AddValueI32(HOPS_PARAM, 3);
		return true;
	}
	--(*ptr);
	return *ptr > 0;
}
