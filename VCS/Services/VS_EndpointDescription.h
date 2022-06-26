/*************************************************
* $Revision: 1 $
* $History: VS_EndpointDescription.h $
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 3.01.10    Time: 15:16
 * Created in $/VSNA/Servers/VCS/Services
 * - VCS refactoried
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 4.11.09    Time: 21:13
 * Created in $/VSNA/Servers/VCS/Storage
 *  - new names
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 2.11.09    Time: 15:03
 * Updated in $/VSNA/Servers/SBSv3_m/Storage
 * - store users by login only
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:58
 * Updated in $/VSNA/Servers/SBSv3_m/Storage
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 7.08.09    Time: 14:48
 * Created in $/VSNA/Servers/SBSv3_m/Storage
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/ServerServices/types
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 7.12.06    Time: 17:26
 * Updated in $/VS/Servers/ServerServices/Types
 * moved endpoint keys save from storage to ep reg srv
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 2.11.06    Time: 19:19
 * Updated in $/VS/Servers/ServerServices/Types
 * added gateway control
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 20.09.06   Time: 13:34
 * Updated in $/VS/Servers/ServerServices/Types
 * changed endpoint keys processing
 * moved reg endpoints to SimpleStorage
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 22.03.04   Time: 20:40
 * Updated in $/VS/Servers/ServerServices/Types
 * added wide property write
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 1.12.03    Time: 20:11
 * Updated in $/VS/servers/serverservices/types
 * endpoints finished
*
* *****************  Version 2  *****************
* User: Stass        Date: 17.09.03   Time: 14:20
* Updated in $/VS/Servers/ServerServices/Types
* id types employed in structs
*
* *****************  Version 1  *****************
* User: Stass        Date: 5.09.03    Time: 14:40
* Created in $/VS/Servers/ServerServices/Types
* Moved types out of BrokerServices project
*
* *****************  Version 1  *****************
* User: Slavetsky    Date: 9/04/03    Time: 4:38p
* Created in $/VS/Servers/MediaBroker/BrokerServices
*
* *****************  Version 14  *****************
* User: Stass        Date: 7.07.03    Time: 15:59
* Updated in $/VS/servers/serverservices
* added broker_id store/rerieve for endpoint
*
* *****************  Version 13  *****************
* User: Smirnov      Date: 20.06.03   Time: 12:04
* Updated in $/VS/Servers/ServerServices
* added endpoints key
*
* *****************  Version 12  *****************
* User: Smirnov      Date: 6.05.03    Time: 20:30
* Updated in $/VS/Servers/ServerServices
* changed protocol  = 4.0 ver
*
* *****************  Version 11  *****************
* User: Smirnov      Date: 25.04.03   Time: 16:07
* Updated in $/VS/Servers/ServerServices
* Changed EpDesc, UsDesc
* Added Login-Logoff Event
* Auto Login moved to Endpoint
*
*************************************************/

#ifndef VS_ENDPOINTDESCRIPTION_H
#define VS_ENDPOINTDESCRIPTION_H

#include "../../common/std/cpplib/VS_UserID.h"
#include "../../common/std/cpplib/VS_WideStr.h"

#include <chrono>
#include <memory>

class VS_EndpointDescription
{
public:
	VS_EndpointDescription(void);
	~VS_EndpointDescription();

	VS_EndpointDescription(const VS_EndpointDescription& ed);
	const VS_EndpointDescription& operator=(const VS_EndpointDescription& ed);

	bool IsValid( void ) const;

	void SetConnectionInfo(void* confIP, const unsigned long confIPSize, bool directConnect);
	// Fields
	enum Status {
		DISCONNECTED_STATUS,
		CONNECTED_STATUS,
		BUSY_STATUS
	};

	VS_SimpleStr		m_name;
	int					m_status;
	VS_SimpleStr		m_broker_id;

	vs_user_id		m_loggedUser;
	vs_user_id		m_lastUser;
	bool			m_autologin;

	enum Type {
		ET_CLIENT=0,
		ET_GATEWAY=2
	};

	int				m_type;
	// network configuration - updated on logon
	std::unique_ptr<unsigned char[]> m_confIP;
	unsigned long	m_confIPSize;
	unsigned long	m_protocolVersion;
	//almost read-only members, not fetched in normal circumstances
	//TODO: remove after switch to DB
	VS_SimpleStr	m_appName;

	VS_SimpleStr	m_clientVersion;
	vs_user_id		m_registrar;
	std::chrono::system_clock::time_point		m_registered;
	std::chrono::system_clock::time_point		m_lastConnected;
	VS_WideStr		m_systemConfiguration;
};

inline bool VS_IsGatewayEndpoint(int endpoint_type)
{
  return endpoint_type==VS_EndpointDescription::ET_GATEWAY;
};

#endif // VS_ENDPOINTDESCRIPTION_H