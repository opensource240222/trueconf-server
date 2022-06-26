#include "VS_RTSP_Session.h"
#include "VS_RTSP_ParserInfo.h"
#include "std-generic/compat/memory.h"
#include <boost/regex.hpp>

static const boost::regex header_re("Session: *([A-Za-z0-9$_.+-]+)(?: *; *timeout=([0-9]+))?", boost::regex::optimize);

std::unique_ptr<VS_BaseField> VS_RTSP_Session_Instance()
{
	return vs::make_unique<VS_RTSP_Session>();
}
TSIPErrorCodes VS_RTSP_Session::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> cpInput;
	std::size_t iInputSize;
	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(cpInput, iInputSize);
	if(TSIPErrorCodes::e_ok != err)
	{
		SetValid(false);
		SetError(err);
		return err;

	}

	boost::cmatch header_match;
	if (!boost::regex_search(cpInput.get(), header_match, header_re))
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	m_sSessionId = header_match.str(1);
	if (header_match.size() >= 2)
	{
		m_timeout = std::chrono::seconds(atoi(header_match[2].first));
	}
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_RTSP_Session::Encode(VS_SIPBuffer &aBuffer) const
{
	if(!IsValid())
		return GetLastClassError();
	if(m_sSessionId.empty())
	{
		return TSIPErrorCodes::e_ok;
	}
	aBuffer.AddData("Session: ");
	aBuffer.AddData(m_sSessionId);
	aBuffer.AddData("\r\n");
	return TSIPErrorCodes::e_ok;
}
VS_RTSP_Session::VS_RTSP_Session() : m_timeout(0) // NOTE: The timeout is measured in seconds, with a default of 60 seconds (1 minute). Maybe bugs.
{
}
TSIPErrorCodes VS_RTSP_Session::Init(VS_RTSP_ParserInfo* call)
{
	m_sSessionId = call->GetSessionsID();
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}