#pragma once

#include "TransceiverLib/VS_RelayMessageSenderInterface.h"

class VS_MainRelayMessage;

namespace ts
{

class NetChannelInterface: public VS_RelayMessageSenderInterface
{
public:
	virtual ~NetChannelInterface() = default;

	virtual void StopActivity() = 0;

	template<class Callback>
	void SetRecvMessageCallBack(Callback && f){
		m_fireProcessRecvMessage = std::forward<Callback>(f);
	}

	template<class Callback>
	void SetOnTransceiverReady(Callback &&call_back) {
		m_fireTransceiverReady = std::forward<Callback>(call_back);
	}

	template<class Callback>
	void SetTransceiverName(Callback &&call_back) {
		m_fireSetTransName = std::forward<Callback>(call_back);
	}

protected:
	std::function<void(boost::shared_ptr<VS_MainRelayMessage>&)>	m_fireProcessRecvMessage;
	std::function<void(const std::string&)>			m_fireTransceiverReady;
	std::function<void(const std::string&)>			m_fireSetTransName;
};

}
