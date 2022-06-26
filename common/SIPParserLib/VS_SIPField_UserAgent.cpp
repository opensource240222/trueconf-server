#include "VS_SIPField_UserAgent.h"
#include "VS_SIPGetInfoInterface.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_UserAgent::e1("(?i)(?:User-Agent) *: *(.+)(?-i)");

VS_SIPField_UserAgent::VS_SIPField_UserAgent()
{
	VS_SIPError::Clean();
}

VS_SIPField_UserAgent::~VS_SIPField_UserAgent()
{
}

TSIPErrorCodes VS_SIPField_UserAgent::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetError(err);
		return err;
	}

	if ( !ptr || !ptr_sz )
	{
		SetError(TSIPErrorCodes::e_buffer);
		return TSIPErrorCodes::e_buffer;
	}

	boost::cmatch m;
	bool regex_match_res;
	try
	{
		regex_match_res = boost::regex_match(ptr.get(), m, e1);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_UserAgent::Decode case1 error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] User-Agent Field: buffer not match, dump |" << ptr.get() << "|";
	// Unknown User-Agent
		SetValid(true);
		SetError(TSIPErrorCodes::e_ok);
		return TSIPErrorCodes::e_ok;
	}

	m_user_agent = m[1];

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_UserAgent::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	aBuffer.AddData("User-Agent: ");
	aBuffer.AddData(m_user_agent);
	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_SIPField_UserAgent_Instance()
{
	return vs::make_unique<VS_SIPField_UserAgent>();
}

void VS_SIPField_UserAgent::Clean() noexcept
{
	VS_SIPError::Clean();

	m_user_agent.clear();
}

TSIPErrorCodes VS_SIPField_UserAgent::Init(const VS_SIPGetInfoInterface& call)
{
	SetValid(true);
	m_user_agent = call.GetServerUserAgent();
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

const std::string &VS_SIPField_UserAgent::GetUserAgent() const
{
	return m_user_agent;
}

#undef DEBUG_CURRENT_MODULE
