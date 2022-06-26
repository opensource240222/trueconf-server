#pragma once
#include "VS_SetConnectionInterface.h"
#include "acs/connection/VS_IOHandler.h"
#include "std/cpplib/VS_Lock.h"
#include "std/cpplib/VS_SimpleStr.h"
#include "std/cpplib/VS_MessageHandler.h"
#include "streams/fwd.h"
#include "streams/Relay/VS_TransmitFrameInterface.h"

#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/signals2.hpp>
#include <queue>
#include <set>
#include <map>

/**
	transmit data to remote process
**/

class VS_WorkThread;
class VS_NetworkRelayMessageBase;
class VS_AuthConnectionInterface;

class VS_RemoteCircuitFrameTransmit :	public VS_SetConnectionInterface,
										VS_Lock,
										public VS_IOHandler,
										public VS_MessageHandler,
										public VS_TransmitFrameInterface
{
	enum MessageType
	{
		e_transmit_frame_mess,
	};
	class TransmitFrameMessage;

	class VS_CircuitFrameTransmitMessageHandler : public VS_MessageHandler {
		std::weak_ptr<VS_RemoteCircuitFrameTransmit> m_frame_transmit;
	public:
		explicit VS_CircuitFrameTransmitMessageHandler(std::shared_ptr<VS_RemoteCircuitFrameTransmit> &frame_transmit) :m_frame_transmit(frame_transmit) {}
		void HandleMessage(const boost::shared_ptr<VS_MessageData> &message) override {
			if (auto fr_tr = m_frame_transmit.lock()) fr_tr->HandleMessage(message);
		}
	};

	///VS_ConnectionTCP										*m_conn;

	std::map<VS_SimpleStr,VS_ConnectionTCP*>			m_conns_by_conf;
	struct ConnectionInfo
	{
		VS_SimpleStr												conf_name;
		std::queue<boost::shared_ptr<VS_NetworkRelayMessageBase>>	writeMessQueue;
		boost::shared_ptr<VS_NetworkRelayMessageBase>				lastWriteMess;
		boost::shared_ptr<unsigned char>							rcv_byte;
	};
	std::map<VS_ConnectionTCP*,ConnectionInfo>			m_conns_info;

	boost::shared_ptr<VS_WorkThread>					m_processing_thread;
	boost::weak_ptr<VS_AuthConnectionInterface>			m_auth_conn;
	//std::queue<boost::shared_ptr<VS_MainRelayMessage>>	m_mess_queue;
	///boost::shared_ptr<VS_NetworkRelayMessageBase>				m_writeMess;

	std::map<VS_ConnectionTCP*,ConnectionInfo>			m_delConn;


	void DeleteConn(VS_ConnectionTCP *conn);
	bool Write(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess, const char *conf_name);
	bool WriteNextMessage(VS_ConnectionTCP *conn);
	bool HandleRead(const unsigned long sz, const VS_Overlapped *ov);
	bool HandleWrite(const unsigned long sz, const VS_Overlapped *ov);


	boost::shared_ptr<VS_MessageHandler>					m_mess_handler;

protected:
	VS_RemoteCircuitFrameTransmit(const boost::shared_ptr<VS_WorkThread> &work_thread, const boost::shared_ptr<VS_AuthConnectionInterface> &auth);
	static void PostConstruct(std::shared_ptr<VS_RemoteCircuitFrameTransmit>& p)
	{
		p->m_mess_handler = boost::make_shared<VS_CircuitFrameTransmitMessageHandler>(p);
	}

public:
	virtual ~VS_RemoteCircuitFrameTransmit();
	void Close();
	void WaitClose();

	///SetConnectionInterface
	virtual bool SetTCPConnection(VS_ConnectionTCP *conn, const void *in_buf, const unsigned long in_len);
	//IOHandler Interface
	virtual void Handle(const unsigned long sz, const VS_Overlapped *ov);
	virtual void HandleError(const unsigned long err, const VS_Overlapped *ov);
	virtual void HandleMessage(const boost::shared_ptr<VS_MessageData> &message) override;

	void TransmitFrame(const char *conf_name, const char *part, const stream::FrameHeader *frame_head, const void *frame_data) override;
};