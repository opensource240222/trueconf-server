#include "VS_SIPField_Auth.h"
#include "VS_RTSP_ParserInfo.h"
#include "VS_SIPAuthScheme.h"
#include "VS_SIPAuthDigest.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"
#include <string>
#include "VS_SIPGetInfoInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_Auth::e(
		"(?i)"
			"("
				"([\\w-]+) *: *"			// Header
				"([\\w-]+) +"				// Scheme
			")+"
			"(?:.*)"					// Data for Scheme
		"(?-i) *"
	);

VS_SIPField_Auth::VS_SIPField_Auth():
	m_header(VS_SIPObjectFactory::SIPHeader::SIPHeader_Invalid), m_auth_scheme(0), m_scheme(SIP_AUTHSCHEME_INVALID)
{
	VS_SIPField_Auth::Clean();
}

VS_SIPField_Auth::VS_SIPField_Auth(VS_SIPObjectFactory::SIPHeader header):
	m_header(VS_SIPObjectFactory::SIPHeader::SIPHeader_Invalid), m_auth_scheme(0), m_scheme(SIP_AUTHSCHEME_INVALID)
{
	VS_SIPField_Auth::Clean();
	m_header = header;
	m_isRTSP = false;
}

VS_SIPField_Auth::~VS_SIPField_Auth()
{
}

TSIPErrorCodes VS_SIPField_Auth::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = aBuffer.GetNextBlockAllocConst(ptr, ptr_sz);
	if ( (TSIPErrorCodes::e_ok != err) || !ptr || !ptr_sz )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	boost::cmatch m;
	bool regex_match_res;
	try
	{
		regex_match_res = boost::regex_match(ptr.get(), m, e);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Auth::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Auth Field: buffer not match, dump |" << ptr.get() << "|";
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	const std::string &skip_string = m[1];
	std::string header = m[2];
	std::string scheme = m[3];

	ptr = nullptr;
	ptr_sz = 0;

	// UpperCase
	std::transform(header.begin(), header.end(), header.begin(), toupper);
	std::transform(scheme.begin(), scheme.end(), scheme.begin(), toupper);

	VS_SIPObjectFactory* factory = VS_SIPObjectFactory::Instance();
	assert(factory);

	m_header = static_cast<VS_SIPObjectFactory::SIPHeader>(VS_SIPObjectFactory::GetHeader(header.c_str()));

	const eSIP_AUTH_SCHEME x = VS_SIPObjectFactory::GetAuthScheme( scheme.c_str() );
	auto field = factory->CreateAuthScheme(x);

	err = field.second;
	if ( (TSIPErrorCodes::e_ok != err) || !field.first)
	{
		dstream3 << "[SIPParserLib::Error] Auth Field: auth scheme not supported";
		SetValid(false);
		SetError(err);
		return err;
	}

	m_auth_scheme = std::move(field.first);

	err = aBuffer.Skip( skip_string.length() );
	if (TSIPErrorCodes::e_ok != err )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	err = m_auth_scheme->Decode(aBuffer);
	if (TSIPErrorCodes::e_ok != err )
	{
		dstream3 << "[SIPParserLib::Error] Auth Field: auth scheme not decoded";
		SetValid(false);
		SetError(err);
		return err;
	}

	m_scheme = x;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Auth::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	const char* header = VS_SIPObjectFactory::GetHeader(m_header);
	const char* scheme = VS_SIPObjectFactory::GetAuthScheme(m_scheme);

	if ( !header || !scheme )
		return TSIPErrorCodes::e_InputParam;

	if ( !m_auth_scheme )
		return TSIPErrorCodes::e_InputParam;

	VS_SIPBuffer tmp_buffer;
	TSIPErrorCodes err = m_auth_scheme->Encode(tmp_buffer);
	if (TSIPErrorCodes::e_ok != err)
		return err;

	std::unique_ptr<char[]> data;
	std::size_t data_sz = 0;

	err = tmp_buffer.GetAllDataAllocConst(data, data_sz);
	if ( (TSIPErrorCodes::e_ok != err) || !data || !data_sz )
	{
		return err;
	}

	aBuffer.AddData(header, std::strlen(header));
	aBuffer.AddData(": ");
	aBuffer.AddData(scheme, std::strlen(scheme));
	aBuffer.AddData(" ");
	aBuffer.AddData(data.get(), data_sz);
	if (m_isRTSP)
		aBuffer.AddData("\r\n");

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Auth::Init(const VS_SIPGetInfoInterface& call)
{
	m_isRTSP = false;
	const auto auth = call.GetAuthScheme();
	if ( !auth )
		return TSIPErrorCodes::e_InputParam;

	m_auth_scheme = auth;
	m_scheme = auth->scheme();

	const TSIPErrorCodes err = m_auth_scheme->Init(call);

	SetValid(true);
	SetError(err);
	return err;
}

TSIPErrorCodes VS_SIPField_Auth::Init( VS_RTSP_ParserInfo* call )
{
	m_isRTSP = true;
	const auto auth = call->GetAuthScheme();
	if ( !auth )
		return TSIPErrorCodes::e_InputParam;

	m_auth_scheme = auth;
	m_scheme = auth->scheme();

	const TSIPErrorCodes err = m_auth_scheme->Init(call);

	SetValid(true);
	SetError(err);
	return err;
}

std::shared_ptr<VS_SIPAuthInfo> VS_SIPField_Auth::GetAuthInfo() const
{
	return m_auth_scheme;
}

void VS_SIPField_Auth::Clean() noexcept
{
	VS_SIPError::Clean();

	m_header = VS_SIPObjectFactory::SIPHeader::SIPHeader_Invalid;
	m_auth_scheme = 0;
	m_scheme = SIP_AUTHSCHEME_INVALID;
	m_isRTSP = false;
}

std::unique_ptr<VS_BaseField> VS_SIPField_Auth_WWWAuthenticate_Instance()
{
	return vs::make_unique<VS_SIPField_Auth>( VS_SIPObjectFactory::SIPHeader::SIPHeader_WWWAuthenticate);
}

std::unique_ptr<VS_BaseField> VS_SIPField_Auth_Authorization_Instance()
{
	return vs::make_unique<VS_SIPField_Auth>( VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization);
}

std::unique_ptr<VS_BaseField> VS_SIPField_Auth_AuthenticationInfo_Instance()
{
	return vs::make_unique<VS_SIPField_Auth>(VS_SIPObjectFactory::SIPHeader::SIPHeader_AuthenticationInfo);
}

std::unique_ptr<VS_BaseField> VS_SIPField_Auth_ProxyAuthenticate_Instance()
{
	return vs::make_unique<VS_SIPField_Auth>(VS_SIPObjectFactory::SIPHeader::SIPHeader_ProxyAuthenticate);
}

std::unique_ptr<VS_BaseField> VS_SIPField_Auth_ProxyAuthorization_Instance()
{
	return vs::make_unique<VS_SIPField_Auth>( VS_SIPObjectFactory::SIPHeader::SIPHeader_ProxyAuthorization );
}

#undef DEBUG_CURRENT_MODULE
