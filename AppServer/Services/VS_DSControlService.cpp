#include "VS_DSControlService.h"
#include "AppServer/Services/VS_Storage.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_DSCONTROL

bool VS_DSControlService::Init( const char *our_endpoint, const char *our_service, const bool permittedAll )
{
	return true;
}

bool VS_DSControlService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)		return true;
	VS_Container cnt;
	switch (recvMess->Type()) {
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
		m_recvMess = recvMess.get();
		if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
			// Search method name
			auto method = cnt.GetStrValueView(METHOD_PARAM);
			if (method.empty())
				break;
			// Process methods
			if (method == DSCOMMAND_METHOD)
				DSCommand_Method(cnt);
			else {
				dprint3("Method %20s| srv %20s | user %20s\n", method.c_str(), recvMess->SrcServer(), recvMess->SrcUser());
				if (method == VIDEOSOURCETYPE_METHOD)
					VideoSourceType_Method(cnt);
				else if (method == DSCONTROL_REQUEST_METHOD)
					DSControlRequest_Method(cnt);
				else if (method == DSCONTROL_RESPONSE_METHOD)
					DSControlResponse_Method(cnt);
				else if (method == DSCONTROL_FINISH_METHOD)
					DSControlFinish_Method(cnt);
				else if (method == DELETEPARTICIPANT_METHOD)
					DeleteParticipant_Method(cnt);
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

/** DS Control**/

// user must send this method only to own server to check ds control rigth
void VS_DSControlService::VideoSourceType_Method(VS_Container &cnt)
{
	const auto src_user = m_recvMess->SrcUser_sv();
	const char *user = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (!conf || !user)
		return;

	if (!src_user.empty()) {
		// message from user
		VS_UserData ud;
		if (!g_storage->FindUser(src_user, ud) || !(ud.m_rights & ud.UR_COMM_SHARE_CONTROL) || src_user != user)
			return;
		const char* server = VS_GetConfEndpoint(conf);
		if (!server)
			return;
		if (strcasecmp(server, OurEndpoint()) != 0) {
			PostRequest(server, 0, cnt, m_recvMess->AddString());
			return;
		}
	}

	VS_ParticipantDescription pd;
	if (!g_storage->FindParticipant(user, pd) || pd.m_conf_id != conf)
		return;

	VS_VideoSourceType type(VST_UNKNOWN);
	cnt.GetValueI32(TYPE_PARAM, type);
	if (pd.m_videoType != type) {
		pd.m_videoType = type;
		g_storage->UpdateParticipant(pd);
		if (type != VST_APPLICATION && type != VST_DESKTOP) {
			// finish control for all
			DSControlFinished(pd.m_user_id.m_str, {});
			m_ds_map.erase(user);
			dstream3 << "Erase Video Source for: " << user;
		}
		else {
			ToPart tp = { pd.m_conf_id.m_str, pd.m_server_id.m_str, {} };
			m_ds_map.emplace(pd.m_user_id.m_str, std::move(tp));
			dstream3 << "Insert Video Source for: " << user << " type: " << type;
		}
		// send part list via CONFERENCE_SRV
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, UPDATEPARTICIPANT_METHOD);
		rCnt.AddValue(USERNAME_PARAM, user);
		PostRequest(OurEndpoint(), 0, rCnt, 0, CONFERENCE_SRV);
	}
}

void VS_DSControlService::DSControlRequest_Method(VS_Container &cnt)
{
	string_view from = cnt.GetStrValueView(FROM_PARAM);
	string_view to = cnt.GetStrValueView(TO_PARAM);
	string_view conf = cnt.GetStrValueView(CONFERENCE_PARAM);
	if (from.empty() || to.empty() || conf.empty())
		return;

	auto it = m_ds_map.find(to);
	auto itf = m_ds_map.find(from);
	bool reverse_found = itf != m_ds_map.end() && itf->second.from_map.find(to) != itf->second.from_map.end();
	if (it == m_ds_map.end() || it->second.conf != conf || reverse_found) {
		dstream1 << "BADSRC Responce for: " << from;

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, DSCONTROL_RESPONSE_METHOD);
		rCnt.AddValue(CONFERENCE_PARAM, conf);
		rCnt.AddValue(FROM_PARAM, from);
		rCnt.AddValue(TO_PARAM, to);
		rCnt.AddValue(RESULT_PARAM, DSCR_BADSRC);
		PostReply(rCnt);
	}
	else
		PostRequest(it->second.server.c_str(), to.data(), cnt); // to is null-terminated

}

void VS_DSControlService::DSControlResponse_Method(VS_Container &cnt)
{
	// assume GetStrValueView() is null-terminated 
	string_view from = cnt.GetStrValueView(FROM_PARAM);
	string_view to = cnt.GetStrValueView(TO_PARAM);
	string_view conf = cnt.GetStrValueView(CONFERENCE_PARAM);
	if (from.empty() || to.empty() || conf.empty())
		return;

	VS_ParticipantDescription pd;
	int32_t result(0); cnt.GetValue(RESULT_PARAM, result);

	auto it = m_ds_map.find(to);
	if (it == m_ds_map.end() || it->second.conf != conf)
		return;
	if (!g_storage->FindParticipant(from.data(), pd))
		return;
	if (result == DSCR_ALLOW)
		it->second.from_map.emplace(from, pd.m_server_id.m_str);
	else
	{
		const auto ifr = it->second.from_map.find(from);
		if (ifr != it->second.from_map.end())
			it->second.from_map.erase(ifr);
	}
	dstream2 << "Make pair (" << from << " ->  " << to << ")  result: " << result;

	PostRequest(pd.m_server_id, pd.m_user_id, cnt);
}

void VS_DSControlService::DSControlFinish_Method(VS_Container &cnt)
{
	string_view from = cnt.GetStrValueView(FROM_PARAM);
	string_view to = cnt.GetStrValueView(TO_PARAM);		// "to" is controlled by "from"
	string_view conf = cnt.GetStrValueView(CONFERENCE_PARAM);
	if (conf.empty())
		return;

	if (!DSControlFinished(to, from) && !to.empty() && !from.empty()) {
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, DSCONTROL_FINISHED_METHOD);
		rCnt.AddValue(FROM_PARAM, from);
		rCnt.AddValue(TO_PARAM, to);
		rCnt.AddValue(CONFERENCE_PARAM, conf);

		PostReply(rCnt);
	}
}

bool VS_DSControlService::DSControlFinished(string_view to, string_view from)
{
	auto DSFinished = [&](const std::string& t, std::string& st, std::string& c, const std::string& f, std::string& sf) {
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, DSCONTROL_FINISHED_METHOD);
		rCnt.AddValue(FROM_PARAM, f);
		rCnt.AddValue(TO_PARAM, t);
		rCnt.AddValue(CONFERENCE_PARAM, c);
		VS_BinBuff buff; buff.Set(rCnt);

		PostRequest(st.c_str(), t.c_str(), buff.Buffer(), buff.Size());
		PostRequest(sf.c_str(), f.c_str(), buff.Buffer(), buff.Size());
		dstream2 << "Del pair (" << f << " -> " << t << ")";
	};

	if (!to.empty()) {
		auto it = m_ds_map.find(to);
		if (it == m_ds_map.end())
			return false;
		if (!from.empty()) {
			auto ifr = it->second.from_map.find(from);
			if (ifr == it->second.from_map.end())
				return false;
			DSFinished(it->first, it->second.server, it->second.conf, ifr->first, ifr->second);
			it->second.from_map.erase(ifr);
		}
		else {
			if (it->second.from_map.empty())
				return false;
			for (auto &ifr : it->second.from_map)
				DSFinished(it->first, it->second.server, it->second.conf, ifr.first, ifr.second);
			it->second.from_map.clear();
		}
		return true;
	}
	else if (!from.empty()) {
		bool ret = false;
		for (auto &it : m_ds_map) {
			auto ifr = it.second.from_map.find(from);
			if (ifr != it.second.from_map.end()) {
				DSFinished(it.first, it.second.server, it.second.conf, ifr->first, ifr->second);
				it.second.from_map.erase(ifr);
				ret = true;
			}
		}
		return ret;
	}
	return false;
}

void VS_DSControlService::DSCommand_Method(VS_Container &cnt)
{
	string_view src_user = m_recvMess->SrcUser_sv();
	string_view to = cnt.GetStrValueView(TO_PARAM);
	if (!src_user.empty() && !to.empty()) {
		const char* p_server = PairControlled(to, src_user);
		if (!p_server)
			return;
		const char* from = cnt.GetStrValueRef(FROM_PARAM);
		if (!from) {
			cnt.AddValue(FROM_PARAM, src_user);
			to = cnt.GetStrValueView(TO_PARAM); // Reload reference because of possible reallocation in the container
		}
		PostRequest(p_server, to.data(), cnt);
	}

	uint32_t s = cnt.GetAllocSize();

	if (s > 2000)
		dstream1 << "Command too big: " << s;
}

void VS_DSControlService::DeleteParticipant_Method(VS_Container &cnt)
{
	string_view user = cnt.GetStrValueView(USERNAME_PARAM);
	DSControlFinished(user, {});
	DSControlFinished({}, user);
	const auto it = m_ds_map.find(user);
	if (it != m_ds_map.end()) {
		m_ds_map.erase(it);
		dstream3 << "DeleteParticipant: Erase Video Source for: " << user;
	}
}

// returns server of controlled participant, if pair exists
const char* VS_DSControlService::PairControlled(string_view to, string_view from)
{
	const auto it = m_ds_map.find(to);
	if (it == m_ds_map.end())
		return nullptr;
	const auto ifr = it->second.from_map.find(from);
	if (ifr == it->second.from_map.end())
		return nullptr;
	return it->second.server.c_str();
}
