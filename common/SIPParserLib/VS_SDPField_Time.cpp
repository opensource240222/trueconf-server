#include "VS_SDPField_Time.h"
#include "VS_SDPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include <memory>
#include "std-generic/compat/memory.h"
#include <cinttypes>

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPField_Time::e("(?i) *t *= *(\\d+) (\\d+) *(?-i)");

VS_SDPField_Time::VS_SDPField_Time()
{
	VS_SDPField_Time::Clean();
}

VS_SDPField_Time::~VS_SDPField_Time()
{
	VS_SDPField_Time::Clean();
}

TSIPErrorCodes VS_SDPField_Time::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = aBuffer.GetNextBlockAllocConst(ptr, ptr_sz);
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
		dstream1 << "VS_SDPField_Time::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::SDPError] Time Field: buffer not match, dump |" << ptr.get() << "|";
		aBuffer.SkipHeader();
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	const std::string &start = m[1];
	const std::string &stop = m[2];
	m_start_time = std::stoull(start);
	m_stop_time = std::stoull(stop);

	aBuffer.SkipHeader();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPField_Time::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	char start[std::numeric_limits<std::uint64_t>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	char stop[std::numeric_limits<std::uint64_t>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };

	snprintf(start, sizeof start, "%" PRId64, m_start_time);
	snprintf(stop, sizeof stop, "%" PRId64, m_stop_time);

	aBuffer.AddData("t=");
	aBuffer.AddData(start, strlen(start));
	aBuffer.AddData(" ");
	aBuffer.AddData(stop, strlen(stop));
	aBuffer.AddData("\r\n");

	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_SDPField_Time_Instance()
{
	return vs::make_unique<VS_SDPField_Time>();
}

TSIPErrorCodes VS_SDPField_Time::Init(const VS_SIPGetInfoInterface& call)
{
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SDPField_Time::Clean() noexcept
{
	VS_SIPError::Clean();

	m_start_time = 0;
	m_stop_time = 0;
}

#undef DEBUG_CURRENT_MODULE
