#pragma once

#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/VS_RemoveTranscoderID.h"
#include <string>

extern const char TRANSPORT_SEPARATOR;
extern const char DEFAULT_DTMF_PREFIX;
extern const char DEFAULT_TELEPHONE_PREFIX[];

extern const char TEL_CALL_ID_PREFIX[];
extern const char SIP_CALL_ID_PREFIX[];
extern const char H323_CALL_ID_PREFIX[];
extern const char RTSP_CALL_ID_PREFIX[];

inline string_view VS_GetNameByCallID(string_view call_id)
{
	typedef std::make_signed<string_view::size_type>::type signed_size_type;
	return call_id.substr(0, call_id.find('@')).substr(string_view::size_type(static_cast<signed_size_type>(call_id.find(':') + 1)), string_view::npos);
}

std::string VS_GetConfNameByCallID(string_view call_id, bool &is_group_conf, bool &is_default_dest);
bool VS_IsSystemTelCallID(string_view call_id);
bool VS_IsCustomTelPrefix(string_view call_id);
bool VS_IsTelephoneCallID(string_view call_id);
bool VS_IsSIPCallID(string_view call_id);
bool VS_IsH323CallID(string_view call_id);
bool VS_IsRTSPCallID(string_view call_id);
bool VS_IsRTPCallID(string_view call_id);
bool VS_IsNotTrueConfCallID(string_view call_id);
std::string VS_GetTelephonePrefix();

std::string& VS_NormalizeCallID(std::string& call_id);
inline std::string VS_NormalizeCallID(string_view call_id)
{
	std::string r(call_id);
	VS_NormalizeCallID(r);
	return r;
}

std::string& VS_SkipSIPPrefix(std::string& call_id);
inline std::string VS_SkipSIPPrefix(string_view call_id)
{
	std::string r(call_id);
	VS_SkipSIPPrefix(r);
	return r;
}

std::string& VS_AddSIPPrefix(std::string& call_id);
inline std::string VS_AddSIPPrefix(string_view call_id)
{
	std::string r(call_id);
	VS_AddSIPPrefix(r);
	return r;
}

std::string& VS_RemoveAtSign(std::string& call_id);
inline std::string VS_RemoveAtSign(string_view call_id)
{
	std::string r(call_id);
	VS_RemoveAtSign(r);
	return r;
}

namespace vs {

std::string PrettyRTPName(string_view u_dn);
std::string PrettyRTSPName(string_view u_dn);
std::string PrettySIPName(string_view u_dn);
std::string PrettyH323Name(string_view u_dn);
std::string PrettyTelName(string_view u_dn);
std::string PrettyCustomTelName(string_view u_dn);

}