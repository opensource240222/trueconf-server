/*************************************************
 * $Revision: 3 $
 * $History: VS_BrokerInfo.h $
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 15.02.08   Time: 21:18
 * Updated in $/VSNA/Servers/ServerServices/types
 * - safe access to g_AppDAta in services
 * - new broker info structure
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 13.11.07   Time: 17:34
 * Updated in $/VSNA/Servers/ServerServices/types
 * new server
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:50
 * Created in $/VSNA/Servers/ServerServices/types
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/ServerServices/types
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 3.08.06    Time: 15:39
 * Updated in $/VS/Servers/ServerServices/Types
 * removed VS_BrokerInfo::States enum
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 2.08.06    Time: 18:01
 * Updated in $/VS/Servers/ServerServices/Types
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 2.08.06    Time: 17:57
 * Updated in $/VS/Servers/ServerServices/Types
 * added endpoints and cpu fetch
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
 * *****************  Version 3  *****************
 * User: Stass        Date: 28.07.03   Time: 13:04
 * Updated in $/VS/Servers/ServerServices
 * added disconnected broker purge
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 9.07.03    Time: 17:39
 * Updated in $/VS/Servers/ServerServices
 * added all broker fetch, connect/accept info set
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 8.07.03    Time: 15:01
 * Created in $/VS/Servers/ServerServices
 * added BrokerInfo structure store/retrieve/touch
 * added conflict detection, touch broker info for cleanup
 *
 *************************************************/

#ifndef VS_BROKER_INFO_H
#define VS_BROKER_INFO_H

#include <chrono>

#include "../../common/std/cpplib/VS_SimpleStr.h"
#include "std-generic/cpplib/VS_Container.h"


class VS_ServerInfo
{
public:
	VS_SimpleStr	m_id;			/// broker id
	VS_ServerTypes	m_type;
    int				m_status;		/// broker status
	unsigned long	m_pingTime;
	unsigned long	m_Distance;
	VS_BinBuff		m_netConfig;
    std::chrono::system_clock::time_point     m_lastConnected;/// DB time of

	VS_ServerInfo(): m_type(ST_UNKNOWN), m_status(0), m_pingTime(0), m_Distance(-1) {}

    static void* Factory(const void* ed) {   return new VS_ServerInfo(*(VS_ServerInfo*)ed); }
    static void Destructor(void* ed) {   delete (VS_ServerInfo*) ed; }
};

#endif // VS_BROKER_INFO_H