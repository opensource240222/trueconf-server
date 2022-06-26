#pragma once

#include "tools/H323Gateway/Lib/VS_H323String.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/compat/functional.h"
#include "std/cpplib/fast_mutex.h"
#include "std-generic/compat/map.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"

#include <chrono>
#include <vector>
#include "net/Address.h"
#include "net/Port.h"

class VS_ParserInterface;
class VS_H225RASParser;
class VS_H225RASParserInfo;

// A class, used for keep gatekeeper's information, shared between call signalling and RAS connections.
class VS_H323GatekeeperStorage
{
public:

	// Structure, that keep information about registred h323-terminal.
	struct Info final
	{
		friend class VS_H323GatekeeperStorage;
		// Parser, that used for register the terminal.
		std::weak_ptr<VS_ParserInterface> parser;
		// Parser context, that describes the registred terminal.
		std::weak_ptr<VS_H225RASParserInfo> context;
	private:
		// When this record was added to map.
		std::chrono::steady_clock::time_point start_time;
	};
	// Return single instance of this class - singleton scheme.
	static VS_H323GatekeeperStorage &Instance();

	// Register a H323 terminal on TCS without password
	enum RegisterResult
	{
		unknown = -1,
		success = 0,
		already_registered
	};
	RegisterResult RegisterH323Terminal(const std::vector<std::string> &callId, const std::vector<std::pair<net::address, net::port>> &ip, const VS_H323String& endpId);

	// Get H323 terminal IP addresses, if he is registered already
	bool GetRegisteredTerminalInfo(string_view id, std::vector<std::pair<net::address, net::port>>& ip);

	// Check if terminal is registered (need at keepAlive)
	bool IsKeepAliveOfRegisteredTerminal(const VS_H323String& endpId);

	// Remove binding of h323-ID -> cs_address
	bool UnregisterH323Terminal(const std::vector<std::pair<net::address, net::port>>& ip);

	// Save information about correspondence between call identifier and h323-terminal,
	// requested call with this call identifier. Used to save ARQ information for CS connection.
	// Note, that <encoded_call_id> must be a call_id, encoded by VS_H323Parser::EncodeDialogID().
	// Parameter <expires> show how much time do this record lives.
	void SaveTerminalInfo(string_view encodedCallId, std::shared_ptr<VS_H323GatekeeperStorage::Info> regInfo);
	// Returns information about h323-terminal, requested call with <encoded_call_id>.
	// Note, that <encoded_call_id> must be a call_id, encoded by VS_H323Parser::EncodeDialogID().
	std::shared_ptr<VS_H323GatekeeperStorage::Info> GetTerminalInfo(string_view encodedCallId);
	// Removes information about h323-terminal, requested call with <encoded_call_id>.
	// Note, that <encoded_call_id> must be a call_id, encoded by VS_H323Parser::EncodeDialogID().
	void RemoveTerminalInfo(string_view encodedCallId);
	// Register new VS_H225RasParser.
	void RegisterRASParser(VS_H225RASParser* parser);
	// Unregister removed VS_H225RASParser.
	void UnregisterRASParser(VS_H225RASParser* parser);
	// Unregister:
	// 1) VSC from all external gatekeepers.
	// 2) All H323-terminals from VCS gatekeeper.
	void UnregisterAll();

protected:
	steady_clock_wrapper &clock() const noexcept
	{
		return m_clock;
	}
private:
	typedef vs::fast_mutex mutex_t;
private:
	// Vector of registered without password H323 terminals on TCS
	mutex_t m_registered_terms_lock;
	struct RegisteredTerminal final
	{
		VS_FORWARDING_CTOR4(RegisteredTerminal, id, ips, endp_id, start_time) {}
		std::string id;					// h323-ID or dialedDigits
		std::vector<std::pair<net::address, net::port>> ips;	// list of CS addresses
		VS_H323String	endp_id;			// endpoint_identifier
		std::chrono::steady_clock::time_point start_time;
	};
	std::vector<RegisteredTerminal> m_registered_terms;	// [first=h323-ID,second=IP_Addrs]
	void CleanRegisteredTerms();

	// Map, that store correspondence between call identifier and h323-terminal,
	// requested call with this call identifier.
	mutex_t m_terminal_infos_lock;
	vs::map<std::string, std::shared_ptr<VS_H323GatekeeperStorage::Info>, vs::str_less> m_terminal_infos;
	void CleanTerminalInfos();

	// List of all VS_H225RasParser instances.
	mutex_t m_ras_parsers_lock;
	std::vector<VS_H225RASParser*> m_ras_parsers;

	// Private constructor - singleton scheme.
	VS_H323GatekeeperStorage();

	mutable steady_clock_wrapper m_clock;
};