#include "VS_SIPField_CallID.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "VS_SIPGetInfoInterface.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_CallID::e(
		"(?i)"
		" *(?:call-id|i) *: *"
		"(.*) *"
		"(?-i)"
	);

VS_SIPField_CallID::VS_SIPField_CallID() : compact(false)
{
	VS_SIPError::Clean();
}

VS_SIPField_CallID::~VS_SIPField_CallID()
{
}

TSIPErrorCodes VS_SIPField_CallID::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
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
		dstream1 << "VS_SIPField_CallID::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}

	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] CallID Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	iValue = m[1];

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_CallID::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if (compact)
		aBuffer.AddData("i: ");
	else
		aBuffer.AddData("Call-ID: ");
	aBuffer.AddData(iValue);

	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_CallID::Value(std::string value)
{
	this->iValue = std::move(value);
}

const std::string &VS_SIPField_CallID::Value() const
{
	return iValue;
}

std::unique_ptr<VS_BaseField> VS_SIPField_CallID_Instance()
{
	return vs::make_unique<VS_SIPField_CallID>();
}
TSIPErrorCodes VS_SIPField_CallID::Init(const VS_SIPGetInfoInterface& call)
{
	if ( call.GetSIPDialogId().empty() )
		return TSIPErrorCodes::e_InputParam;

	this->Value( std::string(call.GetSIPDialogId()) );

	compact = call.IsCompactHeaderAllowed();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_CallID::Clean() noexcept
{
	VS_SIPError::Clean();

	iValue.clear();
}

#undef DEBUG_CURRENT_MODULE
