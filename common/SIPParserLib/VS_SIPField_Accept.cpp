#include "VS_SIPField_Accept.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <boost/regex.hpp>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_Accept::e(
		"(?i)"
		" *(?:accept) *: *"
		"([, /A-Za-z0-9.!%*_+`'~-]+) *" // ", /" (separators) + RFC3261 token
		"(?-i)"
	);

std::unique_ptr<VS_BaseField> VS_SIPField_Accept_Instance()
{
	return vs::make_unique<VS_SIPField_Accept>();
}
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
VS_SIPField_Accept::VS_SIPField_Accept()
{

}
//---------------------------------------------------------------------------------------
TSIPErrorCodes VS_SIPField_Accept::Decode(VS_SIPBuffer &buf)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = buf.GetNextBlockAlloc(ptr, ptr_sz);
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
		dstream1 << "VS_SIPField_Accept::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}

	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] ContentType Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	value_ = m[1];

	SetError(TSIPErrorCodes::e_ok);
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}
//---------------------------------------------------------------------------------------
TSIPErrorCodes VS_SIPField_Accept::Encode(VS_SIPBuffer &buf) const
{
	if ( !IsValid() )
		return GetLastClassError();

	buf.AddData("Accept: ");
	buf.AddData(value_);

	return TSIPErrorCodes::e_ok;
}
//---------------------------------------------------------------------------------------
TSIPErrorCodes VS_SIPField_Accept::Init(const VS_SIPGetInfoInterface& call)
{
	value_ = "application/sdp, application/media_control+xml, text/plain";

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

#undef DEBUG_CURRENT_MODULE
