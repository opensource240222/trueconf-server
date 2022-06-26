#pragma once

#include "../SIPParserBase/VS_ObjectFactory.h"

#include <boost/regex.hpp>

class VS_BaseField;
class VS_SIPBuffer;
class VS_SIPAuthInfo;
class VS_SIPAuthDigest;
class VS_SIPAuthScheme;

enum eSIP_AUTH_SCHEME;
enum eStartLineType : int;
enum eSIP_Events;
enum eSIP_ExtensionPack;
enum eSIP_Events;

const static auto HASHHEXLEN = 32;
typedef char HASHHEX[HASHHEXLEN + 1];

/*********************************************
 * Singleton
 *********************************************/

class VS_SIPObjectFactory: public VS_ObjectFactory
{
public:
	const static boost::regex e;

	enum class SIPHeader
	{
		SIPHeader_Invalid = 0,
		SIPHeader_StartLine,
		SIPHeader_CSeq,
		SIPHeader_Via,
		SIPHeader_To,
		SIPHeader_From,
		SIPHeader_Contact,
		SIPHeader_CallID,
		SIPHeader_Date,
		SIPHeader_Expires,
		SIPHeader_ContentLength,
		SIPHeader_ContentType,
		SIPHeader_MaxForwards,
		SIPHeader_WWWAuthenticate,
		SIPHeader_Authorization,
		SIPHeader_AuthenticationInfo,
		SIPHeader_ProxyAuthenticate,
		SIPHeader_ProxyAuthorization,
		SIPHeader_Event,
		SIPHeader_UserAgent,
		SIPHeader_Unsupported,
		SIPHeader_Require,
		SIPHeader_Supported,
		SIPHeader_SessionExpires,
		SIPHeader_MinSE,
		SIPHeader_MinExpires,
		SIPHeader_SubscriptionState,
		SIPHeader_Allow,
		SIPHeader_Accept,
		SIPHeader_RecordRoute,
		SIPHeader_RetryAfter,
		SIPHeader_Route,
		SIPHeader_Server
	};

	static VS_SIPObjectFactory* Instance();

	CreateFieldResult CreateField(VS_SIPBuffer &aBuffer) const override;
	CreateFieldResult CreateField(VS_SIPObjectFactory::SIPHeader header) const;

	/*VS_ObjectFactory::CreateSIPAuthSchemeResult*/std::pair<std::unique_ptr<VS_SIPAuthScheme>, TSIPErrorCodes> CreateAuthScheme(const eSIP_AUTH_SCHEME scheme) const;
	static eSIP_AUTH_SCHEME GetAuthScheme(const char* scheme);
	static const char* GetAuthScheme(const eSIP_AUTH_SCHEME scheme);

	static const char* GetHeader(const VS_SIPObjectFactory::SIPHeader header);
	static VS_SIPObjectFactory::SIPHeader GetHeader(const char* header);

	static eStartLineType GetMethod(const char* method);
	static const char* GetMethod(const eStartLineType method);

	static eSIP_Events GetEvent(const char* ev);
	static const char* GetEvent(const eSIP_Events ev);

	static eSIP_ExtensionPack GetSIPExtension(const char* ext);
	static const char* GetSIPExtension(const eSIP_ExtensionPack ext);

	// Auth -> MD5
	bool CalcDigestResponse(VS_SIPAuthInfo* auth_info);

	bool IsFieldOfType(const VS_BaseField *base_field, const VS_SIPObjectFactory::SIPHeader header);

	virtual ~VS_SIPObjectFactory();

protected:
	VS_SIPObjectFactory();

private:
	TSIPErrorCodes Init();

	bool CalcHA1(const VS_SIPAuthInfo* auth_info, HASHHEX HA1HEX) const;
	bool CalcHA2(const VS_SIPAuthInfo* auth_info, HASHHEX HA2HEX) const;

	static VS_SIPObjectFactory * iThis;

	const char* UnCompactSIPHeaders(const char* compact_header) const;
};