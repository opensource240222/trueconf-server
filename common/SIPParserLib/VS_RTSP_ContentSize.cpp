#include "VS_RTSP_ContentSize.h"
#include "VS_RTSP_ParserInfo.h"

#include <string>
#include <algorithm>
#include "std-generic/compat/memory.h"

std::unique_ptr<VS_BaseField> VS_RTSP_ContentLength_Instance()
{
	return vs::make_unique<VS_RTSP_ContentLength>();
}

VS_RTSP_ContentLength::VS_RTSP_ContentLength()
	: m_ContentLength(0)
{
}

TSIPErrorCodes VS_RTSP_ContentLength::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> cpInput;
	std::size_t iInputSize = 0;
	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(cpInput, iInputSize);
	if(TSIPErrorCodes::e_ok != err)
	{
		SetValid(false);
		SetError(err);
		return err;

	}
	std::transform(cpInput.get(), cpInput.get() + iInputSize, cpInput.get(), ::toupper);
	const char * ptr = strstr(cpInput.get(), "CONTENT-LENGTH:");
	if(ptr == nullptr)
	{
		SetError(TSIPErrorCodes::e_InputParam);
		SetValid(false);
		return TSIPErrorCodes::e_InputParam;
	}
	ptr+=16;
	m_ContentLength = atoi(ptr);
	SetValid(true);

	return TSIPErrorCodes::e_ok;
}
TSIPErrorCodes VS_RTSP_ContentLength::Encode(VS_SIPBuffer &aBuffer) const
{
	if(!IsValid())
		return GetLastClassError();
	aBuffer.AddData("Content-Length: ");
	aBuffer.AddData(std::to_string(m_ContentLength));
	aBuffer.AddData("\r\n");
	return TSIPErrorCodes::e_ok;
}
TSIPErrorCodes VS_RTSP_ContentLength::Init(VS_RTSP_ParserInfo * info)
{
	m_ContentLength = info->GetContentLength();
	return TSIPErrorCodes::e_ok;
}

std::size_t VS_RTSP_ContentLength::GetContentLength() const
{
	return m_ContentLength;
}