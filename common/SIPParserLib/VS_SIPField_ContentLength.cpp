#include "VS_SIPField_ContentLength.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "VS_SIPGetInfoInterface.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_ContentLength::e(
		"(?i)"
		" *(?:content-length|l) *: *"
		"(\\d+) *"
		"(?-i)"
	);

VS_SIPField_ContentLength::VS_SIPField_ContentLength():
	iValue(0)
	, compact(false)
{
	VS_SIPError::Clean();
}

VS_SIPField_ContentLength::~VS_SIPField_ContentLength()
{
}

TSIPErrorCodes VS_SIPField_ContentLength::Decode(VS_SIPBuffer &aBuffer)
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

	boost::cmatch m;
	bool regex_match_res;
	try
	{
		regex_match_res = boost::regex_match(ptr.get(), m, e);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_ContentLength::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] ContentLength Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	const std::string &value = m[1];

	iValue = atoi( value.c_str() );

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_ContentLength::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if (compact)
		aBuffer.AddData("l: ");
	else
		aBuffer.AddData("Content-Length: ");

	char theValue[std::numeric_limits<unsigned int>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(theValue, sizeof theValue, "%u", iValue);

	aBuffer.AddData(theValue, strlen(theValue));

	return TSIPErrorCodes::e_ok;
}

unsigned int VS_SIPField_ContentLength::Value() const
{
	return iValue;
}

void VS_SIPField_ContentLength::Value(unsigned int value)
{
	iValue = value;
}
std::unique_ptr<VS_BaseField> VS_SIPField_ContentLength_Instance()
{
	return vs::make_unique<VS_SIPField_ContentLength>();
}
TSIPErrorCodes VS_SIPField_ContentLength::Init(const VS_SIPGetInfoInterface& call)
{
	iValue = 0;
	compact = call.IsCompactHeaderAllowed();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_ContentLength::Clean() noexcept
{
	VS_SIPError::Clean();

	iValue = 0;
}

#undef DEBUG_CURRENT_MODULE
