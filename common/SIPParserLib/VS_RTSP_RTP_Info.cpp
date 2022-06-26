#include "VS_RTSP_RTP_Info.h"

#include <cstdlib>
#include <cstring>

VS_BaseField* VS_RTP_Info_Instance()
{
	return new VS_RTSP_RTP_Info;
}

VS_RTSP_RTP_Info::VS_RTSP_RTP_Info()
	: m_rtpInfo({0,0,0})
{
}

TSIPErrorCodes VS_RTSP_RTP_Info::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> input = 0;
	std::size_t size=0;
	aBuffer.GetNextBlockAllocConst(input, size);
	char * iter = strstr(input.get(), "url=trackID=");
	if(iter == nullptr)
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}
	m_rtpInfo.trackId = atoi(iter);
	iter = strstr(input.get(), "seq=");
	if(iter == nullptr)
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}
	m_rtpInfo.seq = atoi(iter);
	iter = strstr(input.get(), "rtptime=");
	if(iter == nullptr)
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}
	m_rtpInfo.rtptime = atoi(iter);

	return TSIPErrorCodes::e_ok;
}
TSIPErrorCodes VS_RTSP_RTP_Info::Encode(VS_SIPBuffer &) const
{
	return TSIPErrorCodes::e_ok;
}