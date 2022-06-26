#include "VS_SIPField_MinSE.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_MinSE::e(
		"(?i)"
		" *(?:min-se) *: +"
		"(.*) *"
		"(?-i)"
	);

VS_SIPField_MinSE::VS_SIPField_MinSE()
{
	VS_SIPField_MinSE::Clean();
}

VS_SIPField_MinSE::~VS_SIPField_MinSE()
{
}

TSIPErrorCodes VS_SIPField_MinSE::Decode(VS_SIPBuffer &aBuffer)
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
		dstream1 << "VS_SIPField_MinSE::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Min-Se Field: buffer not match, dump |" << ptr.get() << "|";
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	const std::string &value = m[1];

	m_value = atoi(value.c_str());

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_MinSE::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	aBuffer.AddData("Min-SE: ");

	char val[std::numeric_limits<unsigned int>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(val, sizeof val, "%u", m_value);

	aBuffer.AddData(val, strlen(val));

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_MinSE::Init(const VS_SIPGetInfoInterface& call)
{
	this->SetValue(90);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_MinSE::Clean() noexcept
{
	VS_SIPError::Clean();

	SetValue(90);
}

std::unique_ptr<VS_BaseField> VS_SIPField_MinSE_Instance()
{
	return vs::make_unique<VS_SIPField_MinSE>();
}

#undef DEBUG_CURRENT_MODULE
