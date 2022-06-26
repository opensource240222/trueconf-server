#pragma once

#include "TransceiverLib/VS_RelayModule.h"
#include "std/cpplib/ASIOThreadPool.h"
#include "std/cpplib/event.h"

#include <memory>


class VS_DebugKeysReader;
class VS_CommandReceiver;
class VS_FrameReceiver;
class VS_FrameSender;
class VS_WebRTCBroadcaster;
class VS_ConfRecorderModuleReceiver;
class VS_RTSPBroadcastModuleReceiver;
class VS_TransceiverPartsMgr;
class VS_ConfControlReceiver;
class VS_RTPModuleReceiver;

class VS_RelayProc : public VS_RelayModule
{
protected:
	VS_RelayProc(const char *serverEP, const char *addrs, std::string circuit_name, const unsigned char *secretData, const unsigned long sz);
	static void PostConstruct(std::shared_ptr<VS_RelayProc>& p)
	{
		p->m_this_weak = p;
	}
public:
	void Run();

	bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess) override;
	const std::string				m_circuit_name;
	std::vector<unsigned char>		m_secretData;
	std::string						m_serverList;
	std::string						m_serverEP;

	vs::ASIOThreadPool m_atp;

	std::shared_ptr<VS_DebugKeysReader>				m_debug_keys_reader;
	std::shared_ptr<VS_CommandReceiver>				m_cmdReceiver;
	std::shared_ptr<VS_FrameReceiver>					m_frameReceiver;
	std::shared_ptr<VS_WebRTCBroadcaster>				m_broadcaster;
	std::shared_ptr<VS_ConfRecorderModuleReceiver>	m_confWriteModule;
	std::shared_ptr<VS_RTSPBroadcastModuleReceiver>		m_RTSPBroadcastModule;
	std::shared_ptr<VS_RTPModuleReceiver>				m_RTPModule;
	std::shared_ptr<VS_TransceiverPartsMgr>			m_partsMgr;
	std::shared_ptr<VS_ConfControlReceiver>			m_confControlReceiver;

	boost::shared_ptr<VS_FrameSender>				m_frameSender;

	vs::event	m_stop_event;
	std::weak_ptr<VS_RelayProc> m_this_weak;

};