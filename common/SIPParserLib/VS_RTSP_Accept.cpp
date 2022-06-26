#include "VS_RTSP_Accept.h"
#include "VS_RTSP_ParserInfo.h"

#include <algorithm>
#include "std-generic/compat/memory.h"

std::unique_ptr<VS_BaseField> VS_RTSP_Accept_Instanse()
{
	return vs::make_unique<VS_RTSP_Accept>();
}
TSIPErrorCodes VS_RTSP_Accept::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> cpInput;
	std::size_t iInputSize=0;
	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(cpInput, iInputSize);
	if(TSIPErrorCodes::e_ok != err)
	{

		SetValid(false);
		SetError(err);
		return err;

	}
	std::transform(cpInput.get(), cpInput.get() + iInputSize, cpInput.get(), ::toupper);
	const char * ptr = strstr(cpInput.get(), "ACCEPT:");
	if( ptr == nullptr)
	{
		SetError(TSIPErrorCodes::e_InputParam);
		SetValid(false);
	}
	ptr+=8;
	m_sAccept.assign(ptr, iInputSize - (ptr - cpInput.get()));
	SetValid(true);
	//delete [] cUpperInput;
	return TSIPErrorCodes::e_ok;
}
TSIPErrorCodes VS_RTSP_Accept::Encode(VS_SIPBuffer &aBuffer) const
{
	if(!IsValid())
		return GetLastClassError();
	aBuffer.AddData("Accept: ");
	aBuffer.AddData(m_sAccept);
	aBuffer.AddData("\r\n");
	return TSIPErrorCodes::e_ok;
}
TSIPErrorCodes VS_RTSP_Accept::Init(VS_RTSP_ParserInfo * parsingInfo)
{
	m_sAccept = parsingInfo->GetAccept();
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}