#include "VS_SIPField_Server.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_Server::e("(?i)(?:Server) *: *(.+)(?-i)");

VS_SIPField_Server::VS_SIPField_Server()
{
	VS_SIPError::Clean();
}

VS_SIPField_Server::~VS_SIPField_Server()
{
}

TSIPErrorCodes VS_SIPField_Server::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
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
		regex_match_res = boost::regex_match(ptr.get(), m, e);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Server::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Server Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(true);
		SetError(TSIPErrorCodes::e_ok);
		return TSIPErrorCodes::e_ok;
	}

	m_server = m[1];

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Server::Encode(VS_SIPBuffer &aBuffer) const
{
	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_SIPField_Server_Instance()
{
	return vs::make_unique<VS_SIPField_Server>();
}

void VS_SIPField_Server::Clean() noexcept
{
	VS_SIPError::Clean();

	m_server.clear();
}

TSIPErrorCodes VS_SIPField_Server::Init(const VS_SIPGetInfoInterface& call)
{
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

const std::string &VS_SIPField_Server::GetServer() const
{
	return m_server;
}

#undef DEBUG_CURRENT_MODULE
