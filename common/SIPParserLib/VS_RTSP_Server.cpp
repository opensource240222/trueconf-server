#include "VS_RTSP_Server.h"

#include "std-generic/compat/memory.h"
#include <algorithm>
#include <cstring>

std::unique_ptr<VS_BaseField> VS_RTSP_Server_Instance()
{
	return vs::make_unique<VS_RTSP_Server>();
}
TSIPErrorCodes VS_RTSP_Server::Decode(VS_SIPBuffer &aBuffer)
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
	const char * ptr = strstr(cpInput.get(), "SERVER:");
	if( ptr == nullptr)
	{
		SetError(TSIPErrorCodes::e_InputParam);
		SetValid(false);
	}
	ptr+=8;
	m_server.assign( ptr, iInputSize - ( ptr - cpInput.get() ) );
	SetValid(true);
	return TSIPErrorCodes::e_ok;

}
TSIPErrorCodes VS_RTSP_Server::Encode(VS_SIPBuffer &aBuffer) const
{
	if(!IsValid())
	{
		return GetLastClassError();
	}
	aBuffer.AddData("Server: ");
	aBuffer.AddData(m_server);
	aBuffer.AddData("\r\n");
	return TSIPErrorCodes::e_ok;
}

const std::string &VS_RTSP_Server::Get(void) const
{
	return m_server;
}
