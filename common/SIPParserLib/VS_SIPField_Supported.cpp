#include "VS_SIPField_Supported.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include "VS_SIPGetInfoInterface.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_Supported::e1(
		"(?i)"
		" *(?:supported|k) *: *"
		"(.*) *"
		"(?-i)"
	);
const boost::regex VS_SIPField_Supported::e2(".* *(?i)(?:timer)(?-i).*");
const boost::regex VS_SIPField_Supported::e3(".* *(?i)(?:100rel)(?-i).*");
const boost::regex VS_SIPField_Supported::e4(".* *(?i)(?:replaces)(?-i).*");

VS_SIPField_Supported::VS_SIPField_Supported():compact(false)
{
	VS_SIPField_Supported::Clean();
}

VS_SIPField_Supported::~VS_SIPField_Supported()
{
}

TSIPErrorCodes VS_SIPField_Supported::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
	if ( (TSIPErrorCodes::e_ok != err) || !ptr || !ptr_sz )
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
		dstream1 << "VS_SIPField_Supported::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Supported Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	FindParam_timer(ptr.get());
	FindParam_100rel(ptr.get());
	FindParam_replaces(ptr.get());

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Supported::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	std::string out = compact ? "k: " : "Supported: ";

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

	if (m_extensions & SIP_EXTENSION_100REL)
	{
		p = VS_SIPObjectFactory::GetSIPExtension(SIP_EXTENSION_100REL);
		if (p)
		{
			if (IsAnyExt)
				out += ", ";
			out += p;
		}
		IsAnyExt = true;
		p = 0;
	}

	if (m_extensions & SIP_EXTENSION_REPLACES)
	{
		p = VS_SIPObjectFactory::GetSIPExtension(SIP_EXTENSION_REPLACES);
		if (p)
		{
			if (IsAnyExt)
				out += ", ";
			out += p;
		}
		IsAnyExt = true;
		p = 0;
	}

	if (m_extensions & SIP_EXTENSION_PATH)
	{
		p = VS_SIPObjectFactory::GetSIPExtension(SIP_EXTENSION_PATH);
		if (p)
		{
			if (IsAnyExt)
				out += ", ";
			out += p;
		}
		IsAnyExt = true;
		p = 0;
	}

	if (m_extensions & SIP_EXTENSION_GRUU) {
		p = VS_SIPObjectFactory::GetSIPExtension(SIP_EXTENSION_GRUU);
		if (p) {
			if (IsAnyExt)
				out += ", ";
			out += p;
		}
		p = 0;
	}

	return aBuffer.AddData(out);
}

TSIPErrorCodes VS_SIPField_Supported::Init(const VS_SIPGetInfoInterface& call)
{
	if (call.GetMessageType() == TYPE_REGISTER) {
		AddExtension(SIP_EXTENSION_PATH);
	} else if (call.IsSessionTimerEnabled() || call.IsSessionTimerUsed()) {
		this->AddExtension(SIP_EXTENSION_TIMER);
	} else {
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	AddExtension(SIP_EXTENSION_GRUU);

	compact = call.IsCompactHeaderAllowed();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_Supported::AddExtension(const eSIP_ExtensionPack ext)
{
	m_extensions |= ext;
}

void VS_SIPField_Supported::Clean() noexcept
{
	VS_SIPError::Clean();

	m_extensions = 0;
}

std::unique_ptr<VS_BaseField> VS_SIPField_Supported_Instance()
{
	return vs::make_unique<VS_SIPField_Supported>();
}

bool VS_SIPField_Supported::FindParam_timer(string_view aInput)
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
		dstream1 << "VS_SIPField_Supported::FindParam_timer error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SIPField_Supported::FindParam_100rel(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(aInput.cbegin(), aInput.cend(), m, e3) )
		{
			m_extensions |= SIP_EXTENSION_100REL;
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Supported::FindParam_100rel error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SIPField_Supported::FindParam_replaces(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(aInput.cbegin(), aInput.cend(), m, e4))
		{
			m_extensions |= SIP_EXTENSION_REPLACES;
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Supported::FindParam_replaces error " << ex.what() << "\n";
	}
	return false;
}

#undef DEBUG_CURRENT_MODULE
