#include "VS_AutoInviteService.h"
#include "AppServer/Services/VS_Storage.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_MCS

const VS_SimpleStr VS_AutoInviteService::AUTOINVITE_PRESENCE_PARAM = (std::string(std::string(LISTENER_USER) + "_autoinvite").c_str());

bool VS_AutoInviteService::Timer(unsigned long tickcount)
{
	std::set<vs_conf_id> confs_to_delete;
	VS_Container users_to_subscribe;
	//PrintDebug();

	{
		std::vector<VS_ConferenceDescription> current_confs;
		g_storage->GetCurrentConferences(current_confs);

		auto &&autoinvites_lock = m_autoinvites.lock();

		for (auto &info : *autoinvites_lock) {
			if (info.subscribed || info.time > m_clock.now()) {
				continue;
			}

			auto conf = std::find_if(begin(current_confs), end(current_confs), [&](const VS_ConferenceDescription &cd) {
				return cd.m_name == info.conf_id;
			});
			if (conf == end(current_confs)) {
				confs_to_delete.insert(info.conf_id);
				continue;
			}

			users_to_subscribe.AddValue(CALLID_PARAM, info.user_id);
			info.subscribed = true;
		}
	}

	if (!users_to_subscribe.IsEmpty())
		m_presenceService->Subscribe(users_to_subscribe, VS_FullID(OurEndpoint(), AUTOINVITE_PRESENCE_PARAM));

	for (const auto &conf_id : confs_to_delete) {
		RemoveConf(conf_id);
	}

	return true;
}

bool VS_AutoInviteService::SendInvite(const vs_conf_id &conferenceId, const vs_user_id &userName)
{
	dstream3 << "AutoInvite SendInvite: conf=" << conferenceId.m_str << ", user=" << userName.m_str;
	std::vector<VS_ConferenceDescription> current_confs;
	g_storage->GetCurrentConferences(current_confs);

	auto conf = std::find_if(begin(current_confs), end(current_confs), [&](const VS_ConferenceDescription &cd) {
		return cd.m_name == conferenceId;
	});
	if (conf != end(current_confs)) {
		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, INVITE_METHOD);
		cnt.AddValue(CALLID_PARAM, userName);
		cnt.AddValue(TYPE_PARAM, conf->m_type);
		cnt.AddValue(CONFERENCE_PARAM, conferenceId);
		cnt.AddValue(SERVER_PARAM, OurEndpoint());
		PostRequest(OurEndpoint(), NULL, cnt);

		return true;
	}

	return false;
}

void VS_AutoInviteService::SetSupport(const vs_conf_id &conferenceName, const vs_user_id &userName, bool supports)
{
	dstream3 << "AutoInvite SetSupport: conf=" << conferenceName.m_str << ", user=" << userName.m_str << ", supports=" << supports;
	if (supports) {
		(*(m_autoinviteUsers.lock()))[conferenceName].insert(userName);
	} else {
		Unsubscribe(conferenceName, userName);
	}
}

bool VS_AutoInviteService::Subscribe(const vs_conf_id &conferenceName, const vs_user_id &userName, std::chrono::seconds after)
{ /// Use std::pair instead of AutoInviteInfo struct
	dstream3 << "AutoInvite Subscribe: conf=" << conferenceName.m_str << ", user=" << userName.m_str << ", after=" << after.count();

	auto &&autoinvite_usr_lock = m_autoinviteUsers.lock();
	auto &&conf_info = (*autoinvite_usr_lock).find(conferenceName);

	if (conf_info != autoinvite_usr_lock->cend()) {
		if (conf_info->second.find(userName) != conf_info->second.cend()) {

			auto &&unlock = autoinvite_usr_lock.scopedUnlock();

			AutoInviteInfo info(conferenceName, userName, after);

			auto &&autoinvites_lock = m_autoinvites.lock();
			auto prev_info_it = autoinvites_lock->find(info);

			if (prev_info_it != autoinvites_lock->cend()) {
				autoinvites_lock->erase(prev_info_it);
			}
			autoinvites_lock->insert(std::move(info));
			//PrintDebug();
			return true;
		}

		return false;
	}

	return false;
}

void VS_AutoInviteService::Unsubscribe(const vs_conf_id &conferenceName, const vs_user_id &userName)
{
	dstream3 << "AutoInvite UnSubs: conf=" << conferenceName.m_str << ", user=" << userName.m_str;
	//PrintDebug();

	bool subscribed = false;
	{
		auto &&autoinvites_lock = m_autoinvites.lock();
		auto invite_it = autoinvites_lock->find(AutoInviteInfo(conferenceName, userName));
		if (invite_it != autoinvites_lock->cend()) {
			subscribed = invite_it->subscribed;
			autoinvites_lock->erase(invite_it);
		}
	}

	if(subscribed)
		m_presenceService->Unsubscribe(userName, VS_FullID(OurEndpoint(), AUTOINVITE_PRESENCE_PARAM));

	{
		auto &&autoinvite_usr_lock = m_autoinviteUsers.lock();
		auto cd_info = autoinvite_usr_lock->find(conferenceName);
		if (cd_info != autoinvite_usr_lock->cend()) {
			auto pd_info = cd_info->second.find(userName);
			if (pd_info != cd_info->second.cend()) {
				cd_info->second.erase(pd_info);
			}
			if (cd_info->second.empty()) {
				autoinvite_usr_lock->erase(cd_info);
			}
		}
	}
}

void VS_AutoInviteService::RemoveConf(const vs_conf_id &conferenceName)
{
	dstream3 << "AutoInvite RemoveConf: conf=" << conferenceName.m_str;
	//PrintDebug();

	m_autoinviteUsers->erase(conferenceName);

	VS_Container users_to_UnSub;

	{
		auto &&autoinvites_lock = m_autoinvites.lock();
		for (auto autoinvite_it = autoinvites_lock->cbegin(); autoinvite_it != autoinvites_lock->cend();) {
			if (autoinvite_it->conf_id == conferenceName) {
				if (autoinvite_it->subscribed)
					users_to_UnSub.AddValue(CALLID_PARAM, autoinvite_it->user_id);
				autoinvite_it = (*autoinvites_lock).erase(autoinvite_it);
			}
			else {
				++autoinvite_it;
			}
		}
	}

	if (!users_to_UnSub.IsEmpty())
		m_presenceService->Unsubscribe(users_to_UnSub, VS_FullID(OurEndpoint(), AUTOINVITE_PRESENCE_PARAM));
}

void VS_AutoInviteService::OnServerChange(const char *call_id, const char *server_id)
{
	dstream3 << "AutoInvite OnServerChange: call_id=" << call_id << ", server=" << server_id;
	if (!call_id || !*call_id || !server_id || !*server_id)
		return;
	vs_user_id call_id_str = call_id;

	std::set<AutoInviteInfo> to_unsubscribe;

	m_autoinvites.withLock([&call_id_str, &to_unsubscribe](autoinvites_t &obj)
	{
		for (const auto &info : obj) {
			if (info.user_id == call_id_str) {
				to_unsubscribe.insert(info);
			}
		}
	});

	for (const auto &info : to_unsubscribe) {
		if (SendInvite(info.conf_id, info.user_id)) {
			Unsubscribe(info.conf_id, info.user_id);
		}
	}
}

//void VS_AutoInviteService::PrintDebug()
//{
//	auto ds = dstream3;
//	ds << "AutoInvite Timer: m_autoinvites=" << m_autoinvites.size() << ", m_autoinviteUsers=" << m_autoinviteUsers.size() << "\n";
//	for (const auto& i : m_autoinvites)
//		ds << "conf=" << i.conf_id.m_str << ",user=" << i.user_id.m_str << ",is_subs=" << i.subscribed /*<< ",time=" << std::put_time(std::localtime(&i.time), "%F %T")*/ << "\n";
//	for (const auto& p : m_autoinviteUsers)
//	{
//		ds << "conf=" << p.first.m_str << " users{";
//		for (const auto& i : p.second)
//			ds << i.m_str << ",";
//		ds << "}";
//	}
//}

bool operator == (const AutoInviteInfo &lhs, const AutoInviteInfo &rhs)
{
	return lhs.conf_id == rhs.conf_id && lhs.user_id == rhs.user_id;
}

bool operator < (const AutoInviteInfo &lhs, const AutoInviteInfo &rhs)
{
	if (lhs.conf_id == rhs.conf_id) {
		return lhs.user_id < rhs.user_id;
	}
	return lhs.conf_id < rhs.conf_id;
}
