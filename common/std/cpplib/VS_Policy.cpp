#include "VS_Policy.h"
#include "VS_RegistryKey.h"
#include "VS_RegistryConst.h"
#include "../debuglog/VS_Debug.h"

#include <cassert>
#include <mutex>

#define DEBUG_CURRENT_MODULE VS_DM_LOGINS

namespace
{
	constexpr unsigned int MAX_REQUESTS_LIMIT(10000);

	void get_duration(const VS_RegistryKey& key, const char* name, VS_Policy::clock_t::duration& outDuration) noexcept
	{
		assert(name);
		unsigned t;
		if (key.GetValue(&t, sizeof(unsigned), VS_REG_INTEGER_VT, name) > 0)
		{
			outDuration = std::chrono::seconds(t);
		}
	}

	VS_Policy::PolicySettings default_settings_vcs() noexcept
	{
		return {};
	}

	VS_Policy::PolicySettings default_settings_as() noexcept
	{
		VS_Policy::PolicySettings ds;
		ds.max_fail_before_ban = 10;
		ds.ban_time = std::chrono::seconds(60 * 60); // 1 hour
		ds.silent_on_ban = false;
		ds.access_deny_on_ban = true;

		return ds;
	}

	struct PolicySettingsDefaultInit final : public VS_Policy::PolicySettingsInitInterface
	{
		unsigned int GetPolicyEndpointSettings(const PolicyEndpointSettings*& obj) const noexcept override;

		static const PolicyEndpointSettings SETTINGS[];
	};

	const VS_Policy::PolicySettingsInitInterface::PolicyEndpointSettings PolicySettingsDefaultInit::SETTINGS[] { { static_cast<unsigned int>(VS_Policy::DEFAULT_TYPE), nullptr , nullptr, nullptr } };

	unsigned PolicySettingsDefaultInit::GetPolicyEndpointSettings(const PolicyEndpointSettings*& obj) const noexcept
	{
		obj = SETTINGS;
		return sizeof(SETTINGS) / sizeof(*SETTINGS);
	}

}//anonymous namespace

const unsigned int VS_Policy::DEFAULT_TYPE = -1;

const VS_Policy::PolicySettingsInitInterface *VS_Policy::GetDefaultPolicySettings()
{
	static PolicySettingsDefaultInit init{};
	return &init;
}

VS_Policy::VS_Policy(const char *protocol, const PolicySettingsInitInterface *settings)
	: m_policy_init(settings)
	, m_protocol(protocol)
{
	UpdateRules();
}

VS_Policy::VS_Policy(VS_Policy &&other) noexcept
{
	std::lock_guard<decltype(other.m_mutex)> _{ other.m_mutex };
	m_policy = std::move(other.m_policy);
	m_policy_init = other.m_policy_init; //trivially-copyable
	m_protocol = std::move(other.m_protocol),
	m_clock = other.m_clock; //trivially-copyable

	other.m_policy_init = nullptr;
	other.m_clock = {};
}

VS_Policy& VS_Policy::operator=(VS_Policy &&other) noexcept
{
	if (this == &other)
		return *this;

	std::lock(m_mutex, other.m_mutex);
	std::lock_guard<decltype(m_mutex)> lk1(m_mutex, std::adopt_lock);
	std::lock_guard<decltype(other.m_mutex)> lk2(other.m_mutex, std::adopt_lock);

	m_policy = std::move(other.m_policy);
	m_policy_init = other.m_policy_init; //trivially-copyable
	m_protocol = std::move(other.m_protocol);
	m_clock = other.m_clock; //trivially-copyable

	other.m_policy_init = nullptr;
	other.m_clock = {};

	return *this;
}

void VS_Policy::UpdateRules()
{
	const PolicySettingsInitInterface::PolicyEndpointSettings *data = nullptr;
	unsigned int len = m_policy_init->GetPolicyEndpointSettings(data);

	assert(data != nullptr ? len > 0 : len == 0);

	const auto set_key_func = [](const char *input, std::string &output) noexcept
	{
		if (input)
		{
			const auto input_len = ::strlen(input);
			if (input_len > 0)
			{
				const char delimeter = '\\';

				output.push_back(delimeter);
				output.append(input, input_len);
			}
		}
	};

	for (; len > 0; ++data, --len)
	{
		std::string key_name = CONFIGURATION_KEY;

		set_key_func(data->root, key_name);
		set_key_func(data->name, key_name);

		PolicySettings ds = data->policy ? *data->policy : GetDefaultSettings();

		VS_RegistryKey key{ false, key_name };
		bool init = true;

		if (key.IsValid())
		{
			key.GetValue(&ds.use_ip, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_USE_IP);
			key.GetValue(&ds.use_login, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_USE_LOGIN);
			key.GetValue(&ds.max_fail_before_ban, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_MAX_FAIL_BEFORE_BAN);
			key.GetValue(&ds.max_fail_before_delay, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_MAX_FAIL_BEFORE_DELAY);
			key.GetValue(&ds.access_deny_on_ban, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_SEND_ACCESS_DENY);
			key.GetValue(&ds.silent_on_ban, sizeof(unsigned), VS_REG_INTEGER_VT, POLICY_SILENT_ON_BAN);

			get_duration(key, POLICY_BAN_TIME, ds.ban_time);
			get_duration(key, POLICY_DELAY_TTL, ds.delay_ttl);
			get_duration(key, POLICY_DELAY_TIME, ds.delay_time);
			get_duration(key, POLICY_DELAY_INTERVAL, ds.delay_interval);
			get_duration(key, POLICY_OBSERVE_INTERVAL, ds.observe_interval);

			auto &&it = m_policy.find(data->type);
			if (it != m_policy.cend())
			{
				if ((!ds.use_ip) == it->second.rules.use_ip || (!ds.use_login) == it->second.rules.use_login)
				{
					// change using of ip and login, remove ban for all
					it->second.endpoints.clear();
				}
				it->second.rules = ds;
				init = false;
			}
		}

		if (init)
		{
			this->m_policy.emplace(data->type, PolicyEndpoint{ ds, {}, clock_t::time_point::max() });
		}
	}
}


inline bool VS_Policy::EndpointInformation::is_banned() const
{
	return ban_expire_time > clock_t::time_point();
}

inline bool VS_Policy::EndpointInformation::is_delayed() const
{
	return delay_expire_time > clock_t::time_point();
}

void VS_Policy::Request(string_view ip, string_view login,
	const std::function<void(bool result)> &cb, const unsigned int type)
{
	bool call_now(true); bool res(true);

	if (!ip.empty() && !login.empty())
	{
		std::lock_guard<decltype(m_mutex)> _{ m_mutex };
		RequestLocked(ip, login, cb, call_now, res, type);
	}

	if (call_now) cb(res);
}

bool VS_Policy::Request(string_view ip, string_view login, const unsigned int type)
{
	bool call_now(true); bool res(true);

	if (!ip.empty() && !login.empty())
	{
		std::lock_guard<decltype(m_mutex)> _{ m_mutex };
		RequestLocked(ip, login, std::function<void(bool)>(), call_now, res, type);
	}

	return res;
}

void VS_Policy::RequestLocked(string_view ip, string_view login,
	const std::function<void(bool result)> &cb, bool &callNow, bool &res, const unsigned int type)
{
	res = true;

	auto &&it = m_policy.find(type);
	if (it != m_policy.cend())
	{
		auto &obj = it->second;
		callNow = true;
		if (!obj.rules.use_ip && !obj.rules.use_login)
		{
			// invalid settings, ignore...
			return;
		}

		EndpointInformation &e = GetEndpointInformation(ip, login, obj, type);

		bool was_banned_before = e.is_banned();
		if (e.is_banned())
			res = false;

		if (res && obj.rules.use_delay() &&
			(e.fail_counter.size() >= obj.rules.max_fail_before_delay || e.is_delayed()) && cb)
		{
			if (e.delayed.size() > MAX_REQUESTS_LIMIT)e.delayed.clear(); // prevent overflow

			DelayedCall c;
			c.add_time = m_clock.now();
			c.cb = cb;
			e.delayed.push_back(c);

			if (e.last_call_delayed == clock_t::time_point()) e.last_call_delayed = c.add_time;

			if (obj.delayed_timeout > e.last_call_delayed + obj.rules.delay_interval)
				obj.delayed_timeout = e.last_call_delayed + obj.rules.delay_interval;

			callNow = false;

			if (!e.is_delayed()) // first delayed request
			{
				e.delay_expire_time = c.add_time + obj.rules.delay_time;

				dstream3 << "Policy: too many requests from endpoint " << ip << ", login = " << login << ", protocol = " << m_protocol << ", type = " << type
					<< (e.fail_counter.size() >= obj.rules.max_fail_before_delay ? ((e.fail_counter.size() >= MAX_REQUESTS_LIMIT) ? (". more than " + std::to_string(MAX_REQUESTS_LIMIT)) : ". " + std::to_string(e.fail_counter.size())) +
						" requests in " + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(obj.rules.observe_interval).count()) + " sec, limit " + std::to_string(obj.rules.max_fail_before_delay) : "") << ". "
					<< " delay for " << std::chrono::duration_cast<std::chrono::seconds>(e.delay_expire_time - m_clock.now()).count() << " sec.";

			}
		}

		// consider login attempt fails.
		ReqFail(e, obj);

		if (e.is_banned() && !e.is_ban_logged)
		{
			e.is_ban_logged = true;
			dstream3 << "Policy: too many requests from endpoint " << ip << ", login = " << login << ", protocol = " << m_protocol << ", type = " << type << ", "
				<<  std::to_string(e.fail_counter.size()) << " requests in " << std::to_string(std::chrono::duration_cast<std::chrono::seconds>(obj.rules.observe_interval).count()) + " sec (limit=" + std::to_string(obj.rules.max_fail_before_ban) << "). "
				<< " Ban expires after " << std::chrono::duration_cast<std::chrono::seconds>(e.ban_expire_time - m_clock.now()).count() << " sec (was " << ((was_banned_before) ? "" : "not ") << "banned before)";
		}
	}
#ifndef NDEBUG
	else
	{
		dstream0 << "Policy: Ignored Request by type = " << type;
	}
#endif
}

void VS_Policy::SetResult(string_view ip, string_view login, bool result, const unsigned int type)
{
	if (ip.empty() || login.empty()) return;

	std::lock_guard<decltype(m_mutex)> _{ m_mutex };
	auto &&it = m_policy.find(type);
	if (it != m_policy.cend())
	{
		if (result)
		{
			it->second.endpoints.erase(GetKeyForMap(ip, login, it->second));
		}
	}
#ifndef NDEBUG
	else
	{
		dstream0 << "Policy: Ignored SetResult by type = " << type;
	}
#endif
}

void VS_Policy::ReqFail(EndpointInformation &e, PolicyEndpoint &obj)
{
	clock_t::time_point current = m_clock.now();

	if (e.fail_counter.size() < MAX_REQUESTS_LIMIT)
		e.fail_counter.push_back(current);


	if (obj.rules.use_ban() && (e.fail_counter.size() >= obj.rules.max_fail_before_ban || e.is_banned()))
	{
		e.ban_expire_time = current + obj.rules.ban_time;
		e.is_ban_logged = false;
	}
}

std::string VS_Policy::GetKeyForMap(string_view ip, string_view login, PolicyEndpoint &obj) const
{
	if (obj.rules.use_ip && !obj.rules.use_login) return std::string(ip);
	if (!obj.rules.use_ip && obj.rules.use_login) return std::string(login);

	std::string s(ip);
	s.push_back('\1');
	s.append(login.data(), login.length());
	return s;
}


VS_Policy::EndpointInformation& VS_Policy::GetEndpointInformation(string_view ip, string_view login, PolicyEndpoint &obj, const unsigned type)
{
	EndpointInformation &e = obj.endpoints[GetKeyForMap(ip, login, obj)];

	if (e.login.empty() && e.ip.empty())
	{
		// appears to be a new object
		e.login = std::string(login);
		e.ip = std::string(ip);
		e.ban_expire_time = clock_t::time_point();
		e.last_call_delayed = clock_t::time_point();
		e.delay_expire_time = clock_t::time_point();
		e.is_ban_logged = false;
		e.fail_counter.clear();
		e.delayed.clear();

	}
	else
	{
		auto t = m_clock.now();

		if (e.is_banned())
		{
			if (e.ban_expire_time <= t)
			{
				e.fail_counter.clear();
				e.ban_expire_time = clock_t::time_point();

				if (!e.is_ban_logged)
				{
					e.is_ban_logged = true;
					dstream3 << "Policy: unbanning endpoint " << ip << ", login " << login << ", protocol " << m_protocol << ", type " << type;
				}
			}
			if (e.delay_expire_time <= t) e.delay_expire_time = clock_t::time_point{};
		}

		if(!e.fail_counter.empty())
		{
			RemoveOld(e.fail_counter, t - obj.rules.observe_interval);
		}
	}
	return e;
}


void VS_Policy::Timeout()
{
	std::unique_lock<decltype(m_mutex)> lock(m_mutex, std::try_to_lock);
	if (lock.owns_lock())
	{
		for (auto &&item : m_policy)
		{
			if (item.second.delayed_timeout < m_clock.now())
			{
				CheckDelayed(item.second);
			}

			RemoveExpired(item.second);
		}
	}
}

void VS_Policy::CheckDelayed(PolicyEndpoint &obj)
{
	std::vector<std::function< void() > > to_call;
	{
		auto current_time = m_clock.now();
		obj.delayed_timeout = clock_t::time_point::max();

		for (auto iter = obj.endpoints.begin(); iter != obj.endpoints.end(); ++iter)
		{
			EndpointInformation &e = iter->second;
			std::vector< DelayedCall > &v = e.delayed;

			std::size_t i(0), j(0);

			for (; i < v.size(); i++)
			{
				if (e.is_banned() || v[i].add_time + obj.rules.delay_ttl < current_time)
				{
					to_call.emplace_back(std::bind(v[i].cb, false));
				}
				else
					if (v[i].add_time + obj.rules.delay_interval <= current_time &&
						e.last_call_delayed + obj.rules.delay_interval <= current_time)
					{
						to_call.emplace_back(std::bind(v[i].cb, true));
						e.last_call_delayed = current_time;
					}
					else
					{
						if (obj.delayed_timeout > v[i].add_time + obj.rules.delay_interval)
							obj.delayed_timeout = v[i].add_time + obj.rules.delay_interval;

						v[j++] = v[i];
					}
			}

			v.resize(j);
		}
	}

	for(size_t i = 0; i < to_call.size(); i++)
	{
		to_call[i]();
	}
}

void VS_Policy::RemoveExpired(PolicyEndpoint &obj)
{
	for (auto i = obj.endpoints.begin(); i != obj.endpoints.cend();)
	{
		EndpointInformation &t = i->second;

		if (t.is_banned() && t.ban_expire_time < m_clock.now())
		{
			t.fail_counter.clear();
			t.ban_expire_time = clock_t::time_point();
			if (!t.is_ban_logged)
			{
				dstream3 << "Policy: unbanning endpoint " << t.ip << ", login = " << t.login << ", protocol = " << m_protocol;
				t.is_ban_logged = true;
			}
		}

		if (!t.fail_counter.empty())
		{
			RemoveOld(t.fail_counter, m_clock.now() - obj.rules.observe_interval);
		}

		if(t.fail_counter.empty() && t.delayed.empty() && !t.is_banned())
		{
			i = obj.endpoints.erase(i);
		}
		else
		{
			++i;
		}
	}
}

VS_UserLoggedin_Result VS_Policy::FailResult(const unsigned int type)
{
	std::lock_guard<decltype(m_mutex)> _{ m_mutex };
	auto &&it = m_policy.find(type);
	if (it != m_policy.cend())
	{
		const auto &obj = it->second;
		if (obj.rules.access_deny_on_ban) return  ACCESS_DENIED;
		if (obj.rules.silent_on_ban) return SILENT_REJECT_LOGIN;
	}
#ifndef NDEBUG
	else
	{
		dstream0 << "Policy: Ignored FailResult by type = " << type;
	}
#endif
	return USER_DISABLED;
}

void VS_Policy::RemoveOld(std::vector<clock_t::time_point> &v, clock_t::time_point lowerBound)
{
	std::size_t i(0), j(0);
	for (; i < v.size(); i++)
		if (v[i] > lowerBound) v[j++] = v[i];

	v.resize(j);
}


void VS_Policy::RemoveOld(std::vector< VS_Policy::DelayedCall > &v, clock_t::time_point lowerBound)
{
	std::size_t i(0), j(0);
	for (; i < v.size(); i++)
		if (v[i].add_time > lowerBound) v[j++] = v[i];

	v.resize(j);
}

bool VS_Policy::is_vcs = true;

void VS_Policy::SetIsVCS(bool x) noexcept
{
	is_vcs = x;
}

inline VS_Policy::PolicySettings VS_Policy::GetDefaultSettings() const
{
	if (is_vcs) return default_settings_vcs();
	return default_settings_as();
}

bool VS_Policy::ShouldBeChecked(VS_ClientType ct) const noexcept
{
	return	ct != CT_GATEWAY
		&& ct != CT_TRANSCODER
		&& ct != CT_TRANSCODER_CLIENT
		&& ct != CT_WEB_CLIENT
		&& ct != CT_SDK;
}

inline bool VS_Policy::PolicySettings::use_ban() const noexcept
{
	return max_fail_before_ban > 0;
}

inline bool VS_Policy::PolicySettings::use_delay() const noexcept
{
	return max_fail_before_delay > 0;
}

#undef DEBUG_CURRENT_MODULE