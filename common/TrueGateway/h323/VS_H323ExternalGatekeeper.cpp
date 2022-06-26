#include "VS_H323ExternalGatekeeper.h"
#include "VS_H323Parser.h"
#include "VS_H225RASParser.h"

VS_H323ExternalGatekeeper& VS_H323ExternalGatekeeper::Instance()
{
	static VS_H323ExternalGatekeeper instance;
	return instance;
}

struct VS_H323ExternalGatekeeper::ConferenceIdView final
{
	string_view myName;
	string_view otherName;

	ConferenceIdView(string_view myName, string_view otherName)
		: myName(myName),
		  otherName(otherName)
	{}

	ConferenceIdView(const VS_H323ExternalGatekeeper::ConferenceId &confId)
		: myName(confId.myName), otherName(confId.otherName)
	{}

	friend bool operator<(const ConferenceIdView& lhs, const ConferenceIdView& rhs)
	{
		return std::tie(lhs.myName, lhs.otherName) < std::tie(rhs.myName, rhs.otherName);
	}

	explicit operator ConferenceId() const
	{
		return ConferenceId{ std::string(myName), std::string(otherName) };
	}
};


void VS_H323ExternalGatekeeper::SetRegistrationContext(std::weak_ptr<VS_ParserInterface> ctx)
{
	std::lock_guard<decltype(m_lock_parser)> _{ m_lock_parser };
	m_parser = ctx;
}

bool VS_H323ExternalGatekeeper::IsUseGatekeeperMode() const
{
	std::lock_guard<decltype(m_lock_parser)> _{ m_lock_parser };
	return !m_parser.expired();
}

bool VS_H323ExternalGatekeeper::ResolveOnExternalGatekeeper(string_view myName, string_view callId, net::address &addr, net::port &port)
{
	if (callId.empty())
		return false;

	if (IsUseGatekeeperMode()) {
		std::shared_ptr<VS_ParserInterface> parser;
		{
			std::lock_guard<decltype(m_lock_parser)> _{ m_lock_parser };
			parser = m_parser.lock();
		}
		if (!parser)
			return false;
		return parser->ResolveOnExternalGatekeeper(myName, callId, addr, port);
	}
	return false;
}

void VS_H323ExternalGatekeeper::Disengage(const std::string &confId, const std::string &callId, bool isIncomingCall)
{
	if(confId.empty())
		return;

	if(IsUseGatekeeperMode())
	{
		std::shared_ptr<VS_ParserInterface> parser;
		{
			std::lock_guard<decltype(m_lock_parser)> _{ m_lock_parser };
			parser = m_parser.lock();
		}
		if (parser)
			std::static_pointer_cast<VS_H225RASParser>(parser)->Disengage(confId, callId, isIncomingCall);
	}
}

bool VS_H323ExternalGatekeeper::ARQForIncomingCall(VS_CsSetupUuie* setup, std::function<bool(const bool, const bool, net::address, net::port)> updateCallState)
{
	if(IsUseGatekeeperMode())
	{
		std::shared_ptr<VS_ParserInterface> parser;
		{
			std::lock_guard<decltype(m_lock_parser)> _{ m_lock_parser };
			parser = m_parser.lock();
		}

		if(parser)
		{
			return std::static_pointer_cast<VS_H225RASParser>(parser)->ARQForIncomingCall(setup, std::move(updateCallState));
		}
	}
	else  // Allows to accept incoming h323 call when gatekeeper is not using
	{
		return updateCallState(true, false, {}, {});
	}

	return false;
}

void VS_H323ExternalGatekeeper::SetConferenceIDTest(string_view myName, string_view otherName, string_view confId)
{
	std::lock_guard<decltype(m_map_locker)> lock{ m_map_locker };

	// Try to find last confID.
	//std::map<std::pair<std::string, std::string>, char*>::iterator pos = m_conference_id_map.find(std::make_pair(std::string(my_name), std::string(other_name)));

	/*if (pos != m_conference_id_map.end())
	{
		// If conference is exists.
		pos->second;
	}
	else*/
	{
		// If this is new conference.
		//std::string  conf_id_str;
		//conf_id_str.Resize(CONFERENCEID_LENGTH + 1);
		//conf_id_str.Append(confId.data(), confId.length());
		//char* temp_buf = new char[32 + 1];
		//do
		//{
		//	/*VS_GenKeyByMD5(temp_buf);
		//	strncpy(conf_id, temp_buf, CONFERENCEID_LENGTH);
		//	conf_id[CONFERENCEID_LENGTH] = '\0';*/
		//	// UPD: use dialog_id generator from VS_H323Parser.
		//	VS_H323Parser::GenerateNewDialogID(conf_id_str);

		//} while (m_r_conference_id_map.find(conf_id_str.m_str) != m_r_conference_id_map.end());
		//delete[] temp_buf;

		//char* conf_id = new char[conf_id_str.Length() + 1];
		//strncpy_s(conf_id, conf_id_str.Length() + 1, conf_id_str.m_str, conf_id_str.Length() + 1);
		//conf_id[conf_id_str.Length()] = '\0';

		// Put confID.
		m_conference_id_map.emplace(ConferenceIdView{ myName, otherName }, confId);
		m_r_conference_id_map.emplace(confId, ConferenceIdView{ myName, otherName });
		//return conf_id;
	}
}

std::string VS_H323ExternalGatekeeper::MakeConferenceID(string_view myName, string_view otherName)
{
	std::lock_guard<decltype(m_map_locker)> lock(m_map_locker);

	// Try to find last confID.
	const auto pos = m_conference_id_map.find(ConferenceIdView{ myName, otherName });
	if(pos != m_conference_id_map.cend())
	{
		// If conference is exists.
		return pos->second;
	}


	// If this is new conference.
	char gen_conf_id[32 + 1] = { 0 };
	do
	{
		/*VS_GenKeyByMD5(temp_buf);
			strncpy(conf_id, temp_buf, CONFERENCEID_LENGTH);
			conf_id[CONFERENCEID_LENGTH] = '\0';*/
			// UPD: use dialog_id generator from VS_H323Parser.
		 VS_H323Parser::GenerateNewDialogID(gen_conf_id);
	}
	while(m_r_conference_id_map.find(string_view{ gen_conf_id, sizeof(gen_conf_id) - 1 }) != m_r_conference_id_map.cend());

	std::string conf_id(gen_conf_id, sizeof(gen_conf_id) - 1);

	// Put confID.
	m_conference_id_map.emplace(ConferenceIdView{ myName, otherName }, conf_id);
	m_r_conference_id_map.emplace(conf_id, ConferenceIdView{ myName, otherName });
	return conf_id;
}

std::string VS_H323ExternalGatekeeper::GetConferenceID(string_view myName, string_view otherName)
{
	std::lock_guard<decltype(m_map_locker)> lock{ m_map_locker };

	auto pos = m_conference_id_map.find(ConferenceIdView{ myName, otherName });
	if(pos != m_conference_id_map.cend())
		return pos->second;

	return {};
}

bool VS_H323ExternalGatekeeper::RemoveConferenceID(string_view confId)
{
	std::lock_guard<decltype(m_map_locker)> lock { m_map_locker } ;

	// pos->first  => conference_id (at std::string format)
	// pos->second => pair of call_id
	auto pos = m_r_conference_id_map.find(confId);

	if(pos == m_r_conference_id_map.cend())
		return false;

	m_conference_id_map.erase(pos->second);
	m_r_conference_id_map.erase(pos);

	return true;
}
