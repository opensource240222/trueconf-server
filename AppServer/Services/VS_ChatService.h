/*
* $Revision: 8 $
* $History: VS_ChatService.h $
 *
 * *****************  Version 8  *****************
 * User: Ktrushnikov  Date: 9.03.11    Time: 20:04
 * Updated in $/VSNA/Servers/AppServer/Services
 * offline chat checks license LE_ROAMING_ON
 *
 * *****************  Version 7  *****************
 * User: Ktrushnikov  Date: 9.03.11    Time: 18:29
 * Updated in $/VSNA/Servers/AppServer/Services
 * added new service OFFLINECHAT_SRV in VCS & BS
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 27.01.11   Time: 13:39
 * Updated in $/VSNA/Servers/AppServer/Services
 * VCS 3.2
 * - offline chat messages in roaming
 * - on of SubscriptionHub::Subscribe() fixed
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 13.10.10   Time: 9:10
 * Updated in $/VSNA/Servers/AppServer/Services
 * [#7944] offline chat messages support in VCS
 * - sqlite added to ^std
 * - confRestrict interface (VCS with AS)
 * - get/set offline chat messages in VS_SimpleStorage
 * - get offline chat messages at login in VCSAuthService
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 5.12.07    Time: 13:13
 * Updated in $/VSNA/Servers/AppServer/Services
 * - valid overload of Init()-method
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 26.11.07   Time: 15:17
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 15.11.07   Time: 15:28
 * Updated in $/VSNA/Servers/AppServer/Services
 * updates
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 13.11.07   Time: 17:32
 * Created in $/VSNA/Servers/AppServer/Services
 * new services
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:50
 * Created in $/VSNA/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 18.09.06   Time: 14:47
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * - async all storage working services
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 29.09.04   Time: 14:24
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * pragma_once removed
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 30.07.04   Time: 18:13
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * Chat sending betwen clusters
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 22.03.04   Time: 18:52
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * UTF8 system chat messages + chat message protocol  ver 11,12,13 support
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 29.09.03   Time: 12:56
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added message types
 *
 * *****************  Version 1  *****************
 * User: Slavetsky    Date: 9/04/03    Time: 4:38p
 * Created in $/VS/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 1.09.03    Time: 13:15
 * Updated in $/VS/Servers/ServerServices
 * send command throw server
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 26.06.03   Time: 13:31
 * Updated in $/VS/Servers/ServerServices
 * console calls removed
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 30.05.03   Time: 20:38
 * Updated in $/VS/Servers/ServerServices
 * check every 12 sec
 * notify by message, add ignore interface
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 21.05.03   Time: 20:00
 * Updated in $/VS/Servers/ServerServices
 * chat for all - slow:
 * need more flexible method on server
*
* *****************  Version 2  *****************
* User: Smirnov      Date: 20.05.03   Time: 15:38
* Updated in $/VS/Servers/ServerServices
* Serisies iterfaces rewrited
*
***********************************************/
#ifndef VS_CHAT_SERVICE_H
#define VS_CHAT_SERVICE_H

#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "std-generic/cpplib/macro_utils.h"
#include "../../common/std/debuglog/VS_Debug.h"

#include "AppServer/Services/VS_Storage.h"
#include "AppServer/Services/ChatDBInterface.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"

namespace { namespace mi = boost::multi_index; }

class VS_ChatService
	: public VS_TransportRouterServiceReplyHelper
{
	enum class ContentSource
	{
		None,
		Slideshow,
		RTPExtraVideo,
	};
	struct slide_info
	{
		VS_FORWARDING_CTOR3(slide_info, user, cmd, client_type) {}
		std::string user;
		std::string cmd;
		int32_t		client_type;
	};
	struct conf_info
	{
		mi::multi_index_container<std::string, mi::indexed_by<
			mi::sequenced<mi::tag<struct seq_tag>>,
			mi::ordered_unique<mi::tag<struct val_tag>, mi::identity<std::string>>
		>> content_senders;
		mi::multi_index_container<slide_info, mi::indexed_by<
			mi::sequenced<mi::tag<struct seq_tag>>,
			mi::ordered_non_unique<mi::tag<struct val_tag>, mi::member<slide_info, std::string, &slide_info::user>>
		>> slide_senders;
		ContentSource rtp_content_source = ContentSource::None;
	};
	vs::map<vs_conf_id, conf_info, vs::less<>> m_confs;
	boost::shared_ptr<VS_ConfRestrictInterface> m_confRestrict;
	std::weak_ptr<ts::IPool> m_transceiversPool;
	std::unique_ptr<ChatDBInterface> m_chatDB;
	bool	m_ChatAlowed;

	bool ProcessSlideShowCommand(VS_Container& cnt);
	bool ProcessExtraVideoFlowCommand(VS_Container& cnt);
	void UpdateSlideShowState(const char* conf, const char* user, bool state, const char* cmd);
	void UpdateContentState(const char* conf, const char* user, bool state);
	void SendSlideCommandToMixer(const char* conf, const char* from, const char* msg);

	// Service implementation
	void SendMessage_Method(const char* from_user, VS_Container &cnt);
	void SendCommand_Method(const char* from_user, VS_Container &cnt);
	void SendSlidesToUser_Method(VS_Container &cnt);
	void ConferenceDeleted_Method(VS_Container &cnt);
	void DeleteParticipant_Method(VS_Container &cnt);
	void SendToUserOrService(const char* from_id, int64_t message_id, const VS_Container& cnt, const char* to, const char* to_server = nullptr, const bool is_offline=true);
	void UpdateDBMsgID(VS_Container& cnt, const int64_t mID);
	bool CheckHops(VS_Container& cnt);
public:
	VS_ChatService(void) : m_ChatAlowed(true) { }
	virtual ~VS_ChatService(void) { }
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	void SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict);
	void SetTransceiversPool(const std::shared_ptr<ts::IPool> &pool);
};

#endif /*VS_CHAT_SERVICE_H*/
