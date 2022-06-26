#include "VS_SIPField_MinExpires.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "std-generic/compat/memory.h"
#include <cinttypes>

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_MinExpires::e(
										"(?i)"
										" *(?:min-expires) *: +"
										"(.*) *"
										"(?-i)"
										);

VS_SIPField_MinExpires::VS_SIPField_MinExpires()
{
	VS_SIPField_MinExpires::Clean();
}


TSIPErrorCodes VS_SIPField_MinExpires::Decode(VS_SIPBuffer &aBuffer)
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
		dstream1 << "VS_SIPField_MinExpires::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 <<  "[SIPParserLib::Error] Min-Expires Field: buffer not match, dump |" << ptr.get() << "|";
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	const std::string &value = m[1];

	m_value = std::chrono::seconds(atoi(value.c_str()));

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_MinExpires::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	aBuffer.AddData("Min-Expires: ");

	char val[std::numeric_limits<uint64_t>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(val, sizeof val, "%" PRIu64, static_cast<uint64_t>(m_value.count()));

	aBuffer.AddData(val, strlen(val));

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_MinExpires::Init(const VS_SIPGetInfoInterface& call)
{
	this->SetValue(std::chrono::seconds(90));

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_MinExpires::Clean() noexcept
{
	VS_SIPError::Clean();

	SetValue(std::chrono::seconds(90));
}

std::unique_ptr<VS_BaseField> VS_SIPField_MinExpires_Instance()
{
	return  vs::make_unique<VS_SIPField_MinExpires>();
}

#undef DEBUG_CURRENT_MODULE
