#include "VS_SIPField_CSeq.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "std-generic/compat/memory.h"
#include "VS_SIPGetInfoInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_CSeq::e(
		"(?i)"
		" *(?:cseq) *: *"
		"(\\d+) +"
		"(\\w+) *"
		"(?-i)"
	);

VS_SIPField_CSeq::VS_SIPField_CSeq():
	iType(TYPE_INVALID), iValue(0)
{
	VS_SIPError::Clean();
}

VS_SIPField_CSeq::~VS_SIPField_CSeq()
{
}

TSIPErrorCodes VS_SIPField_CSeq::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
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
		dstream1 << "VS_SIPField_CSeq::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] CSeq Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	const std::string &value = m[1];
	std::string method = m[2];

	// UpperCase
	std::transform(method.begin(), method.end(), method.begin(), toupper);

	const eStartLineType method_type = VS_SIPObjectFactory::GetMethod(method.c_str());
	if (method_type != TYPE_INVALID)
		iType = method_type;
	else
	{
		dstream3 << "[SIPParserLib::Error] CSeq Field: method_type not supported";
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	iValue = atoi( value.c_str() );

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_CSeq::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	std::string out = "CSeq: ";

	char theValue[std::numeric_limits<unsigned int>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(theValue, sizeof theValue, "%u", iValue);

	out += theValue;
	const char* method = VS_SIPObjectFactory::GetMethod(iType);
	if (method) {
		out += " ";
		out += method;
	} else {
		return TSIPErrorCodes::e_InputParam;
	}

	return aBuffer.AddData(out);
}

eStartLineType VS_SIPField_CSeq::GetType() const
{
	return iType;
}

void VS_SIPField_CSeq::SetType(const eStartLineType type)
{
	iType = type;
}

void VS_SIPField_CSeq::Value(const unsigned int aValue)
{
	iValue = aValue;
}

unsigned int VS_SIPField_CSeq::Value() const
{
	return iValue;
}

TSIPErrorCodes VS_SIPField_CSeq::Init(const VS_SIPGetInfoInterface& call)
{
	if ( call.IsRequest() )
		iValue = call.GetMySequenceNumber();
	else
		iValue = call.GetSipSequenceNumber();

	iType = call.GetMessageType();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_SIPField_CSeq_Instance()
{
	return vs::make_unique<VS_SIPField_CSeq>();
}

void VS_SIPField_CSeq::Clean() noexcept
{
	VS_SIPError::Clean();

	iType = TYPE_INVALID;
	iValue = 0;
}

#undef DEBUG_CURRENT_MODULE
