#include "VS_SIPField_RetryAfter.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"
#include "VS_SIPGetInfoInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_RetryAfter::e(
	"(?i)"
	" *(?:retry-after) *: *"
	"(\\d+) *"						// delta seconds
	"(\\((.*)\\)){0,1} *"			// (comment)
	"(;duration=(\\d+)){0,1}"		// duration=delta seconds
	"(?-i)"
	);

std::unique_ptr<VS_BaseField> VS_SIPField_RetryAfter_Instance()
{
	return vs::make_unique<VS_SIPField_RetryAfter>();
}

VS_SIPField_RetryAfter::VS_SIPField_RetryAfter() :m_value(0), m_duration(0){}

TSIPErrorCodes VS_SIPField_RetryAfter::Decode(VS_SIPBuffer &aBuffer) {
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;
	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);

	if (err != TSIPErrorCodes::e_ok || !ptr || !ptr_sz)
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
		dstream1 << "VS_SIPField_RetryAfter::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}

	if (!regex_match_res) {
		dstream3 << "[SIPParserLib::Error] Retry-After Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	m_value = std::chrono::seconds(::atoll(static_cast<const std::string &>(m[1]).c_str()));
	if (!static_cast<const std::string&>(m[3]).empty())
		m_comment = m[3];
	m_duration = std::chrono::seconds(::atoll(static_cast<const std::string&>(m[5]).c_str()));

	SetError(TSIPErrorCodes::e_ok);
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_RetryAfter::Encode(VS_SIPBuffer &aBuffer) const{
	aBuffer.AddData("Retry-After: ");
	aBuffer.AddData(std::to_string(m_value.count()));

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_RetryAfter::Init(const VS_SIPGetInfoInterface& call)
{
	m_value = std::chrono::duration_cast<std::chrono::seconds>(call.GetRetryAfterValue());
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

#undef DEBUG_CURRENT_MODULE
