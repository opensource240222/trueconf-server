#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

#include <string>
#include "net/Port.h"
#include "net/Protocol.h"

enum eConnectionType : int;

class VS_SIPURI: public VS_BaseField
{
public:
	const static boost::regex e1;
	const static boost::regex e2;
	const static boost::regex e3;
	const static boost::regex e4;
	const static boost::regex e5;
	const static boost::regex e6;
	const static boost::regex e7;
	const static boost::regex opaque_e;

	const static boost::regex ms_opaque_e;
	const static boost::regex ms_route_sig_e;
	const static boost::regex ms_key_info_e;
	const static boost::regex ms_identity_e;
	const static boost::regex ms_fe_e;
	const static boost::regex ms_role_rs_to_e;
	const static boost::regex ms_role_rs_from_e;
	const static boost::regex ms_ent_dest_e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	void Clean() noexcept override;

	VS_SIPURI & operator=(const VS_SIPURI &aURI) = default;
	bool operator!=(const VS_SIPURI &aURI) const;
	bool operator==(const VS_SIPURI &aURI) const;//for unittest

	bool GetAlias_UserHost(char* alias) const;

	VS_SIPURI();
	VS_SIPURI(VS_SIPURI &) = delete;
	~VS_SIPURI();

	// returns URI in a form suitable for Request-URI parameter in SIP requests (without any parameters that are not allowed etc.)
	// returned Request-URI doesn't contain prefixes 'sip:' or 'sips:'
	bool GetRequestURI(std::string &uri) const;

// Get/Set Methods
	bool Name(std::string aName);
	const std::string &Name() const;
	bool IsName() const;

	int URIType() const;
	void URIType(int type);

	const std::string &User() const;
	void User(std::string user);

	// returns address without brackets, so ::1 will be represented as "::1" (not "[::1]").
	const std::string &Host() const;

	void Host(std::string host);

	net::port Port() const;
	void Port(net::port port);

	bool AngleBracket() const;
	void AngleBracket(bool IsAngleBracket);

	const std::string &Tag() const;
	void Tag(std::string tag);

	const std::string &Epid() const;
	void Epid(std::string epid);

	const std::string &maddr() const;
	void maddr(std::string maddr);

	bool lr() const;

	void Transport(const net::protocol transport);
	net::protocol Transport() const;

	void set_opaque(std::string s);
	void SetDoPreDecodeEscaping() { m_do_pre_decode_escaping = true; }
private:
	// "iName" <iURIType:iNick@iHost:iPort;iParam>;iTag=123
	std::string iName;
	int iURIType;
	std::string iUser;
	std::string iHost;
	net::port iPort;

	net::protocol								m_param_transport;
	std::string iParam_maddr;
	std::string iParam_tag;
	std::string										m_param_epid;
	std::string m_param_gruu;
	bool iParam_lr;
	bool m_ms_gruu = false;

	bool iIsAngleBracket;

	std::string m_opaque;

	bool m_do_pre_decode_escaping = false;

	bool FindParam_maddr(string_view aInput);
	bool FindParam_transport(string_view aInput);
	bool FindParam_lr(string_view aInput);
	bool FindParam_tag(string_view aInput);
	bool FindParam_epid(string_view aInput);
	bool FindParam_gruu(string_view aInput);
	bool FindParam_opaque(const std::string &input);

	// ms sip uri parameter extensions https://msdn.microsoft.com/en-us/library/dd949116(v=office.12).aspx
	bool FindParam_MsOpaque(const std::string& input);
	bool FindParam_MsRouteSig(const std::string& input);
	bool FindParam_MsKeyInfo(const std::string& input);
	bool FindParam_MsIdentity(const std::string& input);
	bool FindParam_MsFe(const std::string& input);
	bool FindParam_MsRoleRsTo(const std::string& input);
	bool FindParam_MsRoleRsFrom(const std::string& input);
	bool FindParam_MsEntDest(const std::string& input);
	void FindParam_MsGruu(const std::string& input);

	std::string m_ms_opaque;
	std::string m_ms_route_sig;
	std::string m_ms_key_info;
	std::string m_ms_identity;
	std::string m_ms_fe;
	bool m_ms_role_rs_to = false;
	bool m_ms_role_rs_from = false;
	bool m_ms_ent_dest = false;

	// Returns uri string:
	// if address family == ipv4 then return string like "127.0.0.1"
	// if address family == ipv6 then return string like "[::1]" (with square brackets)
	// if address family is undefined then return ""
	std::string MakeUri() const;
};