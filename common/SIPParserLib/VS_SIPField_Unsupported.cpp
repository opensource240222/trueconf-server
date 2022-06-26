#include "VS_SIPField_Unsupported.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_Unsupported::e1(
		"(?i)"
		" *(?:unsupported) *: *"
		"(.*) *"
		"(?-i)"
	);
const boost::regex VS_SIPField_Unsupported::e2(".* *(?i)(?:timer)(?-i).*");
const boost::regex VS_SIPField_Unsupported::e3(".* *(?i)(?:100rel)(?-i).*");
const boost::regex VS_SIPField_Unsupported::e4(".* *(?i)(?:replaces)(?-i).*");

VS_SIPField_Unsupported::VS_SIPField_Unsupported()
{
	VS_SIPField_Unsupported::Clean();
}

VS_SIPField_Unsupported::~VS_SIPField_Unsupported()
{
}

TSIPErrorCodes VS_SIPField_Unsupported::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
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
		dstream1 << "VS_SIPField_Unsupported::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Unsupported Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	FindParam_timer( ptr.get() );
	FindParam_100rel( ptr.get() );
	FindParam_replaces( ptr.get() );

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Unsupported::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	std::string out = "Unsupported: ";

	const char* p;
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
		p = 0;
	}

	return aBuffer.AddData(out);
}

TSIPErrorCodes VS_SIPField_Unsupported::Init(const VS_SIPGetInfoInterface& call)
{
	this->AddExtension(SIP_EXTENSION_TIMER);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_Unsupported::AddExtension(const eSIP_ExtensionPack ext)
{
	m_extensions |= ext;
}

void VS_SIPField_Unsupported::Clean() noexcept
{
	VS_SIPError::Clean();

	m_extensions = 0;
}

std::unique_ptr<VS_BaseField> VS_SIPField_Unsupported_Instance()
{
	return vs::make_unique<VS_SIPField_Unsupported>();
}

bool VS_SIPField_Unsupported::FindParam_timer(string_view aInput)
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
		dstream1 << "VS_SIPField_Unsupported::FindParam_timer error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SIPField_Unsupported::FindParam_100rel(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if ( boost::regex_match(aInput.cbegin(), aInput.cend(), m, e3) )
		{
			m_extensions |= SIP_EXTENSION_100REL;
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Unsupported::FindParam_100rel error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SIPField_Unsupported::FindParam_replaces(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if ( boost::regex_match(aInput.cbegin(), aInput.cend(), m, e4) )
		{
		    m_extensions |= SIP_EXTENSION_REPLACES;
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Unsupported::FindParam_replaces error " << ex.what() << "\n";
	}
	return false;
}

#undef DEBUG_CURRENT_MODULE
