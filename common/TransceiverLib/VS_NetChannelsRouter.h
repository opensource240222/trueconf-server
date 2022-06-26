#pragma once
#include <map>
#include <string>

#include "std/cpplib/event.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include "std-generic/compat/memory.h"
#include "TransceiverLib_v2/NetChannel.h"


class VS_NetworkRelayMessageBase;
class VS_NamedTransceiverNetChannel;
class VS_MainRelayMessage;

class VS_NetChannelsRouter : public vs::enable_shared_from_this<VS_NetChannelsRouter>
{
public:
	virtual ~VS_NetChannelsRouter();
	void ConnectToServer(string_view addresses, const unsigned char* secretData, const unsigned long sz, const char* channel_id);
	void CloseChannel(const std::string &id);
	void SendMsg(const std::string &channel_id, const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess);

	virtual void ProcessingRcvMessage(const boost::shared_ptr<VS_MainRelayMessage> &rcvMess) = 0;

	void Stop();
	void WaitForStop();
protected:
	VS_NetChannelsRouter(boost::asio::io_service& ios, std::string m_circuit_name);
private:
	template <class Protocol>
	void Connect(string_view addresses, const unsigned char* secretData, const unsigned long sz, const char* channel_id);

	void DeleteChannel(const std::shared_ptr<ts::NetChannelInterface> &channel);

	std::map<std::string,std::shared_ptr<ts::NetChannelInterface>>	m_channels;

	bool				m_isStopped;
	vs::event			m_stop_event;
protected:
	boost::asio::io_service::strand m_strand;
	const std::string	m_circuit_name;
};