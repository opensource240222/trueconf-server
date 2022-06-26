#include "VS_SIPAuthGSS.h"

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include "VS_SIPGetInfoInterface.h"
#include "std-generic/clib/strcasecmp.h"

VS_SIPAuthGSS::VS_SIPAuthGSS(eSIP_AUTH_SCHEME scheme) : m_scheme(scheme), m_cnum(0)
{

}


TSIPErrorCodes VS_SIPAuthGSS::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
	if ((TSIPErrorCodes::e_ok != err) || !ptr || !ptr_sz) {
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
			param_value.erase(0, 1);
		if (param_value.length() > 0 && param_value[param_value.length() - 1] == '\"')
			param_value.erase(param_value.length() - 1, 1);

		if (strcasecmp(param_name.c_str(), "realm") == 0)				this->realm(param_value);
		else if (strcasecmp(param_name.c_str(), "targetname") == 0)	this->targetname(param_value);
		else if (strcasecmp(param_name.c_str(), "version") == 0)		this->version(atoi(param_value.c_str()));
		else if (strcasecmp(param_name.c_str(), "gssapi-data") == 0)  this->m_gssapi_data = std::move(param_value);
		else if (strcasecmp(param_name.c_str(), "opaque") == 0)		this->opaque(param_value);
		else if (strcasecmp(param_name.c_str(), "crand") == 0)		this->m_crand = std::move(param_value);
		else if (strcasecmp(param_name.c_str(), "srand") == 0)		this->m_srand = std::move(param_value);
		else if (strcasecmp(param_name.c_str(), "cnum") == 0)			this->m_cnum = atoi(param_value.c_str());
		else if (strcasecmp(param_name.c_str(), "qop") == 0) {
			if (param_value == "auth")
				this->qop(SIP_AAA_QOP_AUTH);
			else if (param_value == "auth-int")
				this->qop(SIP_AAA_QOP_AUTH_INT);
			else
				this->qop(SIP_AAA_QOP_AUTH);
		}
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPAuthGSS::Encode(VS_SIPBuffer &aBuffer) const
{
	if (!IsValid())
		return GetLastClassError();

	std::string out;

	if (m_qop == SIP_AAA_QOP_AUTH) {
		out += "qop=\"auth\", ";
	}

	out += "realm=\"";
	out += m_realm;
	out += "\"";

	if (m_opaque.length()) {
		out += ", opaque=\"";
		out += m_opaque;
		out += "\"";
	}

	out += ", targetname=\"";
	out += m_targetname;
	out += "\"";

	if (m_gssapi_data.is_initialized())
	{
		out += ", gssapi-data=\"";
		out += m_gssapi_data.get();
		out += "\"";
	}

	if (m_crand.length()) {
		out += ", crand=\"";
		out += m_crand;
		out += "\"";

		out += ", cnum=\"";
		out += std::to_string(m_cnum);
		out += "\"";
	}

	if (m_response.length()) {
		out += ", response=\"";
		out += m_response;
		out += "\"";
	}

	out += ", version=";
	out += std::to_string(m_version);

	if (!out.length()) {
		return TSIPErrorCodes::e_InputParam;
	}

	return aBuffer.AddData(out);
}

TSIPErrorCodes VS_SIPAuthGSS::Init(const VS_SIPGetInfoInterface& call)
{
	const auto ctx_scheme = call.GetAuthScheme();
	if (!ctx_scheme){
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	auto scheme = std::dynamic_pointer_cast<VS_SIPAuthGSS>(ctx_scheme);
	if (!scheme) {
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	*this = *scheme;
	//m_cnum = call->GetCnum();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

std::string VS_SIPAuthGSS::scheme_str() const {
	if (m_scheme == SIP_AUTHSCHEME_NTLM) {
		return "NTLM";
	}
	if (m_scheme == SIP_AUTHSCHEME_KERBEROS) {
		return "Kerberos";
	}
	if (m_scheme == SIP_AUTHSCHEME_TLS_DSK) {
		return "TLS-DSK";
	}
	return {};
}