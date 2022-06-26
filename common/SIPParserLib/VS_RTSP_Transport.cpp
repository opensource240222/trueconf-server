#include "VS_RTSP_Transport.h"
#include "VS_RTSP_ParserInfo.h"
#include <sstream>
#include "std-generic/compat/memory.h"


std::unique_ptr<VS_BaseField> VS_RTSP_Transport_Instance()
{
	return vs::make_unique<VS_RTSP_Transport>();
}

TSIPErrorCodes VS_RTSP_Transport::Decode(VS_SIPBuffer &aBuffer)
{
	SetValid(false);
	std::unique_ptr<char[]> cpInput;
	const char *pos;
	std::size_t size;
	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(cpInput, size);
	if (TSIPErrorCodes::e_ok != err)
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	int res1(0), res2(0);
	if ( (pos = strstr(cpInput.get(), "client_port=")) )
		res1 = sscanf(pos, "client_port=%hu-", &m_LocalPort);
	if ( (pos = strstr(cpInput.get(), "server_port=")) )
		res2 = sscanf(pos, "server_port=%hu-", &m_RemotePort);

	const TSIPErrorCodes res = res1 || res2 ? TSIPErrorCodes::e_ok : TSIPErrorCodes::e_InputParam;

	if (res == TSIPErrorCodes::e_ok) SetValid(true);

	return res;
}


TSIPErrorCodes VS_RTSP_Transport::Encode(VS_SIPBuffer &aBuffer) const
{
	std::stringstream ssValue;

	ssValue << "Transport: RTP/AVP;unicast";

	if(!m_ip.is_unspecified() && m_useRemoteTransceiver)
	{
		ssValue << ";destination=";
		/*ssValue << (m_ip >> 24 & 0xFF) << "." << (m_ip >> 16 & 0xFF) << "."
		<< (m_ip >>  8 & 0xFF) << "." << (m_ip >>  0 & 0xFF) ;*/

		ssValue << m_ip.to_string();
	}

	ssValue<< ";client_port=" << m_LocalPort << "-" << m_LocalPort + 1;

	ssValue << "\r\n";
	aBuffer.AddData(ssValue.str());
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_RTSP_Transport::Init(VS_RTSP_ParserInfo* call)
{
	m_LocalPort = call->GetPort();
	m_ip = call->GetIp();
	m_useRemoteTransceiver = call->UseRemoteTransceiver();
	SetValid(true);
	return TSIPErrorCodes::e_ok;
}

void VS_RTSP_Transport::Clean() noexcept
{
	VS_BaseField::Clean();
	m_LocalPort = 0;
	m_RemotePort = 0;
	m_ip = net::address{};
}

VS_RTSP_Transport::VS_RTSP_Transport()
	: m_LocalPort(0)
	, m_RemotePort(0)
{
	VS_BaseField::Clean();
}
