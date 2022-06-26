/*
* $Revision: 4 $
* $History: VS_ChatService.h $
*
* *****************  Version 4  *****************
* User: Ktrushnikov  Date: 10.08.11   Time: 10:50
* Updated in $/VSNA/Servers/BaseServer/Services
* - process messages in a separate Task (because Resolve())
*
* *****************  Version 3  *****************
* User: Ktrushnikov  Date: 5.08.11    Time: 14:23
* Updated in $/VSNA/Servers/BaseServer/Services
* - roaming: offline chat support VCS -> BS
*
* *****************  Version 2  *****************
* User: Mushakov     Date: 18.12.09   Time: 18:04
* Updated in $/VSNA/Servers/BaseServer/Services
* - Removed VCS_BUILD somewhere
* - Add new field to license
* - Chat service for bsServer renamed
*
* *****************  Version 1  *****************
* User: Ktrushnikov  Date: 12.03.08   Time: 20:01
* Created in $/VSNA/Servers/BaseServer/Services
* - BS: CHAT_SRV added
* - Pass offline chat messages from AS::RESOLVE_SRV to BS::CHAT_SRV
* - Save offline chat messages in DB via sp_friend_im_save stored
* procedure
***********************************************/
#pragma once

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../../common/transport/Router/VS_PoolThreadsService.h"
#include "../../common/std/cpplib/enable_shared_from_this_virtual_boost.h"
#include "storage/VS_DBStorage.h"

#include <queue>

class VS_BSChatService
	: public VS_TransportRouterServiceReplyHelper
	, public boost::enable_shared_from_this_virtual<VS_BSChatService>
{
	VS_Lock						m_thread_order_lock;
	std::vector<unsigned long>	m_thread_order;		// key = uniq id
public:
	VS_BSChatService(void) { }
	virtual ~VS_BSChatService(void) { }

	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	void WaitMyID(const unsigned long id);
	void DeleteMyID(const unsigned long id);
	void NotifyWebAboutOfflineChat();
};

class ProcessChat_Task : public VS_PoolThreadsTask, public VS_TransportRouterServiceReplyHelper
{
	unsigned long			m_id;
	VS_Container			m_in_cnt;
	boost::shared_ptr<VS_BSChatService>	m_srv;
	std::unique_ptr<VS_RouterMessage> m_mess;

	void SaveMessageInDB(VS_Container &cnt);
	void ProcessMessage(VS_Container &cnt);
public:
	ProcessChat_Task(const boost::shared_ptr<VS_BSChatService>& ptr, const unsigned long id, VS_Container& cnt, std::unique_ptr<VS_RouterMessage>&& recvMess);
	~ProcessChat_Task();

	void Run();
};


class NewOfflineChat_Task : public VS_PoolThreadsTask, public VS_TransportRouterServiceReplyHelper
{
	VS_SimpleStr m_call_id;
public:
	NewOfflineChat_Task(VS_SimpleStr call_id);
	~NewOfflineChat_Task();

	void Run();
};