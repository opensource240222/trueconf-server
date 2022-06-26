#include "VS_SDPField_Version.h"
#include "VS_SDPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPField_Version::e(" *(?i)(?:v)(?-i) *= *([0-9]+) *");

VS_SDPField_Version::VS_SDPField_Version()
{
	VS_SDPField_Version::Clean();
}

VS_SDPField_Version::~VS_SDPField_Version()
{
	VS_SDPField_Version::Clean();
}

TSIPErrorCodes VS_SDPField_Version::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

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

	boost::cmatch m;
	bool regex_match_res;
	try
	{
		regex_match_res = boost::regex_match(ptr.get(), m, e);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPField_Version::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::SDPError] Version Field: buffer not match, dump |" << ptr.get() << "|";
		aBuffer.SkipHeader();
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	iValue = atoi( static_cast<const std::string &>(m[1]).c_str() );

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPField_Version::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	char val[std::numeric_limits<unsigned int>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(val, sizeof val, "%u", iValue);

	aBuffer.AddData("v=");
	aBuffer.AddData(val, strlen(val));
	aBuffer.AddData("\r\n");

	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_SDPField_Version_Instance()
{
	return vs::make_unique<VS_SDPField_Version>();
}

TSIPErrorCodes VS_SDPField_Version::Init(const VS_SIPGetInfoInterface& call)
{

	iValue = 0;			// Vesion: 0	(default)

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SDPField_Version::Clean() noexcept
{
	VS_SIPError::Clean();

	iValue = 0;
}

#undef DEBUG_CURRENT_MODULE
