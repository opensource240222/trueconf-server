#include "VS_SIPAuthInfo.h"
#include "../SIPParserBase/VS_Const.h"

VS_SIPAuthInfo::VS_SIPAuthInfo(): m_qop(SIP_AAA_QOP_INVALID),
m_algorithm(SIP_AAA_ALGORITHM_INVALID), m_nc(0), m_version(0), m_auth_type(TYPE_AUTH_INVALID)
{

}

VS_SIPAuthInfo::~VS_SIPAuthInfo()
{

}

VS_SIPAuthInfo& VS_SIPAuthInfo::operator=(const VS_SIPAuthInfo& info)
{

	if (this == &info)
	{
		return *this;
	}
	if(*this != info)
	{
		m_login = info.m_login;
		m_password = info.m_password;
		m_realm = info.m_realm;

		m_method = info.m_method;
		m_qop = info.m_qop;
		m_algorithm = info.m_algorithm;
		m_uri = info.m_uri;

		m_nc = info.m_nc;
		m_nonce = info.m_nonce;
		m_cnonce = info.m_cnonce;

		m_opaque = info.m_opaque;

		m_response = info.m_response;
		m_auth_type = info.m_auth_type;

		m_targetname = info.m_targetname;
		m_version = info.m_version;
	}
	return *this;
}

void VS_SIPAuthInfo::operator=(const VS_SIPAuthInfo*& info)
{
	if ( !info )
		return;

	this->operator=(*info);
}

bool VS_SIPAuthInfo::operator!=(const VS_SIPAuthInfo& info)const
{
	return m_login != info.m_login || m_password != info.m_password || m_realm != info.m_realm || m_method != info.
		m_method || m_qop != info.m_qop || m_algorithm != info.m_algorithm || m_uri != info.m_uri || m_nc != info.m_nc
		|| m_nonce != info.m_nonce || m_cnonce != info.m_cnonce || m_opaque != info.m_opaque || m_response != info.
		m_response || m_auth_type != info.m_auth_type || m_targetname != info.m_targetname || m_version != info.
		m_version;
}

void VS_SIPAuthInfo::AddInfo(const VS_SIPAuthInfo& info)
{
	m_qop = info.m_qop;

	if (info.m_nc > 0)
		m_nc = info.m_nc;

	if (!info.m_login.empty())
		m_login = info.m_login;

	if (!info.m_password.empty())
		m_password = info.m_password;

	if (!info.m_realm.empty())
		m_realm = info.m_realm;

	if (!info.m_uri.empty())
		m_uri = info.m_uri;

	if (!info.m_opaque.empty())
		m_opaque = info.m_opaque;

	if (!info.m_nonce.empty())
		m_nonce = info.m_nonce;

	if (!info.m_cnonce.empty())
		m_cnonce = info.m_cnonce;

	if (!info.m_response.empty())
		m_response = info.m_response;

	if (!info.m_method.empty())
		m_method = info.m_method;

	if (info.m_algorithm != SIP_AAA_ALGORITHM_INVALID)
		m_algorithm = info.m_algorithm;

	if (info.m_auth_type != TYPE_AUTH_INVALID)
		m_auth_type = info.m_auth_type;
}

void VS_SIPAuthInfo::login(std::string str)
{
	m_login = std::move(str);
}

const std::string &VS_SIPAuthInfo::login() const
{
	return m_login;
}

void VS_SIPAuthInfo::password(std::string str)
{
	m_password = std::move(str);
}

const std::string &VS_SIPAuthInfo::password() const
{
	return m_password;
}

void VS_SIPAuthInfo::realm(std::string str)
{
	m_realm = std::move(str);
}

const std::string &VS_SIPAuthInfo::realm() const
{
	return m_realm;
}

void VS_SIPAuthInfo::uri(std::string str)
{
	m_uri = std::move(str);
}

const std::string &VS_SIPAuthInfo::uri() const
{
	return m_uri;
}

void VS_SIPAuthInfo::qop(const eSIP_AAA_QOP qop)
{
	m_qop = qop;
}

eSIP_AAA_QOP VS_SIPAuthInfo::qop() const
{
	return m_qop;
}

void VS_SIPAuthInfo::method(std::string str)
{
	m_method = std::move(str);
}

const std::string &VS_SIPAuthInfo::method() const
{
	return m_method;
}

void VS_SIPAuthInfo::algorithm(const eSIP_AAA_ALGORITHM algorithm)
{
	m_algorithm = algorithm;
}

eSIP_AAA_ALGORITHM VS_SIPAuthInfo::algorithm() const
{
	return m_algorithm;
}

void VS_SIPAuthInfo::nonce(std::string str)
{
	m_nonce = std::move(str);
}

const std::string &VS_SIPAuthInfo::nonce() const
{
	return m_nonce;
}

void VS_SIPAuthInfo::cnonce(std::string str)
{
	m_cnonce = std::move(str);
}

const std::string &VS_SIPAuthInfo::cnonce() const
{
	return m_cnonce;
}

void VS_SIPAuthInfo::opaque(std::string str)
{
	m_opaque = std::move(str);
}

const std::string &VS_SIPAuthInfo::opaque() const
{
	return m_opaque;
}

void VS_SIPAuthInfo::response(std::string str)
{
	m_response = std::move(str);
}

const std::string &VS_SIPAuthInfo::response() const
{
	return m_response;
}

void VS_SIPAuthInfo::targetname(std::string str)
{
	m_targetname = std::move(str);
}

const std::string &VS_SIPAuthInfo::targetname() const
{
	return m_targetname;
}

void VS_SIPAuthInfo::version(int v)
{
	m_version = v;
}

int VS_SIPAuthInfo::version() const
{
	return m_version;
}

void VS_SIPAuthInfo::auth_type(AuthType auth_type)
{
	m_auth_type = auth_type;
}

VS_SIPAuthInfo::AuthType VS_SIPAuthInfo::auth_type() const
{
	return m_auth_type;
}

void VS_SIPAuthInfo::nc(const std::uint32_t nc)
{
	m_nc = nc;
}

unsigned int VS_SIPAuthInfo::nc() const
{
	return m_nc;
}