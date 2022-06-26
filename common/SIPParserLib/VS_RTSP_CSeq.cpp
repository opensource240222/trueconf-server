#include "VS_RTSP_CSeq.h"
#include "VS_RTSP_ParserInfo.h"

#include <string>
#include <algorithm>
#include "std-generic/compat/memory.h"

TSIPErrorCodes VS_RTSP_CSeq::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> input;
	std::size_t iSize=0;
	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(input, iSize);
	if(TSIPErrorCodes::e_ok != err)
	{
		SetValid(false);
		SetError(err);
		return err;

	}
	std::transform(input.get(), input.get() + iSize, input.get(), ::toupper);
	const char * iter = strstr(input.get(), "CSEQ:");
	if(iter == nullptr)
		return TSIPErrorCodes::e_badObjectState;
	iter += 6;
	m_CSeqValue = atoi(iter);
	//delete [] cUpperInput;
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}
TSIPErrorCodes VS_RTSP_CSeq::Encode(VS_SIPBuffer &aBuffer) const
{
	if(!IsValid())
		return GetLastClassError();
	aBuffer.AddData("CSeq: ");
	aBuffer.AddData(std::to_string(m_CSeqValue));
	aBuffer.AddData("\r\n");
	return TSIPErrorCodes::e_ok;
}
std::unique_ptr<VS_BaseField> VS_RTSP_CSeq_Instance()
{
	return vs::make_unique<VS_RTSP_CSeq>();
}
VS_RTSP_CSeq::VS_RTSP_CSeq() : m_CSeqValue(0)
{
}
TSIPErrorCodes VS_RTSP_CSeq::Init(VS_RTSP_ParserInfo * info)
{
	m_CSeqValue = info->GetCseq();
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}