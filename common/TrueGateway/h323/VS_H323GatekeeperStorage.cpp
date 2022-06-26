#include "VS_H323GatekeeperStorage.h"
#include "VS_H225RASParser.h"
#include <boost/algorithm/string/predicate.hpp>
#include <thread>

#include "std-generic/cpplib/ThreadUtils.h"

namespace
{
	// Max wait time for unregister from gatekeeper.
	constexpr std::chrono::seconds EXTERNAL_GATEKEEPER_URQ_WAITTIME(2);

	// A time while we store correspondence between call id and h323-terminal.
	constexpr std::chrono::seconds EXPIRE_TIME(100); // =100sec;
}


VS_H323GatekeeperStorage &VS_H323GatekeeperStorage::Instance()
{
	static VS_H323GatekeeperStorage instance{}; //singleton meyers
	return instance;
}

void VS_H323GatekeeperStorage::CleanTerminalInfos()
{
	const auto now = m_clock.now();
	for (auto it = m_terminal_infos.cbegin(); it != m_terminal_infos.cend(); /**/)
		if (now - it->second->start_time > EXPIRE_TIME)
			it = m_terminal_infos.erase(it);
		else
			++it;
}

void VS_H323GatekeeperStorage::SaveTerminalInfo(string_view encodedCallId,
		std::shared_ptr<VS_H323GatekeeperStorage::Info> regInfo)
{
	if(!regInfo)
		return;
	std::lock_guard<decltype(m_terminal_infos_lock)> lock(m_terminal_infos_lock);
	CleanTerminalInfos();
	regInfo->start_time = m_clock.now();
	m_terminal_infos.emplace(encodedCallId, std::move(regInfo));
}

std::shared_ptr<VS_H323GatekeeperStorage::Info> VS_H323GatekeeperStorage::GetTerminalInfo(string_view encodedCallId)
{
	std::lock_guard<decltype(m_terminal_infos_lock)> lock(m_terminal_infos_lock);
	CleanTerminalInfos();
	const auto pos = m_terminal_infos.find(encodedCallId);
	if(pos == m_terminal_infos.cend())
		return {};
	return pos->second;
}

void VS_H323GatekeeperStorage::RemoveTerminalInfo(string_view encodedCallId)
{
	std::lock_guard<decltype(m_terminal_infos_lock)> lock(m_terminal_infos_lock);
	CleanTerminalInfos();
	const auto pos = m_terminal_infos.find(encodedCallId);
	if(pos != m_terminal_infos.cend())
		m_terminal_infos.erase(pos);
}

void VS_H323GatekeeperStorage::RegisterRASParser(VS_H225RASParser* parser)
{
	if (!parser)
		return;

	std::lock_guard<decltype(m_ras_parsers_lock)> lock(m_ras_parsers_lock);
	m_ras_parsers.push_back(parser);
}

void VS_H323GatekeeperStorage::UnregisterRASParser(VS_H225RASParser* parser)
{
	if (!parser)
		return;
	std::lock_guard<decltype(m_ras_parsers_lock)> lock(m_ras_parsers_lock);
	m_ras_parsers.erase(std::remove(m_ras_parsers.begin(), m_ras_parsers.end(), parser), m_ras_parsers.end());
}

void VS_H323GatekeeperStorage::UnregisterAll()
{
	{
		std::lock_guard<decltype(m_ras_parsers_lock)> lock(m_ras_parsers_lock);
		for (const auto& x : m_ras_parsers)
			x->UnregisterAll();
	}
	vs::SleepFor(EXTERNAL_GATEKEEPER_URQ_WAITTIME);
}

// ==================================================================================

VS_H323GatekeeperStorage::VS_H323GatekeeperStorage()
{
}

void VS_H323GatekeeperStorage::CleanRegisteredTerms()
{
	const auto now = m_clock.now();
	for (auto it = m_registered_terms.cbegin(); it != m_registered_terms.cend(); /**/)
		if (now - it->start_time > VS_H225RASParser::DEFAULT_EXPIRES)
			it = m_registered_terms.erase(it);
		else
			++it;
}

VS_H323GatekeeperStorage::RegisterResult VS_H323GatekeeperStorage::RegisterH323Terminal(
	const std::vector<std::string>& callId, const std::vector<std::pair<net::address, net::port>>& ip,
	const VS_H323String& endpId)
{
	VS_H323GatekeeperStorage::RegisterResult res(unknown);
	std::lock_guard<decltype(m_registered_terms_lock)> lock(m_registered_terms_lock);
	CleanRegisteredTerms();
	for (const auto& id : callId)
	{
		// check if already registered
		if (std::any_of(m_registered_terms.cbegin(), m_registered_terms.cend(), [&](const RegisteredTerminal& t) {
			return t.id == id && t.ips != ip;
		}))
		{
			return already_registered;	// already registered
		}

		// not found yet, can register
		m_registered_terms.emplace_back(id, ip, endpId, m_clock.now());
		res = success;
	}
	return res;
}

bool VS_H323GatekeeperStorage::GetRegisteredTerminalInfo(string_view id, std::vector<std::pair<net::address, net::port>>& ip)
{
	std::lock_guard<decltype(m_registered_terms_lock)> lock(m_registered_terms_lock);
	CleanRegisteredTerms();
	for (auto it = m_registered_terms.crbegin(); it != m_registered_terms.crend(); ++it)
		if (boost::iequals(it->id, id))
		{
			ip = it->ips;
			return true;
		}
	return false;
}

bool VS_H323GatekeeperStorage::IsKeepAliveOfRegisteredTerminal(const VS_H323String &endpId)
{
	bool res(false);
	std::lock_guard<decltype(m_registered_terms_lock)> lock(m_registered_terms_lock);
	CleanRegisteredTerms();
	for (auto& t : m_registered_terms)
	{
		if (t.endp_id == endpId)
		{
			t.start_time = m_clock.now();
			res = true;
		}
	}
	return res;
}

bool VS_H323GatekeeperStorage::UnregisterH323Terminal(const std::vector<std::pair<net::address, net::port>>& ip)
{
	bool res(false);
	std::lock_guard<decltype(m_registered_terms_lock)> lock(m_registered_terms_lock);
	CleanRegisteredTerms();
	for (auto const& a : ip)
		for (auto&& t : m_registered_terms)
		{
			t.ips.erase(std::remove(t.ips.begin(), t.ips.end(), a), t.ips.end());
			res = true;
		}

	m_registered_terms.erase(std::remove_if(m_registered_terms.begin(), m_registered_terms.end(), [](const RegisteredTerminal& t) {
		return t.ips.empty();
	}), m_registered_terms.end());

	return res;
}
