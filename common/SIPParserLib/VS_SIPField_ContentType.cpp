#include "VS_SIPField_ContentType.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "VS_SIPGetInfoInterface.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_ContentType::e(
	"(?i)"
	" *(?:content-type|c) *: *" // TANDBERG fix
	"([-\\w/+]+) *"
	"(;{0,1} *(?:charset|boundary) *= *[-\\w/=\".+]+){0,1}"	// ;charset/boundary=value
	"(?-i)"
	);

VS_SIPField_ContentType::VS_SIPField_ContentType():
	iContentType(CONTENTTYPE_INVALID),
	compact(false)
{
	VS_SIPError::Clean();
}

VS_SIPField_ContentType::~VS_SIPField_ContentType()
{
}

TSIPErrorCodes VS_SIPField_ContentType::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	if ( !ptr || !ptr_sz )
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_buffer);
		return TSIPErrorCodes::e_buffer;
	}

	std::string value;
	boost::cmatch m;
	try
	{
		if (boost::regex_match(ptr.get(), m, e))
			value = m[1];
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_ContentType::Decode error " << ex.what() << "\n";
	}

	// UpperCase
	std::transform(value.begin(), value.end(), value.begin(), toupper);

	if (value == "APPLICATION/SDP")
		iContentType = CONTENTTYPE_SDP;
	else if(value == "APPLICATION/MEDIA_CONTROL+XML")
		iContentType = CONTENTTYPE_MEDIACONTROL_XML;
	else if(value == "APPLICATION/PIDF+XML")
		iContentType = CONTENTTYPE_PIDF_XML;
	else if(value == "APPLICATION/DTMF-RELAY")
		iContentType = CONTENTTYPE_DTMF_RELAY;
	else if (value == "BFCP/BINARY")
		iContentType = CONTENTTYPE_BFCP;
	else if (value == "TEXT/PLAIN" || value == "TEXT/HTML")
		iContentType = CONTENTTYPE_TEXT_PLAIN;
	else if (value == "MULTIPART/ALTERNATIVE")
		iContentType = CONTENTTYPE_MULTIPART;
	else
		iContentType = CONTENTTYPE_UNKNOWN;

	SetError(TSIPErrorCodes::e_ok);
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_ContentType::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if (compact)
		aBuffer.AddData("c: ");
	else
		aBuffer.AddData("Content-Type: ");

	switch(iContentType)
	{
	case CONTENTTYPE_SDP:
		aBuffer.AddData("application/sdp");
		break;
	case CONTENTTYPE_MEDIACONTROL_XML:
		aBuffer.AddData("application/media_control+xml");
		break;
	case CONTENTTYPE_PIDF_XML:
		aBuffer.AddData("application/pidf+xml");
		break;
	case CONTENTTYPE_DTMF_RELAY:
		aBuffer.AddData("application/dtmf-relay");
		break;
	case CONTENTTYPE_BFCP:
		aBuffer.AddData("BFCP/binary");
		break;
	case CONTENTTYPE_TEXT_PLAIN:
		aBuffer.AddData("text/plain");
		break;
	case CONTENTTYPE_TEXT_RTF:
		aBuffer.AddData("text/rtf");
		break;
	default:
			return TSIPErrorCodes::e_UNKNOWN;
		break;
	}

	return TSIPErrorCodes::e_ok;
}

eContentType VS_SIPField_ContentType::GetContentType() const
{
	return iContentType;
}

std::unique_ptr<VS_BaseField> VS_SIPField_ContentType_Instance()
{
	return vs::make_unique<VS_SIPField_ContentType>();
}

TSIPErrorCodes VS_SIPField_ContentType::Init(const VS_SIPGetInfoInterface& call)
{
	iContentType = call.GetContentType();
	compact = call.IsCompactHeaderAllowed();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_ContentType::Clean() noexcept
{
	VS_SIPError::Clean();

	iContentType = CONTENTTYPE_INVALID;
}

#undef DEBUG_CURRENT_MODULE
