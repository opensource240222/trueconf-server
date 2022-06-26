#include "VS_SIPObjectFactory.h"
#include "VS_SIPField_MaxForwards.h"
#include "VS_SIPField_ContentType.h"
#include "VS_SIPField_StartLine.h"
#include "VS_SIPField_Via.h"
#include "VS_SIPField_CSeq.h"
#include "VS_SIPField_CallID.h"
#include "VS_SIPField_ContentLength.h"
#include "VS_SIPField_Contact.h"
#include "VS_SIPField_From.h"
#include "VS_SIPField_To.h"
#include "VS_SIPField_Route.h"
#include "VS_SIPField_RecordRoute.h"
#include "VS_SIPField_Auth.h"
#include "VS_SIPAuthDigest.h"
#include "VS_SIPField_Expires.h"
#include "VS_SIPField_Event.h"
#include "VS_SIPField_UserAgent.h"
#include "VS_SIPField_Unsupported.h"
#include "VS_SIPField_Require.h"
#include "VS_SIPField_Supported.h"
#include "VS_SIPField_SessionExpires.h"
#include "VS_SIPField_MinSE.h"
#include "VS_SIPField_MinExpires.h"
#include "VS_SIPField_SubscriptionState.h"
#include "VS_SIPField_Allow.h"
#include "VS_SIPField_Accept.h"
#include "VS_SIPField_Date.h"
#include "VS_SIPField_RetryAfter.h"
#include "VS_SIPField_Server.h"
#include "VS_SIPAuthBasic.h"
#include "VS_SIPAuthGSS.h"
#include "std/cpplib/md5.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/clib/strcasecmp.h"
#include "std-generic/compat/memory.h"


#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER
const char MD5Separator[] = ":";

VS_SIPObjectFactory * VS_SIPObjectFactory::iThis = nullptr;
const boost::regex VS_SIPObjectFactory::e(" *([[:word:]-]+) *", boost::regex::icase);

VS_SIPObjectFactory::VS_SIPObjectFactory()
{

}

VS_SIPObjectFactory::~VS_SIPObjectFactory()
{
	if ( iThis )
		iThis = 0;
}

VS_SIPObjectFactory* VS_SIPObjectFactory::Instance()
{
	if ( !iThis )
	{
		iThis = new VS_SIPObjectFactory;

		TSIPErrorCodes res = iThis->Init();

		if ( res != TSIPErrorCodes::e_ok )
		{
			iThis->SetError(res);
			iThis->SetValid(false);
		}
	}

	return iThis;
}

TSIPErrorCodes VS_SIPObjectFactory::Init()
{
// MaxForwards
	TSIPErrorCodes res = AddClass(GetHeader(SIPHeader::SIPHeader_MaxForwards), &VS_SIPField_MaxForwards_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// ContentType
	res = AddClass(GetHeader(SIPHeader::SIPHeader_ContentType), &VS_SIPField_ContentType_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// StartLine
	res = AddClass(GetHeader(SIPHeader::SIPHeader_StartLine), &VS_SIPField_StartLine_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Via
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Via), &VS_SIPField_Via_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// CSeq
	res = AddClass(GetHeader(SIPHeader::SIPHeader_CSeq), &VS_SIPField_CSeq_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// CallID
	res = AddClass(GetHeader(SIPHeader::SIPHeader_CallID), &VS_SIPField_CallID_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// ContentLength
	res = AddClass(GetHeader(SIPHeader::SIPHeader_ContentLength), &VS_SIPField_ContentLength_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Contact
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Contact), &VS_SIPField_Contact_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Date
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Date), &VS_SIPField_Date_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;

// From
	res = AddClass(GetHeader(SIPHeader::SIPHeader_From), &VS_SIPField_From_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// To
	res = AddClass(GetHeader(SIPHeader::SIPHeader_To), &VS_SIPField_To_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Route
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Route), &VS_SIPField_Route_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Record-Route
	res = AddClass(GetHeader(SIPHeader::SIPHeader_RecordRoute), &VS_SIPField_RecordRoute_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Retry-After
	res = AddClass(GetHeader(SIPHeader::SIPHeader_RetryAfter), &VS_SIPField_RetryAfter_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;

// WWW-Authenticate
	res = AddClass(GetHeader(SIPHeader::SIPHeader_WWWAuthenticate), &VS_SIPField_Auth_WWWAuthenticate_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Authorization
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Authorization), &VS_SIPField_Auth_Authorization_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// WWW-Authenticate
	res = AddClass(GetHeader(SIPHeader::SIPHeader_AuthenticationInfo), &VS_SIPField_Auth_AuthenticationInfo_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Proxy-Authenticate
	res = AddClass(GetHeader(SIPHeader::SIPHeader_ProxyAuthenticate), &VS_SIPField_Auth_ProxyAuthenticate_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Proxy-Authorization
	res = AddClass(GetHeader(SIPHeader::SIPHeader_ProxyAuthorization), &VS_SIPField_Auth_ProxyAuthorization_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Expires
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Expires), &VS_SIPField_Expires_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Event
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Event), &VS_SIPField_Event_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// User-Agent
	res = AddClass(GetHeader(SIPHeader::SIPHeader_UserAgent), &VS_SIPField_UserAgent_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Unsupported
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Unsupported), &VS_SIPField_Unsupported_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Require
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Require), &VS_SIPField_Require_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;

// Supported
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Supported), &VS_SIPField_Supported_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Session-Expires
	res = AddClass(GetHeader(SIPHeader::SIPHeader_SessionExpires), &VS_SIPField_SessionExpires_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Min-SE
	res = AddClass(GetHeader(SIPHeader::SIPHeader_MinSE), &VS_SIPField_MinSE_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Min-Expires
	res = AddClass(GetHeader(SIPHeader::SIPHeader_MinExpires), &VS_SIPField_MinExpires_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Subscription-State
	res = AddClass(GetHeader(SIPHeader::SIPHeader_SubscriptionState), &VS_SIPField_SubscriptionState_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Allow
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Allow), &VS_SIPField_Allow_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;

// Accept
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Accept), &VS_SIPField_Accept_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;

// Server
	res = AddClass(GetHeader(SIPHeader::SIPHeader_Server), &VS_SIPField_Server_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;

	//TODO: Добавить все филды VS_SIP...Field

	return res;
}

VS_ObjectFactory::CreateFieldResult VS_SIPObjectFactory::CreateField(VS_SIPBuffer &aBuffer) const
{
	std::unique_ptr<const char[]> data;
	std::size_t ptr_sz = 0;
	bool bad_str = false; // TANDBERG fix;

	const TSIPErrorCodes err = aBuffer.GetHeaderAllocConst(":", data, ptr_sz);
	const char *ptr = data.get();
	if ((ptr) && (*ptr == ' ')) { ptr++; bad_str = true; }
	if (TSIPErrorCodes::e_ok != err)
	{
		if (ptr)
		{
			if (bad_str)
			{
				ptr--;
			}
			ptr = 0;
		}
		ptr_sz = 0;
		return CreateFieldResult(nullptr, err);
	}

	boost::cmatch m;
	std::string header("STARTLINE");
	try
	{
		if (boost::regex_match(ptr, m, e))
			header = m[1];
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodec::Decode error " << ex.what() << "\n";
	}
	if ( ptr )
	{
		if (bad_str)
			ptr--;
		ptr = 0;
	}
	ptr_sz = 0;

	header = UnCompactSIPHeaders( header.c_str() );

	GetInstanceResult get_inst_res = GetInstance(header.c_str());
	if (TSIPErrorCodes::e_ok != get_inst_res.error_code )
		return CreateFieldResult(nullptr, get_inst_res.error_code);

	return CreateFieldResult((*(get_inst_res.instance))(), TSIPErrorCodes::e_ok);
}

const char* VS_SIPObjectFactory::GetHeader(const VS_SIPObjectFactory::SIPHeader header)
{
	switch (header)
	{
	case SIPHeader::SIPHeader_StartLine:				return "STARTLINE";
	case SIPHeader::SIPHeader_Via:						return "VIA";
	case SIPHeader::SIPHeader_CSeq:						return "CSEQ";
	case SIPHeader::SIPHeader_To:						return "TO";
	case SIPHeader::SIPHeader_From:						return "FROM";
	case SIPHeader::SIPHeader_Contact:					return "CONTACT";
	case SIPHeader::SIPHeader_CallID:					return "CALL-ID";
	case SIPHeader::SIPHeader_Date:						return "DATE";
	case SIPHeader::SIPHeader_Expires:					return "EXPIRES";
	case SIPHeader::SIPHeader_ContentLength:			return "CONTENT-LENGTH";
	case SIPHeader::SIPHeader_ContentType:				return "CONTENT-TYPE";
	case SIPHeader::SIPHeader_MaxForwards:				return "MAX-FORWARDS";
	case SIPHeader::SIPHeader_WWWAuthenticate:			return "WWW-Authenticate";
	case SIPHeader::SIPHeader_Authorization:			return "Authorization";
	case SIPHeader::SIPHeader_AuthenticationInfo:		return "Authentication-Info";
	case SIPHeader::SIPHeader_ProxyAuthenticate:		return "Proxy-Authenticate";
	case SIPHeader::SIPHeader_ProxyAuthorization:		return "Proxy-Authorization";
	case SIPHeader::SIPHeader_Event:					return "EVENT";
	case SIPHeader::SIPHeader_UserAgent:				return "USER-AGENT";
	case SIPHeader::SIPHeader_Require:					return "REQUIRE";
	case SIPHeader::SIPHeader_Unsupported:				return "UNSUPPORTED";
	case SIPHeader::SIPHeader_Supported:				return "SUPPORTED";
	case SIPHeader::SIPHeader_SessionExpires:			return "SESSION-EXPIRES";
	case SIPHeader::SIPHeader_MinSE:					return "MIN-SE";
	case SIPHeader::SIPHeader_MinExpires:				return "MIN-EXPIRES";
	case SIPHeader::SIPHeader_SubscriptionState:		return "SUBSCRIPTION-STATE";
	case SIPHeader::SIPHeader_Allow:					return "ALLOW";
	case SIPHeader::SIPHeader_Accept:					return "ACCEPT";
	case SIPHeader::SIPHeader_RecordRoute:				return "RECORD-ROUTE";
	case SIPHeader::SIPHeader_Route:					return "ROUTE";
	case SIPHeader::SIPHeader_RetryAfter:				return "RETRY-AFTER";
	case SIPHeader::SIPHeader_Server:					return "SERVER";
	default:											return nullptr;
	}
}

VS_SIPObjectFactory::SIPHeader VS_SIPObjectFactory::GetHeader(const char* header)
{
	if ( !header )
		return SIPHeader::SIPHeader_Invalid;

	if (!strcasecmp(header, "STARTLINE"))
		return SIPHeader::SIPHeader_StartLine;
	if (!strcasecmp(header, "VIA"))
		return SIPHeader::SIPHeader_Via;
	if (!strcasecmp(header, "CSEQ"))
		return SIPHeader::SIPHeader_CSeq;
	if (!strcasecmp(header, "DATE"))
		return SIPHeader::SIPHeader_Date;
	if (!strcasecmp(header, "TO"))
		return SIPHeader::SIPHeader_To;
	if (!strcasecmp(header, "FROM"))
		return SIPHeader::SIPHeader_From;
	if (!strcasecmp(header, "CONTACT"))
		return SIPHeader::SIPHeader_Contact;
	if (!strcasecmp(header, "CALL-ID"))
		return SIPHeader::SIPHeader_CallID;
	if (!strcasecmp(header, "EXPIRES"))
		return SIPHeader::SIPHeader_Expires;
	if (!strcasecmp(header, "CONTENT-LENGTH"))
		return SIPHeader::SIPHeader_ContentLength;
	if (!strcasecmp(header, "CONTENT-TYPE"))
		return SIPHeader::SIPHeader_ContentType;
	if (!strcasecmp(header, "MAX-FORWARDS"))
		return SIPHeader::SIPHeader_MaxForwards;
	if (!strcasecmp(header, "WWW-AUTHENTICATE"))
		return SIPHeader::SIPHeader_WWWAuthenticate;
	if (!strcasecmp(header, "AUTHORIZATION"))
		return SIPHeader::SIPHeader_Authorization;
	if (!strcasecmp(header, "AUTHENTICATION-INFO"))
		return SIPHeader::SIPHeader_AuthenticationInfo;
	if (!strcasecmp(header, "PROXY-AUTHENTICATE"))
		return SIPHeader::SIPHeader_ProxyAuthenticate;
	if (!strcasecmp(header, "PROXY-AUTHORIZATION"))
		return SIPHeader::SIPHeader_ProxyAuthorization;
	if (!strcasecmp(header, "EVENT"))
		return SIPHeader::SIPHeader_Event;
	if (!strcasecmp(header, "USER-AGENT"))
		return SIPHeader::SIPHeader_UserAgent;
	if (!strcasecmp(header, "REQUIRE"))
		return SIPHeader::SIPHeader_Require;
	if (!strcasecmp(header, "UNSUPPORTED"))
		return SIPHeader::SIPHeader_Unsupported;
	if (!strcasecmp(header, "SUPPORTED"))
		return SIPHeader::SIPHeader_Supported;
	if (!strcasecmp(header, "SESSION-EXPIRES"))
		return SIPHeader::SIPHeader_SessionExpires;
	if (!strcasecmp(header, "MIN-SE"))
		return SIPHeader::SIPHeader_MinSE;
	if (!strcasecmp(header, "MIN-EXPIRES"))
		return SIPHeader::SIPHeader_MinExpires;
	if (!strcasecmp(header, "SUBSCRIPTION-STATE"))
		return SIPHeader::SIPHeader_SubscriptionState;
	if (!strcasecmp(header, "ALLOW"))
		return SIPHeader::SIPHeader_Allow;
	if (!strcasecmp(header, "ACCEPT"))
		return SIPHeader::SIPHeader_Accept;
	if (!strcasecmp(header, "RECORD-ROUTE"))
		return SIPHeader::SIPHeader_RecordRoute;
	if (!strcasecmp(header, "ROUTE"))
		return SIPHeader::SIPHeader_Route;
	if (!strcasecmp(header, "SERVER"))
		return SIPHeader::SIPHeader_Server;

	return SIPHeader::SIPHeader_Invalid;
}

VS_ObjectFactory::CreateFieldResult VS_SIPObjectFactory::CreateField(VS_SIPObjectFactory::SIPHeader header) const
{
	const char* h = this->GetHeader(header);

	if (!h)
		return CreateFieldResult(nullptr, TSIPErrorCodes::e_InputParam);

	return VS_ObjectFactory::CreateField(h);
}

const char* VS_SIPObjectFactory::UnCompactSIPHeaders(const char* compact_header) const
{
	if (!strcasecmp(compact_header, "I")) {
		return GetHeader(SIPHeader::SIPHeader_CallID);
	}
	if(!strcasecmp(compact_header, "M")) {
		return GetHeader(SIPHeader::SIPHeader_Contact);
	}
	if(!strcasecmp(compact_header, "L")) {
		return GetHeader(SIPHeader::SIPHeader_ContentLength);
	}
	if(!strcasecmp(compact_header, "C")) {
		return GetHeader(SIPHeader::SIPHeader_ContentType);
	}
	if(!strcasecmp(compact_header, "F")) {
		return GetHeader(SIPHeader::SIPHeader_From);
	}
	if(!strcasecmp(compact_header, "T")) {
		return GetHeader(SIPHeader::SIPHeader_To);
	}
	if (!strcasecmp(compact_header, "V")) {
		return GetHeader(SIPHeader::SIPHeader_Via);
	}
	if (!strcasecmp(compact_header, "K")) {
		return GetHeader(SIPHeader::SIPHeader_Supported);
	}
	if (!strcasecmp(compact_header, "X")) {
		return GetHeader(SIPHeader::SIPHeader_Expires);
	}
	if (!strcasecmp(compact_header, "O")) {
		return GetHeader(SIPHeader::SIPHeader_Event);
	}
	return compact_header;
}

eStartLineType VS_SIPObjectFactory::GetMethod(const char* method)
{
	if ( !method )
		return TYPE_INVALID;

	if (!strcasecmp(method, "INVITE"))
		return TYPE_INVITE;
	if (!strcasecmp(method, "ACK"))
		return TYPE_ACK;
	if (!strcasecmp(method, "BYE"))
		return TYPE_BYE;
	if (!strcasecmp(method, "CANCEL"))
		return TYPE_CANCEL;
	if (!strcasecmp(method, "REGISTER"))
		return TYPE_REGISTER;
	if (!strcasecmp(method, "INFO"))
		return TYPE_INFO;
	if (!strcasecmp(method, "SUBSCRIBE"))
		return TYPE_SUBSCRIBE;
	if (!strcasecmp(method, "NOTIFY"))
		return TYPE_NOTIFY;
	if (!strcasecmp(method, "UPDATE"))
		return TYPE_UPDATE;
	if (!strcasecmp(method, "OPTIONS"))
		return TYPE_OPTIONS;
	if (!strcasecmp(method, "PUBLISH"))
		return TYPE_PUBLISH;
	if (!strcasecmp(method, "MESSAGE"))
		return TYPE_MESSAGE;
	return TYPE_INVALID;
}

const char* VS_SIPObjectFactory::GetMethod(const eStartLineType method)
{
	if ( method == TYPE_INVITE )
		return "INVITE";
	if ( method == TYPE_ACK )
		return "ACK";
	if ( method == TYPE_BYE )
		return "BYE";
	if ( method == TYPE_CANCEL )
		return "CANCEL";
	if ( method == TYPE_REGISTER )
		return "REGISTER";
	if ( method == TYPE_INFO )
		return "INFO";
	if ( method == TYPE_SUBSCRIBE )
		return "SUBSCRIBE";
	if ( method == TYPE_NOTIFY )
		return "NOTIFY";
	if ( method == TYPE_UPDATE )
		return "UPDATE";
	if (method == TYPE_OPTIONS)
		return "OPTIONS";
	if (method == TYPE_PUBLISH)
		return "PUBLISH";
	if (method == TYPE_MESSAGE)
		return "MESSAGE";
	return nullptr;
}

eSIP_ExtensionPack VS_SIPObjectFactory::GetSIPExtension(const char* ext)
{
	if ( !ext )
		return SIP_EXTENSION_INVALID;

	if (!strcasecmp(ext, "TIMER"))
		return SIP_EXTENSION_TIMER;
	if (!strcasecmp(ext, "100REL"))
		return SIP_EXTENSION_100REL;
	if (!strcasecmp(ext, "REPLACES"))
		return SIP_EXTENSION_REPLACES;
	if (!strcasecmp(ext, "PATH"))
		return SIP_EXTENSION_PATH;
	return SIP_EXTENSION_INVALID;
}

const char* VS_SIPObjectFactory::GetSIPExtension(const eSIP_ExtensionPack ext)
{
	if ( ext == SIP_EXTENSION_TIMER )
		return "timer";
	if (ext == SIP_EXTENSION_100REL )
		return "100rel";
	if (ext == SIP_EXTENSION_REPLACES )
		return "replaces";
	if (ext == SIP_EXTENSION_PATH)
		return "path";
	if (ext == SIP_EXTENSION_GRUU)
		return "gruu-10";
	return nullptr;
}

eSIP_Events VS_SIPObjectFactory::GetEvent(const char* ev)
{
	if ( !ev )
		return SIP_EVENT_INVALID;

	if (!strcasecmp(ev, "PRESENCE"))
		return SIP_EVENT_PRESENCE;
	return SIP_EVENT_INVALID;
}

const char* VS_SIPObjectFactory::GetEvent(const eSIP_Events ev)
{
	if ( ev == SIP_EVENT_PRESENCE )
		return "PRESENCE";
	return nullptr;
}

std::pair<std::unique_ptr<VS_SIPAuthScheme>, TSIPErrorCodes> VS_SIPObjectFactory::CreateAuthScheme(eSIP_AUTH_SCHEME scheme) const
{
	if ( scheme == SIP_AUTHSCHEME_BASIC ) {
		return std::make_pair(vs::make_unique<VS_SIPAuthBasic>(), TSIPErrorCodes::e_ok);
	}
	if ( scheme == SIP_AUTHSCHEME_DIGEST ) {
		return std::make_pair(vs::make_unique<VS_SIPAuthDigest>(), TSIPErrorCodes::e_ok);
	}
	if ( scheme == SIP_AUTHSCHEME_NTLM ||
		scheme == SIP_AUTHSCHEME_KERBEROS ||
		scheme == SIP_AUTHSCHEME_TLS_DSK) {
		return std::make_pair(vs::make_unique<VS_SIPAuthGSS>(scheme), TSIPErrorCodes::e_ok);
	}

	return std::make_pair(nullptr, TSIPErrorCodes::e_bad);
}

eSIP_AUTH_SCHEME VS_SIPObjectFactory::GetAuthScheme(const char* scheme)
{
	if (!scheme)
		return SIP_AUTHSCHEME_INVALID;

	if (!strcasecmp(scheme, "BASIC")) {
		return SIP_AUTHSCHEME_BASIC;
	}
	if (!strcasecmp(scheme, "DIGEST")) {
		return SIP_AUTHSCHEME_DIGEST;
	}
	if (!strcasecmp(scheme, "NTLM")) {
		return SIP_AUTHSCHEME_NTLM;
	}
	if (!strcasecmp(scheme, "Kerberos")) {
		return SIP_AUTHSCHEME_KERBEROS;
	}
	if (!strcasecmp(scheme, "TLS-DSK")) {
		return SIP_AUTHSCHEME_TLS_DSK;
	}

	return SIP_AUTHSCHEME_INVALID;
}

const char* VS_SIPObjectFactory::GetAuthScheme(const eSIP_AUTH_SCHEME scheme)
{
	if ( scheme == SIP_AUTHSCHEME_BASIC ) {
		return "Basic";
	}
	if ( scheme == SIP_AUTHSCHEME_DIGEST ) {
		return "Digest";
	}
	if ( scheme == SIP_AUTHSCHEME_NTLM ) {
		return "NTLM";
	}
	if ( scheme == SIP_AUTHSCHEME_KERBEROS ) {
		return "Kerberos";
	}
	if ( scheme == SIP_AUTHSCHEME_TLS_DSK ) {
		return "TLS-DSK";
	}
	return nullptr;
}

bool VS_SIPObjectFactory::CalcHA1(const VS_SIPAuthInfo* auth_info, HASHHEX HA1HEX) const
{
	if ( !auth_info )
		return false;

	{
		MD5 md5;
		md5.Update(auth_info->login());
		md5.Update(MD5Separator);
		md5.Update(auth_info->realm());
		md5.Update(MD5Separator);
		md5.Update(auth_info->password());
		md5.Final();
		md5.GetString(HA1HEX, false);
	}

// Если алгоритм "MD5-Sess", тогда обновить значение HA1HEX
	if ( auth_info->algorithm() == SIP_AAA_ALGORITHM_MD5_SESS )
	{
		MD5 md5;
		md5.Update(HA1HEX, HASHHEXLEN);
		md5.Update(MD5Separator);
		md5.Update(auth_info->nonce());
		md5.Update(MD5Separator);
		md5.Update(auth_info->cnonce());
		md5.Final();
		md5.GetString(HA1HEX, false);
	}

	return true;
}

bool VS_SIPObjectFactory::CalcHA2(const VS_SIPAuthInfo* auth_info, HASHHEX HA2HEX) const
{
	if ( !auth_info )
		return false;

	MD5 md5;
	md5.Update(auth_info->method());
	md5.Update(MD5Separator);
	md5.Update(auth_info->uri());

	if ( auth_info->qop() == SIP_AAA_QOP_AUTH_INT )
	{
		// TODO: Implement
		return false;

//		char* PtrContent = 0;
//		unsigned int ContentSize = (unsigned int) aContent->GetWriteIndex();
//
//		if ( ContentSize > 0 )
//		{
//			if ( e_ok != aContent->GetDataAllocConst(PtrContent, ContentSize) )
//				return false;
//		}else{
//			PtrContent = "";
//			ContentSize = 1;
//		}
//
//		MD5Context ctx2;
//		HASH EntityMD5;
//		HASHHEX HexEntityMD5;
//
//		MD5Init(&ctx2);
//		MD5Update(&ctx2, (unsigned char*) PtrContent, ContentSize);
//		MD5Final(EntityMD5, &ctx);
//
//		ConvertHex(EntityMD5, HexEntityMD5);
//
//		MD5Update(&ctx, MD5Separator, (unsigned int) strlen((const char*) MD5Separator));
//		MD5Update(&ctx, (unsigned char*) HexEntityMD5, (unsigned int) strlen((char*) &HexEntityMD5));
	}

	md5.Final();
	md5.GetString(HA2HEX, false);

	return true;
}

bool VS_SIPObjectFactory::CalcDigestResponse(VS_SIPAuthInfo* auth_info)
{
	if ( !auth_info )
		return false;

	HASHHEX HA1HEX = {0};
	HASHHEX HA2HEX = {0};

	if ( !CalcHA1(auth_info, HA1HEX) )
		return false;

	if ( !CalcHA2(auth_info, HA2HEX) )
		return false;

	MD5 md5;

	{
		md5.Update(HA1HEX, HASHHEXLEN);
		md5.Update(MD5Separator);
		md5.Update(auth_info->nonce());
		md5.Update(MD5Separator);
	}

	if ( auth_info->qop() > SIP_AAA_QOP_INVALID )
	{
		if ( !auth_info->nc() )
			return false;

		std::string nc;

		char ch[(sizeof(decltype(auth_info->nc())) * 2) + 1] = {};
		snprintf(ch, sizeof ch, "%X", auth_info->nc());

		for(unsigned int i=0; i < (8 - strlen(ch)); i++)		// add "0"
			nc += "0";
		nc += ch;

		std::string qop;
		switch( auth_info->qop() )
		{
		case SIP_AAA_QOP_AUTH_INT:
			qop = "auth-int";
			break;

		case SIP_AAA_QOP_AUTH:
		default:
			qop = "auth";
			break;
		}

		md5.Update(nc);
		md5.Update(MD5Separator);
		md5.Update(auth_info->cnonce());
		md5.Update(MD5Separator);
		md5.Update(qop);
		md5.Update(MD5Separator);
	}

	md5.Update(HA2HEX, HASHHEXLEN);
	md5.Final();
	HASHHEX RESPONSEHEX;
	md5.GetString(RESPONSEHEX, false);

	auth_info->response(RESPONSEHEX);

	return true;
}

bool VS_SIPObjectFactory::IsFieldOfType(const VS_BaseField *base_field, VS_SIPObjectFactory::SIPHeader header) {
	const char *h = GetHeader(header);

	GetInstanceResult get_inst_res = GetInstance(h);
	if (TSIPErrorCodes::e_ok != get_inst_res.error_code) {
		return false;
	}

	bool res = typeid(*((*(get_inst_res.instance))())) == typeid(*base_field);

	return res;
}

#undef DEBUG_CURRENT_MODULE
