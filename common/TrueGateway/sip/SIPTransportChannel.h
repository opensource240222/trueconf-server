#pragma once

#include "TrueGateway/sip/SIPChannelEventListener.h"
#include "net/QoSSettings.h"
#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/SharedBuffer.h"
#include "tools/Server/vs_messageQueue.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <queue>
#include "net/Logger/LoggerInterface.h"


class VS_SIPMessage;
namespace sip{

class Channel : public vs::enable_shared_from_this<Channel> {
protected:
	boost::asio::io_service::strand m_strand;
	const unsigned int m_id;
	std::string m_logId;
	VS_SIPInputMessageQueue		m_queueIn;
	std::weak_ptr<ChannelEventListener> m_eventListener;
	net::QoSFlowSharedPtr			m_flow;
	std::shared_ptr<net::LoggerInterface> m_logger;
	net::LoggerInterface::ConnectionInfo m_channel_log_info;

	std::shared_ptr<VS_SIPMessage> GetMsg();
	void ProcessInputMsgs();
	virtual const std::string& LogID() = 0;
public:
	virtual ~Channel() {}
	virtual void Close() = 0;
	virtual void Write(vs::SharedBuffer &&data) = 0;

	unsigned int GetID() const;
	std::string GetLogID();

protected:
	Channel(boost::asio::io_service::strand &strand, unsigned int id, uint32_t msgMaxSize, const net::QoSFlowSharedPtr &flow,
			const std::weak_ptr<ChannelEventListener> &eventListener, const std::shared_ptr<net::LoggerInterface> &logger)
		: m_strand(strand)
		, m_id(id)
		, m_queueIn(msgMaxSize)
		, m_eventListener(eventListener)
		, m_flow(flow)
		, m_logger(logger)
	{}
};
}