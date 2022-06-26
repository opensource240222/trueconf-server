#include "VS_RTSP_UserAgent.h"
#include "VS_RTSP_ParserInfo.h"
#include "std-generic/compat/memory.h"

std::unique_ptr<VS_BaseField> VS_RTSP_UserAgent_Instanse()
{
	return vs::make_unique<VS_RTSP_UserAgent>();
}

VS_RTSP_UserAgent::VS_RTSP_UserAgent()
{
	VS_BaseField::Clean();
}

TSIPErrorCodes VS_RTSP_UserAgent::Decode(VS_SIPBuffer &aBuffer)
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
	const char * ptr = strstr(cpInput.get(), "User-Agent:");
	if(ptr == nullptr)
	{
		SetError(TSIPErrorCodes::e_InputParam);
		SetValid(false);
		return TSIPErrorCodes::e_InputParam;
	}
	ptr+=12;
	m_sUserAgent.assign( ptr, iInputSize - ( ptr - cpInput.get() ) );
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}
TSIPErrorCodes VS_RTSP_UserAgent::Encode(VS_SIPBuffer &aBuffer) const
{
	if(!IsValid())
	{
		return GetLastClassError();
	}
	aBuffer.AddData("User-Agent: ");
	aBuffer.AddData(m_sUserAgent);
	aBuffer.AddData("\r\n");
	return TSIPErrorCodes::e_ok;
}
TSIPErrorCodes VS_RTSP_UserAgent::Init(VS_RTSP_ParserInfo * call)
{
	m_sUserAgent = call->GetUserAgent();
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}

void VS_RTSP_UserAgent::Clean() noexcept
{
	VS_BaseField::Clean();
	m_sUserAgent.clear();
}
