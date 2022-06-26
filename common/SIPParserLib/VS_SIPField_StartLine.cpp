#include "VS_SIPField_StartLine.h"
#include "VS_SIPObjectFactory.h"
#include "VS_SIPURI.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "VS_SIPGetInfoInterface.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_StartLine::e(
		// Request
		"(?i)(?:"
			"([^ ]+) +"			// INVITE
			"(?:[^ ]+) +"		// URI
			"(?:sip/2\\.0)+ *"		// SIP/2.0
		// Response
		")|(?:"
			"(?:sip/2\\.0)+ +"		// SIP/2.0
			"([\\d{3}]+) +"			// Error code
			"([-A-Za-z0-9_*@%#!&~=/`\'\" \\\\|\\.\\?\\$\\^\\+\\(\\)\\[\\]\\<\\>\\{\\}\\:]+)"				// Error string
		")(?-i)"
	);

VS_SIPField_StartLine::VS_SIPField_StartLine():
	iResponseCode(-1)
{
	VS_SIPField_StartLine::Clean();
}

VS_SIPField_StartLine::~VS_SIPField_StartLine()
{
}

TSIPErrorCodes VS_SIPField_StartLine::Decode(VS_SIPBuffer &aBuffer)
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
		dstream1 << "VS_SIPField_StartLine::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] StartLine Field: buffer not match, dump |" << ptr.get() << "|";
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}


	std::string method = m[1];

	if ( !method.empty() )	// Request
	{
		// UpperCase
		std::transform(method.begin(), method.end(), method.begin(), toupper);

		const eStartLineType method_type = VS_SIPObjectFactory::GetMethod(method.c_str());
		if (method_type != TYPE_INVALID)
		{
			iRequestType = method_type;
			aBuffer.Skip( method.length() );
		} else {
			SetValid(false);
			SetError(TSIPErrorCodes::e_InputParam);
			return TSIPErrorCodes::e_InputParam;
		}

		iRequestURI = vs::make_unique<VS_SIPURI>();

		TSIPErrorCodes err = iRequestURI->Decode(aBuffer);		// установит буфер на следующий филд
		if (err != TSIPErrorCodes::e_ok)
		{
			SetValid(false);
			SetError(err);
			return err;
		}

		iSIPProto = SIPPROTO_SIP20;
		iMessageType = MESSAGE_TYPE_REQUEST;

		SetValid(true);
		SetError(TSIPErrorCodes::e_ok);
		return TSIPErrorCodes::e_ok;

	}else{	// Response
		const std::string &theErrCode = m[2];
		iResponseStr = m[3];

		iResponseCode = atoi( theErrCode.c_str() );

		iSIPProto = SIPPROTO_SIP20;
		iMessageType = MESSAGE_TYPE_RESPONSE;

		aBuffer.SkipHeader();

		SetValid(true);
		SetError(TSIPErrorCodes::e_ok);
		return TSIPErrorCodes::e_ok;
	}
}

TSIPErrorCodes VS_SIPField_StartLine::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	const char* p = 0;

	switch(iSIPProto)
	{
	case SIPPROTO_SIP20:
			p = "SIP/2.0";
		break;

	default:
			return TSIPErrorCodes::e_UNKNOWN;
		break;
	}

	switch(iMessageType)
	{
	case MESSAGE_TYPE_REQUEST:
	{
		const auto name_method = VS_SIPObjectFactory::GetMethod(iRequestType);
		if (!name_method)
		{
			return TSIPErrorCodes::e_InputParam;
		}

		std::string method(name_method);
		method += " ";

	    aBuffer.AddData(method);

		if (!iRequestURI)
			return TSIPErrorCodes::e_InputParam;

		iRequestURI->Encode(aBuffer);

		aBuffer.AddData(" ");
		aBuffer.AddData(p, strlen(p));

		return TSIPErrorCodes::e_ok;
	}
	case MESSAGE_TYPE_RESPONSE:
	{
		aBuffer.AddData(p, strlen(p));
		aBuffer.AddData(" ");

		char theCode[std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
		snprintf(theCode, sizeof theCode, "%d", iResponseCode);

		aBuffer.AddData(theCode, strlen(theCode));

		if (!iResponseStr.empty())
		{
			aBuffer.AddData(" ");
			aBuffer.AddData(iResponseStr);
		}

		return TSIPErrorCodes::e_ok;
	}
	default: break;
	}
	return TSIPErrorCodes::e_UNKNOWN;
}

int VS_SIPField_StartLine::GetMessageType() const
{
	return iMessageType;
}

int VS_SIPField_StartLine::GetRequestType() const
{
	return iRequestType;
}

const VS_SIPURI* VS_SIPField_StartLine::GetRequestURI() const
{
	return iRequestURI.get();
}

std::unique_ptr<VS_BaseField> VS_SIPField_StartLine_Instance()
{
	return vs::make_unique<VS_SIPField_StartLine>();
}

void VS_SIPField_StartLine::Clean() noexcept
{
	VS_SIPError::Clean();

	iMessageType = MESSAGE_TYPE_INVALID;
	iSIPProto = SIPPROTO_INVALID;

	/* Request */
	iRequestType = TYPE_INVALID;
	iRequestURI.reset();
	/* Response */
	iResponseCode = -1;
	iResponseStr.clear();
}
void VS_SIPField_StartLine::SetMessageType(int type)
{
	iMessageType = type;
}
void VS_SIPField_StartLine::SetRequestType(eStartLineType type)
{
	iRequestType = type;
}
bool VS_SIPField_StartLine::SetRequestURI(std::unique_ptr<VS_SIPURI>&& uri)
{
	if ( !uri )
		return false;

	iRequestURI = std::move(uri);
	return true;
}
int VS_SIPField_StartLine::GetSIPProto() const
{
	return iSIPProto;
}
void VS_SIPField_StartLine::SetSIPProto(int sip_proto)
{
	iSIPProto = sip_proto;
}
TSIPErrorCodes VS_SIPField_StartLine::Init(const VS_SIPGetInfoInterface& call)
{
	iSIPProto = call.GetSIPProtocol();

	if ( call.IsRequest() )
	{
		iMessageType = MESSAGE_TYPE_REQUEST;
		iRequestType = call.GetMessageType();

		const string_view str = call.GetSipRemoteTarget();
		if (str.empty()) return TSIPErrorCodes::e_InputParam;

		VS_SIPBuffer alias_buff;
		alias_buff.AddData("sip:");
		alias_buff.AddData(str);
		alias_buff.AddData("\r\n");

		iRequestURI = vs::make_unique<VS_SIPURI>();

		iRequestURI->SetDoPreDecodeEscaping();
		TSIPErrorCodes err = iRequestURI->Decode(alias_buff);
        if (err != TSIPErrorCodes::e_ok)
		{
			SetValid(false);
			SetError(err);
			return err;
		}

		iRequestURI->URIType(SIPURI_SIP);

	}else{
		iMessageType = MESSAGE_TYPE_RESPONSE;

		iResponseCode = call.GetResponseCode();

		string_view t = call.GetResponseStr();
		if (t.empty())
		{
			SetValid(true);
			SetError(TSIPErrorCodes::e_InputParam);
			return TSIPErrorCodes::e_InputParam;
		}
		iResponseStr = std::string(call.GetResponseStr());
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

int VS_SIPField_StartLine::GetResponseCode() const
{
	return iResponseCode;
}

int VS_SIPField_StartLine::GetResponseCodeClass() const
{
	const int code = (iResponseCode / 100);
	return ((code >= 1) && (code <= 6))? code: 0;
}

void VS_SIPField_StartLine::SetResponseCode(int code)
{
	iResponseCode = code;
}

const std::string &VS_SIPField_StartLine::GetResponseStr() const
{
	return iResponseStr;
}

void VS_SIPField_StartLine::SetResponseStr(std::string str)
{
	iResponseStr = std::move(str);
}

#undef DEBUG_CURRENT_MODULE
