#pragma once

#include <boost/weak_ptr.hpp>

#include <string>
#include "std-generic/compat/map.h"
#include "std-generic/compat/functional.h"
#include "std/cpplib/fast_mutex.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/string_view.h"
#include "net/Address.h"
#include "net/Port.h"

class VS_ParserInterface;
struct VS_CsSetupUuie;

class VS_H323ExternalGatekeeper
{
	typedef vs::fast_recursive_mutex mutex_t;

	// Parser, that serves H225 RAS channel.
	std::weak_ptr<VS_ParserInterface>	m_parser;
	mutable mutex_t m_lock_parser;
	// Map, that contains id of all conferences in format:
	// <pair<my_name, other_name>, conference_id>
	// my_name - caller's name
	// other_name - whom do we want to call
	// endpoint_id - 16-character's array, id of the conference.

	struct ConferenceId final
	{
		std::string myName;
		std::string otherName;

		friend bool operator<(const ConferenceId& lhs, const ConferenceId& rhs)
		{
			return std::tie(lhs.myName, lhs.otherName) < std::tie(rhs.myName, rhs.otherName);
		}
	};

	struct ConferenceIdView;

	vs::map<ConferenceId, std::string, vs::less<>> m_conference_id_map;
	// Reverse conference map.
	// For resolving conference_id->pair<my_name, other_name>
	vs::map<std::string, ConferenceId, vs::str_less> m_r_conference_id_map;
	// Map locker.
	mutex_t m_map_locker;

public:
	static VS_H323ExternalGatekeeper& Instance();

	void SetRegistrationContext(std::weak_ptr<VS_ParserInterface> ctx);

	bool IsUseGatekeeperMode() const;

	// Call from <my_name> to <call_id>
	// Arguments' format:
	// <my_name>: user@server.com
	// <call_id>: #h323:user
	// Warning: format #h323:user@server does not supports, will return false.
	bool ResolveOnExternalGatekeeper(string_view myName, string_view callId, net::address &addr, net::port &port);

	// Send DRQ to the gatekeeper. Must be invoked at the end of the call.
	void Disengage(const std::string &confId, const std::string &callId, bool isIncomingCall);

	// Send ARQ for incoming call, wait for ACF or ARJ and returns result:
	// true - if we can answer the call;
	// false - otherwise.
	// Use incoming SETUP requect to construct ARQ.
	bool ARQForIncomingCall(VS_CsSetupUuie* setup, std::function<bool(const bool, const bool, net::address, net::port)>);

	// Creates 16-character array - conference id and save it for next usage.
	// Warning: If this pair of my_name+other_name was used earliy, this method will return
	// previous saved value for this pair.
	// If you want to generate new confid, you must use RemoveConferenceID() first.
	// This method can be used instead of GetConferenceID but ONLY if your are shure,
	// that conferenceID will be removed after end of call using RemoveConferenceID.
	// <my_name> want to call <other_name>
	std::string MakeConferenceID(string_view myName, string_view otherName);

	// Returns saved 16-character array - conference id.
	// If confID for this conference wasn't saved, return 0.
	// <my_name> want to call <other_name>
	std::string GetConferenceID(string_view myName, string_view otherName);

	// Remove conference id from map.
	// If confID for this conference wasn't saved, do nothing and returns false.
	// WARNING: conf_id must have '\0' at the end.
	bool RemoveConferenceID(string_view confId);
// only for test. Don't use it!
	void SetConferenceIDTest(string_view myName, string_view otherName, string_view confId);
};