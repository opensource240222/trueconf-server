#include "VS_SIPField_From.h"
#include "VS_SIPObjectFactory.h"
#include "VS_SIPURI.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "VS_SIPGetInfoInterface.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_From::e("(?i)(From|f)(?-i)( *):.*");

VS_SIPField_From::VS_SIPField_From():
	iSIPURI(0)
	, compact(false)
{
	VS_SIPError::Clean();
}

VS_SIPField_From::~VS_SIPField_From()
{
	VS_SIPField_From::Clean();
}

TSIPErrorCodes VS_SIPField_From::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = aBuffer.GetNextBlockAllocConst(ptr, ptr_sz);
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
		dstream1 << "VS_SIPField_From::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] From Field: buffer not match, dump |" << ptr.get() << "|";
		SetError(TSIPErrorCodes::e_match);
        return TSIPErrorCodes::e_match;
	}

	const std::string &header = m[1];
	const std::string &spaces = m[2];

	// ("From" || "f") + ":" + spaces
	aBuffer.Skip(header.length() + spaces.length() + 1 );

	delete iSIPURI;

	iSIPURI = new VS_SIPURI;

	err = iSIPURI->Decode(aBuffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		dstream3 << "[SIPParserLib::Error] From Field: URI not decoded";
		delete iSIPURI;
		iSIPURI = 0;
		SetError(err);
		return err;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_From::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if (compact)
		aBuffer.AddData("f: ");
	else
		aBuffer.AddData("From: ");

	return iSIPURI->Encode(aBuffer);
}

VS_SIPURI* VS_SIPField_From::GetURI() const
{
	return iSIPURI;
}

std::unique_ptr<VS_BaseField> VS_SIPField_From_Instance()
{
	return  vs::make_unique<VS_SIPField_From>();
}

void VS_SIPField_From::Clean() noexcept
{
	VS_SIPError::Clean();

	delete iSIPURI; iSIPURI = 0;
}
void VS_SIPField_From::SetURI(const VS_SIPURI* uri)
{
	if ( !uri )
		return ;

	Clean();
	*iSIPURI = *uri;
}
TSIPErrorCodes VS_SIPField_From::Init(const VS_SIPGetInfoInterface& call)
{
	char alias[256] = {0};

	string_view str;
	string_view tag;
	string_view name;
	string_view epid;

	if ( call.IsRequest() )
	{
		str = call.GetAliasMy();
		tag = call.GetTagMy();
		name = call.GetDisplayNameMy();
		epid = call.GetEpidMy();
	}else{
		str = call.GetAliasRemote();
		tag = call.GetTagSip();
		name = call.GetDisplayNameSip();
		epid = call.GetEpidSip();
	}

	if ( str.empty() ) 	return TSIPErrorCodes::e_InputParam;

	auto alias_sz = str.length();

	memcpy(alias + 4, str.data(), alias_sz);
	memcpy(alias, "sip:", 4);	alias_sz += 4;
	alias[alias_sz] = '\r';		alias_sz++;
	alias[alias_sz] = '\n';		alias_sz++;

	VS_SIPBuffer alias_buff(alias, alias_sz);

	delete iSIPURI;

	iSIPURI = new VS_SIPURI;

	iSIPURI->SetDoPreDecodeEscaping();
	const TSIPErrorCodes err = iSIPURI->Decode(alias_buff);
	if (err != TSIPErrorCodes::e_ok)
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	iSIPURI->Tag(std::string(tag));
	iSIPURI->Name(std::string(name));
	iSIPURI->Epid(std::string(epid));

	iSIPURI->AngleBracket(true);
	iSIPURI->URIType(SIPURI_SIP);

	compact = call.IsCompactHeaderAllowed();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

#undef DEBUG_CURRENT_MODULE
