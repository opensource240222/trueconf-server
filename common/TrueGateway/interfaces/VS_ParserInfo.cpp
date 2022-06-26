#include "VS_ParserInfo.h"

VS_ParserInfo::VS_ParserInfo() : m_expires(0), m_h224_is_enabled(false)
{
}

VS_CallConfig& VS_ParserInfo::GetConfig()
{
	return m_call_config;
}

const VS_CallConfig& VS_ParserInfo::GetConfig() const
{
	return m_call_config;
}

bool VS_ParserInfo::SetConfig(VS_CallConfig cfg)
{
	m_call_config = std::move(cfg);
	return true;
}

void VS_ParserInfo::SetH224Enable(bool value)
{
	m_h224_is_enabled = value;
}

bool VS_ParserInfo::IsH224Enabled() const
{
	return m_h224_is_enabled;
}

void VS_ParserInfo::SetExpires(std::chrono::seconds expires)
{
	m_expires = expires;
}

std::chrono::seconds VS_ParserInfo::GetExpires() const
{
	return m_expires;
}

void VS_ParserInfo::SetAliasMy(std::string str)
{
	m_my_alias = std::move(str);
}

const std::string& VS_ParserInfo::GetAliasMy() const
{
	return m_my_alias;
}

void VS_ParserInfo::SetAliasRemote(std::string str)
{
	m_remote_alias = std::move(str);
}

const std::string& VS_ParserInfo::GetAliasRemote() const
{
	return m_remote_alias;
}

void VS_ParserInfo::SetSIPRemoteTarget(std::string str)
{
	m_sip_remote_target = std::move(str);
}

const std::string& VS_ParserInfo::GetSIPRemoteTarget() const
{
	return m_sip_remote_target;
}

void VS_ParserInfo::SetUser(std::string str)
{
	m_user = std::move(str);
}

const std::string& VS_ParserInfo::GetUser() const
{
	return m_user;
}

void VS_ParserInfo::SetDomain(std::string str)
{
	m_domain = std::move(str);
}

const std::string& VS_ParserInfo::GetDomain() const
{
	return m_domain;
}

void VS_ParserInfo::SetPassword(std::string str)
{
	m_password = std::move(str);
}

const std::string& VS_ParserInfo::GetPassword() const
{
	return m_password;
}

void VS_ParserInfo::SetUserAgent(std::string str)
{
	m_user_agent = std::move(str);
}

const std::string& VS_ParserInfo::GetUserAgent() const
{
	return m_user_agent;
}
