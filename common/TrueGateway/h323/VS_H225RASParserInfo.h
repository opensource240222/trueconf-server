#pragma once

#include "tools/H323Gateway/Lib/VS_H323String.h"
#include "../interfaces/VS_ParserInfo.h"

// A context of VS_H225RASParser.
class VS_H225RASParserInfo : public VS_ParserInfo
{
	net::address m_external_address;

public:
	// An enum, that represents states of registration.
	enum RegistrationState
	{
		// Sets before the first registration on a gatekeeper.
		RS_UNREGISTRED,
		// Sets after we reseive RegistrationReject message.
		RS_REGISTRATION_FAILED,
		// Sets after we receive RegistrationConfirm message.
		RS_REGISTRATION_SUCCESS
	};
	// Constructor.
	VS_H225RASParserInfo()
		:m_reg_state(RS_UNREGISTRED),m_last_rrq_seqnum(-1), m_cur_req_seqnum(0) { }
	// Registration state.
	void SetRegistrationState(RegistrationState rs) {
		m_reg_state = rs;
		if (rs == RS_REGISTRATION_SUCCESS) {
			create_call_config_manager(GetConfig()).SetVerificationResult(VS_CallConfig::VerificationResult::e_Valid);
		} else if (rs == RS_REGISTRATION_FAILED) {
			create_call_config_manager(GetConfig()).SetForbiddenVerification();
		} else if (rs == RS_UNREGISTRED) {
			create_call_config_manager(GetConfig()).SetVerificationResult(VS_CallConfig::e_Unknown);
		}
    }
	RegistrationState GetRegistrationState() const
	{
		return m_reg_state;
	}
	bool IsRegistred() const
	{
		return m_reg_state == RS_REGISTRATION_SUCCESS;
	}
	// Endpoint identifier.
	void SetEndpointIdentifier(const VS_H323String& epid)
	{
		m_endpoint_id = epid;
	}
	const VS_H323String &GetEndpointIdentifier() const
	{
		return m_endpoint_id;
	}
	// Gatekeeper identifier.
	void SetGatekeeperIdentifier(const VS_H323String& epid)
	{
		m_gatekeeper_id = epid;
	}
	const VS_H323String &GetGatekeeperIdentifier() const
	{
		return m_gatekeeper_id;
	}
	// CS addresses from RRQ request of the terminal.
	void ClearRequestCSAddresses()
	{
		m_rrq_cs_addresses.clear();
	}

	void AddRequestCSAddress(std::pair<net::address, net::port> csAddr)
	{
		m_rrq_cs_addresses.push_back(std::move(csAddr));
	}

	const std::vector<std::pair<net::address, net::port>> &GetRequestCSAddresses() const
	{
		return m_rrq_cs_addresses;
	}
	// CS addresses from RCF response to the terminal.
	void ClearResponseCSAddresses()
	{
		m_rcf_cs_addresses.clear();
	}

	void AddResponseCSAddress(std::pair<net::address, net::port> csAddr)
	{
		m_rcf_cs_addresses.push_back(std::move(csAddr));
	}

	const std::vector<std::pair<net::address, net::port>> &GetResponseCSAddresses() const
	{
		return m_rcf_cs_addresses;
	}
	// H323-ID
	void SetH323ID(std::string h323Id)
	{
		m_aliases.emplace_back(std::move(h323Id), VS_H225AliasAddress::e_h323_ID);
	}
	string_view GetH323ID() const
	{
		if (!m_login.empty())
			return m_login;

		auto it = std::find_if(m_aliases.cbegin(), m_aliases.cend(),
			[](const std::pair<std::string, VS_H225AliasAddress::Type>& alias)
		{
			return alias.second == VS_H225AliasAddress::e_h323_ID;
		});

		if (it != m_aliases.cend())
			return it->first;

		return {};
	}
	// Dialed Digits
	void SetDialedDigits(std::string dialedDigits )
	{
		m_aliases.emplace_back(std::move(dialedDigits), VS_H225AliasAddress::e_dialedDigits);
	}

	string_view GetDialedDigits() const
	{
		auto it = std::find_if(m_aliases.cbegin(), m_aliases.cend(),
			[](const std::pair<std::string, VS_H225AliasAddress::Type>& alias)
		{
			return alias.second == VS_H225AliasAddress::e_dialedDigits;
		});

		if (it != m_aliases.cend())
			return it->first;

		return {};
	}

	void SetLogin(std::string login)
	{
		m_login = std::move(login);
	}

	const std::string &GetLogin() const
	{
		return m_login;
	}

	// NAT
	void SetExternalAddress(const net::address& address)
	{
		m_external_address = address;
	}

	const net::address& GetExternalAddress() const
	{
		return m_external_address;
	}
	// Common alias.
	// Use VS_H225AliasAddress to fill one of the alias fields.
	void SetAlias(const VS_H225AliasAddress* alias)
	{
		switch(alias->tag)
		{
		case VS_H225AliasAddress::e_h323_ID:
			{
				SetH323ID(alias->String());
				break;
			}
		case VS_H225AliasAddress::e_dialedDigits:
			{
				SetDialedDigits(alias->String());
				break;
			}
		}
	}

	const std::vector<std::pair<std::string, VS_H225AliasAddress::Type>>& GetAliases() const
	{
		return m_aliases;
	}
	// Last RRQ sequence number
	void SetLastRRQSequenceNumber(unsigned int seqnum)
	{
		m_last_rrq_seqnum = seqnum;
	}
	unsigned int GetLastRRQSequenceNumber() const
	{
		return m_last_rrq_seqnum;
	}
	// Current request sequence number
	void SetCurrentRequestSequenceNumber(unsigned short seqnum)
	{
		m_cur_req_seqnum = seqnum;
	}
	unsigned short GetCurrentRequestSequenceNumber() const
	{
		return m_cur_req_seqnum;
	}
	void IncreaseCurrentRequestSequenceNumber()
	{
		m_cur_req_seqnum = m_cur_req_seqnum != 65535 ? m_cur_req_seqnum + 1 : 1;
	}
	// Dialog ID
	void SetDialogID(std::string dialogId)
	{
		m_dialog_id = std::move(dialogId);
	}

	const std::string &GetDialogID() const
	{
		return m_dialog_id;
	}
	// Transcoder password
	void SetTranscoderPassword(std::string pass)
	{
		m_transcoder_password = std::move(pass);
	}

	const std::string &GetTranscoderPassword() const
	{
		return m_transcoder_password;
	}
	// Last registration time.
	void SetLastRegTime(std::chrono::steady_clock::time_point time)
	{
		m_last_reg_time = time;
	}
	std::chrono::steady_clock::time_point GetLastRegTime() const
	{
		return m_last_reg_time;
	}
	// Transcoder.
	void SetTranscoder(boost::weak_ptr<VS_ClientControlInterface> transcoder)
	{
		m_transcoder = std::move(transcoder);
	}
	boost::weak_ptr<VS_ClientControlInterface> GetTranscoder() const
	{
		return m_transcoder;
	}
private:
	// Registration state.
	RegistrationState m_reg_state;
	// Endpoint identifier of context.
	VS_H323String m_endpoint_id;
	// Gatekeeper identifier of context.
	VS_H323String m_gatekeeper_id;
	// List of CS endpoints, that provides h323-terminal in RRQ reqest.
	// This is always addresses of terminal.
	std::vector<std::pair<net::address, net::port>> m_rrq_cs_addresses;
	// List of CS endpoints, that provides h323-gatekeeper for registred terminal in RCF request.
	// This can be gatekeeper addresses if gatekeeper use "gatekeeperRouted" mode.
	std::vector<std::pair<net::address, net::port>> m_rcf_cs_addresses;
	//login
	std::string m_login;
	// vector of aliases
	std::vector<std::pair<std::string , VS_H225AliasAddress::Type>> m_aliases;
	// Sequence number from last RRQ request.
	// ((unsigned int)-1) if seqnum is undefined.
	unsigned int m_last_rrq_seqnum;
	unsigned short m_cur_req_seqnum;
	// Dialog id.
	std::string m_dialog_id;
	// Password in transcoder's format.
	std::string m_transcoder_password;
	// Time, when was received last registration request.
	std::chrono::steady_clock::time_point m_last_reg_time;
	// Transcoder, that was used for h323-terminal's authorization.
	boost::weak_ptr<VS_ClientControlInterface> m_transcoder;

};
