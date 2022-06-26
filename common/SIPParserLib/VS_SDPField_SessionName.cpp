#include "VS_SDPField_SessionName.h"
#include "VS_SDPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPField_SessionName::e("(?i) *s *= *(.*) *(?-i)");

VS_SDPField_SessionName::VS_SDPField_SessionName()
{
	VS_SDPField_SessionName::Clean();
}

VS_SDPField_SessionName::~VS_SDPField_SessionName()
{
	VS_SDPField_SessionName::Clean();
}

TSIPErrorCodes VS_SDPField_SessionName::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = aBuffer.GetNextBlockAllocConst(ptr, ptr_sz);
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
		dstream1 << "VS_SDPField_SessionName::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::SDPError] SessionName Field: buffer not match, dump |" << ptr.get() << "|";
		aBuffer.SkipHeader();
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	m_name = m[1];

	aBuffer.SkipHeader();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPField_SessionName::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	aBuffer.AddData("s=");
	aBuffer.AddData(m_name.c_str(), m_name.length());
	aBuffer.AddData("\r\n");

	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_SDPField_SessionName_Instance()
{
	return vs::make_unique<VS_SDPField_SessionName>();
}

TSIPErrorCodes VS_SDPField_SessionName::Init(const VS_SIPGetInfoInterface& call)
{
	m_name = "noname";

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SDPField_SessionName::Clean() noexcept
{
	VS_SIPError::Clean();

	m_name.clear();
}

#undef DEBUG_CURRENT_MODULE
