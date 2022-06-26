#include "VS_SIPField_Event.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include <memory>
#include "std-generic/compat/memory.h"
#include "VS_SIPGetInfoInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_Event::e(
		"(?i)"
		" *(?:event|o) *: *"
		"([\\w\\-]+) *"
		"(?-i)"
	);

VS_SIPField_Event::VS_SIPField_Event() : m_event(SIP_EVENT_INVALID), m_compact(false)
{

}

VS_SIPField_Event::~VS_SIPField_Event()
{

}

TSIPErrorCodes VS_SIPField_Event::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = aBuffer.GetNextBlockAllocConst(ptr, ptr_sz);
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
		dstream1 << "VS_SIPField_Event::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Event Field: buffer not match, dump |" << ptr.get() << "|";
		SetError(TSIPErrorCodes::e_match);
        return TSIPErrorCodes::e_match;
	}

	const std::string &ev = m[1];

	m_event = VS_SIPObjectFactory::GetEvent( ev.c_str() );

	aBuffer.SkipHeader();
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Event::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	const char* ev = VS_SIPObjectFactory::GetEvent( m_event );
	if ( !ev )
		return TSIPErrorCodes::e_InputParam;

	if (m_compact)
		aBuffer.AddData("o: ");
	else
		aBuffer.AddData("Event: ");
	aBuffer.AddData(ev, strlen(ev));

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Event::Init(const VS_SIPGetInfoInterface& call)
{
	m_event = SIP_EVENT_PRESENCE;
	m_compact = call.IsCompactHeaderAllowed();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

eSIP_Events VS_SIPField_Event::Event() const
{
	return m_event;
}

void VS_SIPField_Event::Event(const eSIP_Events ev)
{
	m_event = ev;
}

std::unique_ptr<VS_BaseField> VS_SIPField_Event_Instance()
{
	return vs::make_unique<VS_SIPField_Event>();
}

void VS_SIPField_Event::Clean() noexcept
{
	VS_SIPError::Clean();

	m_event = SIP_EVENT_INVALID;
}

#undef DEBUG_CURRENT_MODULE
