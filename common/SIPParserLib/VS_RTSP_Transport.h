#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include "net/Address.h"
#include "net/Port.h"

class VS_RTSP_Transport : public VS_BaseField
{
public:
	VS_RTSP_Transport();
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;
	TSIPErrorCodes Init(VS_RTSP_ParserInfo* call) override;
	net::port GetLocalPort() const { return m_LocalPort; }
	net::port GetRemotePort() const { return m_RemotePort; }
	//int GetIP(){ return m_ip; }
	// Warning! You can use only VS_IPPort::ipv*() value, port and connection  type are undefined.

	const net::address &GetIp() const { return m_ip; }
	void Clean() noexcept override;
private:
	net::port m_LocalPort;
	net::port m_RemotePort;
	net::address m_ip;
	bool m_useRemoteTransceiver = false;

};

std::unique_ptr<VS_BaseField> VS_RTSP_Transport_Instance();