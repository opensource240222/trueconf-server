#include "VS_RTSP_StartLine.h"
#include "VS_RTSP_ParserInfo.h"
#include "std/debuglog/VS_Debug.h"
#include <boost/regex.hpp>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex regRtspRequest("((play|options|describe|get_parameter|setup|teardown)\\srtsp://.*rtsp/[0-9].[0.9])", boost::regex::icase);
const boost::regex regRtspResponse("RTSP/[0-9].[0.9].*", boost::regex::icase);

std::unique_ptr<VS_BaseField> VS_RTSP_StartLine_Instance()
{
	return vs::make_unique<VS_RTSP_StartLine>();
}

TSIPErrorCodes VS_RTSP_StartLine::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> cpInput;
	std::size_t iSize=0;
	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(cpInput, iSize);
	if(TSIPErrorCodes::e_ok != err)
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	try{
		if(boost::regex_match(cpInput.get(), regRtspRequest))
	{
		char* cUrlBegin, *cUrlEnd;
		bResponseOrRequest = false;
		if(strstr(cpInput.get(), "OPTIONS"))
			m_requestType = REQUEST_OPTIONS;
		else
		if(strstr(cpInput.get(), "DESCRIBE"))
			m_requestType = REQUEST_DESCRIBE;
		else
		if(strstr(cpInput.get(), "PLAY"))
			m_requestType = REQUEST_PLAY;
		else
		if(strstr(cpInput.get(), "SETUP"))
			m_requestType = REQUEST_SETUP;
		else
		if(strstr(cpInput.get(), "TEARDOWN"))
			m_requestType = REQUEST_TEARDOWN;
		else
		if(strstr(cpInput.get(), "GET_PARAMETER"))
			m_requestType = REQUEST_GET_PARAMETER;
		cUrlBegin = strstr(cpInput.get(), "rtsp://");
		cUrlEnd = strstr(cpInput.get(), "RTSP/");
		cUrlEnd-=1;
		m_url.assign(cUrlBegin, (cUrlEnd-cUrlBegin));
		SetValid(true);
		return TSIPErrorCodes::e_ok;
	}
		else
			if(boost::regex_match(cpInput.get(), regRtspResponse))
		{
			if(strstr(cpInput.get(), "RTSP/1.0 200 OK"))
			{
				m_responseType = RESPONSE_200_OK;
			} else
			{
				sscanf(cpInput.get(), "RTSP/1.0 %d", &m_returnCode);
			}

			SetValid(true);
			return TSIPErrorCodes::e_ok;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_RTSP_StartLine::Decode() error " << ex.what() << "\n";
		SetValid(false);
		return TSIPErrorCodes::e_match;
	}
	SetValid(false);
	return TSIPErrorCodes::e_InputParam;
}

VS_RTSP_StartLine::VS_RTSP_StartLine()
	: m_responseType(RESPONSE_invalid)
	, m_requestType(REQUEST_invalid)
	, m_returnCode(0)
	, bResponseOrRequest(false)
{
}

TSIPErrorCodes VS_RTSP_StartLine::Encode(VS_SIPBuffer &aBuffer) const
{
	if(!IsValid())
	{
		return GetLastClassError();
	}
	std::string sOutput;
	if(bResponseOrRequest)
	{
		if(m_responseType == RESPONSE_200_OK)
		{
			sOutput+="RTSP/1.0 200 OK\r\n";
		}
	}
	else
	{
		switch (m_requestType)
		{
			case REQUEST_ANNOUNCE: sOutput+="ANNOUNCE "; break;
			case REQUEST_DESCRIBE: sOutput+="DESCRIBE "; break;
			case REQUEST_OPTIONS: sOutput+="OPTIONS "; break;
			case REQUEST_SETUP: sOutput+="SETUP "; break;
			case REQUEST_PLAY: sOutput+="PLAY "; break;
			case REQUEST_PAUSE: sOutput+="PAUSE "; break;
			case REQUEST_TEARDOWN: sOutput+="TEARDOWN "; break;
			case REQUEST_GET_PARAMETER: sOutput+="GET_PARAMETER "; break;

		}

		sOutput+=m_url;

		sOutput+=" RTSP/1.0";
		sOutput+="\r\n";

	}
	aBuffer.AddData(sOutput);

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_RTSP_StartLine::Init(VS_RTSP_ParserInfo * parsingInfo)
{
	bResponseOrRequest = !parsingInfo->IsRequest();
	m_url = parsingInfo->GetUrl();

	if(bResponseOrRequest)
	{
		m_responseType = parsingInfo->GetResponseType();
	}
	else
	{
		m_requestType = parsingInfo->GetRequestType();
		if (m_requestType == REQUEST_SETUP)
		{
			m_url = parsingInfo->GetControlUrl();
		}
	}

	SetValid(true);
	return TSIPErrorCodes::e_ok;
}

int VS_RTSP_StartLine::getReturnCode() const
{
	if(m_responseType == RESPONSE_200_OK)	return 200;
	return m_returnCode;
}

#undef DEBUG_CURRENT_MODULE
