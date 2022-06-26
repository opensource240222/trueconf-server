#include "VS_SDPField_Origin.h"
#include "VS_SDPObjectFactory.h"
#include "VS_SDPConnect.h"
#include "std/debuglog/VS_Debug.h"
#include "utils/escape.h"
#include "exceptions/SIPURIEscapeException.h"
#include <string>
#include "VS_SIPGetInfoInterface.h"
#include <inttypes.h>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPField_Origin::e("( *)(?i)(?:o)(?-i)"		// Header
		"( *)=( *)"
		"([^ ]+)( +)"						// UserName
		"([0-9]+)( +)"						// SessID
		"([0-9]+)( +)"						// Version
		"(.*) *"							// Connect
	);

static const char USER_DOMAIN_SEPARATOR = '@';
static const char EMPTY_ORIGIN [] = "-";

VS_SDPField_Origin::VS_SDPField_Origin(): iConnect(0)
{
	VS_SIPError::Clean();
}

VS_SDPField_Origin::~VS_SDPField_Origin()
{
	VS_SDPField_Origin::Clean();
}

const std::string &VS_SDPField_Origin::UserName() const
{
	return iUserName;
}

const std::string &VS_SDPField_Origin::SessionId() const
{
	return iSessID;
}

const std::string &VS_SDPField_Origin::Version() const
{
	return iVersion;
}

const VS_SDPConnect* VS_SDPField_Origin::Connect() const
{
	return iConnect;
}

TSIPErrorCodes VS_SDPField_Origin::Decode(VS_SIPBuffer &aBuffer)
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
		dstream1 << "VS_SDPField_Origin::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}

	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::SDPError] Origin Field: buffer not match, dump |" << ptr.get() << "|";
		aBuffer.SkipHeader();
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	const std::string &spaces1 = m[1];
	const std::string &spaces2 = m[2];
	const std::string &spaces3 = m[3];
	const std::string &theUserName = m[4];
	const std::string &spaces4 = m[5];
	const std::string &theSessID = m[6];
	const std::string &spaces5 = m[7];
	const std::string &theVersion = m[8];
 	const std::string &spaces6 = m[9];

	ptr_sz = 0;

// UserName
	std::string user_name;
	try
	{
		iUserName = vs::sip_uri_unescape(theUserName);
	}catch(const vs::SIPURIEscapeException &ex)
	{
		dstream1 << "VS_SDPField_Origin::Decode error " << ex.what() << "\n";
		iUserName = EMPTY_ORIGIN;
	}


// SessID
	iSessID = theSessID;

// Version
	iVersion = theVersion;

	const std::size_t n = spaces1.length() + spaces2.length()
		+ spaces3.length() + spaces4.length()
		+ spaces5.length() + spaces6.length()
		+ 1 + theUserName.length()							// Header Length ("o")
		+ theSessID.length() + theVersion.length() + 1;

	aBuffer.Skip(n);

	delete iConnect;

	iConnect = new VS_SDPConnect;
	err = iConnect->Decode(aBuffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		dstream3 << "[SIPParserLib::SDPError] Origin Field: Connect not decoded";
		delete iConnect;
		iConnect = 0;
		SetValid(false);
		SetError(err);
		return err;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPField_Origin::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if ( !iConnect )
		return TSIPErrorCodes::e_InputParam;

	if ( !iConnect->IsValid() )
		return iConnect->GetLastClassError();

	aBuffer.AddData("o=");
	try
	{
		aBuffer.AddData(vs::sip_uri_escape(iUserName));
	}catch(const vs::SIPURIEscapeException &ex)
	{
		dstream1 << "VS_SDPField_Origin::Encode error " << ex.what() << "\n";
		aBuffer.AddData(EMPTY_ORIGIN);
	}
	aBuffer.AddData(" ");
	aBuffer.AddData(iSessID);
	aBuffer.AddData(" ");
	aBuffer.AddData(iVersion);
	aBuffer.AddData(" ");

	const TSIPErrorCodes err = iConnect->Encode(aBuffer);
	aBuffer.AddData("\r\n");

	return err;
}

std::unique_ptr<VS_BaseField> VS_SDPField_Origin_Instance()
{
	return vs::make_unique<VS_SDPField_Origin>();
}

TSIPErrorCodes VS_SDPField_Origin::Init(const VS_SIPGetInfoInterface& call)
{
	string_view alias_my = call.GetAliasMy();
	string_view alias_my_view;
	if (alias_my.empty())
	{
		alias_my_view = EMPTY_ORIGIN;
	}
	else
	{
		alias_my_view = alias_my;
		if (!call.IsDirectionToSip() && call.IsGroupConf())
		{
			alias_my_view = alias_my_view.substr(0, alias_my_view.find_last_of(USER_DOMAIN_SEPARATOR));
		}
	}

	iUserName = std::string(alias_my_view);

	char tmp[std::numeric_limits<std::uint64_t>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };

	snprintf(tmp, sizeof tmp, "%" PRId64, call.GetSdpSessionId());
	iSessID = tmp;

	snprintf(tmp, sizeof tmp, "%u", call.GetSdpSessionVersion());
	iVersion = tmp;

	delete iConnect;

	iConnect = new VS_SDPConnect;
	iConnect->Init(call);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SDPField_Origin::Clean() noexcept
{
	VS_SIPError::Clean();

	delete iConnect; iConnect = 0;
	iUserName.clear();
	iSessID.clear();
	iVersion.clear();
}

#undef DEBUG_CURRENT_MODULE
