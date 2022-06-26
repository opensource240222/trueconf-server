#pragma once
#include "std/cpplib/event.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include <map>
#include <set>
#include <string>
#include <memory>

class VS_TransceiverParticipant;
class VS_MessageHandlerAdapter;
class VS_MessageData;
class VS_MessResult;

class VS_TransceiverPartsMgr
{
public:
	VS_TransceiverPartsMgr(
		std::string serverEP, std::string serverAddr,
		boost::asio::io_service &ios);
	virtual ~VS_TransceiverPartsMgr(){}
	/**search or create*/
	std::shared_ptr<VS_TransceiverParticipant> GetPart(const char *conf_name, const char *part_name);
	void FreePart(const char *conf_name, const char *part_name);
	void StopAndWait(){Stop(); WaitForStop();}

	void Stop();
	void WaitForStop();
private:
	void OnPartDie(const std::weak_ptr<VS_TransceiverParticipant> key);
	std::shared_ptr<VS_TransceiverParticipant> GetPartImpl(const char *conf_name, const char *part_name);
	void StopImpl();
	void FreePartImpl(const std::string &conf_name, const std::string &part_name);

	std::map<std::pair<std::string,std::string>, std::shared_ptr<VS_TransceiverParticipant> > m_parts;
	std::set<std::shared_ptr<VS_TransceiverParticipant>> m_pending_close_lst;
	std::string m_serverAddr;
	std::string m_serverEP;
	bool										m_isStopped;
	vs::event									m_stop_event;

	boost::asio::io_service::strand m_strand;
};