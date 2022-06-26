#pragma once

#include "std-generic/cpplib/VS_ClockWrapper.h"
#include "../CallConfig/VS_CallConfig.h"

#ifndef NDEBUG // debug version
#include "std/debugbreak.h"
#endif

class VS_SignalChannel;

class VS_ParserInfo
{
public:
	VS_ParserInfo();
	VS_ParserInfo(const VS_ParserInfo&) = delete;//for avoiding using copy constructor
	virtual ~VS_ParserInfo() {}

	VS_ParserInfo& operator=(const VS_ParserInfo &info) = delete;

	VS_CallConfig& GetConfig();
	const VS_CallConfig& GetConfig() const;
	virtual bool SetConfig(VS_CallConfig cfg);
	virtual bool IsGroupConf() const { assert(false); return false; };
	virtual void SetGroupConf(bool) { assert(false);};

	void SetH224Enable(bool value);
	bool IsH224Enabled() const;

	void SetExpires(std::chrono::seconds expires);
	std::chrono::seconds GetExpires() const;
	void SetAliasMy(std::string str);
	const std::string& GetAliasMy() const;
	void SetAliasRemote(std::string str);
	const std::string& GetAliasRemote() const;
	void SetSIPRemoteTarget(std::string str);
	const std::string& GetSIPRemoteTarget() const;
	void SetUser(std::string str);
	const std::string& GetUser() const;
	void SetDomain(std::string str);
	const std::string& GetDomain() const;
	void SetPassword(std::string str);
	const std::string& GetPassword() const;
	void SetUserAgent(std::string str);
	const std::string& GetUserAgent() const;

protected:
	steady_clock_wrapper &clock() const noexcept
	{
		return m_clock;
	}
private:
	mutable steady_clock_wrapper m_clock;
private:
	std::string m_my_alias;
	std::string m_remote_alias;
	std::string m_sip_remote_target;
	std::string m_user;
	std::string m_domain;
	std::string m_password;
	std::string m_user_agent;
	VS_CallConfig	m_call_config;
	std::chrono::seconds m_expires;
	bool		m_h224_is_enabled;
};
