#ifdef _WIN32	// not ported
#include "VS_RemoteCircuitFrameTransmit.h"
#include "std/cpplib/VS_WorkThread.h"
#include "VS_ControlRelayMessage.h"
#include "VS_MainRelayMessage.h"
#include "VS_AuthConnectionInterface.h"
#include "acs/connection/VS_ConnectionTCP.h"
#include "acs/connection/VS_IO_Operation_Type.h"
#include "acs/connection/VS_ConnectionOv.h"
#include "acs/Lib/VS_AcsLib.h"
#include <boost/make_shared.hpp>

class VS_RemoteCircuitFrameTransmit::TransmitFrameMessage : public VS_MessageData
{
public:
	TransmitFrameMessage(boost::shared_ptr<VS_MainRelayMessage> &mess) : VS_MessageData(e_transmit_frame_mess,0,0), m_mess(mess)
	{}
	const char *GetConferenceName() const
	{
		if(!m_mess)
			return 0;
		boost::shared_ptr<VS_ControlRelayMessage> temp_mess(new VS_ControlRelayMessage);
		temp_mess->SetMessage(m_mess->GetMess());
		return temp_mess->GetConferenceName();
	}
	const boost::shared_ptr<VS_NetworkRelayMessageBase> GetMessage() const
	{
		return m_mess;
	}
private:
	boost::shared_ptr<VS_MainRelayMessage>	m_mess;
};

VS_RemoteCircuitFrameTransmit::VS_RemoteCircuitFrameTransmit(const boost::shared_ptr<VS_WorkThread> &work_thread,
															 const boost::shared_ptr<VS_AuthConnectionInterface> &auth) : m_processing_thread(work_thread),m_auth_conn(auth)
{
}

VS_RemoteCircuitFrameTransmit::~VS_RemoteCircuitFrameTransmit()
{
}
void VS_RemoteCircuitFrameTransmit::Close()
{
	VS_ConnectionTCP *conn;
	do
	{
		{
			VS_AutoLock lock(this);
			if(m_conns_info.empty())
				break;
			conn = m_conns_info.begin()->first;
		}
		DeleteConn(conn);
	}while(true);
}
bool VS_RemoteCircuitFrameTransmit::SetTCPConnection(VS_ConnectionTCP *conn, const void *in_buf, const unsigned long in_len)
{
	boost::shared_ptr<VS_AuthConnectionInterface> auth = m_auth_conn.lock();
	VS_StartFrameTransmitterMess mess;
	const char *conf_name(0);
	const unsigned char *auth_data(0);
	unsigned long auth_data_ln(0);
	if(!mess.SetMessage(static_cast<const unsigned char*>(in_buf),in_len) || !(conf_name = mess.GetConferenceName()) || !(auth_data = mess.GetAuthData(auth_data_ln)))
		return false;
	if(!auth || !auth->AuthConnection(auth_data,auth_data_ln))
		return false;
	VS_AutoLock lock(this);
	if(m_conns_info.find(conn) != m_conns_info.end())
		return true;
	auto c_it = m_conns_by_conf.find(conf_name);
	if(c_it != m_conns_by_conf.end())
		DeleteConn(c_it->second);
	ConnectionInfo info;
	info.conf_name = conf_name;
	info.rcv_byte.reset(new unsigned char);
	m_conns_by_conf[conf_name] = conn;
	m_conns_info[conn] = info;

	conn->SetIOHandler(this);
	conn->SetOvWriteFields(e_operation_write,reinterpret_cast<VS_ACS_Field>(conn));
	conn->SetOvReadFields(e_operation_read,reinterpret_cast<VS_ACS_Field>(conn));
	m_processing_thread->SetHandledConnection(conn);
	if(!conn->Read(info.rcv_byte.get(),1))
		DeleteConn(conn);
	return true;
}
void VS_RemoteCircuitFrameTransmit::HandleMessage(const boost::shared_ptr<VS_MessageData> &message)
{
	if(!message)
		return;
	unsigned long sz(0);
	unsigned long type(0);
	message->GetMessPointer(type,sz);
	switch(type)
	{
	case e_transmit_frame_mess:
		{
			TransmitFrameMessage * mess = reinterpret_cast<TransmitFrameMessage*>(message.get());
			Write(mess->GetMessage(),mess->GetConferenceName());
		}
		break;
	}
}

void VS_RemoteCircuitFrameTransmit::TransmitFrame(const char *conf_name, const char *part, const stream::FrameHeader *frame_head, const void *frame_data)
{
	assert(m_mess_handler != nullptr);
	boost::shared_ptr<VS_MainRelayMessage> mess(new VS_MainRelayMessage);
	boost::shared_ptr<VS_ControlRelayMessage> temp_mess(new VS_ControlRelayMessage);
	if(!temp_mess->MakeTransmitFrame(conf_name, part,frame_head, frame_data))
		return;
	mess->SetMessage(temp_mess->GetMess());
	if(!m_processing_thread->IsCurrent())
	{
		boost::shared_ptr<VS_MessageData> internal_mess(new TransmitFrameMessage(mess));
		m_processing_thread->Post(m_mess_handler,internal_mess);
		return;
	}
	else
		Write(mess,conf_name);
}

bool VS_RemoteCircuitFrameTransmit::Write(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess, const char *conf_name)
{
	if(!mess || mess->Empty())
		return true;
	VS_AutoLock lock(this);
	std::map<VS_SimpleStr,VS_ConnectionTCP*>::iterator iter = m_conns_by_conf.find(conf_name);
	if(iter==m_conns_by_conf.end())
		return false;
	VS_ConnectionTCP *conn = iter->second;
	std::map<VS_ConnectionTCP*,ConnectionInfo>::iterator inf = m_conns_info.find(conn);
	if(inf == m_conns_info.end())
		return false;

	if(conn->IsWrite() || !inf->second.writeMessQueue.empty())
	{
		inf->second.writeMessQueue.push(mess);
		return WriteNextMessage(conn);
	}
	else
	{
		boost::shared_ptr<std::vector<unsigned char>> buf = mess->GetMess();
		if(conn->Write(&(*buf)[0],buf->size()))
		{
			inf->second.lastWriteMess = mess;
			return true;
		}
		return false;
	}
}

bool VS_RemoteCircuitFrameTransmit::WriteNextMessage(VS_ConnectionTCP *conn)
{
	boost::shared_ptr<VS_NetworkRelayMessageBase> mess;
	VS_AutoLock lock(this);
	std::map<VS_ConnectionTCP*,ConnectionInfo>::iterator inf = m_conns_info.find(conn);
	if(inf == m_conns_info.end())
		return false;
	if(conn->IsWrite() || inf->second.writeMessQueue.empty())
		return true;
	mess = inf->second.writeMessQueue.front();
	inf->second.writeMessQueue.pop();
	boost::shared_ptr<std::vector<unsigned char>> buf = mess->GetMess();
	if(conn->Write(&(*buf)[0],buf->size()))
	{
		inf->second.lastWriteMess = mess;
		return true;
	}
	return false;
}
void VS_RemoteCircuitFrameTransmit::Handle(const unsigned long sz, const VS_Overlapped *ov)
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
void VS_RemoteCircuitFrameTransmit::HandleError(const unsigned long err, const VS_Overlapped *ov)
{
	VS_AutoLock lock(this);
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
bool VS_RemoteCircuitFrameTransmit::HandleRead(const unsigned long sz, const VS_Overlapped *ov)
{
	VS_AutoLock lock(this);
	VS_ConnectionTCP *conn = reinterpret_cast<VS_ConnectionTCP*>(ov->field2);
	if(!conn)
		return false;
	conn->SetReadResult(sz,ov);
	return false;
}
bool VS_RemoteCircuitFrameTransmit::HandleWrite(const unsigned long sz, const VS_Overlapped *ov)
{
	VS_ConnectionTCP *conn(0);
	{
		VS_AutoLock lock(this);
		conn = reinterpret_cast<VS_ConnectionTCP*>(ov->field2);
		if(conn)
		{
			auto inf = m_conns_info.find(conn);
			if (inf == m_conns_info.end())
				return false;
			if(!inf->second.lastWriteMess->GetMess() ||
				(inf->second.lastWriteMess->GetMess()->size() != conn->SetWriteResult(sz,ov) || (m_delConn.find(conn) != m_delConn.end())))
				return false;
		}
	}
	return WriteNextMessage(conn);
}
void VS_RemoteCircuitFrameTransmit::DeleteConn(VS_ConnectionTCP *conn)
{
	VS_AutoLock lock(this);
	if(!conn)
		return;
	std::map<VS_ConnectionTCP*,ConnectionInfo>::iterator inf = m_conns_info.find(conn);
	if(inf!=m_conns_info.end())
	{
		m_delConn.insert(std::pair<VS_ConnectionTCP*,ConnectionInfo>(conn,inf->second));
		m_conns_by_conf.erase(inf->second.conf_name);
		m_conns_info.erase(conn);
	}
	conn->Close();
	if(conn->IsRW())
		return;
	m_delConn.erase(conn);
	delete conn;

}

void VS_RemoteCircuitFrameTransmit::WaitClose()
{
	if(!m_processing_thread || m_processing_thread->IsCurrent())
		return;
	while(true)
	{
		{
			VS_AutoLock	lock(this);
			if(m_delConn.empty())
				return;
		}
		//boost::this_thread::interruptible_wait(1);
		Sleep(1);
	}
}
#endif