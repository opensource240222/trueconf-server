#include "VS_SIPAuthDigest.h"
#include "VS_RTSP_ParserInfo.h"
#include <string>
#include <boost/algorithm/string.hpp>
#include "VS_SIPGetInfoInterface.h"
#include "std/cpplib/VS_Utils.h"
#include "std-generic/clib/strcasecmp.h"

VS_SIPAuthDigest::VS_SIPAuthDigest()
{

}

VS_SIPAuthDigest::~VS_SIPAuthDigest()
{

}

TSIPErrorCodes VS_SIPAuthDigest::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
	if ( (TSIPErrorCodes::e_ok != err) || !ptr || !ptr_sz )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	std::vector<std::string> params;
	{
		char *p = ptr.get();
		boost::split(params, p, boost::is_any_of(","), boost::token_compress_on);
	}

	for (const auto& p : params)		// iterate params: username, realm, nonce, ...
	{
		auto pos = p.find_first_of('=');
		if (pos <= 0 || pos >= p.length())
			continue;
		std::string param_name = p.substr(0, pos);
		++pos;	// skip "=" char
		std::string param_value = p.substr(pos, p.length() - pos);
		boost::trim(param_name);
		boost::trim(param_value);
		if (param_value.length() > 0 && param_value[0] == '\"')
			param_value.erase(0,1);
		if (param_value.length() > 0 && param_value[param_value.length()-1] == '\"')
			param_value.erase(param_value.length() - 1, 1);

		if (strcasecmp(param_name.c_str(), "username") == 0)			this->login(param_value);
		else if (strcasecmp(param_name.c_str(), "realm") == 0)		this->realm(param_value);
		else if (strcasecmp(param_name.c_str(), "uri") == 0)			this->uri(param_value);
		else if (strcasecmp(param_name.c_str(), "nonce") == 0)		this->nonce(param_value);
		else if (strcasecmp(param_name.c_str(), "cnonce") == 0)		this->cnonce(param_value);
		else if (strcasecmp(param_name.c_str(), "response") == 0)		this->response(param_value);
		else if (strcasecmp(param_name.c_str(), "opaque") == 0)		this->opaque(param_value);
		else if (strcasecmp(param_name.c_str(), "nc") == 0) {
			std::size_t val(0);
			if (StrToHex(param_value, val))
				this->nc(val);
		}
		else if (strcasecmp(param_name.c_str(), "qop") == 0) {
			if (param_value == "auth")
				this->qop(SIP_AAA_QOP_AUTH);
			else if(param_value == "auth-int")
				this->qop(SIP_AAA_QOP_AUTH_INT);
			else
				this->qop(SIP_AAA_QOP_AUTH);		// not sure if default is needed, but let it be
		}
		else if (strcasecmp(param_name.c_str(), "algorithm") == 0) {
			if (strcasecmp(param_value.c_str(), "MD5") == 0)
				this->algorithm(SIP_AAA_ALGORITHM_MD5);
			else if (strcasecmp(param_value.c_str(), "MD5-sess") == 0)
				this->algorithm(SIP_AAA_ALGORITHM_MD5_SESS);
			else
				this->algorithm(SIP_AAA_ALGORITHM_MD5);		// Default value [RFC2617]
		}
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPAuthDigest::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	std::string out;

	bool comma = false;
	if (!m_realm.empty())
	{
		out += "realm=\"";
		out += m_realm;
		out += "\"";
		comma = false;
	}

	if (!m_login.empty())
	{
		if (!comma) {	out += ",";	comma = true;	}
		out += "username=\"";
		out += m_login;
		out += "\"";
		comma = false;
	}

	if (!m_uri.empty())
	{
		if (!comma) {	out += ",";	comma = true;	}
		out += "uri=\"";
		out += m_uri;
		out += "\"";
		comma = false;
	}

	if (!m_opaque.empty())
	{
		if (!comma) {	out += ",";	comma = true;	}
		out += "opaque=\"";
		out += m_opaque;
		out += "\"";
		comma = false;
	}

	if (!m_nonce.empty())
	{
		if (!comma) {	out += ",";	comma = true;	}
		out += "nonce=\"";
		out += m_nonce;
		out += "\"";
		comma = false;
	}

	if (!m_cnonce.empty())
	{
		if (!comma) {	out += ",";	comma = true;	}
		out += "cnonce=\"";
		out += m_cnonce;
		out += "\"";
		comma = false;
	}

	if (m_nc)
	{
		if (!comma) {	out += ",";	comma = true;	}
		out += "nc=";

		char ch[(sizeof(decltype(m_nc)) * 2) + 1] = { 0 }; //  1 {0000|0000} bytes in binary - FF hex: (count bytes * 2) + NULL-terminator
		snprintf(ch, sizeof ch, "%X", m_nc);

		for(unsigned int i=0; i < (8 - strlen(ch)); i++)		// add "0"
			out += "0";
		out += ch;
		comma = false;
	}

	if (m_algorithm == SIP_AAA_ALGORITHM_MD5) {
		if (!comma) {	out += ",";	comma = true;	}
		out += "algorithm=MD5";
		comma = false;
	} else if (m_algorithm == SIP_AAA_ALGORITHM_MD5_SESS) {
		if (!comma) {	out += ",";	comma = true;	}
		out += "algorithm=MD5-Sess";
		comma = false;
	}

	if (m_qop == SIP_AAA_QOP_AUTH) {
		if (!comma) {	out += ",";	comma = true;	}
		out += "qop=\"auth\"";
		comma = false;
	} else if (m_qop == SIP_AAA_QOP_AUTH_INT) {
		if (!comma) {	out += ",";	comma = true;	}
		out += "qop=\"auth-int\"";
		comma = false;
	}

	if (!m_response.empty())
	{
		if (!comma) {	out += ",";}
		out += "response=\"";
		out += m_response;
		out += "\"";
	}

	if ( out.empty() )
		return TSIPErrorCodes::e_InputParam;

	return aBuffer.AddData( out );
}

TSIPErrorCodes VS_SIPAuthDigest::Init(const VS_SIPGetInfoInterface& call)
{
	const auto scheme = call.GetAuthScheme();
	if ( !scheme )
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	*(static_cast<VS_SIPAuthScheme*>(this)) = *scheme;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPAuthDigest::Init(VS_RTSP_ParserInfo* call)
{
	if ( !call )
		return TSIPErrorCodes::e_InputParam;

	const auto scheme = call->GetAuthScheme();
	if ( !scheme )
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	*(static_cast<VS_SIPAuthScheme*>(this)) = *scheme;
	algorithm(SIP_AAA_ALGORITHM_INVALID); // Don't include "algorithm=" in encoded parameters

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

 eSIP_AUTH_SCHEME VS_SIPAuthDigest::scheme() const {
	return SIP_AUTHSCHEME_DIGEST;
}

std::string VS_SIPAuthDigest::GenerateNonceValue()
{
	char buf[33] = {};
	VS_GenKeyByMD5(buf);

	std::string ret(buf);
	std::transform(ret.begin(), ret.end(), ret.begin(), tolower);

	return ret;
}

bool VS_SIPAuthDigest::StrToHex(std::string aStr, std::size_t &aRet) const
{
	if (aStr.length() != 8)		// rfc3261: nc-value = 8LHEX
		return false;

	int digit;

	// LowerCase
	std::transform(aStr.begin(), aStr.end(), aStr.begin(), tolower);

	for (std::size_t i = 0; i < 8; i++)
	{
		bool bValid = false;
		if (!bValid && (aStr[i] == '0')) { digit = 0; bValid = true; }
		if (!bValid && (aStr[i] == '1')) { digit = 1; bValid = true; }
		if (!bValid && (aStr[i] == '2')) { digit = 2; bValid = true; }
		if (!bValid && (aStr[i] == '3')) { digit = 3; bValid = true; }
		if (!bValid && (aStr[i] == '4')) { digit = 4; bValid = true; }
		if (!bValid && (aStr[i] == '5')) { digit = 5; bValid = true; }
		if (!bValid && (aStr[i] == '6')) { digit = 6; bValid = true; }
		if (!bValid && (aStr[i] == '7')) { digit = 7; bValid = true; }
		if (!bValid && (aStr[i] == '8')) { digit = 8; bValid = true; }
		if (!bValid && (aStr[i] == '9')) { digit = 9; bValid = true; }
		if (!bValid && (aStr[i] == 'a')) { digit = 10; bValid = true; }
		if (!bValid && (aStr[i] == 'b')) { digit = 11; bValid = true; }
		if (!bValid && (aStr[i] == 'c')) { digit = 12; bValid = true; }
		if (!bValid && (aStr[i] == 'd')) { digit = 13; bValid = true; }
		if (!bValid && (aStr[i] == 'e')) { digit = 14; bValid = true; }
		if (!bValid && (aStr[i] == 'f')) { digit = 15; bValid = true; }
		if (!bValid) { return false; }

		std::size_t mul = 1;
		for (std::size_t j=0; j < 7-i; j++)
			mul = mul * 16;

		aRet = aRet + digit * mul;
	}

	return true;
}
