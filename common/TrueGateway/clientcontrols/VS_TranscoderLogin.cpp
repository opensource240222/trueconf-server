#include "VS_TranscoderLogin.h"

#include <random>
#include <utility>
#include <functional>
#include <mutex>

#include "std/cpplib/md5.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std-generic/clib/strcasecmp.h"

VS_TranscoderLogin::VS_TranscoderLogin()
{}


VS_TranscoderLogin::~VS_TranscoderLogin()
{}


std::string VS_TranscoderLogin::GeneratePassword(const size_t len)
{
	std::string pass;
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist('!', '~'); // all printable ASCII chracters

	for (size_t i = 0; i < len; i++)
	{
		int c = dist(mt);
		pass.push_back(c);
	}

	return pass;
}

uint64_t VS_TranscoderLogin::GetSecondsCountSinceEpoch(void)
{
	auto current_time = std::chrono::system_clock::now().time_since_epoch();
	auto current_time_since_epoch = current_time.count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den;

	return current_time_since_epoch;
}

void VS_TranscoderLogin::CleanExpiredLogins(void)
{
	auto current_time = GetSecondsCountSinceEpoch();
	auto predicate = [&current_time](std::pair<const std::string, LoginData> &v) -> bool {
		return current_time > v.second.login_exceeding_time;
	};

	for (auto it = m_logins.begin(); it != m_logins.end();)
	{
		if (predicate(*it))
			m_logins.erase(it++);
		else
			++it;
	}
}

bool VS_TranscoderLogin::IsValidLogin(string_view login)
{
	CleanExpiredLogins();

	auto res = m_logins.find(login);

	return res != m_logins.end();
}

std::string VS_TranscoderLogin::GenerateTranscoderPass(string_view login, std::chrono::seconds timeoutSec)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	LoginData d;
	d.password = GeneratePassword(DEFAULT_PASS_LEN);
	d.login_exceeding_time = GetSecondsCountSinceEpoch() + timeoutSec.count();

	auto it = m_logins.find(login);
	if (it != m_logins.cend())
		it->second = d;
	else
		m_logins.emplace(login, d);

	return d.password;
}

bool VS_TranscoderLogin::Login(string_view login, string_view md5pass)
{
	if (login.empty())
		return false;

	bool res;
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);

	auto call_id = VS_NormalizeCallID(login);

	if (!IsValidLogin(call_id))
		return false;

	{
		char pass_hash[33] = { '\0' };
		auto ld = m_logins.find(call_id);
		VS_ConvertToMD5(ld->second.password, pass_hash);

		m_logins.erase(call_id);
		res = md5pass.length() == sizeof(pass_hash) - 1 && strncasecmp(md5pass.data(), pass_hash, sizeof(pass_hash) - 1) == 0;
	}
	return res;
}