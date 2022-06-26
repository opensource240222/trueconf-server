#include "VS_CallIDUtils.h"

#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_Replace.h"

#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>

const char TRANSPORT_SEPARATOR = ';';
const char DEFAULT_DTMF_PREFIX = '^';
const char DEFAULT_TELEPHONE_PREFIX[] = "+";

const char TEL_CALL_ID_PREFIX[] = "#tel:";
const char SIP_CALL_ID_PREFIX[] = "#sip:";
const char H323_CALL_ID_PREFIX[] = "#h323:";
const char RTSP_CALL_ID_PREFIX[] = "#rtsp:";

extern const char DEFAULT_DESTINATION_CALLID[];
extern const char DEFAULT_DESTINATION_CALLID_SIP[];
extern const char DEFAULT_DESTINATION_CALLID_H323[];
extern const char GROUPCONF_PREFIX[];

namespace {
	static const char any_separator[] = { TRANSCODER_ID_SEPARATOR, TRANSPORT_SEPARATOR, DEFAULT_DTMF_PREFIX, 0 };
};

std::string VS_GetConfNameByCallID(string_view call_id, bool &is_group_conf, bool &is_default_dest)
{
	if (call_id.empty())
		return {};

	VS_RegistryKey cfg_key(false, CONFIGURATION_KEY);
	VS_RegistryKey tr_key(false, TRANSCODERS_KEY);
	VS_RegistryKey app_prop(false, "AppProperties");

	std::string groupconfName;

	cfg_key.GetString(groupconfName, "Gateway GroupConf Name");
	if (groupconfName.empty())
		tr_key.GetString(groupconfName, "Gateway GroupConf Name");
	if (groupconfName.empty())
		groupconfName = "groupconf";

	std::string groupconfPrefix;

	cfg_key.GetString(groupconfPrefix, "Gateway GroupConf Prefix");
	if (groupconfPrefix.empty())
		tr_key.GetString(groupconfPrefix, "Gateway GroupConf Prefix");

	std::string defDestCallId;
	std::string defDestCallIdSIP;
	std::string defDestCallIdH323;

	app_prop.GetString(defDestCallId, "default_call_destination");
	app_prop.GetString(defDestCallIdSIP, "default_call_destination_sip");
	app_prop.GetString(defDestCallIdH323, "default_call_destination_h323");

	string_view defDest;

	if (boost::iequals(call_id, string_view{ DEFAULT_DESTINATION_CALLID_SIP }))
		defDest = defDestCallIdSIP;
	else if (boost::iequals(call_id, string_view{ DEFAULT_DESTINATION_CALLID_H323 }))
		defDest = defDestCallIdH323;
	else if (boost::iequals(call_id, string_view{ DEFAULT_DESTINATION_CALLID }))
		defDest = defDestCallId;

	if(!defDest.empty())
	{
		call_id = defDest;
		is_default_dest = true;
	}

	string_view username = call_id.substr(0, call_id.find('@'));

	if (boost::iequals(username, groupconfName))
	{
		if (app_prop.GetString(groupconfName, "default_mconf_name") && !groupconfName.empty())
			username = groupconfName;
		else
			username = { "groupconf" };

		is_group_conf = true;
	}
	else if (boost::istarts_with(username, string_view{ GROUPCONF_PREFIX }))
	{
		username = username.substr(::strlen(GROUPCONF_PREFIX));
		is_group_conf = true;
	}
	else if (!groupconfPrefix.empty() && boost::istarts_with(username, groupconfPrefix))
	{
		username = username.substr(groupconfPrefix.length());
		is_group_conf = true;
	}

	return std::string(username);
}

bool VS_IsSystemTelCallID(string_view call_id)
{
	return boost::istarts_with(call_id, TEL_CALL_ID_PREFIX);
}
std::string VS_GetTelephonePrefix()
{
	std::string result(DEFAULT_TELEPHONE_PREFIX);
	VS_RegistryKey(false, CONFIGURATION_KEY).GetString(result, TELEPHONE_PREFIX_TAG);
	return result;
}
bool VS_IsCustomTelPrefix(string_view call_id)
{
	return boost::istarts_with(call_id, VS_GetTelephonePrefix());
}
bool VS_IsTelephoneCallID(string_view call_id)
{
	return VS_IsSystemTelCallID(call_id)
		|| VS_IsCustomTelPrefix(call_id)
		;
}
bool VS_IsSIPCallID(string_view call_id)
{
	return boost::istarts_with(call_id, SIP_CALL_ID_PREFIX);
}
bool VS_IsH323CallID(string_view call_id)
{
	return boost::istarts_with(call_id, H323_CALL_ID_PREFIX);
}
bool VS_IsRTSPCallID(string_view call_id)
{
	return boost::istarts_with(call_id, RTSP_CALL_ID_PREFIX);
}

bool VS_IsRTPCallID(string_view call_id)
{
	return VS_IsSIPCallID(call_id)
	    || VS_IsH323CallID(call_id)
	    || VS_IsRTSPCallID(call_id)
	    || VS_IsTelephoneCallID(call_id)
	    ;
}

bool VS_IsNotTrueConfCallID(string_view call_id)
{
	return call_id.empty()
	    || call_id[0] == '\\'
	    || boost::istarts_with(call_id, "#guest:")
	    || boost::istarts_with(call_id, "#guest2:")
	    || VS_IsRTPCallID(call_id)
	    ;
}

std::string& VS_NormalizeCallID(std::string& call_id)
{
	auto pos = call_id.find_first_of(any_separator);
	if (pos != std::string::npos)
		call_id.erase(pos);
	return call_id;
}

std::string& VS_SkipSIPPrefix(std::string& call_id)
{
	if (VS_IsSIPCallID(call_id) || VS_IsSystemTelCallID(call_id))
		call_id.erase(0, 5);
	return call_id;
}

std::string& VS_AddSIPPrefix(std::string& call_id)
{
	return call_id.insert(0, SIP_CALL_ID_PREFIX);
}

std::string& VS_RemoveAtSign(std::string& call_id)
{
	if (call_id.length() >= 1 && call_id[0] == '@')
		call_id.erase(0, 1);
	return call_id;
}


namespace vs {

static std::string UrlDecode(string_view value)
{
	std::string ret;
	char ch;
	int ii;
	for (int i = 0; i < value.length(); i++) {
		if (value[i] == '%') {
			sscanf(std::string(value.substr(i + 1, 2)).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			ret += ch;
			i = i + 2;
		}
		else if (value[i] == '+') {
			ret += ' ';
		}
		else {
			ret += value[i];
		}
	}
	return (ret);
}

std::string PrettyRTPName(string_view u_dn)
{
	if (VS_IsRTSPCallID(u_dn))
		return PrettyRTSPName(u_dn);
	else if (VS_IsSIPCallID(u_dn))
		return PrettySIPName(u_dn);
	else if (VS_IsH323CallID(u_dn))
		return PrettyH323Name(u_dn);
	else if (VS_IsSystemTelCallID(u_dn))
		return PrettyTelName(u_dn);
	else if (VS_IsCustomTelPrefix(u_dn))
		return PrettyCustomTelName(u_dn);
	else
		return {};
}

std::string PrettyRTSPName(string_view u_dn)
{
	std::string out_dn = UrlDecode(u_dn);
	const auto address_pos = out_dn.find_first_not_of('/', strlen(RTSP_CALL_ID_PREFIX));
	out_dn.erase(0, address_pos);
	return out_dn;
}

std::string PrettySIPName(string_view u_dn)
{
	std::string s{ u_dn };
	VS_NormalizeCallID(s);
	VS_SkipSIPPrefix(s);
	if (!s.empty() && s[0] == '@')
		s.erase(s.begin());
	return s;
}

std::string PrettyH323Name(string_view u_dn)
{
	std::string s{ u_dn };
	VS_NormalizeCallID(s);
	if (VS_IsH323CallID(s))
		s.erase(0, sizeof(H323_CALL_ID_PREFIX)-1);
	VS_RemoveAtSign(s);
	return s;
}

std::string PrettyTelName(string_view u_dn)
{
	u_dn.remove_prefix(sizeof(TEL_CALL_ID_PREFIX) - 1);
	return std::string{ u_dn };
}

std::string PrettyCustomTelName(string_view u_dn)
{
	return std::string{ u_dn };
}

};