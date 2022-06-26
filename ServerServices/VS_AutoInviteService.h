#pragma once

#include "../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "transport/VS_SystemMessage.h"
#include "VS_SubscriptionHub.h"
#include "../common/std/cpplib/VS_ConferenceID.h"
#include "../common/std/cpplib/VS_UserID.h"
#include "AppServer/Services/VS_PresenceService.h"

#include <map>
#include <set>
#include "std-generic/cpplib/VS_ClockWrapper.h"

const std::chrono::seconds AUTOINVITE_TIMEOUT{ 30 };

struct AutoInviteInfo
{
	vs_conf_id conf_id;
	vs_user_id user_id;
	std::chrono::steady_clock::time_point time;
	mutable bool subscribed;

	AutoInviteInfo(const vs_conf_id &cid = {}, const vs_user_id &uid = {}, std::chrono::seconds after = AUTOINVITE_TIMEOUT) :
		conf_id(cid), user_id(uid), time(std::chrono::steady_clock::now() + after), subscribed(false) { }

	friend bool operator == (const AutoInviteInfo &lhs, const AutoInviteInfo &rhs);
	friend bool operator < (const AutoInviteInfo &lhs, const AutoInviteInfo &rhs);
};

class VS_AutoInviteService :
	virtual public VS_TransportRouterServiceHelper,
	virtual public VS_PresenceServiceMember,
	public VS_SubscriptionHub::Listener
{
public:
	static const VS_SimpleStr AUTOINVITE_PRESENCE_PARAM;

	VS_AutoInviteService()
	{
		m_TimeInterval = std::chrono::seconds(1);
	}

	// VS_TransportRouterSimpleService implementation
	bool Timer(unsigned long tickcount) override;

	// VS_Object implementation
	bool IsValid() const { return true; }

	void SetSupport(const vs_conf_id &conferenceName, const vs_user_id &userName, bool supports);
	bool Subscribe(const vs_conf_id &conferenceName, const vs_user_id &userName, std::chrono::seconds after = AUTOINVITE_TIMEOUT);
	void Unsubscribe(const vs_conf_id &conferenceName, const vs_user_id &userName);
	void RemoveConf(const vs_conf_id &conferenceName);

	// VS_SubscriptionHub::Listener implementation
	void OnServerChange(const char *call_id, const char *server_id) override;

protected:
	steady_clock_wrapper &clock() noexcept { return m_clock; }

private:
	typedef std::map<vs_conf_id, std::set<vs_user_id>> autoinvite_usrs_t;
	typedef std::set<AutoInviteInfo> autoinvites_t;

	typedef  vs::fast_mutex mutex_t;

private:
	bool SendInvite(const vs_conf_id &conferenceName, const vs_user_id &userName);
	//void PrintDebug();

private:

	vs::Synchronized<autoinvite_usrs_t, mutex_t> m_autoinviteUsers;
	vs::Synchronized<autoinvites_t, mutex_t> m_autoinvites;

	steady_clock_wrapper m_clock;
};