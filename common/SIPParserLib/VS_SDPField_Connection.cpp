#include "VS_SDPField_Connection.h"
#include "VS_SDPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPField_Connection::e(
		"( *)"
		"(?i)(?:c)(?-i)"
		"( *)=.*"
	);

VS_SDPField_Connection::VS_SDPField_Connection()
{
	VS_SDPField_Connection::Clean();
}

VS_SDPField_Connection::~VS_SDPField_Connection()
{
	VS_SDPField_Connection::Clean();
}

TSIPErrorCodes VS_SDPField_Connection::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = aBuffer.GetNextBlockAllocConst(ptr, ptr_sz);
	if ( (TSIPErrorCodes::e_ok != err) || !ptr_sz || !ptr )
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
		dstream1 << "VS_SDPField_Connection::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::SDPError] Connection Field: buffer not match, dump |" << ptr.get() << "|";
		aBuffer.SkipHeader();
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	const std::string &spaces1 = m[1];
	const std::string &spaces2 = m[2];

	const auto n = spaces1.length() + spaces2.length() + 2;

	aBuffer.Skip(n);

	err = iConnect.Decode(aBuffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		dstream3 << "[SIPParserLib::SDPError] Origin Field: Connect not decoded";
		SetValid(false);
		SetError(err);
		return err;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPField_Connection::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if ( !iConnect.IsValid() )
		return iConnect.GetLastClassError();

	aBuffer.AddData("c=");
	const TSIPErrorCodes err = iConnect.Encode(aBuffer);
	aBuffer.AddData("\r\n");

	return err;
}

std::unique_ptr<VS_BaseField> VS_SDPField_Connection_Instance()
{
	return vs::make_unique<VS_SDPField_Connection>();
}

TSIPErrorCodes VS_SDPField_Connection::Init(const VS_SIPGetInfoInterface& call)
{

	iConnect.Init(call);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

VS_SDPField_Connection & VS_SDPField_Connection::operator=(const VS_SDPField_Connection &conn)
{
	if (this == &conn)
	{
		return *this;
	}
	if (*this != conn)
	{
		VS_BaseField::operator=(conn);
		iConnect.Clean();

		iConnect = conn.iConnect;

		this->SetValid(conn.IsValid());
		this->SetError(conn.GetLastClassError());
	}
	return *this;
}

bool VS_SDPField_Connection::operator!=(const VS_SDPField_Connection &conn) const
{
	if (this->VS_BaseField::operator!=(conn))
	{
		return true;
	}
	return this->iConnect != conn.iConnect;
}

const std::string &VS_SDPField_Connection::GetHost() const
{
	return iConnect.GetHost();
}
void VS_SDPField_Connection::SetHost(std::string host)
{
	iConnect.SetHost(std::move(host));
}

void VS_SDPField_Connection::Clean() noexcept
{
	VS_SIPError::Clean();
}

#undef DEBUG_CURRENT_MODULE
