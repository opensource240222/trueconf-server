/**
 **************************************************************************
 * \file VS_IDTypes.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief define types and include some common headers
 *
 * \b Project Standart Libraries
 * \author StasS
 * \date 05.09.03
 *
 * $Revision: 4 $
 *
 * $History: VS_IDTypes.h $
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:59
 * Updated in $/VSNA/std/cpplib
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 26.03.09   Time: 19:25
 * Updated in $/VSNA/std/cpplib
 * - Initial checkin SBSv3
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 13.11.07   Time: 17:34
 * Updated in $/VSNA/std/cpplib
 * new server
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 15.11.06   Time: 14:40
 * Updated in $/VS/std/cpplib
 * - new endpoint id
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 29.09.04   Time: 14:24
 * Updated in $/VS/std/cpplib
 * pragma_once removed
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 9.10.03    Time: 19:44
 * Updated in $/VS/std/cpplib
 * new methods in client
 * new files in std...
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 30.09.03   Time: 17:15
 * Updated in $/VS/std/cpplib
 * vs_strList class added
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 22.09.03   Time: 12:44
 * Updated in $/VS/std/cpplib
 * vs_user_id switched to char*
 * added network Resolve
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 10.09.03   Time: 15:27
 * Updated in $/VS/std/cpplib
 * vs_cluster_id definition and function
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 5.09.03    Time: 14:40
 * Created in $/VS/std/cpplib
 * Moved types out of BrokerServices project
 *
 * *****************  Version 1  *****************
 * User: Slavetsky    Date: 9/04/03    Time: 4:38p
 * Created in $/VS/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 21.07.03   Time: 14:13
 * Updated in $/VS/Servers/ServerServices
 * Moved vs_conf_id to StdLib, removed null(), added GetEndpoint function
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 18.07.03   Time: 20:02
 * Updated in $/VS/Servers/ServerServices
 * vs_conf_id type changed to string
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 19.06.03   Time: 15:19
 * Updated in $/VS/Servers/ServerServices
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 18.06.03   Time: 16:19
 * Updated in $/VS/Servers/ServerServices
 * protocol rempved to std
 * configuration of all projects corrected
 * some mistakes fixed
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 25.04.03   Time: 19:53
 * Created in $/VS/Servers/ServerServices
 * moved ID types to separate file
 *************************************************/
#ifndef VS_ID_TYPES_H
#define VS_ID_TYPES_H


/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_SimpleStr.h"
#include "VS_Map.h"
#include "std-generic/cpplib/VS_Container.h"
#include "VS_Endpoint.h"
/// vs_conf_id defined here:
#include "VS_ConferenceID.h"
#include "VS_UserID.h"

// For SBSv3
///typedef VS_SimpleStr    vs_ep_id;	// It is CID in NewArch

#endif
