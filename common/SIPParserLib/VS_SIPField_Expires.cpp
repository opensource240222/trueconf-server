#include "VS_SIPField_Expires.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"
#include <string>
#include "VS_SIPGetInfoInterface.h"
#include <cinttypes>

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_Expires::e(
		"(?i)"
		" *(?:expires) *: *"
		"(\\d+) *"
		"(?-i)"
	);

VS_SIPField_Expires::VS_SIPField_Expires():
	iValue(0)
{
	VS_SIPError::Clean();
}

VS_SIPField_Expires::~VS_SIPField_Expires()
{
}

TSIPErrorCodes VS_SIPField_Expires::Decode(VS_SIPBuffer &aBuffer)
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
		dstream1 << "VS_SIPField_Expires::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Expires Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	const std::string &value = m[1];

	iValue = std::chrono::seconds(atoi(value.c_str()));

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Expires::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	aBuffer.AddData("Expires: ");

	char theValue[std::numeric_limits<uint64_t>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(theValue, sizeof theValue, "%" PRIu64, static_cast<uint64_t>(iValue.count()));
	aBuffer.AddData(theValue, strlen(theValue));

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Expires::Init(const VS_SIPGetInfoInterface& call)
{
	iValue = call.GetExpires();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

std::chrono::seconds VS_SIPField_Expires::Value() const
{
	return iValue;
}

void VS_SIPField_Expires::Value(std::chrono::seconds value)
{
	iValue = value;
}

std::unique_ptr<VS_BaseField> VS_SIPField_Expires_Instance()
{
	return vs::make_unique<VS_SIPField_Expires>();
}

void VS_SIPField_Expires::Clean() noexcept
{
	VS_SIPError::Clean();

	iValue = std::chrono::seconds(0);
}

#undef DEBUG_CURRENT_MODULE
