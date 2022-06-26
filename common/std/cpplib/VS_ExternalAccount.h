#pragma once

#include <cstdlib>
#include <string>
#include "net/Port.h"

struct VS_ExternalAccount
{
	std::string m_protocol;
	std::string m_host;
	net::port m_port;
	bool m_isVoIPServer;

	std::string m_login;
	std::string m_password;
	std::string m_TelephonePrefixReplace;

	enum RegistrationBehavior
	{
		REG_DO_NOT_REGISTER = 0,
		REG_REGISTER_ALWAYS = 1,
		REG_REGISTER_ON_CALL= 2,
	};

	RegistrationBehavior m_registrationBehavior;

	VS_ExternalAccount()
		: m_port(0)
		, m_isVoIPServer(false)
		, m_registrationBehavior(REG_DO_NOT_REGISTER)
	{
	}

	VS_ExternalAccount(const char * data)
	{
		Deserealize(data);
	}

	std::string Serealize() const
	{
		std::string s;
		s.reserve(128);

		s += m_protocol;
		s += '\x01';
		s += m_host;
		s += '\x01';
		s += std::to_string(m_port);
		s += '\x01';
		s += m_login;
		s += '\x01';
		s += m_password;
		s += '\x01';
		s += std::to_string(static_cast<int>(m_registrationBehavior));
		s += '\x01';
		s += m_isVoIPServer ? '1' : '0';
		s += '\x01';

		return s;
	}

	void Deserealize( const char* data)
	{
		Clear();
		unsigned id = 0;
		auto start = data;
		for (auto p = start; *p; ++p)
		{
			if (*p == '\x01')
			{
				switch(id)
				{
					case 0: m_protocol.assign(start, p); break;
					case 1: m_host.assign(start, p); break;
					case 2: m_port = atoi(std::string(start, p).c_str()); break;
					case 3: m_login.assign(start, p); break;
					case 4: m_password.assign(start, p); break;
					case 5: m_registrationBehavior = static_cast<RegistrationBehavior>(atoi(std::string(start, p).c_str())); break;
					case 6: m_isVoIPServer = *start == '1'; break;
					case 7: m_TelephonePrefixReplace.assign(start, p); break;
				}

				start = p + 1;
				++id;
			}
		}
	}

private:
	void Clear()
	{
		m_protocol.clear();
		m_host.clear();
		m_port = 0;
		m_login.clear();
		m_password.clear();
		m_TelephonePrefixReplace.clear();
		m_isVoIPServer = false;
		m_registrationBehavior = REG_DO_NOT_REGISTER;
	}
};
