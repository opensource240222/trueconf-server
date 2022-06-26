#include "VS_SIPField_Route.h"
#include "VS_SIPObjectFactory.h"
#include "VS_SIPURI.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "std-generic/compat/memory.h"
#include "VS_SIPGetInfoInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_Route::e(" *(?i)Route(?-i)( *):.*");

VS_SIPField_Route::VS_SIPField_Route():
	iSIPURI(0)
{
	VS_SIPError::Clean();
}

VS_SIPField_Route::~VS_SIPField_Route()
{
	VS_SIPField_Route::Clean();
}

TSIPErrorCodes VS_SIPField_Route::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = aBuffer.GetNextBlockAllocConst(ptr, ptr_sz);
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
		dstream1 << "VS_SIPField_Route::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Route Field: buffer not match, dump |" << ptr.get() << "|";
		SetError(TSIPErrorCodes::e_match);
        return TSIPErrorCodes::e_match;
	}

	const std::string &spaces = m[1];
	const std::size_t nspaces = spaces.length();

	aBuffer.Skip(6 + nspaces);	// "Route:" + spaces

	iSIPURI = new VS_SIPURI;

	err = iSIPURI->Decode(aBuffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		delete iSIPURI;
		iSIPURI = nullptr;

		SetError(err);
		return err;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Route::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	aBuffer.AddData("Route: ");

	return iSIPURI->Encode(aBuffer);
}

TSIPErrorCodes VS_SIPField_Route::Init(const VS_SIPGetInfoInterface& call)
{
	const VS_SIPURI *uri = call.GetNextSipRouteFromSet();
	if (!uri)
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}


	delete iSIPURI;

	iSIPURI = new VS_SIPURI;
	*iSIPURI = *uri;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

VS_SIPURI* VS_SIPField_Route::GetURI() const
{
	return iSIPURI;
}

std::unique_ptr<VS_BaseField> VS_SIPField_Route_Instance()
{
	return vs::make_unique<VS_SIPField_Route>();
}

void VS_SIPField_Route::Clean() noexcept
{
	VS_SIPError::Clean();

	delete iSIPURI; iSIPURI = 0;
}

#undef DEBUG_CURRENT_MODULE
