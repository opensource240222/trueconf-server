#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>

#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/fast_mutex.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/compat/map.h"

class VS_Policy
{
public:
	static const unsigned int DEFAULT_TYPE;

	typedef std::chrono::steady_clock	clock_t;
	typedef clock_wrapper<clock_t>		clock_wrapper_t;

	struct PolicySettings final
	{
		unsigned use_ip = 1;
		unsigned use_login = 1;
		unsigned access_deny_on_ban = 0; // do not use delay
		unsigned silent_on_ban = 1;

		unsigned max_fail_before_ban = 10; // ban after 10 requests per observe_interval=1min;
		unsigned max_fail_before_delay = 0;

		clock_t::duration delay_interval = std::chrono::seconds(5); // delay between calls
		clock_t::duration delay_time = std::chrono::seconds(60 * 60); // switch on delay for next delay_time seconds
		clock_t::duration ban_time = std::chrono::hours(24);
		clock_t::duration delay_ttl = std::chrono::seconds(60); // invalidate request if it waits more then delay_ttl seconds
		clock_t::duration observe_interval = std::chrono::minutes(1);

		bool use_ban() const noexcept;
		bool use_delay() const noexcept;

	};

public:
	class PolicySettingsInitInterface
	{
	public:
		struct PolicyEndpointSettings final
		{
			const unsigned int type;
			const char *root;
			const char *name;
			const PolicySettings *policy;
		};
		PolicySettingsInitInterface() = default;
		virtual ~PolicySettingsInitInterface() {}
		PolicySettingsInitInterface(const PolicySettingsInitInterface& other) = delete;
		PolicySettingsInitInterface(PolicySettingsInitInterface&& other) noexcept = default;
		PolicySettingsInitInterface& operator=(const PolicySettingsInitInterface& other) = delete;
		PolicySettingsInitInterface& operator=(PolicySettingsInitInterface&& other) noexcept = default;
		virtual unsigned int GetPolicyEndpointSettings(const PolicyEndpointSettings *&) const = 0;
	};

private:
	static const PolicySettingsInitInterface *GetDefaultPolicySettings();
public:
	VS_Policy(const char *protocol, const PolicySettingsInitInterface *settings = GetDefaultPolicySettings());
	VS_Policy(VS_Policy &&other) noexcept;
	VS_Policy& operator=(VS_Policy &&other) noexcept;

	VS_Policy(const VS_Policy &other) = delete;
	VS_Policy &operator=(const VS_Policy &other) = delete;

	void Request(string_view ip, string_view login, const std::function<void(bool result)> &cb, const unsigned int type = DEFAULT_TYPE);
	bool Request(string_view ip, string_view login, const unsigned int type = DEFAULT_TYPE);

	void SetResult(string_view ip, string_view login, bool result, const unsigned int type = DEFAULT_TYPE);
	void Timeout();
	VS_UserLoggedin_Result FailResult(const unsigned int type = DEFAULT_TYPE);
	bool ShouldBeChecked(VS_ClientType ct) const noexcept;
	static bool is_vcs;
	static void SetIsVCS(bool) noexcept;
protected:
	clock_wrapper_t& clock() const noexcept { return m_clock; }

	struct DelayedCall final
	{
		clock_t::time_point add_time;
		std::function<void(bool)> cb;
	};

	struct EndpointInformation final
	{
		std::string ip;
		std::string login;

		// for every fail login attempt in last observe_interval contains time of attempt
		std::vector<clock_t::time_point> fail_counter;

		bool is_banned() const;
		clock_t::time_point ban_expire_time;
		bool is_ban_logged;

		bool is_delayed() const;
		clock_t::time_point delay_expire_time;

		std::vector< DelayedCall > delayed;
		clock_t::time_point last_call_delayed;

	};

	struct PolicyEndpoint final
	{
		PolicySettings rules;
		vs::map<std::string, EndpointInformation, vs::str_less> endpoints;
		clock_t::time_point delayed_timeout;
	};

	void UpdateRules();

	void RequestLocked(string_view ip, string_view login,
		const std::function<void(bool)> &cb, bool &callNow, bool &res, const unsigned int type);

	EndpointInformation& GetEndpointInformation(string_view ip, string_view login, PolicyEndpoint &obj, const unsigned type);
	void ReqFail(EndpointInformation &e, PolicyEndpoint &obj);
	std::string GetKeyForMap(string_view ip, string_view login, PolicyEndpoint &obj) const;
	void CheckDelayed(PolicyEndpoint &obj);
	void RemoveExpired(PolicyEndpoint &obj);

	static void RemoveOld(std::vector<clock_t::time_point> &v, clock_t::time_point lowerBound);
	static void RemoveOld(std::vector<DelayedCall> &v, clock_t::time_point lowerBound);
	PolicySettings GetDefaultSettings() const;
private:
	vs::map<int, PolicyEndpoint> m_policy;
	const PolicySettingsInitInterface *m_policy_init;
	std::string m_protocol;

	typedef vs::fast_mutex mutex_t;
	mutex_t m_mutex;

	mutable clock_wrapper_t m_clock;
};
