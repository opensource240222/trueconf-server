#include "VS_SIPField_Require.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"
#include "VS_SIPGetInfoInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_Require::e1(
	"(?i)"
	" *(?:require|k) *: *"
	"(.*) *"
	"(?-i)"
);
const boost::regex VS_SIPField_Require::e2(".* *(?i)(?:timer)(?-i).*");

VS_SIPField_Require::VS_SIPField_Require()
{
	VS_SIPField_Require::Clean();
}

VS_SIPField_Require::~VS_SIPField_Require()
{
}

TSIPErrorCodes VS_SIPField_Require::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
	if ((TSIPErrorCodes::e_ok != err) || !ptr || !ptr_sz)
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	boost::cmatch m;
	bool regex_match_res;
	try
	{
		regex_match_res = boost::regex_match(ptr.get(), m, e1);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Require::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Require Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	FindParam_timer(ptr.get());

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Require::Encode(VS_SIPBuffer &aBuffer) const
{
	if (!IsValid())
		return GetLastClassError();

	std::string out = "Require: ";

	const char* p = 0;
	bool IsAnyExt = false;
	if (m_extensions & SIP_EXTENSION_TIMER)
	{
		p = VS_SIPObjectFactory::GetSIPExtension(SIP_EXTENSION_TIMER);
		if (p)
			out += p;
		p = 0;
		IsAnyExt = true;
	}

	return aBuffer.AddData(out);
}

TSIPErrorCodes VS_SIPField_Require::Init(const VS_SIPGetInfoInterface& call)
{
	if (call.GetMessageType() == TYPE_REGISTER) {
		AddExtension(SIP_EXTENSION_PATH);
	}
	else if (call.IsSessionTimerEnabled()) {
		this->AddExtension(SIP_EXTENSION_TIMER);
	}
	else {
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	//AddExtension(SIP_EXTENSION_GRUU);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_Require::AddExtension(const eSIP_ExtensionPack ext)
{
	m_extensions |= ext;
}

void VS_SIPField_Require::Clean() noexcept
{
	VS_SIPError::Clean();

	m_extensions = 0;
}

std::unique_ptr<VS_BaseField> VS_SIPField_Require_Instance()
{
	return vs::make_unique<VS_SIPField_Require>();
}

bool VS_SIPField_Require::FindParam_timer(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(aInput.cbegin(), aInput.cend(), m, e2))
		{
			m_extensions |= SIP_EXTENSION_TIMER;
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Require::FindParam_timer error " << ex.what() << "\n";
	}
	return false;
}


#undef DEBUG_CURRENT_MODULE
