#include "VS_CallConfig.h"

#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"

#include <boost/algorithm/string/predicate.hpp>

#include <algorithm>
#include <cctype>
#include "std-generic/clib/strcasecmp.h"
#include "std-generic/cpplib/ignore.h"

const bool DEFAULT_FECC_ENABLED = true;
const bool DEFAULT_TELEPHONE_EVENT_ENABLED = true;

VS_CallConfig::VS_CallConfig()
	: Address{ net::address{}, 0, net::protocol::any }
	, SignalingProtocol(VS_CallConfig::UNDEFINED), UseAsTel(false)
	, UseAsDefault(false), IsValid(true), IsFromRegistry(true)
	, Codecs(DEFAULT_ENABLED_CODECS), m_lastVerificationResult(e_Unknown)
{}

VS_CallConfig::RegistrationIdentifierView::RegistrationIdentifierView(string_view aRegistryConfigName,
	string_view aLogin, string_view aName, const net::Endpoint &ep)
	: registryConfigName(aRegistryConfigName)
	, login(aLogin)
	, name(aName)
	, address(ep)
{}

VS_CallConfig::RegistrationIdentifierView::RegistrationIdentifierView(const RegistrationIdentifier& regIdent)
	: registryConfigName(regIdent.registryConfigName)
	, login(regIdent.login)
	, name(regIdent.name)
	, address(regIdent.address)
{}

bool VS_CallConfigManager<void>::GetSIPParam(const char* name, int32_t& value, const char* subkey)
{
	if (!name || !*name)
	{
		return false;
	}
	std::string key_name = SIP_PEERS_KEY;
	if (subkey)
	{
		key_name += '\\';
		key_name += subkey;
	}
	VS_RegistryKey key{ false, key_name };
	if (!key.IsValid())
	{
		return false;
	}
	return key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, name) > 0;
}

bool VS_CallConfigManager<void>::GetSIPParam(const char* name, std::string& value, const char* subkey)
{
	if (!name || !*name)
	{
		return false;
	}
	std::string key_name = SIP_PEERS_KEY;
	if (subkey)
	{
		key_name += '\\';
		key_name += subkey;
	}
	VS_RegistryKey key{ false, key_name };
	if (!key.IsValid())
	{
		return false;
	}
	return key.GetString(value, name);
}

bool VS_CallConfigManager<void>::ParseProtocolSeqString(const char* str, std::vector<net::protocol>& seq)
{
	if (str == nullptr || *str == '\0')
		return false;

	const size_t len = strlen(str);
	seq.clear();

	// split into tokens
	{
		std::string token;
		bool inside_token;

		for (size_t i = 0; i <= len; i++)
		{
			if (isalpha(str[i]))
			{
				token.push_back(str[i]);
				inside_token = true;
			}
			else
			{
				inside_token = false;
			}

			if (!inside_token && !token.empty())
			{
				if (strcasecmp(&token[0], "TCP") == 0 && std::find(seq.begin(), seq.end(), net::protocol::TCP) == seq.end())
				{
					seq.push_back(net::protocol::TCP);
				}
				else if (strcasecmp(&token[0], "UDP") == 0 && std::find(seq.begin(), seq.end(), net::protocol::UDP) == seq.end())
				{
					seq.push_back(net::protocol::UDP);
				}
				else if (strcasecmp(&token[0], "TLS") == 0 && std::find(seq.begin(), seq.end(), net::protocol::TLS) == seq.end())
				{
					seq.push_back(net::protocol::TLS);
				}
				else if (strcasecmp(&token[0], "BOTH") == 0 && std::find(seq.begin(), seq.end(), net::protocol::any) == seq.end())
				{
					seq.push_back(net::protocol::any);
				}
				else
				{
					token.clear();
					continue;
				}

				token.clear();
			}
		}
	}
	return !seq.empty();
}

VS_CallConfigManager<const VS_CallConfig>::VS_CallConfigManager(const VS_CallConfig& config)
	: callConfig_(config)
{
}

bool VS_CallConfigManager<const VS_CallConfig>::NeedPermanentConnection() const
{
	auto &call_config = Get();
	return call_config.IsValid && (call_config.sip.RegistrationBehavior.get_value_or(VS_CallConfig::REG_UNDEFINED)
		== VS_CallConfig::REG_REGISTER_ALWAYS || NeedVerification());
}

bool VS_CallConfigManager<const VS_CallConfig>::NeedVerification() const
{
	auto &call_config = Get();
	if (call_config.sip.RegistrationBehavior.get_value_or(VS_CallConfig::REG_UNDEFINED) == VS_CallConfig::REG_DO_NOT_REGISTER
		|| call_config.sip.RegistrationBehavior.get_value_or(VS_CallConfig::REG_UNDEFINED) == VS_CallConfig::REG_UNDEFINED)
	{
		return false;
	}
	if (!call_config.IsFromRegistry || !(call_config.SignalingProtocol == VS_CallConfig::SIP ||
		call_config.SignalingProtocol == VS_CallConfig::H225RAS))
	{
		return false;
	}

	return true;
}

VS_CallConfig::VerificationResult VS_CallConfigManager<const VS_CallConfig>::GetLastVerificationResult() const
{
	return Get().m_lastVerificationResult;
}

bool VS_CallConfigManager<const VS_CallConfig>::IsValidRegistrationData() const
{
	auto &self = Get();
	return !self.Login.empty() && !self.HostName.empty();
}

bool VS_CallConfigManager<const VS_CallConfig>::IsVoip(VS_CallConfig::eSignalingProtocol voipProtocol) const
{
	auto &call_config = Get();
	switch (call_config.SignalingProtocol) {
	case VS_CallConfig::SIP:
		return voipProtocol == VS_CallConfig::SIP;
	case VS_CallConfig::H323:
	case VS_CallConfig::H225RAS:
		return voipProtocol == VS_CallConfig::H323;
	default:
		return false;
	}
}

std::string VS_CallConfigManager<const VS_CallConfig>::GetConfigIdentificator() const
{
	auto &self = Get();
	return self.Login.empty() ? "@" + self.HostName : self.Login;
}

std::string VS_CallConfigManager<const VS_CallConfig>::GetUser() const
{
	auto &self = Get();
	return self.Login + "@" + self.HostName;
}

bool VS_CallConfigManager<const VS_CallConfig>::IsRegistrationOnCall() const
{
	return Get().sip.RegistrationBehavior.get_value_or(VS_CallConfig::REG_UNDEFINED) == VS_CallConfig::REG_REGISTER_ON_CALL;
}

VS_CallConfig::RegistrationIdentifier VS_CallConfigManager<const VS_CallConfig>::GetRegistrationIdentifier() const
{
	return VS_CallConfig::RegistrationIdentifier( GetRegistrationIdentifierView() );
}

VS_CallConfig::RegistrationIdentifierView VS_CallConfigManager<const VS_CallConfig>::GetRegistrationIdentifierView() const
{
	auto &&self = Get();

	string_view name;
	if (self.SignalingProtocol == VS_CallConfig::SIP)
	{
		name = self.sip.AuthName;
	}
	else if (self.SignalingProtocol == VS_CallConfig::H225RAS || self.SignalingProtocol == VS_CallConfig::H323)
	{
		name = self.h323.DialedDigit;
	}

	return VS_CallConfig::RegistrationIdentifierView{ self.RegistryConfigName, self.Login, name,
		self.Address
	};
}

bool IsSubHost(const std::string& host1, const std::string& host2) {
	return !host2.empty() && boost::iends_with(host1, host2);
}

bool VS_CallConfigManager<const VS_CallConfig>::TestEndpointsEqual(const VS_CallConfig &c2) const
{
	auto &&c1 = Get();
	const bool sameSignalingProtocol = c1.SignalingProtocol == c2.SignalingProtocol ||
		(c1.SignalingProtocol == VS_CallConfig::H225RAS && c2.SignalingProtocol == VS_CallConfig::H323) ||
		(c1.SignalingProtocol == VS_CallConfig::H323 && c2.SignalingProtocol == VS_CallConfig::H225RAS);

	if (!sameSignalingProtocol) return false;

	const bool isSubHost = IsSubHost(c1.HostName, c2.HostName);

	const bool ip_present = !c1.Address.addr.is_unspecified() && !c2.Address.addr.is_unspecified();

	bool the_same_address = false;

	if (c1.Address.addr.is_v4() == c2.Address.addr.is_v4() && ip_present)
	{
		the_same_address = c1.Address.addr == c2.Address.addr;
	}

	const bool host_present = !c1.HostName.empty() && !c2.HostName.empty();

	if(host_present && !ip_present)
	{
		return isSubHost;
	}

	if (ip_present && !host_present)
	{
		return the_same_address;
	}

	if(host_present && ip_present)
	{
		return isSubHost || the_same_address;
	}

	return (!host_present && !ip_present) && ((c1.UseAsDefault && c2.UseAsDefault) || (c1.UseAsTel && c2.UseAsTel));
}

void VS_CallConfigManager<VS_CallConfig>::SetPendingVerification()
{
	SetVerification(VS_CallConfig::VerificationResult::e_Pending, false);
}

void VS_CallConfigManager<VS_CallConfig>::SetValidVerification(const bool storeLastTime)
{
	SetVerification(VS_CallConfig::VerificationResult::e_Valid, storeLastTime);
}

void VS_CallConfigManager<VS_CallConfig>::SetCanNotCheckVerification(const bool storeLastTime)
{
	SetVerification(VS_CallConfig::VerificationResult::e_CanNotCheck, storeLastTime);
}

void VS_CallConfigManager<VS_CallConfig>::SetForbiddenVerification(const bool storeLastTime)
{
	SetVerification(VS_CallConfig::VerificationResult::e_Forbidden, storeLastTime);
}

void VS_CallConfigManager<VS_CallConfig>::ResetVerificationResults()
{
	SetVerification(VS_CallConfig::e_Unknown, false);
}

inline VS_CallConfigManager<const VS_CallConfig>::const_ref_t& VS_CallConfigManager<const VS_CallConfig>::Get() const
{
	return callConfig_;
}

VS_CallConfigManager<VS_CallConfig>::VS_CallConfigManager(VS_CallConfig &config) :
	VS_CallConfigManager<const VS_CallConfig>(config)
{
}

void VS_CallConfigManager<VS_CallConfig>::SetVerificationResult(const VS_CallConfig::VerificationResult res, const bool storeLastTime)
{
	SetVerification(res, storeLastTime);
}

void VS_CallConfigManager<VS_CallConfig>::MergeWith(const VS_CallConfig& config2)
{
	auto &self = Get();
	const bool sameSignalingProtocol = self.SignalingProtocol == config2.SignalingProtocol ||
		(self.SignalingProtocol == VS_CallConfig::H225RAS && config2.SignalingProtocol == VS_CallConfig::H323) ||
		(self.SignalingProtocol == VS_CallConfig::SIP && config2.SignalingProtocol == VS_CallConfig::SIP) ||
		(self.SignalingProtocol == VS_CallConfig::H323 && config2.SignalingProtocol == VS_CallConfig::H225RAS);

	assert(sameSignalingProtocol
		|| ((self.SignalingProtocol == VS_CallConfig::H225RAS ||
			self.SignalingProtocol == VS_CallConfig::SIP ||
			self.SignalingProtocol == VS_CallConfig::H323) && config2.SignalingProtocol == VS_CallConfig::UNDEFINED));

	// Common
	if (!config2.Address.addr.is_unspecified())
	{
		self.Address.addr = config2.Address.addr;
	}

	if (config2.Address.port != 0 && self.SignalingProtocol == config2.SignalingProtocol)
	{
		self.Address.port = config2.Address.port;
	}

	bool merge_types = false;
	if (config2.Address.protocol != net::protocol::none && config2.Address.protocol != net::protocol::any)
	{
		// sanity checks
		if (self.SignalingProtocol == VS_CallConfig::H323)
		{
			self.Address.protocol = net::protocol::TCP;
		}
		else if (self.SignalingProtocol == VS_CallConfig::H225RAS)
		{
			self.Address.protocol = net::protocol::UDP;
		}
		else if (self.SignalingProtocol == VS_CallConfig::RTSP && (config2.Address.protocol != net::protocol::none
			&& config2.Address.protocol != net::protocol::UDP))
		{
			merge_types = true;
		}
		else if (self.SignalingProtocol == VS_CallConfig::SIP)
		{
			merge_types = true;
		}
		else if (self.SignalingProtocol == VS_CallConfig::UNDEFINED) // for unit tests
		{
			merge_types = true;
		}
	}
	else if(self.Address.protocol == net::protocol::none && config2.Address.protocol != net::protocol::none)
	{
		merge_types = true;
	}

	if (merge_types)
	{
		self.Address.protocol = config2.Address.protocol;
	}

	if (!config2.ConnectionTypeSeq.empty())
	{
		self.ConnectionTypeSeq.assign(config2.ConnectionTypeSeq.begin(), config2.ConnectionTypeSeq.end());
	}

	if (!config2.TcId.empty())
	{
		self.TcId = config2.TcId;
	}

	if (!config2.HostName.empty())
	{
		self.HostName = config2.HostName;
	}

	self.UseAsDefault |= config2.UseAsDefault;
	self.UseAsTel |= config2.UseAsTel;

	if (self.Login.empty() || self.Login == config2.Login)
	{
		self.Login = config2.Login;
		if (self.Password.empty()) self.Password = config2.Password;

		if (config2.sip.RegistrationBehavior.is_initialized() &&
			config2.sip.RegistrationBehavior.get() != VS_CallConfig::REG_UNDEFINED)
		{
			self.sip.RegistrationBehavior = config2.sip.RegistrationBehavior;
		}
	}

	if (config2.Bandwidth.is_initialized())
	{
		self.Bandwidth = config2.Bandwidth;
	}

	if (!config2.Codecs.empty() && config2.Codecs != DEFAULT_ENABLED_CODECS)
	{
		self.Codecs = config2.Codecs;
	}

	if (!config2.TelephonePrefixReplace.empty())
	{
		self.TelephonePrefixReplace = config2.TelephonePrefixReplace;
	}

	if (!config2.RegistryConfigName.empty())
	{
		self.RegistryConfigName = config2.RegistryConfigName;
	}

	// SIP
	if (config2.sip.IsKeepAliveSendEnabled.is_initialized())
	{
		self.sip.IsKeepAliveSendEnabled = config2.sip.IsKeepAliveSendEnabled;
	}

	if (!config2.sip.FromUser.empty())
	{
		self.sip.FromUser = config2.sip.FromUser;
	}

	if (!config2.sip.FromDomain.empty())
	{
		self.sip.FromDomain = config2.sip.FromDomain;
	}

	if (!config2.sip.ContactDomain.empty())
	{
		self.sip.ContactDomain = config2.sip.ContactDomain;
	}

	if (config2.sip.SessionTimers.Enabled.is_initialized())
	{
		self.sip.SessionTimers.Enabled = config2.sip.SessionTimers.Enabled;
	}

	if (config2.sip.SessionTimers.RefreshPeriod.is_initialized())
	{
		self.sip.SessionTimers.RefreshPeriod = config2.sip.SessionTimers.RefreshPeriod;
	}

	if (config2.sip.SessionTimers.AddToRequireHeader.is_initialized())
	{
		self.sip.SessionTimers.AddToRequireHeader = config2.sip.SessionTimers.AddToRequireHeader;
	}

	if (config2.sip.BFCPEnabled.is_initialized())
	{
		self.sip.BFCPEnabled = config2.sip.BFCPEnabled;
	}

	if (config2.sip.BFCPRoles.is_initialized() && config2.sip.BFCPRoles.get() != SDP_FLOORCTRL_ROLE_INVALID)
	{
		if (!self.sip.BFCPRoles.is_initialized() || self.sip.BFCPRoles.get() == SDP_FLOORCTRL_ROLE_INVALID)
		{
			self.sip.BFCPRoles = config2.sip.BFCPRoles;
		}
	}

	if (config2.sip.DefaultBFCPProto.is_initialized() && config2.sip.DefaultBFCPProto.get() != net::protocol::none)
	{
		self.sip.DefaultBFCPProto = config2.sip.DefaultBFCPProto;
	}

	if (config2.sip.CompactHeader.is_initialized())
	{
		self.sip.CompactHeader = config2.sip.CompactHeader;
	}

	if (config2.sip.UseSingleBestCodec.is_initialized())
	{
		self.sip.UseSingleBestCodec = config2.sip.UseSingleBestCodec;
	}

	if (config2.sip.NoRtpmapForAudioStaticPT.is_initialized())
	{
		self.sip.NoRtpmapForAudioStaticPT = config2.sip.NoRtpmapForAudioStaticPT;
	}

	if (config2.sip.NoRtpmapForVideoStaticPT.is_initialized())
	{
		self.sip.NoRtpmapForVideoStaticPT = config2.sip.NoRtpmapForVideoStaticPT;
	}

	if (config2.sip.ICEEnabled.is_initialized())
	{
		self.sip.ICEEnabled = config2.sip.ICEEnabled;
	}

	if (config2.sip.SRTPEnabled.is_initialized())
	{
		self.sip.SRTPEnabled = config2.sip.SRTPEnabled;
	}

	if (!config2.sip.AuthName.empty())
	{
		self.sip.AuthName = config2.sip.AuthName;
	}

	if (!config2.sip.AuthDomain.empty())
	{
		self.sip.AuthDomain = config2.sip.AuthDomain;
	}

	if (config2.sip.RequestOPTIONS.is_initialized())
	{
		self.sip.RequestOPTIONS = config2.sip.RequestOPTIONS;
	}

	if (config2.sip.TelephoneEventSignallingEnabled.is_initialized())
	{
		self.sip.TelephoneEventSignallingEnabled = config2.sip.TelephoneEventSignallingEnabled;
	}

	if (!config2.sip.OutboundProxy.empty())
	{
		self.sip.OutboundProxy = config2.sip.OutboundProxy;
	}

	self.sip.UriEscapeMethod = config2.sip.UriEscapeMethod;

	self.sip.UserStatusScheme = config2.sip.UserStatusScheme;


	// H323...
	if (!config2.h323.DialedDigit.empty())
	{
		self.h323.DialedDigit = config2.h323.DialedDigit;
	}
	if (!config2.h323.ExternalAddress.is_unspecified())
	{
		self.h323.ExternalAddress = config2.h323.ExternalAddress;
	}
	self.h323.ExternalAddressScheme = config2.h323.ExternalAddressScheme;

	if (config2.h323.ConventionalSirenTCE.is_initialized())
	{
		self.h323.ConventionalSirenTCE = config2.h323.ConventionalSirenTCE;
	}

	if (config2.h323.H239Enabled.is_initialized())
	{
		self.h323.H239Enabled = config2.h323.H239Enabled;
	}

	if (config2.h323.EnableH263plus2.is_initialized())
	{
		self.h323.EnableH263plus2 = config2.h323.EnableH263plus2;
	}

	if (config2.h323.Q931_rate_multiplier.is_initialized())
	{
		self.h323.Q931_rate_multiplier = config2.h323.Q931_rate_multiplier;
	}

	if (config2.h323.EnabledH235.is_initialized())
	{
		self.h323.EnabledH235 = config2.h323.EnabledH235;
	}

	if (config2.H224Enabled.is_initialized())
	{
		self.H224Enabled = config2.H224Enabled;
	}

	// resolveResult
	if (!config2.resolveResult.NewCallId.empty())
	{
		self.resolveResult.NewCallId = config2.resolveResult.NewCallId;
	}

	if (!config2.resolveResult.NewCallId.empty())
	{
		self.resolveResult.NewCallId = config2.resolveResult.NewCallId;
	}

	// custom codec params
	if (config2.codecParams.h264_payload_type.is_initialized())
		self.codecParams.h264_payload_type = config2.codecParams.h264_payload_type;
	if (config2.codecParams.h264_force_cif_mixer.is_initialized())
		self.codecParams.h264_force_cif_mixer = config2.codecParams.h264_force_cif_mixer;
	if (config2.codecParams.h264_snd_preferred_width.is_initialized())
		self.codecParams.h264_snd_preferred_width = config2.codecParams.h264_snd_preferred_width;
	if (config2.codecParams.h264_snd_preferred_height.is_initialized())
		self.codecParams.h264_snd_preferred_height = config2.codecParams.h264_snd_preferred_height;
	if (config2.codecParams.gconf_to_term_width.is_initialized())
		self.codecParams.gconf_to_term_width = config2.codecParams.gconf_to_term_width;
	if (config2.codecParams.gconf_to_term_height.is_initialized())
		self.codecParams.gconf_to_term_height = config2.codecParams.gconf_to_term_height;
	if (config2.codecParams.siren_swap_bytes.is_initialized())
		self.codecParams.siren_swap_bytes = config2.codecParams.siren_swap_bytes;
	if (config2.isAuthorized.is_initialized()) self.isAuthorized = config2.isAuthorized;

	//	// When adding new value please consider adding it also to the CheckValue() function in "VS_CallConfig_tests.cpp".
	//	// It is needed for proper unit testing of VS_CallConfig.
}

void VS_CallConfigManager<VS_CallConfig>::LoadValues(VS_CallConfig::ValueReaderInterface& reader)
{
	auto &self = Get();
	bool val_bool = false;
	int32_t val_int = 0;
	std::string val_str;
	std::vector<net::protocol> val_conntype;

	if (reader.ReadString("Hostname", val_str))
	{
		self.HostName = val_str;
	}

	if (reader.ReadString("Login", val_str))
	{
		self.Login = val_str;
	}

	if (reader.ReadString("Password", val_str))
	{
		self.Password = val_str;
	}

	if (reader.ReadString("TelephonePrefixReplace", val_str))
	{
		self.TelephonePrefixReplace = val_str;
	}

	if (reader.ReadString("fromuser", val_str))
	{
		self.sip.FromUser = val_str;
	}

	size_t n1 = self.Login.find('@');
	if (n1 != std::string::npos)
	{
		self.sip.FromDomain = self.Login.substr(n1 + 1, std::string::npos);
		self.Login.erase(n1, std::string::npos);
	}

	if (reader.ReadString("fromdomain", val_str))
	{
		self.sip.FromDomain = val_str;
	}

	if (reader.ReadProtocolSeq("Protocol", val_conntype))
	{
		self.Address.protocol = val_conntype[0];

		self.ConnectionTypeSeq.assign(val_conntype.begin(), val_conntype.end());
	}

	if (reader.ReadString("DialedDigit", val_str))
	{
		self.h323.DialedDigit = val_str;
	}

	if (reader.ReadString("Enabled Codecs", val_str))
	{
		self.Codecs = val_str;
	}

	if (reader.ReadProtocolSeq("BFCP Protocol", val_conntype))
	{
		self.sip.DefaultBFCPProto = val_conntype[0];
	}

	if (reader.ReadString("External Address", val_str))
	{
		self.h323.ExternalAddress = net::address::from_string(val_str, vs::ignore<boost::system::error_code>());
	}
	if (reader.ReadInteger("External Address Scheme", val_int))
	{
		self.h323.ExternalAddressScheme = static_cast<VS_CallConfig::eExternalAddressScheme>(val_int);
	}

	if (reader.ReadBool("Conventional Siren TCE", val_bool))
	{
		self.h323.ConventionalSirenTCE = val_bool;
	}

	if (reader.ReadBool("Enable H263plus2", val_bool))
	{
		self.h323.EnableH263plus2 = val_bool;
	}

	if (reader.ReadBool("H235 Enabled", val_bool))
	{
		self.h323.EnabledH235 = val_bool;
	}

	if (reader.ReadInteger("Port", val_int))
	{
		self.Address.port = static_cast<net::port>(val_int);
	}

	if (reader.ReadInteger("RegisterStrategy", val_int))
	{
		if (val_int >= VS_CallConfig::REG_DO_NOT_REGISTER && val_int <= VS_CallConfig::REG_UNDEFINED)
		{
			self.sip.RegistrationBehavior = static_cast<VS_CallConfig::eRegistrationBehavior>(val_int);
		}
	}

	if (reader.ReadBool("IsVoIPServer", val_bool))
	{
		self.UseAsTel = val_bool;
	}

	if (reader.ReadBool("SendKeepAlive", val_bool))
	{
		self.sip.IsKeepAliveSendEnabled = val_bool;
	}

	if (reader.ReadBool("BFCP Enabled", val_bool))
	{
		self.sip.BFCPEnabled = val_bool;
	}

	if (reader.ReadInteger("BFCP Roles", val_int))
	{
		self.sip.BFCPRoles = val_int & (SDP_FLOORCTRL_ROLE_C_ONLY | SDP_FLOORCTRL_ROLE_S_ONLY | SDP_FLOORCTRL_ROLE_C_S);
	}

	if (reader.ReadInteger("Gateway Bandwidth", val_int))
	{
		self.Bandwidth = val_int;
	}

	if (reader.ReadBool("Session Timers Enabled", val_bool)) {
		self.sip.SessionTimers.Enabled = val_bool;
	}
	if (reader.ReadBool("Session Timers Add To Require Header", val_bool)) {
		self.sip.SessionTimers.AddToRequireHeader = val_bool;
	}
	if (reader.ReadInteger("Session Timers Refresh Period", val_int)) {
		self.sip.SessionTimers.RefreshPeriod = val_int;
	}

	if (reader.ReadBool("CompactHeader", val_bool))
	{
		self.sip.CompactHeader = val_bool;
	}

	if (reader.ReadBool("UseSingleBestCodec", val_bool))
	{
		self.sip.UseSingleBestCodec = val_bool;
	}

	if (reader.ReadBool("NoRtpmapForAudioStaticPayload", val_bool))
	{
		self.sip.NoRtpmapForAudioStaticPT = val_bool;
	}

	if (reader.ReadBool("NoRtpmapForVideoStaticPayload", val_bool)) {
		self.sip.NoRtpmapForVideoStaticPT = val_bool;
	}
	if (reader.ReadInteger("UriEscapeMethod", val_int)) {
		self.sip.UriEscapeMethod = static_cast<VS_CallConfig::EscapeMethod>(val_int);
	}

	if (reader.ReadBool("Enable telephone-event signaling in SDP", val_bool)) {
		self.sip.TelephoneEventSignallingEnabled = val_bool;
	}

	if (reader.ReadBool("H239 Enabled", val_bool)) {
		self.h323.H239Enabled = val_bool;
	}

	if (reader.ReadInteger("Q931 Rate Multiplier", val_int)) {
		self.h323.Q931_rate_multiplier = static_cast<uint8_t>(val_int);
	}

	if (reader.ReadBool("H224 Enabled", val_bool)) {
		self.H224Enabled = val_bool;
	}

	if (reader.ReadInteger("H264 Payload Type", val_int)) {
		self.codecParams.h264_payload_type = static_cast<uint16_t>(val_int);
	}

	if (reader.ReadInteger("H264 To Terminal Video Width", val_int)) {
		self.codecParams.h264_snd_preferred_width = static_cast<uint16_t>(val_int);
	}

	if (reader.ReadInteger("H264 To Terminal Video Height", val_int)) {
		self.codecParams.h264_snd_preferred_height = static_cast<uint16_t>(val_int);
	}

	if (reader.ReadBool("H264 Mixer Mode CIF", val_bool)) {
		self.codecParams.h264_force_cif_mixer = val_bool;
	}

	if (reader.ReadBool("Siren Swap Bytes", val_bool)) {
		self.codecParams.siren_swap_bytes = val_bool;
	}

	if (reader.ReadString("Authorization Name", val_str)) {
		self.sip.AuthName = val_str;
	}

	if (reader.ReadInteger("GConf To Term Video Height", val_int)) {
		self.codecParams.gconf_to_term_height = val_int;
	}

	if (reader.ReadInteger("GConf To Term Video Width", val_int)) {
		self.codecParams.gconf_to_term_width = val_int;
	}

	if (reader.ReadString("contactdomain", val_str)) {
		self.sip.ContactDomain = val_str;
	}

	if (reader.ReadBool("Request OPTIONS", val_bool)) {
		self.sip.RequestOPTIONS = val_bool;
	}

	if (reader.ReadBool("ICE Enabled", val_bool)) {
		self.sip.ICEEnabled = val_bool;
	}

	if (reader.ReadBool("SRTP Enabled", val_bool)) {
		self.sip.SRTPEnabled = val_bool;
	}

	if (reader.ReadString("Authorization domain", val_str)) {
		self.sip.AuthDomain = val_str;
	}

	if (reader.ReadString("Default Proxy", val_str, true)) {
		self.DefaultProxyConfigurationName = val_str;
	}

	if (reader.ReadBool("Authorized", val_bool)) {
		self.isAuthorized = val_bool;
	}

	if (reader.ReadInteger("UserStatusScheme", val_int)) {	// before it was a web_config only flag
		self.sip.UserStatusScheme = static_cast<VS_CallConfig::eUserStatusScheme>(val_int);
	}

	if (reader.ReadString("Outbound Proxy", val_str))
	{
		self.sip.OutboundProxy = val_str;
	}

	// When adding new value please consider adding it also to the CheckValue() function in "VS_CallConfig_tests.cpp".
	// It is needed for proper unit testing of VS_CallConfig.
}

void VS_CallConfigManager<VS_CallConfig>::SetVerification(const VS_CallConfig::VerificationResult res, const bool storeLastTime)
{
	auto &self = Get();
	if (!self.IsFromRegistry || !(self.SignalingProtocol == VS_CallConfig::SIP || self.SignalingProtocol == VS_CallConfig::H225RAS)
		|| self.RegistryConfigName.empty()) return;

	const char *peers;
	self.m_lastVerificationResult = res;
	if (self.SignalingProtocol == VS_CallConfig::SIP)
	{
		peers = SIP_PEERS_KEY;
	}
	else
	{
		peers = H323_PEERS_KEY;
	}

	constexpr std::size_t size_key = 128 /*peer name*/ + 512 /*max size record name*/ + 1 /*\*/ + 1/*0-terminator*/;
	assert(size_key >= std::char_traits<char>::length(peers) + 1 /*\*/ + self.RegistryConfigName.length() + 1 /*0-terminator*/);
	char key_name[size_key];
	const auto peers_len = std::char_traits<char>::length(peers);
	std::size_t len_key = peers_len;
	memcpy(key_name, peers, peers_len);
	key_name[len_key++] = 0x5C; // '\'
	memcpy(key_name + len_key, self.RegistryConfigName.data(), self.RegistryConfigName.length());
	len_key += self.RegistryConfigName.length();
	key_name[len_key] = '\0';

	VS_RegistryKey key(false, string_view{ key_name, len_key }, false);
	if (key.IsValid())
	{
		key.SetValue(&res, sizeof(res), VS_REG_INTEGER_VT, LAST_AUTHORIZATION_RESULT);
		if (storeLastTime)
		{
			uint32_t time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			key.SetValue(&time, sizeof(time), VS_REG_INTEGER_VT, LAST_CHECK_TIME);
		}
	}
}

inline VS_CallConfigManager<VS_CallConfig>::ref_t VS_CallConfigManager<VS_CallConfig>::Get() const
{
	return const_cast<ref_t>(VS_CallConfigManager<const VS_CallConfig>::Get());
}



bool operator<(const VS_CallConfig::RegistrationIdentifierView& lhs,
               const VS_CallConfig::RegistrationIdentifierView& rhs) noexcept
{
	return std::tie(lhs.registryConfigName, lhs.login, lhs.name, lhs.address.addr, lhs.address.port,
	                lhs.address.protocol) < std::tie(rhs.registryConfigName, rhs.login, rhs.name,
	                                                   rhs.address.addr, rhs.address.port,
	                                                   rhs.address.protocol
	);
}

bool operator<=(const VS_CallConfig::RegistrationIdentifierView& lhs,
                const VS_CallConfig::RegistrationIdentifierView& rhs) noexcept
{
	return !(rhs < lhs);
}

bool operator>(const VS_CallConfig::RegistrationIdentifierView& lhs,
               const VS_CallConfig::RegistrationIdentifierView& rhs) noexcept
{
	return rhs < lhs;
}

bool operator>=(const VS_CallConfig::RegistrationIdentifierView& lhs,
                const VS_CallConfig::RegistrationIdentifierView& rhs) noexcept
{
	return !(lhs < rhs);
}

bool operator==(const VS_CallConfig::RegistrationIdentifierView& lhs,
                const VS_CallConfig::RegistrationIdentifierView& rhs) noexcept
{
	return std::tie(lhs.registryConfigName, lhs.login, lhs.name, lhs.address.addr, lhs.address.port) == std
		::tie(rhs.registryConfigName, rhs.login, rhs.name, rhs.address.addr, rhs.address.port);
}

bool operator!=(const VS_CallConfig::RegistrationIdentifierView& lhs,
                const VS_CallConfig::RegistrationIdentifierView& rhs) noexcept
{
	return !(lhs == rhs);
}


bool operator==(const VS_CallConfig::RegistrationIdentifier& lhs, const VS_CallConfig::RegistrationIdentifier& rhs) noexcept
{
	return std::tie(lhs.registryConfigName, lhs.login, lhs.name, /*Do not compare Connection Types (variable "type")*/
	                lhs.address.addr, lhs.address.port) == std::tie(rhs.registryConfigName, rhs.login, rhs.name,
	                                                                /*Do not compare Connection Types (variable "type")*/
	                                                                rhs.address.addr, rhs.address.port);
}


bool operator!=(const VS_CallConfig::RegistrationIdentifier& lhs, const VS_CallConfig::RegistrationIdentifier& rhs) noexcept
{
	return !(lhs == rhs);
}


bool operator<(const VS_CallConfig::RegistrationIdentifier& lhs, const VS_CallConfig::RegistrationIdentifier& rhs) noexcept
{
	return std::tie(lhs.registryConfigName, lhs.login, lhs.name, lhs.address.addr, lhs.address.port, lhs.address.protocol) <
		std::tie(rhs.registryConfigName, rhs.login, rhs.name, rhs.address.addr, rhs.address.port, rhs.address.protocol);
}

bool operator<=(const VS_CallConfig::RegistrationIdentifier& lhs, const VS_CallConfig::RegistrationIdentifier& rhs) noexcept
{
	return !(rhs < lhs);
}

bool operator>(const VS_CallConfig::RegistrationIdentifier& lhs, const VS_CallConfig::RegistrationIdentifier& rhs) noexcept
{
	return rhs < lhs;
}

bool operator>=(const VS_CallConfig::RegistrationIdentifier& lhs, const VS_CallConfig::RegistrationIdentifier& rhs) noexcept
{
	return !(lhs < rhs);
}
