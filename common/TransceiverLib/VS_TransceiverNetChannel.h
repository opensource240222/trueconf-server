#pragma once
#include "acs/connection/VS_IOHandler.h"
#include "TransceiverLib/NetChannelInterface.h"
#include "VS_SetConnectionInterface.h"
#include <boost/signals2.hpp>
#include "../std/cpplib/event.h"

#include <set>
#include <queue>

class VS_ConnectionTCP;
class VS_WorkThread;
class VS_MainRelayMessage;
class VS_MessageHandlerAdapter;
class VS_MessageData;
class VS_MessResult;
class VS_AuthConnectionInterface;


class VS_TransceiverNetChannel :public VS_IOHandler,
								public ts::NetChannelInterface,
								public VS_SetConnectionInterface

{
	enum MessageType
	{
		e_set_connection,
		e_stop,
		e_send_msg,
		e_get_remote_address,
	};
	class SendMsgMessage;

public:

	VS_TransceiverNetChannel(const boost::shared_ptr<VS_WorkThread> &thread);
	virtual ~VS_TransceiverNetChannel();
	boost::signals2::connection ConnectToOnConnectionDie(const boost::signals2::signal<void(void)>::slot_type &slot)
	{
		return m_fireOnConnectionDie.connect(slot);
	}
	bool SendMsg(const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess) override;
	net::address GetRemoteAddress() const override;

	bool SetTCPConnection(VS_ConnectionTCP *conn, const void *in_buf, const unsigned long in_len) override;
	bool SetChannelConnection(VS_ConnectionTCP *conn);
	void SetConnectionAuthenticator(const boost::shared_ptr<VS_AuthConnectionInterface> &auth_conn);
	void StopActivity() override;
	void Stop();
	void WaitForStop();

private:
	boost::shared_ptr<VS_MainRelayMessage>	m_rcvMess;
	///void HandleMessage(const boost::shared_ptr<VS_MessageData> &message);

private:
	void Handle(const unsigned long sz, const struct VS_Overlapped* ov) override;
	void HandleError(const unsigned long err, const struct VS_Overlapped* ov) override;

	bool HandleRead(const unsigned long sz, const VS_Overlapped *ov);
	bool HandleWrite(const unsigned long sz, const VS_Overlapped *ov);
	bool WriteNextMessage();
	void DeleteConn(VS_ConnectionTCP *conn);

	void HandleMess(const boost::shared_ptr<VS_MessageData> &mess);
	void HandleMessWithRes(const boost::shared_ptr<VS_MessageData> &mess,const boost::shared_ptr<VS_MessResult> &res);


	VS_ConnectionTCP						*m_channelConn;
	boost::shared_ptr<VS_WorkThread>		m_workThread;
	bool									m_isStopped;
	vs::event								m_stop_event;
	boost::shared_ptr<VS_NetworkRelayMessageBase>				m_writeMess;
	std::queue<boost::shared_ptr<VS_NetworkRelayMessageBase>>	m_mess_queue;

	boost::shared_ptr<VS_MessageHandlerAdapter>		m_messHandler;
	boost::signals2::connection						m_handleMessConn;
	boost::signals2::connection						m_handleMessWithResultConn;

	std::set<VS_ConnectionTCP*>						m_waiting_del_conns;
	boost::signals2::signal<void(void)>				m_fireOnConnectionDie;
	boost::weak_ptr<VS_AuthConnectionInterface>		m_auth_conn;
};
