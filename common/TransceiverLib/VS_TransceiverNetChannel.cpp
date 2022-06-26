#ifdef _WIN32 // not ported
#include "VS_TransceiverNetChannel.h"
#include "std/cpplib/VS_WorkThread.h"
#include "std/cpplib/VS_MessageHandlerAdapter.h"
#include "VS_MainRelayMessage.h"
#include "acs/connection/VS_ConnectionTCP.h"
#include "acs/connection/VS_IO_Operation_Type.h"
#include "acs/connection/VS_ConnectionOv.h"
#include "VS_AuthConnectionInterface.h"
#include "net/ConvertAddress.h"
#include "net/Handshake.h"

#include <boost/make_shared.hpp>

class VS_TransceiverNetChannel::SendMsgMessage : public VS_MessageData
{
private:
	boost::shared_ptr<VS_NetworkRelayMessageBase> m_mess;
public:
	SendMsgMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess) : VS_MessageData(e_send_msg,0,0),m_mess(mess)
	{}
	const boost::shared_ptr<VS_NetworkRelayMessageBase> GetMsg() const
	{
		return m_mess;
	}
};

VS_TransceiverNetChannel::VS_TransceiverNetChannel(const boost::shared_ptr<VS_WorkThread> &thread)
	: m_channelConn(0)
	, m_workThread(thread)
	, m_isStopped(false)
	, m_stop_event(true)
{
	m_messHandler = boost::signals2::deconstruct<VS_MessageHandlerAdapter>();
	m_handleMessConn = m_messHandler->ConnectToHandleMessage(boost::bind(&VS_TransceiverNetChannel::HandleMess,this,_1));
	m_handleMessWithResultConn = m_messHandler->ConnectToHandleMessageWithResult(boost::bind(&VS_TransceiverNetChannel::HandleMessWithRes,this,_1,_2));
}
VS_TransceiverNetChannel::~VS_TransceiverNetChannel()
{
	assert(!m_channelConn);
	m_handleMessConn.disconnect();
	m_handleMessWithResultConn.disconnect();
}
bool VS_TransceiverNetChannel::SetTCPConnection(VS_ConnectionTCP * conn, const void * in_buf, const unsigned long in_len)
{
	auto auth = m_auth_conn.lock();
	if (!auth || !auth->AuthConnection((const unsigned char*)in_buf + sizeof(net::HandshakeHeader), in_len - sizeof(net::HandshakeHeader)))
		return false;

	auto res = SetChannelConnection(conn);
	if (res && m_fireTransceiverReady && m_fireSetTransName) {
		const auto& transName = auth->GetAuthenticatedName();
		m_fireSetTransName(transName);
		m_fireTransceiverReady(transName);
	}
	return res;
}
bool VS_TransceiverNetChannel::SetChannelConnection(VS_ConnectionTCP *conn)
{
	if(!m_workThread->IsCurrent())
	{
		boost::shared_ptr<VS_MessageData> mess(new VS_MessageData(e_set_connection,conn,sizeof(conn)));
		boost::shared_ptr<VS_SendMessageResult<bool>> result(new VS_SendMessageResult<bool>(false));
		m_workThread->Send(m_messHandler,mess,result);
		return result->result();
	}
	if(m_channelConn && m_channelConn->IsRW() || m_isStopped || !conn || !conn->IsValid() ||conn->IsRW())
		return false;
	delete m_channelConn;
	m_channelConn = conn;
	m_channelConn->SetOvReadFields(e_operation_read,reinterpret_cast<VS_ACS_Field>(m_channelConn));
	m_channelConn->SetOvWriteFields(e_operation_write,reinterpret_cast<VS_ACS_Field>(m_channelConn));
	m_channelConn->SetIOHandler(this);
	if(!m_workThread->SetHandledConnection(m_channelConn))
	{
		m_channelConn->SetIOHandler(0);
		m_channelConn = 0;
		return false;
	}
	m_rcvMess.reset(new VS_MainRelayMessage());
	unsigned long read_sz(0);
	unsigned char *rcv_buf = m_rcvMess->GetBufToRead(read_sz);
	bool ret(false);
	if(!(ret = m_channelConn->Read(rcv_buf,read_sz)))
		DeleteConn(m_channelConn);
	return ret;
}

void VS_TransceiverNetChannel::SetConnectionAuthenticator(const boost::shared_ptr<VS_AuthConnectionInterface>& auth_conn)
{
	m_auth_conn = auth_conn;
}

void VS_TransceiverNetChannel::StopActivity()
{
	Stop();
	WaitForStop();
}

void VS_TransceiverNetChannel::Handle(const unsigned long sz, const VS_Overlapped *ov)
{
	if(!ov)
		return;
	switch(ov->field1)
	{
	case e_operation_read:
		if(!HandleRead(sz,ov))
			DeleteConn(reinterpret_cast<VS_ConnectionTCP*>(ov->field2));
		break;
	case e_operation_write:
		if(!HandleWrite(sz,ov))
			DeleteConn(reinterpret_cast<VS_ConnectionTCP*>(ov->field2));
		break;
	default:
		DeleteConn(reinterpret_cast<VS_ConnectionTCP*>(ov->field2));
	}
}

void VS_TransceiverNetChannel::HandleError(const unsigned long err, const VS_Overlapped *ov)
{
	if(!ov)
		return;
	VS_ConnectionTCP *conn = reinterpret_cast<VS_ConnectionTCP*>(ov->field2);
	switch(ov->field1)
	{
	case e_operation_read:
		conn->SetReadResult(0,ov);
		break;
	case e_operation_write:
		conn->SetWriteResult(0,ov);
		break;
	}
	DeleteConn(conn);
}
bool VS_TransceiverNetChannel::HandleRead(const unsigned long sz, const VS_Overlapped *ov)
{

	VS_ConnectionTCP *conn = reinterpret_cast<VS_ConnectionTCP*>(ov->field2);
	if(!conn)
		return false;
	int bytesRead;
	if (0 > (bytesRead = conn->SetReadResult(sz, ov, 0, true)))
		return false;
	if(conn != m_channelConn)
		return false;
	m_rcvMess->SetReadBytes(bytesRead);
	m_fireProcessRecvMessage(m_rcvMess);
	unsigned long read_sz(0);
	unsigned char *buf = m_rcvMess->GetBufToRead(read_sz);
	return m_channelConn ? m_channelConn->Read(buf, read_sz) : false;
}
bool VS_TransceiverNetChannel::HandleWrite(const unsigned long sz, const VS_Overlapped *ov)
{
	VS_ConnectionTCP *conn = reinterpret_cast<VS_ConnectionTCP*>(ov->field2);
	if(conn)
	{
		if(!m_writeMess->GetMess() || (m_writeMess->GetMess()->size() != conn->SetWriteResult(sz,ov) || (m_waiting_del_conns.find(conn)!=m_waiting_del_conns.end())))
			return false;
		if(conn!=m_channelConn)
			return false;
	}
	return WriteNextMessage();
}
bool VS_TransceiverNetChannel::SendMsg(const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess)
{
	if(!m_workThread->IsCurrent())
	{
		boost::shared_ptr<VS_MessageData> message(new SendMsgMessage(mess));
		m_workThread->Post(m_messHandler,message);
		return true;
	}
	if(!mess || mess->Empty())
		return true;
	if(!m_channelConn)
		return false;
	if(m_channelConn->IsWrite() || !m_mess_queue.empty())
	{
		m_mess_queue.push(mess);
		return WriteNextMessage();
	}
	else
	{
		unsigned long sz(0);
		const unsigned char *buf = mess->GetMess(sz);
		if(m_channelConn->Write(buf,sz))
		{
			m_writeMess = mess;
			return true;
		}
		return false;
	}
}
net::address VS_TransceiverNetChannel::GetRemoteAddress() const
{
	if (!m_workThread->IsCurrent())
	{
		auto result = boost::make_shared<VS_SendMessageResult<net::address>>();
		m_workThread->Send(m_messHandler, boost::make_shared<VS_MessageData>(e_get_remote_address, nullptr, 0), result);
		return result->result();
	}
	if (!m_channelConn || !m_channelConn->IsValid())
		return {};
	return net::ConvertAddress(m_channelConn->GetPeerAddress());
}
bool VS_TransceiverNetChannel::WriteNextMessage()
{
	boost::shared_ptr<VS_NetworkRelayMessageBase> mess;
	if(!m_channelConn)
		return false;
	if(m_channelConn->IsWrite() || m_mess_queue.empty())
		return true;
	mess = m_mess_queue.front();
	m_mess_queue.pop();
	boost::shared_ptr<std::vector<unsigned char>> buf = mess->GetMess();
	if(m_channelConn->Write(&(*buf)[0],buf->size()))
	{
		m_writeMess = mess;
		return true;
	}
	return false;
}
void VS_TransceiverNetChannel::HandleMess(const boost::shared_ptr<VS_MessageData> &message)
{
	if(!message)
		return;
	unsigned long type(0);
	unsigned long sz(0);
	message->GetMessPointer(type,sz);
	switch(type)
	{
	case e_stop:
		Stop();
		break;
	case e_send_msg:
		SendMsgMessage *snd_mess = reinterpret_cast<SendMsgMessage*>(message.get());
		SendMsg(snd_mess->GetMsg());
		break;
	}

}
void VS_TransceiverNetChannel::HandleMessWithRes(const boost::shared_ptr<VS_MessageData> &message, const boost::shared_ptr<VS_MessResult> &res)
{
	if(!message)
		return;
	unsigned long sz(0);
	unsigned long type(0);
	void * p = message->GetMessPointer(type,sz);
	if (!res)
		return;
	switch(type)
	{
	case e_set_connection:
		static_cast<VS_SendMessageResult<bool>*>(res.get())->result() = SetChannelConnection(static_cast<VS_ConnectionTCP*>(p));
		break;
	case e_get_remote_address:
		static_cast<VS_SendMessageResult<net::address>*>(res.get())->result() = GetRemoteAddress();
		break;
	}
}
void VS_TransceiverNetChannel::Stop()
{
	if(!m_workThread->IsCurrent())
	{
		boost::shared_ptr<VS_MessageData> mess(new VS_MessageData(e_stop,0,0));
		m_workThread->Post(m_messHandler,mess);
		return;
	}
	m_isStopped = true;
	DeleteConn(m_channelConn);
}
void VS_TransceiverNetChannel::WaitForStop()
{
	/**
		TODO: it is possible that this will be deleted (and wait for stop) from context m_workThread, so m_workThread have to have ability start second thread for completion i/o;
	**/
	if(m_workThread->IsCurrent())
		return;
	m_stop_event.wait();
}
void VS_TransceiverNetChannel::DeleteConn(VS_ConnectionTCP *conn)
{
	bool notify(false);
	bool fireConnDie(false);

	if(!conn)
		notify = m_isStopped && m_waiting_del_conns.empty() && !m_channelConn;
	else
	{
		conn->Close();
		if(conn == m_channelConn)
		{
			fireConnDie = true;
			m_channelConn = 0;
		}
		if(conn->IsRW())
		{
			m_waiting_del_conns.insert(conn);
			return;
		}
		else
		{
			m_waiting_del_conns.erase(conn);
			delete conn;
			notify = m_isStopped && m_waiting_del_conns.empty() && !m_channelConn;
		}
	}
	if(fireConnDie)
		m_fireOnConnectionDie();
	if (notify)
		m_stop_event.set();
}
#endif