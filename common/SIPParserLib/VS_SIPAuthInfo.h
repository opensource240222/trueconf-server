#pragma once

#include <string>
#include "std-generic/cpplib/string_view.h"

enum eStartLineType : int;
enum eSIP_AAA_QOP : int;
enum eSIP_AAA_ALGORITHM : int;

class VS_SIPAuthInfo
{
public:

	enum AuthType
	{
		TYPE_AUTH_INVALID,
		TYPE_AUTH_USER_TO_USER,
		TYPE_AUTH_PROXY_TO_USER
	};

	VS_SIPAuthInfo();
	VS_SIPAuthInfo(const VS_SIPAuthInfo&) = delete;
	virtual ~VS_SIPAuthInfo();

	VS_SIPAuthInfo& operator=(const VS_SIPAuthInfo& info);
	void operator=(const VS_SIPAuthInfo*& info);
	void AddInfo(const VS_SIPAuthInfo& info);

	void login(std::string str);
	const std::string &login() const;

	void password(std::string str);
	const std::string &password() const;

	void realm(std::string str);
	const std::string &realm() const;

	void uri(std::string str);
	const std::string &uri() const;

	void qop(const eSIP_AAA_QOP qop);
	eSIP_AAA_QOP qop() const;

	void method(std::string str);
	const std::string &method() const;

	void algorithm(const eSIP_AAA_ALGORITHM algorithm);
	eSIP_AAA_ALGORITHM algorithm() const;

	void nc(const std::uint32_t nc);
	std::uint32_t nc() const;

	void nonce(std::string);
	const std::string &nonce() const;

	void cnonce(std::string);
	const std::string &cnonce() const;

	void opaque(std::string);
	const std::string &opaque() const;

	void response(std::string str);
	const std::string &response() const;

	void targetname(std::string str);
	const std::string &targetname() const;

	void version(int v);
	int version() const;

	void auth_type(AuthType auth_type);
	AuthType auth_type() const;

protected:
// ----------- ! ! !  Только для MD5  ! ! ! -----------

	std::string 			m_login;
	std::string				m_password;
	std::string				m_realm;

	std::string				m_method;
	eSIP_AAA_QOP			m_qop;
	eSIP_AAA_ALGORITHM		m_algorithm;
	std::string				m_uri;

	std::string				m_opaque;
	std::string				m_nonce;
	std::string				m_cnonce;
	std::uint32_t			m_nc;

	std::string				m_response;

	std::string				m_targetname;
	int						m_version;

private:

	AuthType				m_auth_type;
	bool operator!=(const VS_SIPAuthInfo& info)const;
};