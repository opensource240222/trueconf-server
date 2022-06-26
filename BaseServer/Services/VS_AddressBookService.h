/*************************************************
 * $Revision: 12 $
 * $History: VS_AddressBookService.h $
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 17.07.12   Time: 23:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - LoginConfigurator() was removed
 * - messages from configurator are handled by SessionID
 * - fix TransportMessage::IsFromServer()
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 10.05.12   Time: 21:55
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - PersonDetailes handled for transcoders
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 5.08.11    Time: 23:15
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - AddToAddressBook for BS same like VCS
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 3.08.11    Time: 0:30
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - support VCS to BS request of display_name
 * - g_BasePresenceService added
 * - GetDn in roaming small refactoring
 * - declaration of SearchAddressBook_Task class to header file
 *
 * *****************  Version 8  *****************
 * User: Ktrushnikov  Date: 15.07.11   Time: 15:20
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #8535, #8917
 * - user groups supported in Visicron.dll and BS server
 * (VS_AddressBookService, VS_DBStorage)
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 15.03.11   Time: 16:10
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - 8553 automatic updated user info supported
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 18.12.09   Time: 18:04
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Removed VCS_BUILD somewhere
 * - Add new field to license
 * - Chat service for bsServer renamed
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 3.09.09    Time: 11:38
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Save abooks at AS:
 *   - VS_Container: GetLongValueRef() added
 *   - TRANSPORT_SRCUSER_PARAM: pass user from Transport to Container
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 13.11.08   Time: 18:32
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - BS Events added
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 12.12.07   Time: 20:40
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed init
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * BS - new iteration
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 28.11.07   Time: 18:33
 * Created in $/VSNA/Servers/BaseServer/Services
 * first bs ver
 *
 **************************************************/

#ifndef VS_ADDRESS_BOOK_SERVICE_H
#define VS_ADDRESS_BOOK_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../../common/transport/Router/VS_PoolThreadsService.h"
#include "../../common/std/cpplib/VS_Map.h"
#include "../../common/std/cpplib/VS_Lock.h"
#include "../../ServerServices/VS_ABStorageInterface.h"
#include <boost/shared_ptr.hpp>
#include <set>

class VS_ConfRestrictInterface;

class VS_AddressBookService :
	public VS_TransportRouterServiceReplyHelper
{
	unsigned long			m_BSEventsTime;
	boost::shared_ptr<VS_ABStorageInterface> m_abStorage;
	boost::shared_ptr<VS_ConfRestrictInterface> m_confRestrict;
	//VS_ABStorageInterface	*m_abStorage;

public:
	VS_AddressBookService()
	{
		m_TimeInterval = std::chrono::seconds(5);
	}

	virtual ~VS_AddressBookService(void) { }

	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll) override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	void SetABStorage(const boost::shared_ptr<VS_ABStorageInterface>& abStorage);
	void SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict);

//Methods
	void UserRegistrationInfo_Method(const char *user);
	void AddToAddressBook_Method(VS_AddressBook ab, VS_Container& cnt,std::unique_ptr<VS_RouterMessage>&&);
	void RemoveFromAddressBook_Method(VS_AddressBook ab,const vs_user_id& user, VS_Container& cnt, std::unique_ptr<VS_RouterMessage> &&m);
	void UpdateAddressBook_Method(VS_AddressBook ab, VS_Container& cnt, std::unique_ptr<VS_RouterMessage>&&);
	void SearchAddressBook_Method(VS_AddressBook ab,const char *query, long client_hash, VS_Container& cnt, std::unique_ptr<VS_RouterMessage>&&);
	void OnAddressBookChange_Method(VS_AddressBook ab, VS_Container &cnt);
	void OnManageGroups_Method(const VS_ManageGroupCmd cmd, const long gid, const VS_SimpleStr& call_id, VS_WideStr gname, const long seq_id);

};

class SearchAddressBook_Task: public VS_PoolThreadsTask, public VS_TransportRouterServiceReplyHelper
{
public:
	VS_AddressBook			m_ab;
	std::string				m_query;
	VS_SimpleStr			m_server;
	vs_user_id	  			m_user;
	bool	  				m_to_user;
	long					m_client_hash;
	boost::shared_ptr<VS_ABStorageInterface> m_abStorage;
	VS_Container			m_in_cnt;
	std::unique_ptr<VS_RouterMessage> m_mess;
	boost::shared_ptr<VS_ConfRestrictInterface> m_conf_restrict;

	SearchAddressBook_Task(VS_AddressBook ab, const char* query, const char* server,
                         const char* user, unsigned long client_hash, boost::shared_ptr<VS_ABStorageInterface> abStorage, VS_Container& cnt, bool to_user, std::unique_ptr<VS_RouterMessage> &&recvMess, const boost::shared_ptr<VS_ConfRestrictInterface> &conf_restrict);

	~SearchAddressBook_Task();
	void Run();
};

class AddToAddressBook_Task: public VS_PoolThreadsTask, public VS_TransportRouterServiceReplyHelper
{
public:
	VS_AddressBook			m_ab;
	vs_user_id	  			m_user;
	boost::shared_ptr<VS_ABStorageInterface> m_abStorage;
	VS_Container			m_in_cnt;
	VS_SimpleStr			m_call_id2;
	bool					m_IsTransportSrcUser;
	std::unique_ptr<VS_RouterMessage> m_mess;

	AddToAddressBook_Task(VS_AddressBook ab, const char* user, boost::shared_ptr<VS_ABStorageInterface> abStorage,
		VS_Container& cnt, bool IsTransportSrcUser, std::unique_ptr<VS_RouterMessage> &&recvMess);
	~AddToAddressBook_Task();
	void Run();
};

class UpdateAddressBook_Task : public VS_PoolThreadsTask, public VS_TransportRouterServiceReplyHelper
{
public:
	VS_AddressBook			m_ab;
	vs_user_id	  			m_user;
	boost::shared_ptr<VS_ABStorageInterface> m_abStorage;
	VS_Container			m_in_cnt;
	VS_SimpleStr			m_call_id2;
	bool					m_IsTransportSrcUser;
	std::unique_ptr<VS_RouterMessage> m_mess;

	UpdateAddressBook_Task(VS_AddressBook ab, const char* user, boost::shared_ptr<VS_ABStorageInterface> abStorage,
		VS_Container& cnt, bool IsTransportSrcUser, std::unique_ptr<VS_RouterMessage>&& recvMess);
	~UpdateAddressBook_Task();
	void Run();
};

class RemoveFromAddressBook_Task: public VS_PoolThreadsTask, public VS_TransportRouterServiceReplyHelper
{
public:
	VS_AddressBook			m_ab;
	vs_user_id	  			m_user;
	boost::shared_ptr<VS_ABStorageInterface> m_abStorage;
	VS_Container			m_in_cnt;
	VS_SimpleStr			m_call_id2;
	bool					m_IsTransportSrcUser;
	std::unique_ptr<VS_RouterMessage> m_mess;

	RemoveFromAddressBook_Task(VS_AddressBook ab, const char* user, boost::shared_ptr<VS_ABStorageInterface> abStorage,
		VS_Container& cnt, bool IsTransportSrcUser, std::unique_ptr<VS_RouterMessage>&& recvMess);
	~RemoveFromAddressBook_Task();
	void Run();
};


class OnAddressBookChange_Task: public VS_PoolThreadsTask, public VS_TransportRouterServiceReplyHelper
{
public:
	VS_Container			m_in_cnt;
	VS_AddressBook			m_ab;
	std::string				m_query;
	VS_SimpleStr			m_server;
	bool	  				m_to_user;
	long					m_client_hash;
	boost::shared_ptr<VS_ABStorageInterface> m_abStorage;

	OnAddressBookChange_Task(VS_AddressBook ab, const char* query, const char* server,
                        boost::shared_ptr<VS_ABStorageInterface> abStorage, VS_Container& cnt);
	~OnAddressBookChange_Task(){};
	void Run();
};



#endif // VS_ADDRESS_BOOK_SERVICE_H
