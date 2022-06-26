#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include "VS_SIPObjectFactory.h"
#include <boost/regex.hpp>

class VS_SIPAuthScheme;

enum eSIP_AUTH_SCHEME;

class VS_SIPField_Auth: public VS_BaseField
{
public:
	const static boost::regex e;

	explicit VS_SIPField_Auth(VS_SIPObjectFactory::SIPHeader header);
	~VS_SIPField_Auth();

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	TSIPErrorCodes Init(VS_RTSP_ParserInfo* call) override;

	std::shared_ptr<VS_SIPAuthInfo> GetAuthInfo() const;
	eSIP_AUTH_SCHEME scheme() const { return m_scheme; }

private:
	VS_SIPField_Auth();

	VS_SIPObjectFactory::SIPHeader		m_header;

	std::shared_ptr<VS_SIPAuthScheme>	m_auth_scheme;
	eSIP_AUTH_SCHEME					m_scheme;
	bool								m_isRTSP;
};

std::unique_ptr<VS_BaseField> VS_SIPField_Auth_WWWAuthenticate_Instance();
std::unique_ptr<VS_BaseField> VS_SIPField_Auth_Authorization_Instance();
std::unique_ptr<VS_BaseField> VS_SIPField_Auth_AuthenticationInfo_Instance();
std::unique_ptr<VS_BaseField> VS_SIPField_Auth_ProxyAuthenticate_Instance();
std::unique_ptr<VS_BaseField> VS_SIPField_Auth_ProxyAuthorization_Instance();