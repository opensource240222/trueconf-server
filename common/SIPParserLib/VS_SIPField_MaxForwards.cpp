#include "VS_SIPField_MaxForwards.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include <memory>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_MaxForwards::e(
		"(?i)"
		" *(?:max-forwards) *: *"
		"(.*) *"
		"(?-i)"
	);

VS_SIPField_MaxForwards::VS_SIPField_MaxForwards():
	iValue(0)
{
	VS_SIPError::Clean();
}

VS_SIPField_MaxForwards::~VS_SIPField_MaxForwards()
{
}

TSIPErrorCodes VS_SIPField_MaxForwards::Decode(VS_SIPBuffer& aBuffer)
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
		dstream1 << "VS_SIPField_MaxForwards::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Max-Forwards Field: buffer not match, dump |" << ptr.get() << "|";
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	const std::string &value = m[1];

	iValue = atoi(value.c_str());

	SetError(TSIPErrorCodes::e_ok);
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_MaxForwards::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	aBuffer.AddData("Max-Forwards: ");

	char theValue[std::numeric_limits<unsigned int>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(theValue, sizeof theValue, "%u", iValue);
	aBuffer.AddData(theValue, strlen(theValue));

	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_SIPField_MaxForwards_Instance()
{
	return vs::make_unique<VS_SIPField_MaxForwards>();
}

TSIPErrorCodes VS_SIPField_MaxForwards::Init(const VS_SIPGetInfoInterface& call)
{
	iValue = 70;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_MaxForwards::Clean() noexcept
{
	VS_SIPError::Clean();

	iValue = 0;
}

#undef DEBUG_CURRENT_MODULE
