#pragma once

#include "std-generic/cpplib/synchronized.h"
#include "../sip/VS_SIPSeqSSPI.h"

#include <boost/optional/optional.hpp>

#include <string>
#include <vector>
#include <cstdint>
#include "std-generic/cpplib/string_view.h"
#include "net/Endpoint.h"
#include "SIPParserBase/VS_Const.h"

extern const bool DEFAULT_FECC_ENABLED;
extern const bool DEFAULT_TELEPHONE_EVENT_ENABLED;

struct VS_RegCtx
{
private:
	unsigned m_cnum;
public:
	VS_SIPSeqSSPI	sspi;

	unsigned cnum() const {
		return m_cnum;
	}

	unsigned get_incr_cnum() {
		return m_cnum++;
	}

	explicit VS_RegCtx(VS_SIPSeqSSPI::eType type) : m_cnum(0), sspi(type) {}
};

struct VS_CallConfig
{
	enum eSignalingProtocol
	{
		UNDEFINED,
		SIP,
		RTSP,
		H323,
		H225RAS
	};

	enum eRegistrationBehavior
	{
		REG_DO_NOT_REGISTER = 0,
		REG_REGISTER_ALWAYS = 1,
		REG_REGISTER_ON_CALL= 2,
		REG_UNDEFINED		= 4,
	};

	enum class EscapeMethod {
		URI,
		Unicode
	};

	enum class eUserStatusScheme : uint8_t {
		ONLY_USER_AVAIL	= 0,	// for normal sip
		SKYPE4BUSINESS	= 1,	// all statuses, to make possible ReqInvite (bug#51495)
	};

	enum class eExternalAddressScheme : uint8_t {
		ONLY_INTERNET_ADDRESS = 0,
		ALWAYS_EXTERNAL = 1,
		NEVER = 2,
	};

	class ValueReaderInterface { // see VS_Identifier.cpp and VS_CallConfigCorrector.cpp for examples of usage.
	public:
		virtual ~ValueReaderInterface(){}
		virtual bool ReadBool(const char *name, bool &val) = 0;
		virtual bool ReadInteger(const char *name, int32_t &val) = 0;
		virtual bool ReadString(const char *name, std::string &val, bool canBeEmpty = false) = 0;
		virtual bool ReadProtocolSeq(const char *name, std::vector<net::protocol> &seq) = 0;
	};

	std::string			TcId;
	std::string 		HostName;
	std::string			Login;
	std::string			Password;
	boost::optional<bool> isAuthorized;

	net::Endpoint Address;


	std::vector<net::protocol> ConnectionTypeSeq;
	eSignalingProtocol	SignalingProtocol;

	bool				UseAsTel;	// for "#tel", "+" and IsVoIPServer
	bool				UseAsDefault; // when host is not specified
	bool				IsValid;
	bool				IsFromRegistry;
	std::string         TelephonePrefixReplace;
	std::string			DefaultProxyConfigurationName = "#default";
	std::string			RegistryConfigName;

	boost::optional<std::uint32_t>	       Bandwidth;
	std::string                            Codecs;
	struct
	{
		std::string                            OutboundProxy;
		
		boost::optional<bool>                  IsKeepAliveSendEnabled;
		boost::optional<eRegistrationBehavior> RegistrationBehavior;

		std::string                            FromUser;
		std::string                            FromDomain;
		std::string                            ContactDomain;

		struct
		{
			boost::optional<bool>				Enabled;
			boost::optional<int>				RefreshPeriod;
			boost::optional<bool>				AddToRequireHeader;
		} SessionTimers;

		boost::optional<bool>                  BFCPEnabled;
		boost::optional<unsigned int>          BFCPRoles;
		boost::optional<net::protocol>         DefaultBFCPProto;
		boost::optional<bool>                  CompactHeader;
		boost::optional<bool>                  UseSingleBestCodec;

		boost::optional<bool>                  NoRtpmapForAudioStaticPT;
		boost::optional<bool>                  NoRtpmapForVideoStaticPT;
		std::string                            AuthName;
		//boost::optional<bool>                  SkipOPTIONS;
		boost::optional<bool>                  RequestOPTIONS;
		boost::optional<bool>				   ICEEnabled;
		boost::optional<bool>				   SRTPEnabled;

		std::string                            AuthDomain;
		EscapeMethod						   UriEscapeMethod = EscapeMethod::URI;

		boost::optional<bool>				   TelephoneEventSignallingEnabled;
		eUserStatusScheme					   UserStatusScheme = eUserStatusScheme::ONLY_USER_AVAIL;
	} sip;

	struct
	{
	} rtsp;

	struct
	{
		std::string                            DialedDigit;
		boost::optional<bool>                  ConventionalSirenTCE;
		boost::optional<bool>                  H239Enabled;
		boost::optional<bool>                  EnableH263plus2;
		boost::optional<int8_t>                Q931_rate_multiplier;
		boost::optional<bool>				   EnabledH235;
		net::address						   ExternalAddress;
		eExternalAddressScheme				   ExternalAddressScheme = eExternalAddressScheme::ONLY_INTERNET_ADDRESS;
	} h323;

	boost::optional<bool>	H224Enabled;

	struct
	{
		std::string NewCallId;
		std::string dtmf;

	} resolveResult;

	struct
	{
		boost::optional<std::uint32_t> gconf_to_term_width;
		boost::optional<std::uint32_t> gconf_to_term_height;
		boost::optional<std::uint16_t> h264_payload_type;
		boost::optional<std::uint16_t> h264_snd_preferred_width;
		boost::optional<std::uint16_t> h264_snd_preferred_height;
		boost::optional<bool> h264_force_cif_mixer;
		boost::optional<bool> siren_swap_bytes;
	} codecParams;

	enum VerificationResult
	{
		e_Unknown = -1,
		e_Pending = 0,
		e_Valid = 1,
		e_Forbidden = 2,
		e_CanNotCheck = 3,
		e_ServerUnreachable = 4
	};

	VS_CallConfig();

	struct RegistrationIdentifier
	{
		std::string registryConfigName;
		std::string login;
		std::string name; //sip: authName; h323: DialedDigit
		net::Endpoint address;
		friend bool operator==(const RegistrationIdentifier& lhs, const RegistrationIdentifier& rhs) noexcept;
		friend bool operator!=(const RegistrationIdentifier& lhs, const RegistrationIdentifier& rhs) noexcept;
		friend bool operator<(const RegistrationIdentifier& lhs, const RegistrationIdentifier& rhs) noexcept;
		friend bool operator<=(const RegistrationIdentifier& lhs, const RegistrationIdentifier& rhs) noexcept;
		friend bool operator>(const RegistrationIdentifier& lhs, const RegistrationIdentifier& rhs) noexcept;
		friend bool operator>=(const RegistrationIdentifier& lhs, const RegistrationIdentifier& rhs) noexcept;
	};

	struct RegistrationIdentifierView
	{
		RegistrationIdentifierView(string_view aRegistryConfigName, string_view aLogin, string_view aName,
			const net::Endpoint &ep);
		RegistrationIdentifierView(const RegistrationIdentifier& regIdent);
		string_view registryConfigName;
		string_view login;
		string_view name;
	 	const net::Endpoint& address;

		friend bool operator<(const RegistrationIdentifierView& lhs, const RegistrationIdentifierView& rhs) noexcept;
		friend bool operator<=(const RegistrationIdentifierView& lhs, const RegistrationIdentifierView& rhs) noexcept;
		friend bool operator>(const RegistrationIdentifierView& lhs, const RegistrationIdentifierView& rhs) noexcept;
		friend bool operator>=(const RegistrationIdentifierView& lhs, const RegistrationIdentifierView& rhs) noexcept;
		friend bool operator==(const RegistrationIdentifierView& lhs, const RegistrationIdentifierView& rhs) noexcept;
		friend bool operator!=(const RegistrationIdentifierView& lhs, const RegistrationIdentifierView& rhs) noexcept;
		explicit operator RegistrationIdentifier() const
		{
			return RegistrationIdentifier{ std::string(registryConfigName), std::string(login), std::string(name), address };
		}
	};

private:
	VerificationResult m_lastVerificationResult;

	template<typename T>
	friend class VS_CallConfigManager;
};


template<typename T>
class VS_CallConfigManager{/*stub*/};

template<>
class VS_CallConfigManager<void>
{
public:
	VS_CallConfigManager() = default;
	VS_CallConfigManager(const VS_CallConfigManager<void>& other) = delete;
	VS_CallConfigManager(VS_CallConfigManager<void>&& other) noexcept = default;
	VS_CallConfigManager<void>& operator=(const VS_CallConfigManager<void>& other) = delete;
	VS_CallConfigManager<void>& operator=(VS_CallConfigManager<void>&& other) noexcept = delete;
	~VS_CallConfigManager() = default;

	static bool GetSIPParam(const char* name, int32_t& value, const char* subkey = nullptr);
	static bool GetSIPParam(const char* name, std::string& value, const char* subkey = nullptr);
	static bool ParseProtocolSeqString(const char *str, std::vector<net::protocol> &seq);
};

template<>
class VS_CallConfigManager<const VS_CallConfig> : public VS_CallConfigManager<void>
{
	typedef const VS_CallConfig &const_ref_t;
public:
	VS_CallConfigManager(const VS_CallConfig &config);
	bool NeedPermanentConnection() const;
	bool NeedVerification() const;
	VS_CallConfig::VerificationResult GetLastVerificationResult() const;
	bool IsValidRegistrationData() const;
	bool IsVoip(VS_CallConfig::eSignalingProtocol voipProtocol) const;
	std::string GetConfigIdentificator() const;
	std::string GetUser() const;
	bool IsRegistrationOnCall() const;
	VS_CallConfig::RegistrationIdentifier GetRegistrationIdentifier() const;
	VS_CallConfig::RegistrationIdentifierView GetRegistrationIdentifierView() const;
	bool TestEndpointsEqual(const VS_CallConfig &c2) const;
protected:
	const_ref_t Get() const;
private:
	const_ref_t callConfig_;
};

template<>
class VS_CallConfigManager<VS_CallConfig> : public VS_CallConfigManager<const VS_CallConfig>
{
public:
	VS_CallConfigManager(VS_CallConfig &config);
	void SetVerificationResult(const VS_CallConfig::VerificationResult res, const bool storeLastTime = false);
	void SetPendingVerification();
	void SetValidVerification(const bool storeLastTime = true);
	void SetCanNotCheckVerification(const bool storeLastTime = true);
	void SetForbiddenVerification(const bool storeLastTime = true);
	void ResetVerificationResults();
	void MergeWith(const VS_CallConfig &config2);
	void LoadValues(VS_CallConfig::ValueReaderInterface &reader);
private:
	typedef VS_CallConfig& ref_t;
private:
	void SetVerification(const VS_CallConfig::VerificationResult res, const bool storeLastTime);
	ref_t Get() const;
};


template<typename T>
VS_CallConfigManager<T> create_call_config_manager(T& callConfig)
{
	return VS_CallConfigManager<T>{callConfig};
}

typedef VS_CallConfigManager<void> DefaultCallManager;