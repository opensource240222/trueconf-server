/*************************************************
 * $Revision: 5 $
 * $History: Common.h $
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 23.10.09   Time: 14:33
 * Updated in $/VSNA/Servers/ServerServices
 * - BSEvent moved to common.h
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 12.02.08   Time: 20:31
 * Updated in $/VSNA/Servers/ServerServices
 * passing aliases to as
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:13
 * Updated in $/VSNA/Servers/ServerServices
 * GetAppProperties method realized
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 13.11.07   Time: 17:34
 * Updated in $/VSNA/Servers/ServerServices
 * new server
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:50
 * Created in $/VSNA/Servers/ServerServices
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/ServerServices
 *
 * *****************  Version 21  *****************
 * User: Stass        Date: 28.07.06   Time: 16:21
 * Updated in $/VS/Servers/ServerServices
 * address book caching support
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 29.09.04   Time: 14:24
 * Updated in $/VS/Servers/ServerServices
 * pragma_once removed
 *
 * *****************  Version 19  *****************
 * User: Stass        Date: 22.01.04   Time: 15:42
 * Updated in $/VS/Servers/ServerServices
 * moved debug print funcs
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 2.10.03    Time: 19:18
 * Updated in $/VS/Servers/ServerServices
 * added coonections in multistream conf
 *
 * *****************  Version 17  *****************
 * User: Stass        Date: 5.09.03    Time: 20:04
 * Updated in $/VS/Servers/ServerServices
 * Added CallIDInfo struct
 *
 * *****************  Version 16  *****************
 * User: Stass        Date: 5.09.03    Time: 14:40
 * Updated in $/VS/Servers/ServerServices
 * Moved types out of BrokerServices project
 *
 * *****************  Version 1  *****************
 * User: Slavetsky    Date: 9/04/03    Time: 4:38p
 * Created in $/VS/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 15  *****************
 * User: Stass        Date: 8.07.03    Time: 15:01
 * Updated in $/VS/Servers/ServerServices
 * added BrokerInfo structure store/retrieve/touch
 * added conflict detection, touch broker info for cleanup
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 27.06.03   Time: 17:07
 * Updated in $/VS/Servers/ServerServices
 * Protocol Version = 5
 *
 * *****************  Version 13  *****************
 * User: Stass        Date: 23.05.03   Time: 17:26
 * Updated in $/VS/Servers/ServerServices
 * added protocol version check and warning
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 15.05.03   Time: 20:45
 * Updated in $/VS/Servers/ServerServices
 * fixed version whith callbacks
 * extended part info and their methods
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 28.04.03   Time: 18:51
 * Updated in $/VS/Servers/ServerServices
 * added VS_ParticipantDescription
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 25.04.03   Time: 19:53
 * Updated in $/VS/Servers/ServerServices
 * moved ID types to separate file
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 25.04.03   Time: 19:22
 * Updated in $/VS/Servers/ServerServices
 * moved id types to Common.h
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 25.04.03   Time: 16:07
 * Updated in $/VS/Servers/ServerServices
 * Changed EpDesc, UsDesc
 * Added Login-Logoff Event
 * Auto Login moved to Endpoint
 *
 *************************************************/

#ifndef SERVER_SERVICES_COMMON_H
#define SERVER_SERVICES_COMMON_H

#include "../common/std/CallLog/VS_ConferenceDescription.h"
#include "../common/std/CallLog/VS_ParticipantDescription.h"
#include "Types/VS_BrokerInfo.h"
#include "Types/VS_StreamPartDesc.h"

#endif
