#include "VS_SIPField_To.h"
#include "VS_SIPObjectFactory.h"
#include "VS_SIPURI.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "std-generic/compat/memory.h"
#include "VS_SIPGetInfoInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_To::e("(?i)(To|t)(?-i)( *):.*");

VS_SIPField_To::VS_SIPField_To():
	iSIPURI(0)
	, compact(false)
{
	VS_SIPError::Clean();
}

VS_SIPField_To::~VS_SIPField_To()
{
	VS_SIPField_To::Clean();
}

TSIPErrorCodes VS_SIPField_To::Decode(VS_SIPBuffer &aBuffer)
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
		dstream1 << "VS_SIPField_To::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] To Field: buffer not match, dump |" << ptr.get() << "|";
		SetError(TSIPErrorCodes::e_match);
        return TSIPErrorCodes::e_match;
	}

	const std::string &header = m[1];
	const std::string &spaces = m[2];

	// ("To" || "t") + ":" + spaces
	aBuffer.Skip(header.length() + spaces.length() + 1 );

	delete iSIPURI;
	iSIPURI = new VS_SIPURI;

	err = iSIPURI->Decode(aBuffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		dstream3 << "[SIPParserLib::Error] To Field: URI not decoded";
		delete iSIPURI;
		iSIPURI = 0;
		SetError(err);
		return err;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_To::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	const std::string out = compact ? "t: " : "To: ";
	aBuffer.AddData(out);

	return iSIPURI->Encode(aBuffer);
}

VS_SIPURI* VS_SIPField_To::GetURI() const
{
	return iSIPURI;
}

std::unique_ptr<VS_BaseField> VS_SIPField_To_Instance()
{
	return vs::make_unique<VS_SIPField_To>();
}

void VS_SIPField_To::Clean() noexcept
{
	VS_SIPError::Clean();

	delete iSIPURI; iSIPURI = 0;
}
void VS_SIPField_To::SetURI(const VS_SIPURI* uri)
{
	if ( !uri )
		return ;

	Clean();
	*iSIPURI = *uri;
}
TSIPErrorCodes VS_SIPField_To::Init(const VS_SIPGetInfoInterface& call)
{
	char alias[256] = {0};

	string_view str;
	string_view tag{};
	string_view epid;
	string_view displayName;

	if ( call.IsRequest() )
	{
		str = call.GetAliasRemote();
		/*
			A CANCEL request SHOULD NOT be sent to cancel a request other than
			INVITE.
			...
			The Request - URI, Call - ID, To, the numeric part of CSeq, and From header
			fields in the CANCEL request MUST be identical to those in the
			request being cancelled, including tags.
			https://tools.ietf.org/html/rfc3261#section-9.1

			In INVITE tag for To is empty;
		*/
		if(TYPE_CANCEL != call.GetMessageType())
			tag = call.GetTagSip();
		epid = call.GetEpidSip();
		displayName = call.GetDisplayNameSip();
	}else{
		str = call.GetAliasMy();
		tag = call.GetTagMy();
		epid = call.GetEpidMy();
		displayName = call.GetDisplayNameMy();
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

	//unsigned long ip = 0;
	//unsigned short port = 0;
	//eConnectionType conn_type = CONNECTIONTYPE_INVALID;

	//if ( !call->GetSrcCsAddress(ip, port, conn_type) )
	//{
	//	SetValid(false);
	//	SetError(e_InputParam);
	//	return e_InputParam;
	//}

	iSIPURI->Tag(std::string(tag));
	iSIPURI->Epid(std::string(epid));
	iSIPURI->Name(std::string(displayName));

	iSIPURI->AngleBracket(true);
	iSIPURI->URIType(SIPURI_SIP);

	compact = call.IsCompactHeaderAllowed();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

#undef DEBUG_CURRENT_MODULE
