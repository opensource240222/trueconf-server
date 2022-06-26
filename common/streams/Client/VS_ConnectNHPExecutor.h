#pragma once
#include <chrono>
#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
///#include <boost/thread.hpp>
#include "boost/tuple/tuple.hpp"
#include <boost/array.hpp>

#include "../../std/cpplib/VS_TimeoutHandler.h"
#include "../../acs/connection/VS_IOHandler.h"

class VS_WorkThreadEvents;
class VS_ConnectionSock;
class VS_ConnectionUDP;
class VS_MessageData;
class VS_MessageHandlerAdapter;
class VS_MessResult;
class VS_NHP_HandshakeHelper;

class VS_ConnectNHPExecutor : public VS_TimeoutHandler,public VS_IOHandler,
	public boost::enable_shared_from_this<VS_ConnectNHPExecutor>
{
public:
	VS_ConnectNHPExecutor(const char *ourEp, const char *clEp, const char *SrvEp, const char *ConferenceName, const unsigned long timeout, const char *source_ip);
	virtual ~VS_ConnectNHPExecutor();
	boost::signals2::connection ConnectToSetNHPConn(const boost::signals2::signal<void (VS_ConnectionSock *, const unsigned long, const unsigned long)>::slot_type slot)
	{
		boost::signals2::connection empty;
		if(m_fireSetNHPConn.num_slots()>0)
			return empty;
		return m_fireSetNHPConn.connect(slot);
	}
	void MakeNHP();
	void Stop();
private:
	virtual void Timeout();
	virtual void Handle(const unsigned long sz, const struct VS_Overlapped *ov);
	virtual void HandleError(const unsigned long err, const struct VS_Overlapped *ov);

	void StartNHP();
	void CreateNHPConn(std::pair<unsigned long,unsigned short> );
	void DeleteConn(VS_ConnectionUDP *c);

//callbacks
	void ServerFound(VS_ConnectionUDP *c);
	void ConnectionReady(VS_ConnectionUDP *c,const unsigned long ip, const unsigned short port, const unsigned long UIDR, const unsigned long UIDS);

	bool CheckTime() const
	{
		return m_finishBefore > std::chrono::steady_clock::now();
	}
	void HandleMess(const boost::shared_ptr<VS_MessageData> &mess);
	//void HandleMessWithRes(const boost::shared_ptr<VS_MessageData> &mess,const boost::shared_ptr<VS_MessResult> &res);

	void ThreadTerminated();

	boost::signals2::signal<void (VS_ConnectionSock *, const unsigned long/*UIDR*/,const unsigned long/*UIDS*/)> m_fireSetNHPConn;
	boost::shared_ptr<VS_ConnectNHPExecutor>			m_this;
	boost::signals2::connection							m_ThreadTerminatedConn;

	std::string											m_clientEp;
	std::string											m_ourEp;
	std::string											m_srvEp;
	std::string											m_conferenceName;
	std::string											m_source_ip;
	unsigned long										m_timeout;

	//typedef boost::tuple<boost::shared_ptr<VS_NHP_HandshakeHelper>,
typedef std::pair<
		boost::shared_ptr<VS_NHP_HandshakeHelper>,
		boost::shared_ptr<std::vector<unsigned char>>> HelperAndSBud;

typedef boost::tuple<
		boost::shared_ptr<VS_NHP_HandshakeHelper>,
		boost::shared_ptr<std::vector<unsigned char>>,
		boost::shared_ptr<std::vector<unsigned char>>,
		boost::shared_ptr<boost::array<char,16>>,
		unsigned long*,
		unsigned short*>
		ConnectionData;


	std::map<VS_ConnectionUDP*, ConnectionData>			m_nhp_conns;
	std::chrono::steady_clock::time_point				m_finishBefore;
	boost::shared_ptr<VS_WorkThreadEvents>				m_workerThread;
	boost::shared_ptr<VS_MessageHandlerAdapter>			m_messHandler;

	VS_ConnectionUDP *m_readyConn;
	unsigned long m_connected_ip;
	unsigned short m_connected_port;
	unsigned long m_UIDR;
	unsigned long m_UIDS;

	bool m_connectionIsReady;
};