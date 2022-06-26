#include "VS_RTSP_Content_type.h"
#include "VS_RTSP_ParserInfo.h"

#include <algorithm>
#include "std-generic/compat/memory.h"

std::unique_ptr<VS_BaseField> VS_RTSP_Content_type_Instance()
{
	return vs::make_unique<VS_RTSP_Content_type>();
}

TSIPErrorCodes VS_RTSP_Content_type::Decode(VS_SIPBuffer &aBuffer)
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
	std::transform(cpInput.get(), cpInput.get() + iInputSize, cpInput.get(), ::toupper);
	const char * ptr = strstr(cpInput.get(), "CONTENT-TYPE:");
	if(ptr == nullptr)
	{
		SetError(TSIPErrorCodes::e_InputParam);
		SetValid(false);
		return TSIPErrorCodes::e_InputParam;
	}
	ptr+=14;
	m_sContentType.assign( ptr, iInputSize - ( ptr - cpInput.get() ) );
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_RTSP_Content_type::Encode(VS_SIPBuffer &aBuffer) const
{
	if(!IsValid())
		return GetLastClassError();
	aBuffer.AddData("Content-Type: ");
	aBuffer.AddData(m_sContentType);
	aBuffer.AddData("\r\n");
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_RTSP_Content_type::Init(VS_RTSP_ParserInfo * info)
{
	m_sContentType=info->GetContentType();
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}

const std::string &VS_RTSP_Content_type::GetContentType() const
{
	return m_sContentType;
}
