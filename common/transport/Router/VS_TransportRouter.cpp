#include "net/DNSUtils/VS_DNS.h"
#if defined(_WIN32) // Not ported yet

//////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Реализация поддержки транспортного протокола на сервере
//
//  Created: 12.11.02     by  A.Slavetsky
//
//////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_TransportRouter.cpp
/// \brief Реализация поддержки транспортного протокола на сервере
/// \note
///
/**
 ****************************************************************************
 * $Revision: 161 $
 *
 * $History: VS_TransportRouter.cpp $
 *
 * *****************  Version 161  *****************
 * User: Ktrushnikov  Date: 12.07.12   Time: 19:59
 * Updated in $/VSNA/transport/Router
 * #11674: remove m_early_msgs
 *
 * *****************  Version 160  *****************
 * User: Ktrushnikov  Date: 12.07.12   Time: 19:35
 * Updated in $/VSNA/transport/Router
 * #12161: chat didn't work, due to uppercase in TrueConfID of DstUID
 *
 * *****************  Version 159  *****************
 * User: Ktrushnikov  Date: 16.05.12   Time: 15:55
 * Updated in $/VSNA/transport/Router
 * #11938
 * - fix crash at early messages goes in recursion (crash on iterators
 * with m_early_msgs)
 *
 * *****************  Version 158  *****************
 * User: Ktrushnikov  Date: 15.05.12   Time: 18:05
 * Updated in $/VSNA/transport/Router
 * #11674: m_early_msgs
 * - save early login-messages, that arrive from configurator before
 * Services are ready
 *
 * *****************  Version 157  *****************
 * User: Mushakov     Date: 19.04.12   Time: 20:25
 * Updated in $/VSNA/transport/Router
 *  - send VS_TRANSPORT_MANAGING_DISCONNECT by shutdown
 *
 * *****************  Version 156  *****************
 * User: Mushakov     Date: 27.03.12   Time: 18:30
 * Updated in $/VSNA/transport/Router
 *  - server shutdown optimized
 *
 * *****************  Version 155  *****************
 * User: Mushakov     Date: 29.02.12   Time: 17:15
 * Updated in $/VSNA/transport/Router
 * - Connecting client to unsecure server handled
 *
 * *****************  Version 154  *****************
 * User: Mushakov     Date: 2.11.11    Time: 20:52
 * Updated in $/VSNA/transport/Router
 * - extermal processing of router messages ability added
 * - Forwarding messaged to any service ability added
 *
 * *****************  Version 153  *****************
 * User: Mushakov     Date: 5.10.11    Time: 21:36
 * Updated in $/VSNA/transport/Router
 *  - ssl refactoring (SetCert interfaces)
 *
 * *****************  Version 152  *****************
 * User: Mushakov     Date: 13.07.11   Time: 17:35
 * Updated in $/VSNA/transport/Router
 * - Roaming
 *
 * *****************  Version 151  *****************
 * User: Mushakov     Date: 7.07.11    Time: 18:08
 * Updated in $/VSNA/transport/Router
 * - getting rs address thru DNS
 * - Get server name by address (in server)
 * - fix bugs in start services sequence (AS)
 *  - Roaming in service added;
 *
 *
 * *****************  Version 150  *****************
 * User: Dront78      Date: 25.05.11   Time: 18:53
 * Updated in $/VSNA/transport/Router
 * - armadillo optimizations disabled totally
 *
 * *****************  Version 149  *****************
 * User: Mushakov     Date: 24.05.11   Time: 18:23
 * Updated in $/VSNA/transport/Router
 * - offline Registration Supported
 *
 * *****************  Version 148  *****************
 * User: Mushakov     Date: 6.05.11    Time: 20:43
 * Updated in $/VSNA/transport/Router
 *  - new reg; new reg cert; cert chain supported in tc_server
 *
 * *****************  Version 147  *****************
 * User: Mushakov     Date: 23.03.11   Time: 17:49
 * Updated in $/VSNA/transport/Router
 *  - Server Name Verification added at registration
 *
 * *****************  Version 146  *****************
 * User: Mushakov     Date: 17.03.11   Time: 18:09
 * Updated in $/VSNA/transport/Router
 *  - numOfBothConnection was removed
 *
 * *****************  Version 145  *****************
 * User: Mushakov     Date: 2.03.11    Time: 18:17
 * Updated in $/VSNA/transport/Router
 *
 * *****************  Version 144  *****************
 * User: Mushakov     Date: 2.03.11    Time: 17:43
 * Updated in $/VSNA/transport/Router
 *  - roaming
 *
 * *****************  Version 143  *****************
 * User: Mushakov     Date: 23.02.11   Time: 4:05
 * Updated in $/VSNA/transport/Router
 * - SecureHandshake ver. 2 added
 *
 * *****************  Version 142  *****************
 * User: Mushakov     Date: 3.01.11    Time: 21:26
 * Updated in $/VSNA/transport/Router
 * - update cert without restart
 * - Issue cert time notBefor =-30 days (sm)
 *
 * *****************  Version 141  *****************
 * User: Ktrushnikov  Date: 23.11.10   Time: 16:55
 * Updated in $/VSNA/transport/router
 * - don't process TestAuthority()
 *
 * *****************  Version 140  *****************
 * User: Mushakov     Date: 1.11.10    Time: 21:00
 * Updated in $/VSNA/transport/Router
 * - cert update added
 * - registration from server added
 * - authorization servcer added
 *
 * *****************  Version 139  *****************
 * User: Mushakov     Date: 10.09.10   Time: 20:24
 * Updated in $/VSNA/transport/Router
 * - Registration on SM added
 *
 * *****************  Version 138  *****************
 * User: Ktrushnikov  Date: 25.05.10   Time: 17:33
 * Updated in $/VSNA/transport/Router
 * #6902
 * - chek for '.','#' (not for '.','.','#')
 *
 * *****************  Version 137  *****************
 * User: Mushakov     Date: 29.03.10   Time: 15:47
 * Updated in $/VSNA/transport/Router
 * - log string added
 *
 * *****************  Version 136  *****************
 * User: Ktrushnikov  Date: 7.03.10    Time: 17:28
 * Updated in $/VSNA/transport/Router
 * - All storages are inherited from VS_DBStorageInterface (DB, Reg, LDAP,
 * Trial)
 * - g_vcs_storage changed to g_dbStorage
 * - TConferenceStatistics added
 * - Process of LogPartStat added for VCS(file) & BS(null)
 * - fixed with d78 TransportRoute::DeleteService (dont delete deleted
 * service)
 * BS::LogSrv: suppress_missed_call_mail property added
 *
 * *****************  Version 135  *****************
 * User: Mushakov     Date: 8.02.10    Time: 18:04
 * Updated in $/VSNA/transport/Router
 * - some commemts removed
 * - readingtrial minutes added
 *
 * *****************  Version 134  *****************
 * User: Melechko     Date: 1.02.10    Time: 16:30
 * Updated in $/VSNA/transport/router
 *
 * *****************  Version 133  *****************
 * User: Melechko     Date: 21.01.10   Time: 18:18
 * Updated in $/VSNA/transport/router
 * fix nanomites
 *
 * *****************  Version 132  *****************
 * User: Mushakov     Date: 21.01.10   Time: 15:55
 * Updated in $/VSNA/transport/router
 *  - SSL Disabled when getting of PKey or Cert failed
 *
 * *****************  Version 131  *****************
 * User: Mushakov     Date: 21.01.10   Time: 15:13
 * Updated in $/VSNA/transport/router
 *  - AV strlen(0) fixed
 *  - Check ServerName len bug fixed
 *
 * *****************  Version 130  *****************
 * User: Dront78      Date: 19.01.10   Time: 14:18
 * Updated in $/VSNA/transport/router
 * - bugfix 6902
 *
 * *****************  Version 129  *****************
 * User: Dront78      Date: 18.01.10   Time: 13:04
 * Updated in $/VSNA/transport/router
 *
 * *****************  Version 128  *****************
 * User: Mushakov     Date: 16.01.10   Time: 18:57
 * Updated in $/VSNA/transport/router
 * - SSL without EncryptLSP
 * - code for SSL throu EncryptLSP removed
 *
 * *****************  Version 127  *****************
 * User: Dront78      Date: 22.12.09   Time: 15:39
 * Updated in $/VSNA/transport/router
 * - enlarged stream deleted connection buffer
 * - fixed variables initialization
 * - fixed deleted loop counter
 *
 * *****************  Version 126  *****************
 * User: Dront78      Date: 15.12.09   Time: 9:02
 * Updated in $/VSNA/transport/router
 * - fixed forgotten return
 *
 * *****************  Version 125  *****************
 * User: Dront78      Date: 14.12.09   Time: 16:17
 * Updated in $/VSNA/transport/router
 * - fixed some server crashes
 * - added bad message logging
 *
 * *****************  Version 124  *****************
 * User: Mushakov     Date: 19.11.09   Time: 15:12
 * Updated in $/VSNA/transport/router
 * - VS_ServCertInfoInterface::GetPublicKey modified
 *
 * *****************  Version 123  *****************
 * User: Mushakov     Date: 30.10.09   Time: 14:49
 * Updated in $/VSNA/transport/router
 *  - Sign added in TransportHandshake
 *  - Sign Verify added in TransportRouter
 *  - hserr_verify_failed transport result code added
 *
 * *****************  Version 122  *****************
 * User: Ktrushnikov  Date: 26.02.09   Time: 19:02
 * Updated in $/VSNA/transport/router
 * - rool back to 106 version (no map)
 *
 * *****************  Version 106  *****************
 * User: Mushakov     Date: 18.06.08   Time: 17:04
 * Updated in $/VSNA/transport/router
 * - VS_MemoryLeak included
 * - Logging to smtp service added
 *
 * *****************  Version 105  *****************
 * User: Mushakov     Date: 16.06.08   Time: 19:15
 * Updated in $/VSNA/transport/router
 * - big count reconnects fixed
 *
 * *****************  Version 104  *****************
 * User: Mushakov     Date: 23.05.08   Time: 20:49
 * Updated in $/VSNA/transport/router
 * - the first connecting AS to SM takes long time
 *
 * *****************  Version 103  *****************
 * User: Dront78      Date: 19.05.08   Time: 14:33
 * Updated in $/VSNA/transport/router
 * - pipe queue increased
 * - memory leak removed
 *
 * *****************  Version 102  *****************
 * User: Smirnov      Date: 5.05.08    Time: 20:31
 * Updated in $/VSNA/transport/router
 * - removed unnecessary allocation, debug messages
 *
 * *****************  Version 101  *****************
 * User: Mushakov     Date: 30.04.08   Time: 20:04
 * Updated in $/VSNA/transport/router
 * strick endpoint connect timeout
 *
 * *****************  Version 100  *****************
 * User: Smirnov      Date: 21.04.08   Time: 17:15
 * Updated in $/VSNA/transport/router
 * - login of logged in user, transport interfaces cleaning
 *
 * *****************  Version 99  *****************
 * User: Ktrushnikov  Date: 15.04.08   Time: 21:21
 * Updated in $/VSNA/transport/router
 * - bugfix: moved DeleteEndpoint(CS_HSERROR) into if (!SetConnection)
 *
 * *****************  Version 98  *****************
 * User: Smirnov      Date: 14.04.08   Time: 19:40
 * Updated in $/VSNA/transport/router
 * - service and endpoints message average size corrected
 *
 * *****************  Version 97  *****************
 * User: Mushakov     Date: 10.04.08   Time: 14:32
 * Updated in $/VSNA/transport/router
 * SymmetricCrypt wrapper added. It for crypting stream
 *
 * *****************  Version 96  *****************
 * User: Dront78      Date: 4.04.08    Time: 18:47
 * Updated in $/VSNA/transport/router
 * - messages language changed to English
 *
 * *****************  Version 95  *****************
 * User: Mushakov     Date: 1.04.08    Time: 16:39
 * Updated in $/VSNA/transport/router
 * -key lengthes modified
 *
 * *****************  Version 94  *****************
 * User: Mushakov     Date: 27.03.08   Time: 16:50
 * Updated in $/VSNA/transport/router
 *
 * *****************  Version 93  *****************
 * User: Mushakov     Date: 25.03.08   Time: 17:51
 * Updated in $/VSNA/transport/router
 *  - SSL added
 *  - fixed bug: Connect to server with another name
 *
 * *****************  Version 92  *****************
 * User: Dront78      Date: 17.03.08   Time: 20:02
 * Updated in $/VSNA/transport/router
 * - send messages statistics added
 *
 * *****************  Version 91  *****************
 * User: Smirnov      Date: 14.03.08   Time: 13:13
 * Updated in $/VSNA/transport/router
 * - added increased number of messages for server endpoint
 *
 * *****************  Version 90  *****************
 * User: Smirnov      Date: 13.03.08   Time: 18:29
 * Updated in $/VSNA/transport/router
 * - format logs
 *
 * *****************  Version 89  *****************
 * User: Smirnov      Date: 12.03.08   Time: 22:33
 * Updated in $/VSNA/transport/router
 * - memory leak detected
 *
 * *****************  Version 88  *****************
 * User: Ktrushnikov  Date: 8.03.08    Time: 15:49
 * Updated in $/VSNA/transport/router
 * - debug output fixed: mess.ReleaseMessage() added to prevent delete 2
 * times
 *
 * *****************  Version 87  *****************
 * User: Smirnov      Date: 7.03.08    Time: 15:32
 * Updated in $/VSNA/transport/router
 * - debug info
 *
 * *****************  Version 86  *****************
 * User: Ktrushnikov  Date: 5.03.08    Time: 21:26
 * Updated in $/VSNA/transport/router
 * - lock for LocalRequest params added
 * - VS_TransportRouter_MessageQueue::DeleteMsg(): don't create notify
 * message, cause it is useless and would produce memory leak in
 * TR::ProcessingMessage()
 * - __dbg_printf() changed to dprint4() with _DEBUG definition
 * - method added: VS_TransportMessage::ReleaseMessage(): to clean this
 * with no delete of mess
 *
 * *****************  Version 85  *****************
 * User: Dront78      Date: 23.02.08   Time: 16:50
 * Updated in $/VSNA/transport/router
 *
 * *****************  Version 84  *****************
 * User: Dront78      Date: 20.02.08   Time: 19:35
 * Updated in $/VSNA/transport/router
 * - monitor structures updated
 *
 * *****************  Version 83  *****************
 * User: Stass        Date: 14.02.08   Time: 21:41
 * Updated in $/VSNA/transport/router
 *
 * *****************  Version 82  *****************
 * User: Dront78      Date: 14.02.08   Time: 19:40
 * Updated in $/VSNA/transport/router
 * - old architecture clients reject added to avoid reconnect flood
 *
 * *****************  Version 81  *****************
 * User: Dront78      Date: 14.02.08   Time: 17:32
 * Updated in $/VSNA/transport/router
 * - old handshakes finally removed
 * - fixed handshake error codes
 *
 * *****************  Version 80  *****************
 * User: Dront78      Date: 13.02.08   Time: 17:59
 * Updated in $/VSNA/transport/router
 * Transport managed messages updated.
 *
 * *****************  Version 79  *****************
 * User: Dront78      Date: 6.02.08    Time: 12:20
 * Updated in $/VSNA/transport/router
 * - VS_TransportMessage changed;
 * - transport updates;
 * - conditional updates;
 *
 * *****************  Version 78  *****************
 * User: Mushakov     Date: 5.02.08    Time: 17:01
 * Updated in $/VSNA/transport/router
 * memory leak fixed
 * if server name is not correct  it is changed by server
 *
 * *****************  Version 77  *****************
 * User: Smirnov      Date: 31.01.08   Time: 22:14
 * Updated in $/VSNA/transport/router
 * - conditions rewrited, added to AUTH_SRV_
 * - logoff user on point disconnect
 *
 * *****************  Version 76  *****************
 * User: Dront78      Date: 25.01.08   Time: 17:38
 * Updated in $/VSNA/transport/router
 * Unauthorized managed ping allowed.
 *
 * *****************  Version 75  *****************
 * User: Ktrushnikov  Date: 25.01.08   Time: 15:29
 * Updated in $/VSNA/transport/router
 * - check if uid or cid length is null
 * - set cid to VS_PointParam
 *
 * *****************  Version 74  *****************
 * User: Dront78      Date: 25.01.08   Time: 12:52
 * Updated in $/VSNA/transport/router
 * - kill messages for frong user at our server
 * - forefilling updated to support new format VS_ClientMessage
 *
 * *****************  Version 73  *****************
 * User: Dront78      Date: 24.01.08   Time: 18:07
 * Updated in $/VSNA/transport/router
 * GoAuthorize hops value fixed.
 *
 * *****************  Version 72  *****************
 * User: Dront78      Date: 24.01.08   Time: 17:50
 * Updated in $/VSNA/transport/router
 * Transport thread-safe authorization functions added.
 *
 * *****************  Version 71  *****************
 * User: Ktrushnikov  Date: 24.01.08   Time: 17:23
 * Updated in $/VSNA/transport/router
 * - ConnectPoint deleted
 *
 * *****************  Version 70  *****************
 * User: Ktrushnikov  Date: 24.01.08   Time: 16:15
 * Updated in $/VSNA/transport/router
 * - memory leak fixed
 *
 * *****************  Version 69  *****************
 * User: Ktrushnikov  Date: 24.01.08   Time: 16:10
 * Updated in $/VSNA/transport/router
 * - Дернуть OnPointConnect_Event(HS_ERROR), если connect прошел (есть
 * TCP-соединение), но Handshake не прошел (не наш протокол) [случай
 * только если мы коннектимся: функция ConnectPoint() из сервайса]
 *
 * *****************  Version 68  *****************
 * User: Dront78      Date: 23.01.08   Time: 17:58
 * Updated in $/VSNA/transport/router
 * - forefilling updated: froced SrcServer for unauthorized messages.
 *
 * *****************  Version 67  *****************
 * User: Dront78      Date: 23.01.08   Time: 15:01
 * Updated in $/VSNA/transport/router
 * volatile isShutdown added.
 *
 * *****************  Version 66  *****************
 * User: Dront78      Date: 22.01.08   Time: 18:34
 * Updated in $/VSNA/transport/router
 * Wrong forefilling memory access fixed.
 *
 * *****************  Version 65  *****************
 * User: Ktrushnikov  Date: 22.01.08   Time: 17:25
 * Updated in $/VSNA/transport/router
 * - if no messages - delete endpoint
 * - ControlInquiry type in Forced disconnect endpoint was set to All (not
 * to delete one endpoint)
 * - OnPointConnect/Disconnet conditions added in AppServer
 *
 * *****************  Version 64  *****************
 * User: Ktrushnikov  Date: 17.01.08   Time: 16:18
 * Updated in $/VSNA/transport/router
 * - printf changed to dprint4 in OnEndpointConnect()
 *
 * *****************  Version 63  *****************
 * User: Ktrushnikov  Date: 17.01.08   Time: 16:13
 * Updated in $/VSNA/transport/router
 * - Bug fix: IsTransportTotallyConnected/Disconnected() deleted. (Alex
 * add deleted)
 * - Call conditions in OnEndpointConnected() always (not only when
 * onEndpointConnectTime == 2)
 *
 * *****************  Version 62  *****************
 * User: Ktrushnikov  Date: 16.01.08   Time: 19:15
 * Updated in $/VSNA/transport/router
 * - OnPointConnect_Event() added
 *
 * *****************  Version 61  *****************
 * User: Mushakov     Date: 16.01.08   Time: 17:59
 * Updated in $/VSNA/transport/router
 * удалил из протакола индентификаторы транспорта (двусоединенный или
 * односоединенный)
 * убрал неиспользующиеся функции из VS_TransportLib
 *
 * *****************  Version 60  *****************
 * User: Stass        Date: 15.01.08   Time: 18:47
 * Updated in $/VSNA/transport/router
 * OnEndpointDisconnect parameters changed.
 *
 * *****************  Version 59  *****************
 * User: Mushakov     Date: 14.01.08   Time: 17:48
 * Updated in $/VSNA/transport/router
 * old GoSetConnection and SetConnections were removed
 *
 * *****************  Version 58  *****************
 * User: Mushakov     Date: 14.01.08   Time: 17:29
 * Updated in $/VSNA/transport/router
 * old SetConnection's removed
 *
 * *****************  Version 57  *****************
 * User: Mushakov     Date: 14.01.08   Time: 17:28
 * Updated in $/VSNA/transport/router
 *
 * *****************  Version 56  *****************
 * User: Dront78      Date: 14.01.08   Time: 16:12
 * Updated in $/VSNA/transport/router
 * Two-sockets transport removed.
 *
 * *****************  Version 55  *****************
 * User: Dront78      Date: 10.01.08   Time: 12:49
 * Updated in $/VSNA/transport/router
 * Changed message handling with no resolve service.
 *
 * *****************  Version 54  *****************
 * User: Dront78      Date: 10.01.08   Time: 12:42
 * Updated in $/VSNA/transport/router
 * Updated routing table : RESOLVE_SRV added.
 *
 * *****************  Version 53  *****************
 * User: Dront78      Date: 29.12.07   Time: 17:02
 * Updated in $/VSNA/transport/router
 * GetEndpointByUID updated.
 ****************************************************************************/

//#define   _MY_DEBUG_

#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "acs/connection/VS_ConnectionByte.h"
#include "acs/connection/VS_ConnectionMsg.h"
#include "acs/connection/VS_ConnectionOv.h"
#include "acs/connection/VS_ConnectionTCP.h"
#include "acs/connection/VS_ConnectionTLS.h"
#include "acs/Lib/VS_AcsLib.h"
#include "acs/Lib/VS_LoadCertsForTLS.h"
#include "../VS_ServCertInfoInterface.h"
#include "net/DNSUtils/VS_DNSUtils.h"
#include "net/EndpointRegistry.h"
#include "std-generic/clib/strcasecmp.h"
#include "std-generic/cpplib/ignore.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/cpplib/move_handler.h"
#include "std/cpplib/event.h"
#include "std/cpplib/fast_mutex.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_IntConv.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_Utils.h"
#include "std/debuglog/VS_Debug.h"
#include "tools/Server/VS_Server.h"
#include "transport/Lib/VS_TransportLib.h"
#include "transport/VS_RouterMessExtHandlerInterface.h"
#include "transport/VS_TransportDefinitions.h"
#include "VS_TransportHandler.h"
#include "VS_TransportMonitor.h"
#include "VS_TransportRouter.h"
#include "VS_TransportRouterServiceTypes.h"
#include "std/VS_RegServer.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"

#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_SecureHandshake.h"
#include "SecureLib/VS_Sign.h"
#include "SecureLib/VS_SymmetricCrypt.h"

#include "net/QoSSettings.h"

#include <boost/filesystem/operations.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/variant.hpp>

#include <Windows.h>
#include <direct.h>
#include <io.h>
#include <process.h>
#include <in6addr.h>

#include "vs_hash.h"
#include <cassert>
#include <fstream>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#define   VS_TR_TIMEOUT_THREAD_SHUTDOWN   200000
#define   VS_TR_REPEAT_ERROR_NUMBER   10

#define   VS_TR_PIPE_SERVICE_WRITE          5
#define   VS_TR_PIPE_SERVICE_READ           6
#define   VS_TR_CONNECTION_DEAD_WRITE       7
#define   VS_TR_CONNECTION_DEAD_READ        8
#define   VS_TR_CONTROL_WRITE               9
#define   VS_TR_CONTROL_READ                10
#define   VS_TR_MONITOR_WRITE               11
#define   VS_TR_MONITOR_READ                12
#define   VS_TR_MONITOR_CONNECT             13


#define	  VS_TR_BTH_CONNECTION_WRITE		16
#define	  VS_TR_BTH_CONNECTION_READ			17

#define	  VS_TR_BTH_SECURE_HANDSHAKE_READ		18
#define	  VS_TR_BTH_SECURE_HANDSHAKE_WRITE		19

///Connecttion death time in seconds.
#define   VS_TR_CONN_SUMP_DEPTH_TIME   10000

#define   VS_TR_DIRECTORY_LOGS_NAME   ".\\trs_logs\\"

const char   VS_TrPrefixMonPipe[] = VS_TR_PREFIX_MON_PIPE;
static const char   VS_TrDirLogsName[] = VS_TR_DIRECTORY_LOGS_NAME;
#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT
extern struct VS_Overlapped   ovZero;
extern std::string g_tr_endpoint_name;

const unsigned long ACCESS_DENIED_FOR_NEW_CONNECTIONS = 100;
const unsigned long DIE_TIMEOUT = 3000;
const unsigned long maxTimePerOperation = 250;
const unsigned long MAX_MESS_FOR_SERVER = 20000;
const unsigned long MAX_MESS_FOR_ROUTERSIDE_SERVECES = 20000;






static
class DiagnosticTimerStatistic
{
protected:
    unsigned long start,end,count,summ,average;
public:
    DiagnosticTimerStatistic():start(0),end(0),count(0),summ(0),average(0)
    {	}
    void AddPoint(const unsigned long & aStart,
                  const unsigned long & aStop)
    {
        if (aStop<aStart) return;
        summ +=(aStop - aStart);
        count++;
        return;
    }
    unsigned long GetCountNow()
    {
        return count;
    }
    unsigned long GetAverageNow()
    {
        return (count==0)?0:(summ/count);
    }
    void ClearSum()
    {
        summ = 0;
        count = 0;
    }
private:
}  transportStatistic;
//end DiagnosticTimerStatistic

class DiagnosticTimer
{
public:
    unsigned long start;
    DiagnosticTimer(DiagnosticTimerStatistic * stat)
            :start(GetTickCount()),statistic(stat)
    {
    }
    ~DiagnosticTimer()
    {
        const unsigned long stop = GetTickCount();
        statistic->AddPoint(start,stop);
    }
protected:
    DiagnosticTimerStatistic * statistic;
};
//end DiagnosticTimer

class VS_TransportNameContainer
{

private:
	typedef std::vector< std::string > TYPE_NameContainer;
    typedef TYPE_NameContainer::iterator TYPE_NameContainer_iterator;
protected:
    void * mutex;
    DWORD iCurrentThreadId;
    TYPE_NameContainer iContainer;
    TYPE_NameContainer_iterator iCurrent,iIt;
    bool ThreadSafeIn()
    {
        if (!mutex) return true;
        DWORD res = WaitForSingleObject(mutex,10000);
        if (res!=WAIT_OBJECT_0) return false;
        return true;
    }
    void ThreadSafeOut()
    {
        if (!mutex) return;
        ReleaseMutex(mutex);
    }
    bool GetCurrentName(const char *&aName)
    {
        if (iCurrent>iContainer.end())
        {
            dprint4("[ error state in container. I have catch it! #1 ]\n");
            return false;
        }
        if (iCurrent<iContainer.begin())
        {
            dprint4("[ error state in container. I have catch it! #2 ]\n");
            return false;
        }
        if (iCurrent==iContainer.end()) return false;
		aName = iCurrent->c_str();
        return true;
    }
public:
    VS_TransportNameContainer():iIt(iContainer.end()),iCurrent(iContainer.end()),mutex((void*)CreateMutex(0,0,0))
    {
    }
    virtual ~VS_TransportNameContainer()
    {
        ::CloseHandle((HANDLE)mutex);
        iContainer.clear();
    }
    bool RemoveAll()
    {
        if (ThreadSafeIn())
        {
            if (iContainer.empty())
            {
                ThreadSafeOut();
                return false;
            }
            iContainer.clear();
            ThreadSafeOut();
            return true;
        }
        return false;
    }
    bool RemoveCurrent()
    {
        if (ThreadSafeIn())
        {
            if ((iContainer.empty()) || (iCurrent==iContainer.end()))
            {
                ThreadSafeOut();
                return false;
            }
            iCurrent = iContainer.erase( iCurrent );
            ThreadSafeOut();
            return true;
        }
        return false;
    }
    bool Show()
    {

        if (ThreadSafeIn())
        {

            for (iIt = iContainer.begin();
                    iIt !=iContainer.end();
                    iIt++)
            {
                dprint4("%s\n",iIt->c_str());
            }
            ThreadSafeOut();
            return true;
        }
        return false;

    }
    bool RemoveName(const char * aName)
    {
        if (!aName || !*aName) return false;
        if (ThreadSafeIn())
        {
            if (iContainer.empty())
            {
                ThreadSafeOut();
                return false;
            }
            for (iIt=iContainer.begin();
                    iIt!=iContainer.end();
                    iIt++)
            {
				if (strcasecmp(iIt->c_str(),
                             aName)==0)
                {
                    iContainer.erase( iIt );
                    ThreadSafeOut();
                    return true;
                }
            }
            ThreadSafeOut();
        }
        return false;
    }
    bool AddName(const char * aName)
    {
        if (!aName || !*aName) return false;
        if (ThreadSafeIn())
        {
            ///same search
            for (iIt=iContainer.begin();
                    iIt!=iContainer.end();
                    iIt++)
            {
				if (strcasecmp(iIt->c_str(),
                             aName)==0)
                {
                    ThreadSafeOut();
                    return true;
                }
            }
            ///addact
			std::string str = std::string(aName);
            iContainer.push_back( str );
            ThreadSafeOut();
            return true;
        }
        return false;
    }
    bool GetFirstName(const char *& aName)
    {
        iCurrentThreadId = GetCurrentThreadId();
        if (IsEmpty()) return false;
        iCurrent = iContainer.begin();
		return GetCurrentName(aName);
    }
    bool GetNextName(const char *& aName)
    {
        ///searching can be doing only one thread simaltinously,
        ///thread - that last call GetFirstName function.
        if (IsEmpty()) return false;

        if (iCurrentThreadId!=GetCurrentThreadId()) return false;

        if (iCurrent!=iContainer.end()) iCurrent++;
        else iCurrent = iContainer.begin();

        if (iCurrent==iContainer.end())
            return false;

        return GetCurrentName(aName);
    }
    bool IsEmpty()
    {
        bool res = true;
        if (ThreadSafeIn())
        {
            res = iContainer.empty();
            ThreadSafeOut();
        }
        return res;
    }
};
// end VS_TransportNameContainer

class VS_TransportRouterCriteria
{
public:
    VS_TransportRouterCriteria()
            :iCriteriaValue(0),iStopFlag(0),iLastSz(0)
    {
    }
    const unsigned long & operator++()
    {
        return ++iCriteriaValue;
    }
    const unsigned long &
    operator+=( const unsigned long & aValue)
    {
        if (iStopFlag)
            return iCriteriaValue;
        iCriteriaValue +=  aValue;
        return iCriteriaValue;
    }
    void operator=(const unsigned long aValue)
    {
        if (iStopFlag)
            return;
        if (aValue==0)
        {
            iCont.push_back( iCriteriaValue );
            iCriteriaValue = 0;
        }
        else
        {
            iCriteriaValue = aValue;
        }
    }
    void GetStatistic(unsigned long &aSize,
                      unsigned long * &aArray)
    {
        if (iCont.empty())
        {
            iStopFlag = true;
            aSize = 0;
            aArray = 0;
            return;
        }
        iStopFlag = true;
        aSize = (unsigned long)iCont.size();
        aArray = &*iCont.begin();
    }
    void Renew()
    {
        iCont.clear();
        iCriteriaValue = 0;
        iStopFlag = false;
    }
    unsigned long GetCurrentValue() const
    {
        return iCriteriaValue;
    }
    unsigned long GetLastSaved()
    {
        unsigned long theSize = (unsigned long)iCont.size();
        if (theSize)
        {
            unsigned long * res = &*iCont.begin();
            return res[theSize-1];

        }
        return GetCurrentValue();
    }
    unsigned long GetLastSumAv()
    {
        unsigned long theSize = (unsigned long)iCont.size();
        if (iLastSz<theSize) //a!=0
        {
            unsigned long a = theSize - iLastSz;
            return GetLastSum()/a;
        }
        return 0;
    }
    unsigned long GetLastSum()
    {
        if (iCont.empty())
        {
            iLastSz = 0;
            return 0;
        }
        unsigned long theSize = (unsigned long)iCont.size();
        unsigned long i = iLastSz;
        unsigned long theSum = 0;
        unsigned long * ptr = &*iCont.begin();
        for (;i<theSize;++i)
        {
            theSum += ptr[i];
        }
        iLastSz = theSize;
        return theSum;
    }
protected:
    unsigned long iCriteriaValue;
    std::vector<unsigned long> iCont;
    bool iStopFlag;
    unsigned long iLastSz;
};
//end VS_TransportRouterCriteria

class VS_TranaportRouterObserver
{
public:
    VS_TranaportRouterObserver()
            :observerTicks(0)
    {
    }
    static const unsigned int max_creteria = 16;
    VS_TransportRouterCriteria crt[max_creteria];

    void UpdateAll()
    {
        observerTicks++;
        for (i=0;i<max_creteria;++i)
        {
            crt[i] = 0;
        }
    }
    void GetStatisticAll()
    {
        for (i=0;i<max_creteria;++i)
        {
            crt[i].GetStatistic(sz[i],ptr[i]);
        }
        for (i=0;i<max_creteria;++i)
        {
            crt[i].Renew();
        }
        observerTicks = 0;
    }
    unsigned long GetCounts() const {
        return observerTicks;
    }
private:
    unsigned long observerTicks;
    unsigned long * ptr[max_creteria];
    unsigned long sz[max_creteria];
    unsigned int i;
};
//end VS_TranaportRouterObserver
struct VS_TransportRouter_Service;
static const unsigned long Max_Message_Size = 65535;

struct VS_TransportRouter_ImplementationBase : public VS_TransportRouter_CallService
{
    VS_TransportRouter_ImplementationBase( const char* private_key_pass )
            : hiocp(0),indexSump(0), hops(1)
            , currDiffTm(0), currTm(0), sumCurrDiffTm(0), tickDiffTm(0), tickTm(0)
            , dirLogsName(VS_TrDirLogsName), flagLogs(true)
            , sendBytes(0), lastSendBytes(0), receiveBytes(0), lastReceiveBytes(0)
            , out_bytes(0.), in_bytes(0.), lastBitrateTm(0), tMessagesLog(0),routerDebugFlag(0)
            , trDebugLog(0)
            ,allDeleted(0),added2Port(0),deletedFromPort(0),m_counter(0)
            ,max_removed_time(0),lastRouterTick(0),routerEndTick(0),m_cert_buf_ln(0)
			,m_privateKeyPass(private_key_pass)

    {
        {

            VS_RegistryKey key(false, CONFIGURATION_KEY);
            unsigned   hops = 0;
            if (key.GetValue(&hops, sizeof(hops), VS_REG_INTEGER_VT, REQUESTED_HOP_COUNT_KEY_NAME) == sizeof(hops)
                    && hops < 254)	VS_TransportRouter_ImplementationBase::hops = (unsigned char)hops;
            char   file_name[512];
            ZeroMemory( (void *)file_name, sizeof(file_name) );
            if (key.GetValue(file_name, sizeof(file_name) - 1, VS_REG_STRING_VT, TRANSPORT_MESSAGES_LOGS_FILE_TAG_NAME) > 0 && *file_name)
            {
                tMessagesLog = new VS_AcsLog( file_name, 50000, 25000, "./" );
                if (tMessagesLog && !tMessagesLog->IsValid())
                {
                    delete tMessagesLog;
                    tMessagesLog = 0;
                }
            }

			// try to initialize certificate, privatekey, and chain
			ReadCryptoData();
			m_last_write = key.GetLastWriteTime();
        }

    }
    // end VS_TransportRouter_ImplementationBase::VS_TransportRouter_ImplementationBase

    virtual ~VS_TransportRouter_ImplementationBase( void )
    {
    }

	bool ReadCryptoData(void)
	{
		VS_RegistryKey key(false, CONFIGURATION_KEY);
		VS_Certificate cert;
		bool key_cert_status = true, chain_status = true;

		std::unique_ptr<char, free_deleter> pkey_buf;
		if (!key.GetValue(pkey_buf, VS_REG_BINARY_VT, SRV_PRIVATE_KEY) ||
			!m_pKey.SetPrivateKey(pkey_buf.get(), store_PEM_BUF, /*SERVER_CERT_PRIVATE_KEY_PASS*/m_privateKeyPass) ||
			!(m_cert_buf_ln = key.GetValue(m_Cert, VS_REG_BINARY_VT, SRV_CERT_KEY)) || !m_Cert || !cert.SetCert(m_Cert.get(), m_cert_buf_ln, store_PEM_BUF))
		{
			m_Cert = nullptr;
			m_cert_buf_ln = 0;
			key_cert_status = false;
		}

		std::unique_ptr<void, free_deleter> cert_buf;
		int sz(0);
		if ((sz = key.GetValue(cert_buf, VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY)) > 0)
		{
			cert_chain.Deserialize(cert_buf.get(), sz);
		}
		else
		{
			chain_status = false;
		}

		return key_cert_status && chain_status;
	}


    // end VS_TransportRouter_ImplementationBase::~VS_TransportRouter_ImplementationBase
    virtual inline void ForcedDisconnectByPeer(const char *endpoint_name) = 0;
    virtual inline void ProcessingMessages( VS_RouterMessage * ) = 0;
    virtual inline bool ConnectConnection( const char *endpoint_name, /*const unsigned type,*/
                                           const unsigned long maxLackMs ) = 0;
	virtual inline void CancelEndpointDisconnect(const char *aEndpointName )=0;
	virtual inline VS_TransportRouter_Service *GetService( const char *serviceName ) = 0;
    vs::fast_recursive_mutex mtx;
    vs::fast_mutex mtx_read;
    HANDLE hiocp;
    std::string endpointName;
    unsigned long lastRouterTick;
    typedef std::vector<VS_Connection*> VS_TYPE_ConnsContainre;
    typedef VS_TYPE_ConnsContainre::iterator VS_TYPE_ConnsIt;
    VS_TYPE_ConnsContainre m_deletedSslProcessingConns;
	VS_SimpleStr	m_privateKeyPass;
    struct VS_RemoveConn
    {
        VS_RemoveConn():conn(0),
                time(0),flag(0)
        {}
        VS_Connection* conn;
        unsigned long time;
        char flag;
    };
protected:
    typedef std::map< unsigned long , VS_RemoveConn> VS_TYPE_RemovedConns;
    typedef VS_TYPE_RemovedConns::iterator VS_TYPE_RemovedConnsIt;
    VS_TYPE_RemovedConns m_removed;
public:
    unsigned long   routerEndTick;
    unsigned long   indexSump;
    unsigned char   hops;
    unsigned long allDeleted,added2Port,deletedFromPort;
    unsigned long   currDiffTm, currTm, sumCurrDiffTm, tickDiffTm, tickTm,routerDebugFlag;
    const char   *dirLogsName;
    bool   flagLogs;
    unsigned __int64   sendBytes, lastSendBytes, receiveBytes, lastReceiveBytes;
    double   out_bytes, in_bytes;
    unsigned long   lastBitrateTm;
    VS_AcsLog   *tMessagesLog, *trDebugLog;
    VS_HashFunctionsIncapsulator hash_incap;
    ///Observer
    VS_TranaportRouterObserver observer;
	VS_PKey	m_pKey;
	std::unique_ptr<char, free_deleter> m_Cert;
	unsigned long m_cert_buf_ln;
	VS_Container cert_chain;
	std::string m_last_write;


    ///Observer
    unsigned long m_counter;
    inline void ShowConn(const char * endpoint , const unsigned int type, const VS_Connection  * conn,
                         const char * where_it = 0 )
    {
        if (trDebugLog && trDebugLog->IsValid())
            trDebugLog->TPrintf("\n\t %s Show: %s %d 0x%X",
                               (where_it!=0)?(where_it):("N"),endpoint,type, conn);
    }
    static const unsigned long maxCouterIntervalR = VS_TR_CONN_SUMP_DEPTH_TIME;///xSeconds - life of deleted conns
	static const unsigned long counterBarrier = 10;

    inline void PeriodicDeleteConn()
    {
        if (!m_deletedSslProcessingConns.empty())
        {
            VS_TYPE_ConnsIt it = m_deletedSslProcessingConns.begin();
            while (it != m_deletedSslProcessingConns.end())
            {
                if ((*it) && (!(*it)->IsSecureHandshakeInProgress()))
                {
                    ShowConn("ep",0,(*it),"Delete SSL in queue");
                    delete (*it);
                    (*it) = 0;
                    it = m_deletedSslProcessingConns.erase( it );
                } else ++it;
            }
        }
        if (m_counter>=counterBarrier)
        {
            m_counter = 0;
            if (!m_removed.empty())
            {
                MakeCloseDeletedConn();
                MakeDeleteDeletedConn();
                AutoEraseDeletedConn();
            }
        }else
        {
            ++m_counter;
        }

    }
private:
    unsigned long max_removed_time;
    inline void FastRemoveDeleted(VS_Connection *sock,bool isFromRouteDelete = false)
    {
        if (m_removed.empty())
            return;

        unsigned long key = 0;
        memcpy(&key,&sock,4);

        VS_TYPE_RemovedConnsIt it(m_removed.find( key ));
        if (it!=m_removed.end())
        {
            unsigned long a = GetTickCount() - it->second.time;
            ShowConn("ep",0,(it->second.conn),"Remove from delayed queue");
            m_removed.erase( key );
            if (a>max_removed_time)
            {
                max_removed_time = a;
            }
        }
    }
    inline void AutoEraseDeletedConn()
    {
        if (m_removed.empty())
            return;

        VS_TYPE_RemovedConnsIt it = m_removed.begin();
        while (it != m_removed.end())
        {
            if (it->second.flag == 2) {
                ShowConn("ep",0,(it->second.conn),"Remove from delayed queue-was deleted");
                it = m_removed.erase( it );
            } else ++it;
        }
    }

    inline void MakeDeleteDeletedConn()
    {
        if (m_removed.empty())
            return;

        unsigned long a = GetTickCount();
        VS_TYPE_RemovedConnsIt it(m_removed.begin());
        for (;it!=m_removed.end();++it)
        {
            if ((( a - it->second.time ) > maxCouterIntervalR*1000)
                    && (it->second.flag == 1))
            {
                ShowConn("ep",0,(it->second.conn),"DELETE from delayed queue");
                if (it->second.conn)
                {
                    delete it->second.conn;
                }
                it->second.conn = 0;
                it->second.flag = 2;
                it->second.time = a;
                ++deletedFromPort;
            }
        }
    }

    inline void MakeCloseDeletedConn()
    {
        if (m_removed.empty())
            return;

        unsigned long a = GetTickCount();
        VS_TYPE_RemovedConnsIt it(m_removed.begin());
        for (;it!=m_removed.end();++it)
        {
            if ((( a - it->second.time ) > maxCouterIntervalR*1000)
                    && (it->second.flag==0))
            {
                ShowConn("ep",0,(it->second.conn),"CLOSE from delayed queue");
                if (it->second.conn)
                    it->second.conn->Close();
                it->second.flag = 1;
                it->second.time = a;
            }
        }
    }
protected:
    inline void RemoveAllDeleteConns()
    {
        if (m_removed.empty())
            return;


        VS_TYPE_RemovedConnsIt it(m_removed.begin());
        unsigned long a = GetTickCount();
        for (;it!=m_removed.end();++it)
        {
            if ( it->second.conn )
            {
                it->second.conn->Close();
                delete it->second.conn;
                it->second.conn = 0;
                it->second.flag = 2;
                it->second.time = a;
            }
        }
        m_removed.clear();
    }
public:
    virtual void ShowPeriod()
    {
		unsigned long b = GetTickCount();
        dprint4("Last transport period: %ld ms\n", b - lastRouterTick);
        if (b-lastRouterTick > 10000)
        {
            dprint4("Warning transport lock!\n");
        }
    }
    inline void AddRouteDeleteConn(VS_Connection *sock)
    {
        ShowConn("ep",0,(sock),"ADD to delayed queue");
        VS_RemoveConn rconn;
        unsigned long key = 0;
        memcpy(&key,&sock,4);

        rconn.conn = sock;
        rconn.time = GetTickCount();
        m_removed[ key ] =  rconn ;
        ++added2Port;
    }
    inline void DeleteConn( VS_Connection *sock ,
                            const char * functionWhere = 0,
                            const char * endpoint = 0,
                            bool isFromRouteDelete = false)
    {
        std::string str1(functionWhere);
        str1.append(" -> DeleteConn");
        ShowConn(endpoint,0,(sock),str1.c_str());
        if (!sock)	return;
        if (sock->IsRW()) {
            return;
        }
        if (routerDebugFlag>0 && trDebugLog)
        {
            trDebugLog->TPrintf(" DeleteConn in %s Endpoint: %s\n",
                               (functionWhere!=0)?functionWhere:"unset" ,
                               (endpoint!=0)?endpoint:"unset" );
        }

        unsigned long i = 0;
        FastRemoveDeleted(sock, isFromRouteDelete);
        if (isFromRouteDelete)
        {
            ++deletedFromPort;
        }
        ++allDeleted;
        sock->SetOvFields();
        sock->Close();
        ShowConn("ep", 0, (sock), "delete DeleteConn");
        delete sock;
    }
    // end VS_StreamsRouter_Participant::DeleteConn

    inline bool IsThereEndpointRegistry( const char *endpointName )
    {
        return net::endpoint::GetCountConnectTCP(endpointName, false) != 0;
    }
    // end of VS_StreamsRouter_Participant::IsThereEndpointRegistry

    inline void MessageLog( VS_RouterMessage &mess , VS_AcsLog * trLog , char * info)
    {

        if (!trLog) return;
        const char *dst = mess.DstService();
        const char *src = mess.SrcService();
		const char *const type = mess.AddString();

        const auto sz = mess.BodySize();
        unsigned long i = 0;
        unsigned __int64 hash = 0;
        unsigned __int64 crc64 = 0;
        if (sz<=Max_Message_Size)
        {
            const auto message_body = mess.Body();
            hash  = hash_incap.MakeHash( message_body , sz );
            crc64 = hash_incap.MakeCheckSummCRC32( message_body , sz );
        }
        if (!dst || !*dst)
        {

            trLog->TPrintf( "\n\t%sSrc:\"%-10.10s\",\"%-16.16s\".Dst:\"%-10.10s\"                   .TLim:%10.10u.BSz:%3.3u. Seq:%10.10u H:0x%16.16I64X CRC: 0x%16.16I64X RealFrom:%10.10s B:",
                           (mess.IsNotify() ? "Notify. " : (mess.IsRequest() ? "Request." :"Reply.  ")),
                           mess.SrcCID(), mess.SrcService(), mess.DstCID(),
                           mess.TimeLimit(), sz , mess.SeqNum(), hash , crc64, info);
        }
        else
        {
            trLog->TPrintf( "\n\t%sSrc:\"%-10.10s\",\"%-16.16s\".Dst:\"%-10.10s\",\"%-16.16s\".TLim:%10.10u.BSz:%3.3u. Seq:%10.10u H:0x%16.16I64X CRC: 0x%16.16I64X RealFrom:%10.10s B:",

                           (mess.IsNotify() ? "Notify. " : (mess.IsRequest() ? "Request." :"Reply.  ")),
                           mess.SrcCID(), mess.SrcService(), mess.DstCID(), mess.DstService(),
                           mess.TimeLimit(),  sz , mess.SeqNum(), hash , crc64 , info );

        }
        if ((!src || !*src) && (type && *type))
        {
			switch (type[0])
			{
			case VS_TRANSPORT_MANAGING_CONNECT :
				trLog->Printf(" VS_TRANSPORT_MANAGING_CONNECT");
				break;
			case VS_TRANSPORT_MANAGING_DISCONNECT :
				trLog->Printf(" VS_TRANSPORT_MANAGING_DISCONNECT");
				break;
			case VS_TRANSPORT_MANAGING_PING :
				trLog->Printf(" VS_TRANSPORT_MANAGING_PING");
				break;
			default :
				trLog->Printf(" VS_TRANSPORT_MANAGING_UNKNOWN");
			}
		}
	}
    // end VS_TransportRouter_Implementation::MessageLog

    inline void MessageLog( VS_RouterMessage &mess )
    {
        if (!*mess.DstService())	return;
        tMessagesLog->Printf( "%sSrc:\"%-10.10s\",\"%-16.16s\".Dst:\"%-10.10s\",\"%-16.16s\".TLim:%u.BSz:%u.\n",
                              //(mess.IsRequest() ? "Request." : (mess.IsReply() ? "Reply.  " : "Notify. ")),
                              (mess.IsNotify() ? "Notify. " : (mess.IsRequest() ? "Request." :"Reply.  ")),
                              mess.SrcCID(), mess.SrcService(), mess.DstCID(), mess.DstService(),
                              mess.TimeLimit(), mess.BodySize() );
    }
    // end VS_TransportRouter_Implementation::MessageLog
};
// end VS_TransportRouter_ImplementationBase struct

//////////////////////////////////////////////////////////////////////////////////////////

enum class MessageDeleteReason
{
	send = 0,
	limit_restriction,
	time_out,
	endpoint_delete,
};

class VS_TransportMessageQueue
{
public:
	VS_TransportMessageQueue(size_t max_size)
		: m_max_size(max_size)
	{
	}

	const transport::Message& Front() const
	{
		return m_msgs.front();
	}

	transport::Message Pop(MessageDeleteReason reason)
	{
		auto msg = std::move(m_msgs.front());
		if (reason != MessageDeleteReason::send)
			DescribeDelete(msg, reason);
		m_msgs.pop_front();
		return msg;
	}

	void Push(transport::Message&& msg)
	{
		if (m_msgs.size() >= m_max_size)
			Pop(MessageDeleteReason::limit_restriction);
		m_msgs.push_back(std::move(msg));
	}

	void PushFront(transport::Message&& msg)
	{
		if (m_msgs.size() >= m_max_size)
			return;
		m_msgs.push_front(std::move(msg));
	}

	bool IsEmpty() const
	{
		return m_msgs.empty();
	}

    void ProcessingTick(unsigned long tickDiffTm)
	{
		for (auto it = m_msgs.begin(); it != m_msgs.end(); )
		{
			if (it->TimeLimit() < tickDiffTm)
			{
				DescribeDelete(*it, MessageDeleteReason::time_out);
				it = m_msgs.erase(it);
			}
			else
			{
				it->SetTimeLimit(it->TimeLimit() - tickDiffTm);
				++it;
			}
		}
	}

private:
	void DescribeDelete(const transport::Message& mess, MessageDeleteReason r)
	{
		const char* reason;
		switch (r)
		{
		case MessageDeleteReason::send:              reason = "BySend"; break;
		case MessageDeleteReason::limit_restriction: reason = "MsgLimit"; break;
		case MessageDeleteReason::time_out:          reason = "TimeOut"; break;
		case MessageDeleteReason::endpoint_delete:   reason = "DeleteEndpoint"; break;
		default:                                     reason = "Unknown reason"; break;
		}
		dprint1("DeleteMsg rs=%s | %s:%s:[%s] ==> %s:%s:[%s] | ttl=%5.5u\n",
			reason, mess.SrcServer(), mess.SrcUser() ? mess.SrcUser(): mess.SrcCID(), mess.SrcService(),
			mess.DstServer(), mess.DstUser(), mess.DstService(), mess.TimeLimit());
	}

	std::list<transport::Message> m_msgs;
	size_t m_max_size;
};

//////////////////////////////////////////////////////////////////////////////////////////

struct VS_TransportRouterPipeService
{
    VS_TransportRouterPipeService( VS_TransportRouter_ImplementationBase *tr,
                                   const unsigned long index,
                                   VS_TransportRouterService_Implementation *imp ) :
            tr(tr), isValid(false), index(index), sendQueue(MAX_MESS_FOR_ROUTERSIDE_SERVECES),
            pipe(0), writeSize(0), readSize(sizeof(transport::MessageFixedPart)),
            readBuffer(0), stateRcv(0)
    {
        pipe = new VS_ConnectionByte;
        if (!pipe || !pipe->IsValid() || !pipe->Open( imp->pipe, vs_pipe_type_duplex )
                || !pipe->SetIOCP( tr->hiocp )
                || !pipe->Read((void *)&readHead, sizeof(transport::MessageFixedPart)))
			return;
        pipe->SetOvWriteFields( VS_TR_PIPE_SERVICE_WRITE, (VS_ACS_Field)index );
        pipe->SetOvReadFields( VS_TR_PIPE_SERVICE_READ, (VS_ACS_Field)index );
        isValid = true;
    }
    // end VS_TransportRouterPipeService::VS_TransportRouterPipeService

    ~VS_TransportRouterPipeService( void ) {
        DeletePipe();
    }
    // end VS_TransportRouterPipeService::~VS_TransportRouterPipeService

    VS_TransportRouter_ImplementationBase* tr;
    bool   isValid;
    const unsigned long   index;
    VS_TransportMessageQueue sendQueue;
    VS_ConnectionByte   *pipe;
    unsigned long   writeSize, readSize;
    transport::MessageFixedPart readHead;
    transport::Message writeMsg;
    unsigned char* readBuffer;
    unsigned   stateRcv;

    inline void DeletePipe( void )
    {
#ifdef _MY_DEBUG_
        printf("VS_TransportRouterPipeService::DeletePipe():\n");
#endif
        if (pipe)
        {
            if (!pipe->IsRW())
            {
                tr->DeleteConn( pipe, "DeletePipe" );
            }
            else
            {
                pipe->SetOvReadFields( VS_TR_CONNECTION_DEAD_READ, (VS_ACS_Field)pipe );
                pipe->SetOvWriteFields( VS_TR_CONNECTION_DEAD_WRITE, (VS_ACS_Field)pipe );
                pipe->Disconnect();
                tr->AddRouteDeleteConn( pipe );
            }
            pipe = 0;
        }
        if (readBuffer) {
            free( (void *)readBuffer );
            readBuffer = 0;
        }
    }
    // end VS_TransportRouterPipeService::DeletePipe

    inline bool Write( void )
    {
        if (pipe->IsWrite() || sendQueue.IsEmpty())
            return true;
        const auto msg = sendQueue.Front().Data();
        const auto sz = sendQueue.Front().Size();
        if (!pipe->Write(msg, sz))
            return false;
        writeMsg = sendQueue.Pop(MessageDeleteReason::send);
        writeSize = sz;
        return true;
    }
    // end VS_TransportRouterPipeService::Write

    inline bool ProcessingMessages( VS_RouterMessage *mess )
    {
        sendQueue.Push(std::move(*mess));
        delete mess;
        return Write();
    }
    // end VS_TransportRouterPipeService::ProcessingMessages

    inline bool Read( VS_RouterMessage *&readMess )
    {
        switch (stateRcv)
        {
        case 0 :
            if (!readHead.mark1 || readHead.version < 1
                    || readHead.head_length < sizeof(transport::MessageFixedPart) + 6)
            {	/* Here it will be necessary to throw off in TRACE */
                return false;
            }
            readSize = readHead.head_length + readHead.body_length + 1;
            ::free(readBuffer);
            readBuffer = (unsigned char *)malloc( (size_t)readSize );
            if (!readBuffer)
            {	/* Here it will be necessary to throw off in TRACE */
                return false;
            }
            *(transport::MessageFixedPart*)readBuffer = readHead;
            readSize -= sizeof(transport::MessageFixedPart);
            stateRcv = 1;
            if (!pipe->Read((void *)&readBuffer[sizeof(transport::MessageFixedPart)], readSize))
            {
                dprint4("VS_TransportRouterPipeService Read pipe->Read return false; case 0; readSize = %ld;Err = %ld;\n", readSize, GetLastError());
                return false;
            }
            return true;
        case 1 :
        {
            VS_RouterMessage* mess = new VS_RouterMessage(readBuffer, sizeof(transport::MessageFixedPart) + readSize);
            if (mess->IsValid())
				readMess = mess;
            else
            {	/* Here it will be necessary to throw off in TRACE */
                dprint4("VS_TransportRouterPipeService Read  transport::Message( readBuffer )\n");
                delete mess;
                return false;
            }
        }
        free(readBuffer);
        readBuffer = 0;
        readSize = sizeof(transport::MessageFixedPart);
        stateRcv = 0;
        if (!pipe->Read((void *)&readHead, sizeof(transport::MessageFixedPart)))
        {
            dprint4("VS_TransportRouterPipeService Read pipe->Read return false; case 1; readSize = %ld;Err = %ld;\n", readSize, GetLastError());
            return false;
        }
        return true;
        default :	/* Here it will be necessary to throw off in TRACE */
            dprint4("VS_TransportRouterPipeService case default\n");
            return false;
        }
    }
    // end VS_TransportRouterPipeService::Read

    inline bool PipeWrite( unsigned long bTrans, VS_Overlapped &ov )
    {
        int   res = pipe->SetWriteResult( bTrans, &ov );
        if (res == -2)		return true;
        if (res != (int)writeSize)		return false;
        writeSize = 0;
        return Write();
    }
    // end VS_TransportRouterPipeService::PipeWrite

    inline bool PipeRead( unsigned long bTrans, VS_Overlapped &ov, VS_RouterMessage *&readMess )
    {
        int   res = pipe->SetReadResult( bTrans, &ov );
        if (res == -2)	return true;
        if (res!=(int)readSize)
        {
            dprint4("pipe->SetReadResult failed return %d;bTrans = %ld, readSize = %ld\n", res, bTrans, readSize);
        }
        bool b = res==(int)readSize && Read( readMess );
        if (!b && res==(int)readSize)
        {
            dprint4("PipeRead Read false;bTrans = %ld, readSize = %ld\n", bTrans, readSize);
        }
        return b;
    }
    // end VS_TransportRouterPipeService::PipeRead
};
// end VS_TransportRouterPipeService struct

struct VS_TransportRouter_Service
{
    VS_TransportRouter_Service( VS_TransportRouter_ImplementationBase *tr,
                                const unsigned long index,
                                const char *endpointName, const char *serviceName,
                                VS_TransportRouterServiceBase *service, bool withOwnThread )
		: isValid(false),
            tr(tr), index(index), withOwnThread(withOwnThread), pipeService(0), servBase(0),
            rcvMess(0), totalRcvMessSz(0), minRcvMessSz(0), maxRcvMessSz(0),
            sndMess(0), totalSndMessSz(0), minSndMessSz(0), maxSndMessSz(0)
    {
        *VS_TransportRouter_Service::serviceName = *serviceType = 0;
        strncpy( VS_TransportRouter_Service::serviceName, serviceName, sizeof(VS_TransportRouter_Service::serviceName) );
        VS_TransportRouter_Service::serviceName[sizeof(VS_TransportRouter_Service::serviceName) - 1] = 0;
        VS_TransportRouterServiceBase_Implementation   *baseImp = 0;
		servBase = service;
        if (withOwnThread)
        {
            strncpy( serviceType, "Async", sizeof(serviceType) );
            VS_TransportRouterService_Implementation   *imp = new VS_TransportRouterService_Implementation( service, endpointName, serviceName, tr );
            if (!imp)	return;
            if (!imp->isValid) {
                delete imp;
                return;
            }
            pipeService = new VS_TransportRouterPipeService( tr, index, imp );
            if (!pipeService || !pipeService->isValid) {
                delete imp;
                return;
            }
            servBase->imp.reset(imp);
        } else
        {
            strncpy( serviceType, "Call", sizeof(serviceType) );
            VS_TransportRouterServiceBase_Implementation   *imp = new VS_TransportRouterServiceBase_Implementation( servBase, endpointName, serviceName, tr );
            if (!imp)	return;
            if (!imp->isValid) {
                delete imp;
                return;
            }
            servBase->imp.reset(imp);
        }
        if (!servBase->Init(servBase->imp->OurEndpoint(), servBase->imp->OurService()))
            return;
        isValid = true;
    }

    ~VS_TransportRouter_Service( void )
    {
        if (isValid)
			servBase->Destroy(servBase->imp->OurEndpoint(), servBase->imp->OurService());
        if (pipeService)	delete pipeService;
    }

    bool   isValid;
    VS_TransportRouter_ImplementationBase   *tr;
    char   serviceName[VS_TRANSPORT_MAX_SIZE_SERVICE_NAME + 1];
    const unsigned long   index;
	bool withOwnThread;
    VS_TransportRouterPipeService   *pipeService;
    VS_TransportRouterServiceBase   *servBase;
    char   serviceType[10];
    unsigned __int64   rcvMess, totalRcvMessSz;
    unsigned long   minRcvMessSz, maxRcvMessSz;
    unsigned __int64   sndMess, totalSndMessSz;
    unsigned long   minSndMessSz, maxSndMessSz;

    inline void AddRcvMsgStatistics( const unsigned long length )
    {
        ++rcvMess;
		totalRcvMessSz+=length;
        if (minRcvMessSz > length || !minRcvMessSz)		minRcvMessSz = length;
        if (maxRcvMessSz < length || !maxRcvMessSz)		maxRcvMessSz = length;
    }
    // end VS_TransportRouter_Service::AddRcvMsgStatistics

    inline void AddSndMsgStatistics( const unsigned long length )
    {
        ++sndMess;
		totalSndMessSz+=length;
        if (minSndMessSz > length || !minSndMessSz)		minSndMessSz = length;
        if (maxSndMessSz < length || !maxSndMessSz)		maxSndMessSz = length;
    }
    // end VS_TransportRouter_Service::AddSndMsgStatistics

    inline void FillMonitorStruct( VS_TransportMonitor::TmReply::Service &service )
    {
        service.type = TM_TYPE_PERIODIC_SERVICE;
        strncpy( service.serviceName, serviceName, sizeof(service.serviceName) );
        service.serviceName[sizeof(service.serviceName) - 1] = 0;
        strncpy( service.serviceType, serviceType, sizeof(service.serviceType) );
        service.serviceType[sizeof(service.serviceType) - 1] = 0;
        service.rcvMess = rcvMess;
        service.minRcvMessSz = minRcvMessSz;
		service.aveRcvMessSz = (unsigned long)(rcvMess ? totalRcvMessSz/rcvMess : 0);
        service.maxRcvMessSz = maxRcvMessSz;
        service.sndMess = sndMess;
        service.minSndMessSz = minSndMessSz;
		service.aveSndMessSz = (unsigned long)(sndMess ? totalSndMessSz/sndMess : 0);
        service.maxSndMessSz = maxSndMessSz;
    }
};

struct VS_TransportRouterServeceList
{
    struct VS_TransportRouterServeceListItem
    {
        std::string mServiceName;
    };
    std::vector<VS_TransportRouterServeceListItem> m_listCont;
    std::vector<VS_TransportRouterServeceListItem>::iterator m_it;
    bool AddToList(const char * serviceName)
    {
        std::string str(serviceName);
        VS_TransportRouterServeceListItem item;
        item.mServiceName = str;
        m_listCont.push_back(item);
        return true;
    }
    bool IsInList(const char * serviceName)
    {
        if (m_listCont.empty())
            return false;
        for (m_it=m_listCont.begin();
                m_it!=m_listCont.end();
                ++m_it)
        {
            if (strcasecmp(m_it->mServiceName.c_str(),serviceName)==0)
                return true;
        }
        return false;
    }
};
struct VS_TransportRouter_AuthorizationEndpoint
{
    VS_TransportRouter_AuthorizationEndpoint():m_authoriztionState(0)
    {
        endpointID[0] = 0;
    }
    ~VS_TransportRouter_AuthorizationEndpoint()
    {}
    inline bool isAuthorizedEndpoint()
    {
        return (e_authorized==m_authoriztionState);
    }
    inline bool AuthorizeEndpoint(const char *new_uid)
    {
        if (!new_uid || !*new_uid) return false;
        strncpy(endpointID, new_uid, VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1);
        m_authoriztionState=e_authorized;
        return true;
    }
    inline bool AuthorizeServer()
    {
        m_authoriztionState=e_authorized;
        return true;
    }
	inline bool UnauthorizeServer()
	{
        m_authoriztionState=e_unauthorized;
        return true;
	}
    inline bool UnauthorizeEndpoint()
    {
        endpointID[0] = 0;
        m_authoriztionState=e_unauthorized;
        return true;
    }
    int m_authoriztionState;
    enum VS_TransportEndpointState{
        e_none = 0,
        e_authorized,
        e_unauthorized
    };
    char endpointID[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
};
//////////////////////////////////////////////////////////////////////////////////////////
struct VS_TransportRouter_Endpoint : public VS_TransportRouter_AuthorizationEndpoint
{
	VS_TransportRouter_Endpoint(VS_TransportRouter_ImplementationBase *tr,
		const unsigned long index, const char *CID, const char *SID,
		const unsigned char ep_hops,
		const unsigned long maxLackMs, const unsigned long maxMsgs,
		const unsigned long maxConnSilenceMs, const unsigned long fatalSilenceCoef)
		:  tr(tr), isValid(false), index(index), sendQueue((SID && *SID) ? MAX_MESS_FOR_SERVER : maxMsgs),
		bothConn(0),
		maxLackMs(maxLackMs), maxConnSilenceMs(maxConnSilenceMs), hops(ep_hops),
		fatalSilenceCoef(fatalSilenceCoef), bothVersion(0), flagLogs(false),
		isBothAccept(true),
		activeTm(tr->currTm), writeSize(0), readSize(sizeof(transport::MessageFixedPart)),
		rcvWriteSize(0),maxSndSilenceMs(0), maxSndSilenceCoefMs(0), maxRcvSilenceMs(0),
		maxRcvSilenceCoefMs(0), lastReadTm(0), lastWriteTm(0),writeBothBuffer(0),readBothBuffer(0),
		stateRcv(0), nReconnects(0), connectionDate(time(0)),bothCondIpFlag(false),
		rcvRecTms(0),bothLastConDt(0),bothLastDisDt(0),
		rcvMess(0), totalRcvMessSz(0), minRcvMessSz(0), maxRcvMessSz(0),
		sndRecTms(0),
		sndMess(0), totalSndMessSz(0), minSndMessSz(0), maxSndMessSz(0),
		sAcsLog(0), tAcsLog(0),
		m_IsConnForbidden(false),
		newSecureAbility(false),
		oldSecureAbility(false),
		delBoth(0),delayBoth(0),m_realReconnects(0),m_reconnectFlag(0),
		m_hBothConnectThread(0),m_IsForcedDisconnect(false),isConnectSecure(true),m_ReadCrypt(0),
		m_WriteCrypt(0),m_isSecureHandshakeInProgress(false),writeEncrBuf(0), readEncrBuf(0),rcvEncrSize(0),m_srvCert(0),
		m_secure_hadshake_ver(0), m_IsTCPPingSupported(true)
    {
        if (!this) return;
        *both = 0;
        if (!CID && !SID) return;
        if ((CID && !*CID) || (SID && !*SID)) return;
        if (SID) // if Server-Server connect - then copy SID to endpointName
        {
            strncpy( VS_TransportRouter_Endpoint::endpointID, SID, sizeof(VS_TransportRouter_Endpoint::endpointID) );
            ////m_authoriztionState=e_authorized; // d78 fast bug
        }
        else
        {
            endpointID[0] = 0; // d78 zerostr
        }
        if (CID)
        {
            strncpy( m_CID, CID, sizeof(m_CID) );
        } else
        {
            m_CID[0] = 0; // d78 zerostr
        }
        m_CID[sizeof(m_CID) - 1] = 0;
        if (tr == 0) return;
        if (tr->flagLogs && !tAcsLog)
        {
            tAcsLog = new VS_AcsLog( m_CID,  0x100000 * 4, 0x100000*3 , VS_AcsTransportExtLog, tr->dirLogsName );
            if (tAcsLog && !tAcsLog->IsValid())
            {
                delete tAcsLog;
                tAcsLog = 0;
            }
            else
            {
                flagLogs = true;
            }
        }
        isValid = true;

		//if network trace enabled
		VS_RegistryKey reg_key(false, CONFIGURATION_KEY);
		uint32_t is_enable(0);
		reg_key.GetValue(&is_enable, sizeof(is_enable), VS_REG_INTEGER_VT, TRANSPORT_FULL_TRACE_ENABLED_TAG);
		if (!is_enable)
			return;
		std::string trace_dir = "transport traces";

		boost::system::error_code ec;
		boost::filesystem::create_directory(trace_dir, ec);
		if (ec) {
			dstream0 << "VS_TransportRouter_Endpoint: Error creating '" << trace_dir << "': " << ec.message();
			throw;
		}

		std::string file_name_prefix = *VS_TransportRouter_Endpoint::endpointID ? VS_TransportRouter_Endpoint::endpointID : m_CID;
		std::string in_encr_file_name = trace_dir + "\\" + file_name_prefix + "_in_encr.tcdump";
		std::string in_decr_file_name = trace_dir + "\\" + file_name_prefix + "_in_decr.tcdump";

		std::string out_encr_file_name = trace_dir + "\\" + file_name_prefix + "_out_encr.tcdump";
		std::string out_decr_file_name = trace_dir + "\\" + file_name_prefix + "_out_decr.tcdump";

		m_in_encrypted_trace.open(in_encr_file_name,std::ios::binary | std::ios::app);
		m_in_decrypted_trace.open(in_decr_file_name, std::ios::binary | std::ios::app);

		m_out_encrypted_trace.open(out_encr_file_name, std::ios::binary | std::ios::app);
		m_out_decrypted_trace.open(out_decr_file_name, std::ios::binary | std::ios::app);

		//m_in_encrypted_trace.open()
    }
    // end VS_TransportRouter_Endpoint::VS_TransportRouter_Endpoint

    ~VS_TransportRouter_Endpoint( void )
    {
        if (tr && tr->trDebugLog) tr->trDebugLog->TPrintf("\n\t ~ID %s",*m_CID? m_CID : endpointID );
        DeleteConns();
        if (sAcsLog)	delete sAcsLog;
        if (tAcsLog)	delete tAcsLog;
		if(m_srvCert)	free(m_srvCert);
		if(m_hBothConnectThread)	CloseHandle(m_hBothConnectThread);
    }
    // end VS_TransportRouter_Endpoint::~VS_TransportRouter_Endpoint

    VS_TransportRouter_ImplementationBase* tr;
    bool   isValid;
    const uint32_t index;
    VS_TransportMessageQueue sendQueue;
    unsigned char   hops;
    char	m_CID[VS_ACS_MAX_SIZE_CID + 1];
    VS_Connection *bothConn;
    const uint32_t   maxLackMs, maxConnSilenceMs, fatalSilenceCoef;
    uint32_t   bothVersion;
    bool flagLogs, isBothAccept;
    uint32_t   activeTm, writeSize, readSize, rcvWriteSize, rcvEncrSize,
    maxSndSilenceMs, maxSndSilenceCoefMs, maxRcvSilenceMs, maxRcvSilenceCoefMs,
    lastReadTm, lastWriteTm;

	bool                newSecureAbility;//TLS

	bool				oldSecureAbility;//SecureHandshake and various crypters
	bool				isConnectSecure;
	VS_SecureHandshake	m_HandshakeManager;
	VS_SymmetricCrypt		*m_ReadCrypt,*m_WriteCrypt;
	bool				m_isSecureHandshakeInProgress;
	unsigned int		m_secure_hadshake_ver;

	char *m_srvCert;
	VS_PKey m_srvPrivateKey;
	VS_Container m_srv_cert_chain;



    transport::MessageFixedPart readHead;
    transport::Message writeMsg;
    unsigned char	*writeBothBuffer, *readBothBuffer, *writeEncrBuf, *readEncrBuf;
    static const unsigned maxReconnects = 0;//1;//20;
    unsigned stateRcv, nReconnects;

    const time_t   connectionDate;
    bool   bothCondIpFlag;
    unsigned long   rcvRecTms;
    time_t	bothLastConDt, bothLastDisDt;
    unsigned __int64   rcvMess, totalRcvMessSz;
    unsigned long   minRcvMessSz, maxRcvMessSz;
    char   both[10];
    unsigned long   sndRecTms;
    unsigned long m_realReconnects;
    unsigned long m_reconnectFlag;
    unsigned __int64   sndMess, totalSndMessSz;
    unsigned long   minSndMessSz, maxSndMessSz;
    VS_AcsLog   *sAcsLog, *tAcsLog;

    HANDLE	m_hBothConnectThread;
	bool	m_IsConnForbidden;

    static const int connectionsThreadsLimit = 1;
    unsigned long delBoth, delayBoth;

	bool			m_IsForcedDisconnect;
	bool			m_IsTCPPingSupported;

	//bool			m_isDumpEnabled;


	std::ofstream	m_in_encrypted_trace;
	std::ofstream	m_in_decrypted_trace;
	std::ofstream	m_out_encrypted_trace;
	std::ofstream	m_out_decrypted_trace;

	enum class trace_direction
	{
		in,
		out
	};

    enum EndpointState
    {
        e_onTime = 0,
        e_null,
        e_connectDone,
        e_pingRecived
    };

	static bool IsTCPKeepAliveAllowed()
	{
		VS_RegistryKey key(false, CONFIGURATION_KEY);
		unsigned long is_enable(1);
		key.GetValue(&is_enable,sizeof(is_enable),VS_REG_INTEGER_VT, "TCP KeepAlive");
		return 0 != is_enable;
	}

	inline bool CopySrvCert()
	{
		if(!tr || m_isSecureHandshakeInProgress || !tr->m_Cert)
			return false;
		if(m_srvCert)
		{
			free(m_srvCert);
			m_srvCert = 0;
		}
		char *buf(0);
		uint32_t sz(0);
		tr->m_pKey.GetPrivateKey(store_PEM_BUF,buf,&sz);
		if(!sz)
			return false;
		buf = new char [sz];
		if(!tr->m_pKey.GetPrivateKey(store_PEM_BUF,buf,&sz))
		{
			delete [] buf;
			return false;
		}
		if(!m_srvPrivateKey.SetPrivateKey(buf,store_PEM_BUF))//SERVER_CERT_PRIVATE_KEY_PASS
		{
			delete [] buf;
			return false;
		}
		delete [] buf;
		m_srvCert = strdup(tr->m_Cert.get());
		if(tr->cert_chain.IsValid())
			tr->cert_chain.CopyTo(m_srv_cert_chain);
		return !!m_srvCert;
	}
	inline bool IsConnectProcessing()
	{
		if(m_hBothConnectThread)
		{
			if(WAIT_TIMEOUT==WaitForSingleObject(m_hBothConnectThread,0))
				return true;
			else
				return false;
		}
		else
			return false;
	}
    inline void WaitBothConnThread()
    {
        if (m_hBothConnectThread)
        {
            WaitForSingleObject(m_hBothConnectThread,INFINITE);
            CloseHandle(m_hBothConnectThread);
            m_hBothConnectThread = 0;
        }
    }
    // end VS_TransportRouter_Endpoint::WaitBothConnThread

    inline void WaitConnectingThread()
    {
        WaitBothConnThread();
    }
    // end VS_TransportRouter_Endpoint::WaitConnectingThread

    inline bool GetPermissionOnConnectAct( /*const unsigned type */)
    {
        if ((bothConn && bothConn->IsValid())||m_IsConnForbidden) return false;
        return true;
    }
    // end VS_TransportRouter_Endpoint::GetPermissionOnConnectAct

    inline void DeleteConns( void )
    {
        DeleteBothConn();
    }
    // end VS_TransportRouter_Endpoint::DeleteConns

    inline void DeleteBothConn( void )
    {
        if (bothConn)
        {
#ifdef _MY_DEBUG_
            printf("VS_TransportRouter_Endpoint::DeleteBothConn(): bothVersion: %u, bothConn: %p\n", bothVersion, bothConn);
#endif
            if (!bothConn->IsRW())
            {
                tr->DeleteConn( bothConn , "DeleteBothConn" , *m_CID ? m_CID : endpointID );
                ++delBoth;
            }
            else
            {
                bothConn->SetOvReadFields( VS_TR_CONNECTION_DEAD_READ, (VS_ACS_Field)bothConn );
                bothConn->SetOvWriteFields( VS_TR_CONNECTION_DEAD_WRITE, (VS_ACS_Field)bothConn );
                bothConn->Disconnect();
                tr->AddRouteDeleteConn( bothConn );
                ++delayBoth;

            }
            tr->ShowConn( *m_CID ? m_CID : endpointID, VS_ACS_LIB_BOTH , bothConn, "DeleteBothConn" );
            bothConn = 0;
            *both = 0;
            bothLastDisDt = time(0);
        } else
        {
            tr->ShowConn(*m_CID ? m_CID : endpointID , VS_ACS_LIB_BOTH , 0 , "DeleteBothConn !0" );
        }
        if (readBothBuffer)
        {
            free((void *)readBothBuffer);
            readBothBuffer = 0;
        }
		if(readEncrBuf)
		{
			free(readEncrBuf);
			readEncrBuf = 0;
		}
		isConnectSecure = false;
        readSize = sizeof(transport::MessageFixedPart);
        stateRcv = 0;
        bothVersion = 0;
        bothCondIpFlag = false;
        if (writeBothBuffer)
        {
            free( (void*)writeBothBuffer);
            writeBothBuffer = 0;
        }
        writeSize = 0;
        flagLogs = false;
		m_isSecureHandshakeInProgress = false;
		if(m_ReadCrypt)
			VS_SecureHandshake::ReleaseSymmetricCrypt(&m_ReadCrypt);
		if(m_WriteCrypt)
			VS_SecureHandshake::ReleaseSymmetricCrypt(&m_WriteCrypt);
		if(writeEncrBuf)
			free(writeEncrBuf);
		if(readEncrBuf)
			free(readEncrBuf);

		writeEncrBuf = readEncrBuf = 0;
		m_ReadCrypt = m_WriteCrypt = 0;
    }
    // end VS_TransportRouter_Endpoint::DeleteBothConn

    inline bool DeleteConnectBothConn()
    {
        DeleteBothConn();
        return (!isBothAccept && !m_IsForcedDisconnect) ? ConnectBothConn() : true;
    }
    // end VS_TransportRouter_Endpoint::DeleteConnectBothConn

    inline void DisconnectMe( void )
    {
        if (!bothConn) return;
		VS_RouterMessage* mess = new VS_RouterMessage;
		if (mess) {
			const char opcode[] = { VS_TRANSPORT_MANAGING_DISCONNECT, '\0' };
			if (hops) {  ///server
				*mess = transport::Message(true, ~0, VS_TRANSPORT_TIMELIFE_DISCONNECT, tr->endpointName, {}, opcode, {},    {}, {}, {}, {}, endpointID, "", 1);
			} else if ( isAuthorizedEndpoint() ) {
				*mess = transport::Message(true, ~0, VS_TRANSPORT_TIMELIFE_DISCONNECT, tr->endpointName, {}, opcode, {},    {}, {}, endpointID, tr->endpointName, {}, "", 1);
			} else {
				*mess = transport::Message(true, ~0, VS_TRANSPORT_TIMELIFE_DISCONNECT, tr->endpointName, {}, opcode, m_CID, {}, {}, {}, tr->endpointName, {}, "", 1);
			}
			ProcessingMessages( mess );
			dprint3("Disconnect message was sent to peer: %s\n",*m_CID ? m_CID : endpointID);
		}
    }
    // end VS_TransportRouter_Endpoint::DisconnectMe

    inline void MakeAPing()
    {
		if (tr && tr->trDebugLog)
			tr->trDebugLog->TPrintf("\n MakeAPing, m_CID = %s, m_IsTCPPingSupported =%d\n", hops ? endpointID : m_CID, m_IsTCPPingSupported);

        if (!bothConn) return;
		VS_RouterMessage* mess = new VS_RouterMessage;
		if (mess)
		{
			const char opcode[] = { VS_TRANSPORT_MANAGING_PING, '\0' };
			if (hops) {  ///server
				*mess = transport::Message(true, ~0, VS_TRANSPORT_TIMELIFE_PING, {}, {}, opcode, {},    {}, {}, {}, {}, endpointID, "", 1);
			} else if ( isAuthorizedEndpoint() ) {
				*mess = transport::Message(true, ~0, VS_TRANSPORT_TIMELIFE_PING, {}, {}, opcode, {},    {}, {}, endpointID, {}, {}, "", 1);
			} else {
				*mess = transport::Message(true, ~0, VS_TRANSPORT_TIMELIFE_PING, {}, {}, opcode, m_CID, {}, {}, {}, {}, {}, "", 1);
			}
			ProcessingMessages(mess);
		}
	}
    // end VS_TransportRouter_Endpoint::MakeAPing

    inline void ProcessingTick( void )
    {
        if (bothConn && !m_IsTCPPingSupported)
        {
            const unsigned long diffWriteTm(tr->currTm - lastWriteTm);
            if (diffWriteTm > maxSndSilenceCoefMs)
            {
                if (tr && tr->trDebugLog)
                    tr->trDebugLog->TPrintf("\n\t Endpoint::ProcessingTick::DeleteBoth by timeout %s",hops ? endpointID : m_CID);
                DeleteConnectBothConn();
            }
            else if (sendQueue.IsEmpty() && !bothConn->IsWrite() && diffWriteTm > maxSndSilenceMs)
            {
                MakeAPing();
            }
            const unsigned long diffReadTm(tr->currTm - lastReadTm);
            if (diffReadTm > maxRcvSilenceCoefMs )
            {
                if (tr && tr->trDebugLog)
                    tr->trDebugLog->TPrintf("\n\t Endpoint::ProcessingTick::DeleteConnectBothConn by timeout %s",hops ? endpointID : m_CID);
                DeleteConnectBothConn();
            }
        } else if (!bothConn)
        {
		    if (tr && tr->trDebugLog)
                tr->trDebugLog->TPrintf("\n\t Endpoint::ProcessingTick::DeleteConnectBothConn by !both %s",hops ? endpointID : m_CID);
            DeleteConnectBothConn();
		}
    }
    // end VS_TransportRouter_Endpoint::ProcessingTick

    inline void WriteToBoth()
    {

		if (!bothConn || bothConn->IsWrite() || sendQueue.IsEmpty() || m_isSecureHandshakeInProgress || (
			oldSecureAbility && !isConnectSecure))
        {
            return;
        }
        const auto msg = sendQueue.Front().Data();
        const uint32_t sz_body = sendQueue.Front().BodySize();
        uint32_t sz = sendQueue.Front().Size();

		//trace decrypted
		TRACE_DECRYPTED(msg, sz, trace_direction::out);

		if(isConnectSecure)
		{
			if(m_WriteCrypt)
			{
				uint32_t write_sz(0);
				m_WriteCrypt->Encrypt(msg,sz,0,&write_sz);
                ::free(writeEncrBuf);
				writeEncrBuf = (unsigned char *)malloc(write_sz);
				if(!m_WriteCrypt->Encrypt(msg, sz,writeEncrBuf,&write_sz) || !bothConn->Write(writeEncrBuf,write_sz))
				{
					++tr->observer.crt[13];
					if (tr && tr->trDebugLog)
						tr->trDebugLog->TPrintf("\n\t Endpoint::Write::DeleteConnectSndConn by !write or !Encrypt %s",hops ? endpointID : m_CID);
					DeleteConnectBothConn();
					return;
				}
				else
					sz = write_sz;

				TRACE_ENCRYPTED(writeEncrBuf, write_sz, trace_direction::out);
			}
		}
		else
		{
			if (!bothConn->Write( (const void *)msg, sz ))
			{
				++tr->observer.crt[13];
				if (tr && tr->trDebugLog)
					tr->trDebugLog->TPrintf("\n\t Endpoint::Write::DeleteConnectSndConn by !write %s",hops ? endpointID : m_CID);
				DeleteConnectBothConn();
				return;
			}

		}
        AddSndMsgStatistics( sz_body );

        writeMsg = sendQueue.Pop(MessageDeleteReason::send);

        writeSize = sz; //реальный sz
    }
    // end VS_TransportRouter_Endpoint::WriteToBoth

    inline void ProcessingMessages( VS_RouterMessage *mess )
    {
		if(!isValid)
		{
			tr->CancelEndpointDisconnect(hops ? endpointID : m_CID); //Отменяем удаление Endpoint'а, так как кто-то отправил ему сообщение после того, как его поместили в очередь на удаление
			isValid = true; //EP опять валиден
		}
		sendQueue.Push(std::move(*mess));
        WriteToBoth();
        delete mess;
    }
    // end VS_TransportRouter_Endpoint::ProcessingMessages

    inline void ProcessingManaging( VS_RouterMessage *mess )
    {
#ifdef _MY_DEBUG_
        printf("VS_TransportRouter_Participant::ProcessingManaging( %p )\n", mess);
#endif
		const char *type = mess->AddString();
		if (type && *type) {
			switch (type[0]) {
			case VS_TRANSPORT_MANAGING_PING :
				break;

			case VS_TRANSPORT_MANAGING_CONNECT :
				{
					dprint4("Connect endpoint! : %s\n", hops ? endpointID : m_CID);
				}
				break;
			case VS_TRANSPORT_MANAGING_DISCONNECT :
				{
					dprint4("Disconnect endpoint! : %s\n", hops ? endpointID : m_CID);
					tr->ForcedDisconnectByPeer( hops ? endpointID : m_CID);
				}
				break;
			default :
				{
					dprint1("Invalid message! : %s\n", hops ? endpointID : m_CID);
				}
			}
		}
        delete mess;
    }
    // end VS_TransportRouter_Endpoint::ProcessingManaging

    inline void BothWrite( unsigned long bTrans, VS_Overlapped &ov )
    {
        if (!bothConn)
        {
            DeleteConnectBothConn();
            return;
        }
        int ret = bothConn->SetWriteResult(bTrans,&ov);
        if (ret == -2)
            return;
        if (ret != (int)writeSize)
        {
			sendQueue.PushFront(std::move(writeMsg));

			if(writeEncrBuf)
			{
				free(writeEncrBuf);
				writeEncrBuf = 0;
			}

            if (tr && tr->trDebugLog)
            {
                tr->trDebugLog->TPrintf("\n\t Bad result of BothWrite : %d error: %d",
                                       ret, GetLastError());
                tr->ShowConn( hops ? endpointID : m_CID ,  0 , bothConn );
            }
            ++tr->observer.crt[11];
            DeleteConnectBothConn();
            return;
        }
        tr->sendBytes += writeSize;
        if (writeBothBuffer)
        {
            free( (void *)writeBothBuffer);
            writeBothBuffer = 0;
        }
		if(writeEncrBuf)
			free(writeEncrBuf);
		writeEncrBuf  = 0;
        writeSize = 0;
        if (oldSecureAbility)
        {
			if ((isBothAccept) && (!isConnectSecure)&&(!m_isSecureHandshakeInProgress))
            {

				if(!m_HandshakeManager.Init(m_secure_hadshake_ver,handshake_type_Server) || !m_HandshakeManager.SetPrivateKey(&m_srvPrivateKey) || !bothConn)
				{
                    ++tr->observer.crt[2];
                    if (tr && tr->trDebugLog)
                        tr->trDebugLog->TPrintf("\n\t ServerSecureHandshake Endpoint::SetConnection::DeleteConnectBothConn. HandshakeManager initialization failed; %s",hops ? endpointID : m_CID);
                    DeleteConnectBothConn();
                    return;
				}
				bothConn->SetOvReadFields(VS_TR_BTH_SECURE_HANDSHAKE_READ,(VS_ACS_Field)index);
				bothConn->SetOvWriteFields(VS_TR_BTH_SECURE_HANDSHAKE_WRITE,(VS_ACS_Field)index);
				if(!SecureHandshake())
				{
					m_isSecureHandshakeInProgress = false;
                    ++tr->observer.crt[2];
                    if (tr && tr->trDebugLog)
                        tr->trDebugLog->TPrintf("\n\t ServerSecureHandshake Endpoint::SetConnection::DeleteConnectBothConn. ServerSecureHandshake return false; %s",hops ? endpointID : m_CID);
                    DeleteConnectBothConn();
                    return;
				}
                return;
            }
        }
        /**
        	Если IsAccept и первый раз отправили данные, то есть отправили Header
        */
        if ((ret == rcvWriteSize) && isBothAccept && bothConn && !bothConn->IsRead())
        {
            rcvWriteSize = 0;
			if(isConnectSecure)
			{
				if(m_ReadCrypt)
				{
					readSize = 0;
					m_ReadCrypt->Encrypt((const unsigned char*)&readHead, sizeof(transport::MessageFixedPart), 0, &readSize);
					if (readSize != sizeof(transport::MessageFixedPart)) // cipher in block mode
						readSize--;
                    ::free(readEncrBuf);
					readEncrBuf = (unsigned char *)malloc(readSize);
					if(!bothConn->Read(readEncrBuf,readSize))
					{
						++tr->observer.crt[12];
						DWORD err = GetLastError();
						if (tr && tr->trDebugLog)
							tr->trDebugLog->TPrintf("\n\t Endpoint::BothWrite first read encrypted failed err = %d %s",err,hops ? endpointID : m_CID);
						DeleteConnectBothConn();
						return;
					}
				}
			}
			else if (!bothConn->Read((void*)&readHead, sizeof(transport::MessageFixedPart)))
			{
				++tr->observer.crt[12];
				DWORD err = GetLastError();
				if (tr && tr->trDebugLog)
					tr->trDebugLog->TPrintf("\n\t Endpoint::BothWrite first read failed err = %d %s",err,hops ? endpointID : m_CID);
				DeleteConnectBothConn();
				return;
			}
        }
        WriteToBoth();
        lastWriteTm = tr->currTm;
    }

	inline bool SecureHandshake()
	{
		m_isSecureHandshakeInProgress = true;
		void *buf(0);
		uint32_t sz(0);
		if(!tr)
			return false;
		switch(m_HandshakeManager.Next())
		{
		case secure_st_SendCert:
			{
				if(!m_srvCert)
					return false;
				VS_Container cnt;
				cnt.AddValue(SRV_CERT_KEY,m_srvCert,(unsigned long)strlen(m_srvCert) + 1);
				m_srv_cert_chain.Reset();
				while(m_srv_cert_chain.Next())
				{
					if(!!m_srv_cert_chain.GetName() && (0 == strcasecmp(m_srv_cert_chain.GetName(),CERTIFICATE_CHAIN_PARAM)))
					{
						size_t sz(0);
						const char *cert_in_chain = (const char*)m_srv_cert_chain.GetBinValueRef(sz);
						if(sz && cert_in_chain)
							cnt.AddValue(CERTIFICATE_CHAIN_PARAM,cert_in_chain,sz);
					}
				}
				size_t sz_tmp;
				if(!cnt.SerializeAlloc(buf,sz_tmp))
					return false;
				sz = sz_tmp;
				if(writeBothBuffer)
				{
					free(writeBothBuffer);
					writeBothBuffer = 0;
				}
				unsigned long write_sz = sz + sizeof(sz);
				writeBothBuffer = (unsigned char*)malloc((size_t)write_sz);
				memcpy(writeBothBuffer,&sz,sizeof(sz));
				memcpy(writeBothBuffer+sizeof(sz),buf,sz);
				free(buf);
				buf = 0;
				sz = 0;
				if(!bothConn->Write(writeBothBuffer,write_sz))
				{
					free(writeBothBuffer);
					writeBothBuffer = 0;
					return false;
				}
				else
				{
					writeSize = write_sz;
					return true;
				}
			}
			break;
		case secure_st_SendPacket:
			if(!m_HandshakeManager.PreparePacket(&buf,&sz))
				return false;
			else
			{
				if(writeBothBuffer)
					free(writeBothBuffer);
				writeBothBuffer = (unsigned char*)malloc(sz);
				memcpy(writeBothBuffer,buf,sz);
				m_HandshakeManager.FreePacket(&buf);
				buf = 0;
				if(!bothConn->Write(writeBothBuffer,sz))
				{
					free(writeBothBuffer);
					writeBothBuffer = 0;
					return false;
				}
				else
				{
					writeSize = sz;
					return true;
				}
			}
			break;
		case secure_st_GetPacket:
			if(!m_HandshakeManager.PreparePacket(&buf,&sz))
				return false;
			else
			{
				if(buf)
					m_HandshakeManager.FreePacket(&buf);
				if(readBothBuffer)
				{
					free(readBothBuffer);
					readBothBuffer = 0;
				}
				readBothBuffer = (unsigned char*)malloc(sz);
				readSize = sz;
				if(!bothConn->Read(readBothBuffer,sz))
				{
					free(readBothBuffer);
					readBothBuffer = 0;
					return false;
				}
				else
					return true;
			}
			break;
		case secure_st_Error:
			if(writeBothBuffer)
				free(writeBothBuffer);
			writeBothBuffer = 0;
			if(readBothBuffer)
				free(readBothBuffer);
			readBothBuffer = 0;
			return false;
			break;
		case secure_st_Finish:
			{
				/**
					проверить имя
				*/
				if(m_HandshakeManager.GetVersion()>1)
				{
					char *cert_buf(0);
					uint32_t cert_sz(0);
					m_HandshakeManager.GetCertificate(cert_buf,cert_sz);
					if(cert_sz)
					{
						bool bRes(false);
						cert_buf = new char[cert_sz];
						do
						{
							if(!m_HandshakeManager.GetCertificate(cert_buf,cert_sz))
								break;
							VS_Certificate cert;
							if(!cert.SetCert(cert_buf,cert_sz,store_PEM_BUF))
								break;
							std::string srv_name;
							if(!cert.GetSubjectEntry("commonName",srv_name))
								break;
							if(!strcasecmp(endpointID,srv_name.c_str()))
								bRes = true;

						}while(false);
						delete [] cert_buf;
						cert_buf = 0;
						cert_sz= 0;
						if(!bRes)
							return false;
					}
				}
			}

			m_WriteCrypt = m_HandshakeManager.GetWriteSymmetricCrypt();
			m_ReadCrypt = m_HandshakeManager.GetReadSymmetricCrypt();
			if(!m_WriteCrypt || !m_ReadCrypt)
			{
				if(m_ReadCrypt)
					VS_SecureHandshake::ReleaseSymmetricCrypt(&m_ReadCrypt);
				if(m_WriteCrypt)
					VS_SecureHandshake::ReleaseSymmetricCrypt(&m_WriteCrypt);
				m_ReadCrypt = m_WriteCrypt = 0;
				return false;
			}
			// trace symmetric keys |key_sz|key
			std::vector<char> key_buf;
			uint32_t  out_sz(0);
			std::string token = "NEW_SYMMETRIC_KEYS";
			if (m_in_encrypted_trace.is_open())
			{

				key_buf.resize(m_ReadCrypt->GetKeyLen());
				if (m_ReadCrypt->GetKey(key_buf.size(), reinterpret_cast<unsigned char*>(&key_buf[0]), &out_sz) && key_buf.size() == out_sz)
				{
					m_in_encrypted_trace.write(token.c_str(), token.length());
					m_in_encrypted_trace.write(reinterpret_cast<char*>(&out_sz), 4);
					m_in_encrypted_trace.write(&key_buf[0], key_buf.size());
				}
				else
					m_in_encrypted_trace.close();
			}
			if (m_out_encrypted_trace.is_open())
			{
				key_buf.resize(m_WriteCrypt->GetKeyLen());
				if (m_WriteCrypt->GetKey(key_buf.size(), reinterpret_cast<unsigned char*>(&key_buf[0]), &out_sz) && key_buf.size() == out_sz)
				{
					m_out_encrypted_trace.write(token.c_str(), token.length());
					m_out_encrypted_trace.write(reinterpret_cast<char*>(&out_sz), 4);
					m_out_encrypted_trace.write(&key_buf[0], key_buf.size());
				}
				else
					m_out_encrypted_trace.close();
			}



			bothConn->SetOvWriteFields( VS_TR_BTH_CONNECTION_WRITE, (VS_ACS_Field)index );
			bothConn->SetOvReadFields( VS_TR_BTH_CONNECTION_READ, (VS_ACS_Field)index );
			if(writeBothBuffer)
				free(writeBothBuffer);
			writeBothBuffer = 0;
			if(readBothBuffer)
				free(readBothBuffer);
			readBothBuffer = 0;
			BothSecureHandshaked();
			return true;
		};
		return false;
	}
	inline void SecureHandshakeRead(unsigned long bTrans, VS_Overlapped &ov)
	{
        if (!bothConn)
        {
            if (tr && tr->trDebugLog)
                tr->trDebugLog->TPrintf("\n\t Endpoint::SecureHandshakeRead::DeleteConnectBothConn !bothConn %s", hops ? endpointID : m_CID);

            DeleteConnectBothConn();
            return;
        }
        int res = bothConn->SetReadResult(bTrans, &ov );
        if (res == -2)
            return;
		if(res!=(int)readSize || !m_HandshakeManager.ProcessPacket(readBothBuffer,res)|| !SecureHandshake())
		{
			m_isSecureHandshakeInProgress = false;
            ++tr->observer.crt[12];
            if (tr && tr->trDebugLog)
                tr->trDebugLog->TPrintf("\n\t Endpoint::BothRead::DeleteConnectBothConn %d %s",res,hops ? endpointID : m_CID);

            DeleteConnectBothConn();
            return;
		}
	}

	inline void SecureHandshakeWrite(unsigned long bTrans, VS_Overlapped &ov)
	{
        if (!bothConn)
        {
            DeleteConnectBothConn();
            return;
        }
        int ret = bothConn->SetWriteResult(bTrans,&ov);
        if (ret == -2)
            return;
		if (ret != (int)writeSize || !SecureHandshake())
		{
			m_isSecureHandshakeInProgress = false;
			if(writeBothBuffer)
			{
				free(writeBothBuffer);
				writeBothBuffer = 0;
			}
            if (tr && tr->trDebugLog)
            {
                tr->trDebugLog->TPrintf("\n\t Bad result of SecureHandshakeWrite : %d error: %d",
                                       ret, GetLastError());
                tr->ShowConn( hops ? endpointID : m_CID ,  0 , bothConn );
            }
            ++tr->observer.crt[11];
            DeleteConnectBothConn();
		}
	}

    inline bool ReadyToDestroy()
    {
        /**
			В каких случаях эта функция нужна. Когда происходит удаление ендпоинтов.
			Опасность состоит том, что в этот момент ендпоинт может находиться
			в состоянии коннекта и его нельзя удалять (иначе будет АВ).
			-------------------------------------------------------------------
			Когда функция возвращает false:
				(numberOfRecverConnections!=0) || (numberOfSenderConnections!=0)
				Существуют нити, осуществляющие коннект.
			-------------------------------------------------------------------
			Когда функция возвращает true:
				!sndConn
				Сендер отсутствует и никто не конектит его.

				сообщений на посылку нет и икто их не пишет
				( (!this->nMsgs) && (!sndConn->IsWrite()) )
			Если функция возвращает true, то флаг валидности ендпоинта
			выставляется в false. Ожидается, что ендпоинт будет немедленно
			удален!
			-------------------------------------------------------------------
        **/
		bool res = !IsConnectProcessing();
        if (!res) return false;
        res = (!bothConn || (sendQueue.IsEmpty() && !bothConn->IsWrite()));
        if (!res) return false;
        isValid = false;
        return true;
    }
    // end VS_TransportRouter_Endpoint::ReadyToDestroy

	inline bool IsValidEndpointId(const char *Id, size_t check_length)
	{
		// this code designed to check server name with memory bounder
		if (!Id || (strnlen(Id, check_length + 1) > check_length)) return false;
		if (!*Id) return true; // empty endpoint is valid

		const char regex_c[] = {'.','#'};
		bool regex_b[] = {false, false, false};
		unsigned long regex_cnt = 0;

		for (unsigned long i = 0; i < check_length; ++i) {
			if (regex_cnt < 2 && Id[i] == regex_c[regex_cnt]) {
				regex_b[regex_cnt] = true;
				++regex_cnt;
			} else {
				if ((Id[i] < 35) || (Id[i] > 122)) return false;
			}
		}
		return (regex_b[0] && regex_b[1]);
	}
    // end VS_TransportRouter_Endpoint::IsValidEndpointId
private:
	inline void TRACE_ENCRYPTED(const unsigned char *buf, const uint32_t sz, const trace_direction dir)
	{
		std::ofstream  &f = dir == trace_direction::in ? m_in_encrypted_trace : m_out_encrypted_trace;
		if (f.is_open())
			f.write(reinterpret_cast<const char*>(buf), sz);
	}
	inline void TRACE_DECRYPTED(const unsigned char *buf, const uint32_t sz, const trace_direction dir)
	{
		std::ofstream  &f = dir == trace_direction::in ? m_in_decrypted_trace : m_out_decrypted_trace;
		if (f.is_open())
			f.write(reinterpret_cast<const char*>(buf), sz);
	}
public:

    inline bool ReadBoth( VS_RouterMessage *&readMess )
    {
		unsigned long encr_sz(0);
		unsigned long decr_sz(0);
        switch (stateRcv)
        {
        case 0 :
			if(isConnectSecure)
			{
				if(!m_ReadCrypt)
					return false;

				decr_sz = sizeof(transport::MessageFixedPart);
				encr_sz = readSize;
				uint32_t headsize = readSize + m_ReadCrypt->GetBlockSize();
				unsigned char *decr_buf = (unsigned char*)malloc(headsize);

				//trace encrypted
				TRACE_ENCRYPTED(readEncrBuf, readSize,trace_direction::in);

				if (!m_ReadCrypt->Decrypt(readEncrBuf, readSize, decr_buf, &headsize) || headsize != sizeof(transport::MessageFixedPart))
				{
					free(decr_buf);
					return false;
				}
				memcpy(&readHead,decr_buf,headsize);
				free(decr_buf);
				free(readEncrBuf);
				readEncrBuf = 0;
			}
			//trace decrypted
			TRACE_DECRYPTED((unsigned char*)&readHead, sizeof(transport::MessageFixedPart), trace_direction::in);

            if (!readHead.mark1 || readHead.version < 1
                    || readHead.head_length < sizeof(transport::MessageFixedPart) + 6)
            {
                /* Here it will be necessary to throw off in TRACE */
                return false;
            }
            readSize = readHead.head_length + readHead.body_length + 1;
            ::free(readBothBuffer);
            readBothBuffer = (unsigned char *)malloc( (size_t)readSize );
            if (!readBothBuffer)
            {
                return false;
            }
            *(transport::MessageFixedPart*)readBothBuffer = readHead;
            readSize -= sizeof(transport::MessageFixedPart);
            stateRcv = 1;
			if(isConnectSecure)
			{
				if(!m_ReadCrypt)
					return false;
				uint32_t sz(0);
				m_ReadCrypt->Encrypt(readBothBuffer,readSize+decr_sz,0,&sz);
				readSize = sz - encr_sz;
				::free(readEncrBuf);
                readEncrBuf = (unsigned char*)malloc(readSize);
				if(!bothConn->Read((void*)readEncrBuf,readSize))
					return false;
				return true;
			}
            else if (!bothConn->Read((void *)&readBothBuffer[sizeof(transport::MessageFixedPart)], readSize))
                return false;
            return true;
        case 1 :
        {
			if(isConnectSecure)
			{
				if(!m_ReadCrypt)
				{
					return false;
				}

				TRACE_ENCRYPTED(readEncrBuf, readSize, trace_direction::in);

				uint32_t mess_len = readSize+m_ReadCrypt->GetBlockSize();
				unsigned char *decr_buf = (unsigned char*)malloc(mess_len);
				if(!m_ReadCrypt->Decrypt(readEncrBuf, readSize, decr_buf, &mess_len) || (mess_len != readHead.head_length + readHead.body_length + 1 - sizeof(transport::MessageFixedPart)))
				{
					free(decr_buf);
					return false;
				}
				memcpy(&readBothBuffer[sizeof(transport::MessageFixedPart)], decr_buf, mess_len);
				free(readEncrBuf);
				readEncrBuf = 0;
				free(decr_buf);

				readSize = mess_len;

			}
			TRACE_DECRYPTED(&readBothBuffer[sizeof(transport::MessageFixedPart)], readSize, trace_direction::in);

            VS_RouterMessage* mess = new VS_RouterMessage(readBothBuffer, sizeof(transport::MessageFixedPart) + readSize);
            if (mess->IsValid())
            {
                AddRcvMsgStatistics(mess->BodySize());
				readMess = mess;

				if (!IsValidEndpointId(mess->DstServer(), reinterpret_cast<const transport::MessageFixedPart*>(mess->Data())->head_length - (reinterpret_cast<const uint8_t*>(mess->DstServer()) - mess->Data()) - 1)) {
					const time_t currTime(time(0)); char buff[MAX_PATH+1] = {};
					ctime_s(buff, MAX_PATH, &currTime);
					dprint1("%s: Endpoint::ReadBoth Broken message %s source IP: %s:%s\n", buff, this->isAuthorizedEndpoint() ? this->endpointID : this->m_CID, bothConn ? bothConn->GetPeerIp() : "null()", bothConn ? bothConn->GetPeerPort() : "null()");
					//tr->tAcsLog->PrintHex(mess->Data(), reinterpret_cast<transport::MessageFixedPart*>(mess->Data())->head_length + reinterpret_cast<transport::MessageFixedPart*>(mess->Data())->body_length);
                    ::free(readBothBuffer);
					readBothBuffer = nullptr;
					readSize = sizeof(transport::MessageFixedPart);
					stateRcv = 0;
					*mess = transport::Message();
					return false;
				}

                /**	Выполнить предзаполнение, задать User_id и server_id **/
                if (!this->isAuthorizedEndpoint()) {
                    readMess->SetSrcCID(this->m_CID);
					if(!hops)//for clients only
						readMess->SetSrcServer("");
					readMess->SetSrcUser("");
					readMess->SetDstCID("");
					if(!hops)//for clients only
						readMess->SetDstServer("");
					readMess->SetDstUser("");
                }
				else {
                    readMess->SetSrcCID("");
                    readMess->SetDstCID("");

					if (!this->hops) { // Message from user
						readMess->SetSrcUser(this->endpointID);

						const char *dsts = readMess->DstServer();
						bool bdsts = dsts && *dsts;
						const char *dstu = readMess->DstUser();
						bool bdstu = dstu && *dstu;

						if (bdsts && bdstu) {
							if (strcmp(dsts, tr->endpointName.c_str())) { // !IsOurEndpointName
								readMess->SetSrcServer(tr->endpointName);
							}
						}
						else {
							readMess->SetSrcServer(tr->endpointName);
							if (!bdstu && !bdsts) {
								readMess->SetDstServer(tr->endpointName);
							}
						}
					}
					else {
	                    const char *srcs = readMess->SrcServer();
						if (!srcs || !*srcs) { // support new transport managed messages from v9
							readMess->SetSrcServer(this->endpointID);
						}
					}
				}
			}
			else
			{	/* Here it will be necessary to throw off in TRACE */
				delete mess;
				return false;
			}
		}
        ::free(readBothBuffer);
        readBothBuffer = nullptr;
        readSize = sizeof(transport::MessageFixedPart);
        stateRcv = 0;
		if(isConnectSecure)
		{
			if(m_ReadCrypt)
			{
				readSize = 0;
				m_ReadCrypt->Encrypt((const unsigned char*)&readHead, sizeof(transport::MessageFixedPart), 0, &readSize);
				if (readSize != sizeof(transport::MessageFixedPart)) // cipher in block mode
					readSize--;
                ::free(readEncrBuf);
				readEncrBuf = (unsigned char*)malloc(readSize);
				if(!bothConn->Read(readEncrBuf,readSize))
					return false;
			}
		}
        else if (!bothConn->Read((void *)&readHead, sizeof(transport::MessageFixedPart)))
            return false;
        return true;
        default :
            /* Here it will be necessary to throw off in TRACE */
            return false;
        }
    }
    // end VS_TransportRouter_Endpoint::ReadBoth

    inline void BothRead( unsigned long bTrans, VS_Overlapped &ov, VS_RouterMessage *&readMess )
    {
        if (!bothConn)
        {
            if (tr && tr->trDebugLog)
                tr->trDebugLog->TPrintf("\n\t Endpoint::BothRead::DeleteConnectBothConn !bothConn %s", hops ? endpointID : m_CID);

            DeleteConnectBothConn();
            return;
        }
        int res = bothConn->SetReadResult(bTrans, &ov );
#ifdef _MY_DEBUG_
        printf( "VS_TransportRouter_Endpoint::BothRead: SetReadResult return %i.\n", res );
#endif
        if (res == -2)
            return;
        if (res != (int)readSize || !ReadBoth( readMess ))
        {
            ++tr->observer.crt[12];
            if (tr && tr->trDebugLog)
                tr->trDebugLog->TPrintf("\n\t Endpoint::BothRead::DeleteConnectBothConn %d %s",res,hops ? endpointID : m_CID);

            DeleteConnectBothConn();
            return;
        }
        tr->receiveBytes += readSize;
        lastReadTm = tr->currTm;
    }
    // end VS_TransportRouter_Endpoint::BothRead

    inline bool ConnectBothConnAct()
    {
#ifdef _MY_DEBUG_
        printf("VS_TransportRouter_Participant::ConnectBothConn: bothConn: %p\n", bothConn);
#endif
        if (bothConn || IsConnectProcessing())	return true;
		if(m_hBothConnectThread)
		{
			CloseHandle(m_hBothConnectThread);
			m_hBothConnectThread = 0;
		}
        if (!isValid || !tr->ConnectConnection(endpointID, maxLackMs - 5000))
        {
            return false;
        }
        ++nReconnects;
        return true;
    }
    // end VS_TransportRouter_Endpoint::ConnectBothConnAct

    inline bool ConnectBothConn()
    {
        if (bothConn)	return true;
        return tr->IsThereEndpointRegistry(endpointID) && ConnectBothConnAct();
    }
    // end VS_TransportRouter_Endpoint::ConnectBothConn

    inline bool ConnectConnectionsAct( void )
    {
        return ConnectBothConnAct();
    }
    // end VS_TransportRouter_Endpoint::ConnectConnectionsAct

    inline bool ConnectConnections( void )
    {
        return tr->IsThereEndpointRegistry( endpointID ) && ConnectConnectionsAct();
    }
    // end VS_TransportRouter_Endpoint::ConnectConnections

    inline bool SetConnection(  const unsigned long version, VS_Connection *conn,
                                const bool isAccept, const unsigned short maxConnSilenceMs,
                                const unsigned char fatalSilenceCoef, const unsigned long secure_handshake_ver,
								const bool remote_tcpKeepAliveSupport)
    {
#ifdef _MY_DEBUG_
//        printf( "VS_TransportRouter_Participant::SetConnection( CID: %s, type: %s, conn: %08X )\n", m_CID, _MD_CTYPE_(type), _MD_POINT_(conn) );
#endif

        if(m_hBothConnectThread&&!IsConnectProcessing())
		{
			CloseHandle(m_hBothConnectThread);
			m_hBothConnectThread = 0;
		}

        unsigned long a(GetTickCount()),b(0),c(0),d(0),e(0),f(0),g(0),e1(0);
        if (m_reconnectFlag == 3)
            ++m_realReconnects;

        if (!conn)
        {
            if (!isAccept)
            {
                DeleteBothConn();
                if (nReconnects > maxReconnects )
                    return false;

				if (sendQueue.IsEmpty()) // ktrushnikov add: if no messages - delete endpoint
					return false;

				return ConnectBothConn();
            }
            return ConnectConnections();
        }

		const VS_TlsContext* context = conn ? conn->GetTlsContext() : nullptr;
		std::string commonName;
		if (/* is tls connection */context &&
		     /* is server */hops &&
		     (/* cert check failed */context->certCheckStatus != VS_TlsContext::ccs_success ||
		      /* server name doesn't exist */!context->cert.GetSubjectEntry("commonName", commonName) ||
		      /* server name is wrong */commonName != endpointID
			 )
		   )
		{
			dstream1 << "Someone pretending to be a server connected to us but provided invalid certificate!\n"
				<< "This might mean that said someone is haxxing us or just that "
				<< "his certificate should be updated.\n";
            tr->DeleteConn( conn, "connection is from server but cert check didn't pass", m_CID);
            return ConnectConnections();
		}
        nReconnects = 0;
		bool isCertExist(false);
		oldSecureAbility =
			context || (!(isCertExist = CopySrvCert())) && isAccept ? false :
			(VS_SSL_SUPPORT_BITMASK & version) ? true : false;

		m_secure_hadshake_ver = isCertExist?secure_handshake_ver:1;

		if (!conn->SetSizeBuffers(-1, -1))
		{
            tr->DeleteConn( conn, "!conn->SetSizeBuffers(-1, -1)",m_CID);
            return ConnectConnections();
		}

        isBothAccept = isAccept;

        DeleteBothConn();
        bothConn = conn;
        ++rcvRecTms;
        ++sndRecTms;
        bothLastConDt = time(0);
        strcpy(both,"TCP");
        lastWriteTm = tr->currTm;
        bothVersion = version;

        unsigned long   loc_maxConnSilenceMs, loc_fatalSilenceCoef;
        if (!isAccept)
        {
            loc_maxConnSilenceMs = maxConnSilenceMs;
            loc_fatalSilenceCoef = fatalSilenceCoef;
        }
        else
        {
            loc_maxConnSilenceMs = VS_TransportRouter_Endpoint::maxConnSilenceMs;
            loc_fatalSilenceCoef = VS_TransportRouter_Endpoint::fatalSilenceCoef;
        }

		bool local_tcpKeepAliveSupport = IsTCPKeepAliveAllowed();
		m_IsTCPPingSupported = false;
		if (remote_tcpKeepAliveSupport && local_tcpKeepAliveSupport)		// if both remote and local handshake has keepAlive bit = 1
			 m_IsTCPPingSupported = conn->SetKeepAliveMode(true, 20000, 2000);		// try set keepAlive

		//printf_time("kt: ep->SetConnection: m_IsTCPPingSupported=%d, local=%d, remote=%d\n", m_IsTCPPingSupported, local_tcpKeepAliveSupport, remote_tcpKeepAliveSupport);

        net::HandshakeHeader* hs = nullptr;

		conn->SetOvWriteFields( VS_TR_BTH_CONNECTION_WRITE, (VS_ACS_Field)index );
		conn->SetOvReadFields( VS_TR_BTH_CONNECTION_READ, (VS_ACS_Field)index );

		if (isAccept)
        {
            bool bSSLSupport;
            if (oldSecureAbility)
                bSSLSupport = true;
            else
                bSSLSupport = false;

			unsigned char resultCode(0);
			if ((version & ~VS_SSL_SUPPORT_BITMASK) < VS_MIN_TRANSPORT_VERSION) {
				resultCode = hserr_antikyou;
			} else if ((version & ~VS_SSL_SUPPORT_BITMASK) > VS_CURRENT_TRANSPORT_VERSION) {
				resultCode = hserr_antikme;
			}

            hs = VS_FormTransportReplyHandshake( hops ? endpointID : m_CID,
                                                 resultCode,
                                                 (const unsigned short)loc_maxConnSilenceMs,
                                                 (const unsigned char)loc_fatalSilenceCoef,
                                                 tr->hops,
                                                 tr->endpointName.c_str(),
                                                 bSSLSupport, m_IsTCPPingSupported); // only if both endpoints support tcp keep alive and SetKeepAliveMode(...) returned true; (mu)
            if (!hs)
            {
                DeleteBothConn();
                return false;
            }
            const unsigned long sz = sizeof(net::HandshakeHeader) + hs->body_length + 1;
            VS_Buffer   buffer = { sz, (void *)hs };
            const bool   res = conn->RWrite( &buffer, 1 );
            free( (void *)hs );
            if (!res)
            {
                ///Observer
                ++tr->observer.crt[3];
                return DeleteConnectBothConn();
            }
            rcvWriteSize = writeSize = sz;

        }

        if (!isAccept)
        {
            unsigned long a1(GetTickCount()),b1(GetTickCount()),c1(0),d1(0);
            if (oldSecureAbility)
            {
				bothConn->SetOvReadFields(VS_TR_BTH_SECURE_HANDSHAKE_READ,(VS_ACS_Field)index);
				bothConn->SetOvWriteFields(VS_TR_BTH_SECURE_HANDSHAKE_WRITE,(VS_ACS_Field)index);

				if(!m_HandshakeManager.Init(m_secure_hadshake_ver,handshake_type_Client) || ((m_secure_hadshake_ver == 2)&&(!m_HandshakeManager.SetPrivateKey(&m_srvPrivateKey))) || !SecureHandshake())
				{
					m_isSecureHandshakeInProgress = false;
                    e1 = GetTickCount();
                    if (e1 - a1>maxTimePerOperation)
                    {
                        dprint4("Section ClientSecureHandshake lasts: %ld \n", e1-a1);
                    }
                    return DeleteConnectBothConn();
				}

                b1 = GetTickCount();
            }
            else if (!conn->Read((void *)&readHead, sizeof(transport::MessageFixedPart)))
            {
                ++tr->observer.crt[2];

                if (tr && tr->trDebugLog)
                    tr->trDebugLog->
                    TPrintf("Endpoint::SetConnection::DeleteConnectBothConn !conn->Read( (void *)&readH %s\n",m_CID);
                d1 = GetTickCount();
                if (d1 - b1>maxTimePerOperation)
                {
                    dprint4("Section !Read lasts: %ld \n", d1-b1);
                }

                return DeleteConnectBothConn();
            }
            WriteToBoth();
        }
        lastReadTm = tr->currTm;
        strcpy( both, "TCP" );
        bothVersion = version;
        maxRcvSilenceMs = loc_maxConnSilenceMs;
        maxRcvSilenceCoefMs = loc_maxConnSilenceMs * loc_fatalSilenceCoef;
        maxSndSilenceMs = loc_maxConnSilenceMs;
        maxSndSilenceCoefMs = loc_maxConnSilenceMs * loc_fatalSilenceCoef;
        flagLogs = tr->flagLogs;
        bothCondIpFlag = true;
        if (m_reconnectFlag < 3)
        {
            m_reconnectFlag |= (2) ;
        }
        return true;
    }

    void BothSecureHandshaked()
    {
		m_isSecureHandshakeInProgress = false;

        if (!bothConn)
        {
            DeleteConnectBothConn();
            return;
        }
		isConnectSecure = true;
		if(m_ReadCrypt)
		{
			readSize = 0;
			m_ReadCrypt->Encrypt((const unsigned char*)&readHead, sizeof(transport::MessageFixedPart), 0, &readSize);
			if (readSize != sizeof(transport::MessageFixedPart)) // cipher in block mode
				readSize--;
            ::free(readEncrBuf);
			readEncrBuf = (unsigned char *)malloc(readSize);
			if ( !bothConn->Read(readEncrBuf, readSize ))
			{
				++tr->observer.crt[2];
				if (tr && tr->trDebugLog)
					tr->trDebugLog->
					TPrintf("\n\t Endpoint::SetConnection::DeleteConnectBothConn !conn->Read( (void *)&readH %s",hops ? endpointID : m_CID);
				DeleteConnectBothConn();
			}
			else
				WriteToBoth();
		}
		else
		{
			++tr->observer.crt[2];
			if (tr && tr->trDebugLog)
				tr->trDebugLog->
				TPrintf("\n\t Endpoint::SetConnection::DeleteConnectBothConn !conn->Read( (void *)&readH %s",hops ? endpointID : m_CID);
			DeleteConnectBothConn();
		}
    }

    inline void AddRcvMsgStatistics( const unsigned long length )
    {
        ++rcvMess;
		totalRcvMessSz+=length;
        if (minRcvMessSz > length || !minRcvMessSz)		minRcvMessSz = length;
        if (maxRcvMessSz < length || !maxRcvMessSz)		maxRcvMessSz = length;
    }
    // end VS_TransportRouter_Endpoint::AddRcvMsgStatistics

    inline void AddSndMsgStatistics( const unsigned long length )
    {
        ++sndMess;
		totalSndMessSz+=length;
        if (minSndMessSz > length || !minSndMessSz)		minSndMessSz = length;
        if (maxSndMessSz < length || !maxSndMessSz)		maxSndMessSz = length;
    }
    // end VS_TransportRouter_Endpoint::AddSndMsgStatistics

    inline void FillMonitorStruct( VS_TransportMonitor::TmReply::Endpoint &endpoint )
    {
        const char* pCh;
        memset( (void *)&endpoint, 0, sizeof(endpoint) );
        endpoint.type = TM_TYPE_PERIODIC_ENDPOINT;
		if (hops) {
			endpoint.ep_type = 2;
		} else if (m_authoriztionState == e_authorized) {
			endpoint.ep_type = 1;
		} else {
			endpoint.ep_type = 0;
		}
		strncpy( endpoint.cid, m_CID, sizeof(endpoint.cid) );
        endpoint.cid[sizeof(endpoint.cid) - 1] = 0;
		strncpy( endpoint.username, (m_authoriztionState == e_authorized) ? endpointID : m_CID, sizeof(endpoint.username) );
        endpoint.username[sizeof(endpoint.username) - 1] = 0;
		endpoint.connectionDate = connectionDate;
		strncpy( endpoint.protocol, both, sizeof(endpoint.protocol) );
        endpoint.protocol[sizeof(endpoint.protocol) - 1] = 0;
		endpoint.reconnects = rcvRecTms;
		endpoint.lastConnectDateTime = bothLastConDt;
		endpoint.lastDisconnectDateTime = bothLastDisDt;
        if (bothConn)
        {
            pCh = bothConn->GetBindIp();
            if (pCh) {
				strncpy( endpoint.localHost, pCh, sizeof(endpoint.localHost) );
                endpoint.localHost[sizeof(endpoint.localHost) - 1] = 0;
            }
            pCh = bothConn->GetBindPort();
            if (pCh) {
				endpoint.localPort = (unsigned short)strtoul(pCh, 0, 0);
            }
            pCh = bothConn->GetPeerIp();
            if (pCh) {
				strncpy( endpoint.remoteHost, pCh, sizeof(endpoint.remoteHost) );
                endpoint.remoteHost[sizeof(endpoint.remoteHost) - 1] = 0;
            }
            pCh = bothConn->GetPeerPort();
            if (pCh) {
				endpoint.remotePort = (unsigned short)strtoul(pCh, 0, 0);
            }
        }
        endpoint.rcvMess = rcvMess;
        endpoint.minRcvMessSz = minRcvMessSz;
        endpoint.aveRcvMessSz = (unsigned long)(rcvMess ? totalRcvMessSz/rcvMess : 0);
        endpoint.maxRcvMessSz = maxRcvMessSz;

        endpoint.sndMess = sndMess;
        endpoint.minSndMessSz = minSndMessSz;
        endpoint.aveSndMessSz = (unsigned long)(sndMess ? totalSndMessSz/sndMess : 0);
        endpoint.maxSndMessSz = maxSndMessSz;
    }
    // end VS_TransportRouter_Endpoint::FillMonitorStruct
};
// end VS_TransportRouter_Endpoint struct

//////////////////////////////////////////////////////////////////////////////////////////

const char   trHandleNamePrefix[] = "Handler Of Transport Router. Endpoint: ";

//////////////////////////////////////////////////////////////////////////////////////////

namespace  mi = boost::multi_index;
struct VS_TransportRouter_Implementation : public VS_TransportRouter_ImplementationBase,
            public VS_TransportRouter_SetConnection,
            public VS_TransportMonitor
{
    VS_TransportRouter_Implementation(VS_RoutersWatchdog * watchDog, const char *private_key_pass) :
			VS_TransportRouter_ImplementationBase(private_key_pass), isInit(false),
			transportHandler(0), hthr(0), ocp(0), icp(0), mcp(0), tmRequest(0), tmReply(0),
            tmReadState(0), statePeriodic(0), flagMcpConnect(false), flagMcpWrite(false),
            tmReadSize(0), indexEndpointCount(0), indexServiceCount(0), acs(0),
            service(0), maxServices(0), nServices(0), maxIndServices(0), endpoint(0),
            postMess(0), maxEndpoints(0), nEndpoints(0), endPostMess(0), headPostMess(0),
            maxIndEndpoints(0), nRecursionDepthSendMess(0), routerTickSizeMs(0),
            anti_cycle_rcv_endpoint(0),processingTickTm(1000),routerDebug(0),numberOfConnectThreades(0),
            isShutdown(0),aSetConn(0),bSetConn(0),m_statisticLog(0),m_isStatiscticOn(0),
            m_connsAlive(0), m_watchDog(watchDog),m_hFindServerParamsThreadHandle(0),m_hNewQueueElementEvent(0),
            m_hExitFindServeParamsThreadEvent(0),
			m_serversCertInfo(0)


    {
		assert(m_watchDog != nullptr);
        memset( (void *)&ci, 0, sizeof(ci) );
        memset( (void *)&cr, 0, sizeof(cr) );
        Shutdown();
        isShutdown = 0;
        InitializeCriticalSection(&m_QueueSect);
        m_hNewQueueElementEvent = CreateEvent(0,FALSE,FALSE,0);
        m_hExitFindServeParamsThreadEvent = CreateEvent(0,FALSE,FALSE,0);

		funcIsRoamingAllowed = [](const char *){return true; };

    }
    // end VS_TransportRouter_Implementation::VS_TransportRouter_Implementation

    ~VS_TransportRouter_Implementation( void )
    {
		unsigned long tick = GetTickCount();
        Shutdown();
        DeleteCriticalSection(&m_QueueSect);
        CloseHandle(m_hNewQueueElementEvent);
        CloseHandle(m_hExitFindServeParamsThreadEvent);
		if (trDebugLog && trDebugLog->IsValid())
			trDebugLog->TPrintf("\n\tTermination of the transport router tooks %d mills\n",GetTickCount() - tick);
    }
    // end VS_TransportRouter_Implementation::~VS_TransportRouter_Implementation

    bool	isInit;
    VS_TransportHandler   *transportHandler;
    char   handlerName[VS_ACS_MAX_SIZE_HANDLER_NAME + 1];
    HANDLE   hthr;
    VS_ConnectionMsg   *ocp, *icp;
    VS_ConnectionByte   *mcp;
    TmRequest   *tmRequest;
    TmReply   *tmReply;
    unsigned   tmReadState, statePeriodic, routerDebug;
    bool   flagMcpConnect, flagMcpWrite;
    unsigned long   tmReadSize, indexEndpointCount, indexServiceCount,processingTickTm;
    VS_AccessConnectionSystem   *acs;
    VS_TransportRouter_Service   **service;
    VS_TransportRouter_Endpoint   **endpoint;
    VS_RouterMessage   **postMess;
    VS_TransportRouterServeceList m_permittedList;
    bool		m_isStatiscticOn;
    VS_AcsLog   *m_statisticLog;
    unsigned long m_connsAlive;
	VS_RoutersWatchdog * m_watchDog;

    std::queue<VS_SimpleStr>	m_stringsForFindQueue;
	std::list<HANDLE>			m_getSrvNameThreads;
	std::set<VS_RouterMessExtHandlerInterface*> m_message_ext_handlers;

    HANDLE	m_hNewQueueElementEvent;
    HANDLE	m_hExitFindServeParamsThreadEvent;
    HANDLE	m_hFindServerParamsThreadHandle;

    CRITICAL_SECTION	m_QueueSect;

	VS_ServCertInfoInterface	*m_serversCertInfo;

	std::function<bool(const char*)> funcIsRoamingAllowed;


	using ResponseNotifyMethodT = boost::variant<std::promise<VS_RouterMessage*>, ResponseCallBackT>;
	struct WaitingForResponseT
	{
		WaitingForResponseT(WaitingForResponseT&&src) :service_name(std::move(src.service_name)), notify_method(std::move(src.notify_method)), time_of_death(std::move(src.time_of_death))
		{}
		VS_FORWARDING_CTOR3(WaitingForResponseT, service_name, notify_method, time_of_death) {}
		std::string service_name;
		ResponseNotifyMethodT notify_method;
		std::chrono::steady_clock::time_point	time_of_death;
	};
	class notify_visitor : public boost::static_visitor<>
	{
		VS_RouterMessage *mess;
	public:
		notify_visitor(VS_RouterMessage *m) :mess(m)
		{}
		void operator()(std::promise<VS_RouterMessage*> &p) const
		{
			p.set_value(mess);
		}
		void operator()(ResponseCallBackT &cb) const
		{
			cb(mess);
		}
	};

	boost::multi_index_container<
		WaitingForResponseT,
		mi::indexed_by<
		mi::ordered_unique<mi::tag<struct name>, mi::member<WaitingForResponseT, std::string, &WaitingForResponseT::service_name>>,
		mi::ordered_non_unique<mi::tag<struct expiration>, mi::member<WaitingForResponseT, std::chrono::steady_clock::time_point, &WaitingForResponseT::time_of_death>>
		>
	> m_waiting_for_response_storage;



    /////BugFix Old Brokers Cycle
    VS_TransportRouter_Endpoint * anti_cycle_rcv_endpoint;
    /////BugFix Old Brokers Cycle end
    ///Full disconnect
    VS_TransportNameContainer orderToDeleteEndpoints;
    ///Full disconnect
    ///Connect while shutdown
    volatile LONG isShutdown;
    ///Connect while shutdown
    unsigned long   maxServices, nServices, maxIndServices,
    maxEndpoints, nEndpoints, maxIndEndpoints,
    maxEndpointQueueMess, maxEndpointLackMs,
    maxConnSilenceMs, fatalSilenceCoef,
    maxPostMess, nPostMess, endPostMess, headPostMess,
    maxRecursionDepthSendMess, nRecursionDepthSendMess,
    routerTickSizeMs;
    unsigned long aSetConn,bSetConn;
    //unsigned long   lastRouterTick;
    volatile LONG numberOfConnectThreades;

    enum VS_TR_Cmd { vs_tr_unknown = 0, vs_tr_disconnect_endpoint
                     , vs_tr_start_router, vs_tr_set_transport_key, vs_tr_add_service, vs_tr_add_call_service
                     , vs_tr_delete_service,vs_tr_set_connection, vs_tr_get_statistics
                     , vs_tr_add_route, vs_tr_delete_route_gate, vs_tr_delete_route, vs_tr_delete_gate, vs_tr_replace_gate
                     , vs_tr_add_processing_mes, vs_tr_is_there_endpoint, vs_tr_is_there_route
                     , vs_tr_full_disconnect_all, vs_tr_full_disconnect_endpoint
                     , vs_tr_authorize
                     , vs_tr_unauthorize
                     , vs_tr_isauthorize
					 , vs_tr_getCIDByUID
					 , vs_tr_getIPByCID
					 , vs_tr_addMsgStatistics
					 , vs_tr_setServCertInfoInterface
					 , vs_tr_add_router_mess_ext_handler
					 , vs_tr_remove_router_mess_ext_handler
					 , vs_tr_serverVerificationFailed
					 , vs_tr_request_response_future
					 , vs_tr_request_response_callback
                   };
    union ControlInquiry
        {	struct CntrlInquiry
        {
            VS_TR_Cmd   cmd;
        };	// end CntrlInquiry struct
        struct IsAuthorize : public CntrlInquiry
        {
            char   uid[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
        };  // end FullDisconnectEndpoint struct
        struct Unauthorize : public CntrlInquiry
        {
            char   uid[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
        };  // end FullDisconnectEndpoint struct
        struct Authorize : public CntrlInquiry
        {
            char   uid[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
            char   new_uid [VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
        };  // end FullDisconnectEndpoint struct
        struct FullDisconnectEndpoint : public CntrlInquiry
        {
			struct context
			{
				char   endpoint[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
				CheckEndpointPredT pred;
			} *ctx_;
        };  // end FullDisconnectEndpoint struct
        struct DisconnectEndpoint : public CntrlInquiry
        {
            char   endpoint[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
        };	// end DisconnectEndpoint struct
        struct AddService : public CntrlInquiry
        {
            char   serviceName[VS_TRANSPORT_MAX_SIZE_SERVICE_NAME + 1];
            VS_TransportRouterServiceBase   *service;
            bool isPermittedAll;
        };	// end AddService struct
        struct AddCallService : public CntrlInquiry
        {
            char   serviceName[VS_TRANSPORT_MAX_SIZE_SERVICE_NAME + 1];
            VS_TransportRouterServiceBase   *callService;
            bool permittedForAll;
        };	// end AddCallService struct
        struct RemoveService : public CntrlInquiry
        {
            char   serviceName[VS_TRANSPORT_MAX_SIZE_SERVICE_NAME + 1];
        };	// end RemoveService struct
        struct SetConnection : public CntrlInquiry
        {
            char			cid[VS_ACS_MAX_SIZE_CID + 1];
            char			sid[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
            unsigned long	version;
            VS_Connection   *conn;
            bool   isAccept;
            unsigned short   maxConnSilenceMs;
            unsigned char   fatalSilenceCoef;
			bool	tcpKeepAliveSupport;
            unsigned char   hops;
			unsigned long	rnd_data_ln;
			unsigned char	rnd_data[VS_RND_DATA_FOR_SIGN_SZ];
			unsigned long	sign_ln;
			unsigned char	sign[VS_SIGN_SIZE];
			bool			hs_error;		// handshake error
        };
        struct AddProcessingMes : public CntrlInquiry
        {
            VS_RouterMessage   *mess;
        };	// end AddProcessingMes struct
        struct IsThereEndpoint : public CntrlInquiry
        {
            char   endpoint[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
        };	// end IsThereEndpoint struct
        struct GetCIDByUID : public CntrlInquiry
        {
            const char *uid;
            std::string* cid;
        };
		struct GetIPByCID : public CntrlInquiry
		{
			const char *cid;
			char *ipbuff;
		};
		struct MsgStatistics : public CntrlInquiry
		{
			const char *serviceName;
			unsigned long body;
		};
		struct SetServCertInfoInterface : public CntrlInquiry
		{
			VS_ServCertInfoInterface	*pInterface;
		};
		struct AddRouterMessExtHandler : public CntrlInquiry
		{
			VS_RouterMessExtHandlerInterface *ext_handler;
		};
		struct RemoveRouterMessExtHandler: public CntrlInquiry
		{
			VS_RouterMessExtHandlerInterface *ext_handler;
		};
		struct ServerVerificationFailed : public CntrlInquiry
		{};
		struct RequestResponseFuture : public CntrlInquiry
		{
			struct context
			{
				std::promise<VS_RouterMessage* > promise_;
				RequestLifeTimeT lifetime_;
				VS_RouterMessage *msg_;
			} *ctx_;
		};
		struct RequestResponseCallBack : public CntrlInquiry
		{
			struct context
			{
				ResponseCallBackT cb_;
				RequestLifeTimeT lifetime_;
				VS_RouterMessage *msg_;
			} *ctx_;
		};
        CntrlInquiry				cntrlInquiry;
        DisconnectEndpoint			disconnectEndpoint;
        AddService					addService;
        AddCallService				addCallService;
        RemoveService				removeService;
        SetConnection				setConnection;
        AddProcessingMes			addProcessingMes;
        IsThereEndpoint				isThereEndpoint;
        FullDisconnectEndpoint		fullDisconnectEndpoint;
        Authorize					authorize;
        Unauthorize					unauthorize;
        IsAuthorize					isauthorize;
        GetCIDByUID					getCIDByUID;
		GetIPByCID					getIPByCID;
		MsgStatistics				addMsgStatistics;
		AddRouterMessExtHandler		addRouterMessExtHandler;
		RemoveRouterMessExtHandler	removeRouterMessExtHandler;
		SetServCertInfoInterface	setServCertInfoInterface;
		ServerVerificationFailed	serverVerificationFailed;
		RequestResponseFuture		requestResponseFuture;
		RequestResponseCallBack		requestResponseCallBack;
    };	// end ControlInquiry struct
    union ControlResponse
        {	struct CntrlResponse
        {
            VS_TR_Cmd   cmd;
            bool	res;
        };	// end CntrlResponse struct
        struct GetStatistics : public CntrlResponse
        {
            VS_TransportRouterStatistics   stat;
        };	// end GetStatistics struct
        CntrlResponse	cntrlResponse;
        GetStatistics	getStatistics;
    };	// end ControlResponse struct
    ControlInquiry   ci;
    ControlResponse   cr;

    inline void ResetToConnectMcp( void )
    {
#ifdef _MY_DEBUG_
        puts( "VS_TransportRouter_Implementation::ResetToConnectMcp()" );
#endif
        flagMcpConnect = flagMcpWrite = false;
        if (mcp)
        {
            if (!mcp->IsRW())	DeleteConn( mcp, "ResetToConnectMcp" );
            else {
                mcp->SetOvReadFields( VS_TR_CONNECTION_DEAD_READ, (VS_ACS_Field)mcp );
                mcp->SetOvWriteFields( VS_TR_CONNECTION_DEAD_WRITE, (VS_ACS_Field)mcp );
                mcp->Disconnect();
                //printf("\n\t MCP has rotted!");
            }
            mcp = 0;
        }
        if (tmRequest) {
            delete tmRequest;
            tmRequest = 0;
        }
        if (tmReply) {
            delete tmReply;
            tmReply = 0;
        }
        bool   againFlag = false;
		std::string   nameServer = g_tr_endpoint_name;
go_again:
        if (nameServer.empty())	return;
		VS_FilterPath(nameServer.begin(), nameServer.end(), nameServer.begin());
        char   *namePipe = (char *)malloc( 512 );
        if (!namePipe)	return;
        mcp = new VS_ConnectionByte;
        if (!mcp) {
            free( (void *)namePipe );
            return;
        }
        sprintf( namePipe, "%s%s%s\\%s", VS_PipeDir, VS_Servers_PipeDir, nameServer.c_str(), VS_TrPrefixMonPipe );
        if (!mcp->Create( namePipe, vs_pipe_type_duplex ) || !mcp->SetIOCP( hiocp ))
        {
            delete mcp;
            mcp = 0;
            free( (void *)namePipe );
            return;
        }
        free( (void *)namePipe );
        mcp->SetOvReadFields( (const VS_ACS_Field)VS_TR_MONITOR_CONNECT );
        if (!mcp->Connect())
        {
            if (mcp->State() != vs_pipe_state_connected || againFlag)
            {
                delete mcp;
                mcp = 0;
                return;
            }
            mcp->SetOvWriteFields( (const VS_ACS_Field)VS_TR_MONITOR_WRITE );
            mcp->SetOvReadFields( (const VS_ACS_Field)VS_TR_MONITOR_READ );
            tmRequest = new TmRequest;
            if (!tmRequest) {
                delete mcp;
                mcp = 0;
                return;
            }
            tmReply = new TmReply;
            if (!tmReply)
            {
                delete tmRequest;
                tmRequest = 0;
                delete mcp;
                mcp = 0;
                return;
            }
            if (!mcp->Read( (void *)&tmRequest->type, sizeof(tmRequest->type) ))
            {
                delete tmReply;
                tmReply = 0;
                delete tmRequest;
                tmRequest = 0;
                delete mcp;
                mcp = 0;
                againFlag = true;
                goto go_again;
            }
            tmReadState = 0;
            tmReadSize = sizeof(tmRequest->type);
            flagMcpConnect = true;
        }
    }
    // end VS_TransportRouter_Implementation::ResetToConnectMcp

    inline void WriteMcp( const void *buf, const unsigned long size )
    {
        if (!mcp->Write( buf, size )) {
            ResetToConnectMcp();
            return;
        }
        flagMcpWrite = true;
    }
    // end VS_TransportRouter_Implementation::WriteMcp

    inline unsigned long GetServiceIndex( const char *serviceName )
    {
        for (unsigned long i = 0; i <= maxIndServices; ++i)
            if (service[i] && !strcmp( service[i]->serviceName, serviceName ))
                return i;
        return ~0;
    }
    // end VS_TransportRouter_Implementation::GetServiceIndex

    inline void DeleteService( const unsigned long index )
    {
        VS_TransportRouter_Service   *serv = service[index];
		service[index] = 0;
		--nServices;
        delete serv;
        if (index >= maxIndServices)
        {
            maxIndServices = index;
            while (maxIndServices && !service[--maxIndServices]);
        }
    }
    // end VS_TransportRouter_Implementation::DeleteService

    inline void DeleteService( const char *serviceName )
    {
        const unsigned long   index = GetServiceIndex( serviceName );
        if (index < maxServices)	DeleteService( index );
    }
    // end VS_TransportRouter_Implementation::DeleteService

    inline unsigned long GetFreeServiceIndex( void )
    {
        if (nServices >= maxServices)	return ~0;
        for (unsigned long i = 0; i < maxServices; ++i)
            if (!service[i])	return i;
        return ~0;
    }
    // end VS_TransportRouter_Implementation::GetFreeServiceIndex

    inline VS_TransportRouter_Service *GetService( const char *serviceName )
    {
        const unsigned long   index = GetServiceIndex( serviceName );
        if (index < maxServices)	return service[index];
        return 0;
    }
    // end VS_TransportRouter_Implementation::GetService

	inline void DeleteEndpoint( const unsigned long index, const VS_PointParams::Condition_Reason reason = VS_PointParams::CR_UNKNOWN, bool wasConnected = false )
    {
        if (index >= maxEndpoints)	return;
        VS_TransportRouter_Endpoint   *ep = endpoint[index];
        if (!ep)	return;
		dprint4("DeleteEndpoint: : %s\n",ep->hops ? ep->endpointID : ep->m_CID);

		orderToDeleteEndpoints.RemoveName(ep->hops ? ep->endpointID : ep->m_CID);

        ep->DeleteConns();

        for (unsigned long i = 0; i < maxServices; ++i)
        {
            VS_TransportRouter_Service   *srv = service[i];
            if (!srv)	continue;
            ++nRecursionDepthSendMess;
			VS_PointParams prm;
			prm.uid = ep->endpointID;
			prm.cid = ep->m_CID;
			prm.reazon = reason;
			prm.type = ep->hops ? VS_PointParams::PT_SERVER : VS_PointParams::PT_CLIENT;
			prm.ver = ep->bothVersion;
			bool res(true);
			if ( wasConnected )
				res = srv->servBase->OnPointDisconnected_Event( &prm );
			else
				res = srv->servBase->OnPointConnected_Event( &prm );
            if (!res)
                DeleteService( i );
            if (nRecursionDepthSendMess == 1 && nPostMess)
                CycleUnderPostMessages();
            --nRecursionDepthSendMess;
        }
        while (!ep->sendQueue.IsEmpty())
            ep->sendQueue.Pop(MessageDeleteReason::endpoint_delete);
        delete ep;
        endpoint[index] = 0;
        --nEndpoints;
        if (index >= maxIndEndpoints)
        {
            maxIndEndpoints = index;
            while (maxIndEndpoints && !endpoint[--maxIndEndpoints]);
        }
    }
    // end VS_TransportRouter_Implementation::DeleteEndpoint

    inline unsigned long GetFreeEndpointIndex( void )
    {
        if (nEndpoints >= maxEndpoints)		return ~0;
        for (unsigned long i = 0; i < maxEndpoints; ++i)
            if (!endpoint[i])	return i;
        return ~0;
    }
    // end VS_TransportRouter_Implementation::GetFreeEndpointIndex

    inline unsigned long GetEndpointIndex( const char *endpointName )
    {
        if (!(endpointName && *endpointName)) return ~0;
        for (unsigned long i = 0; i <= maxIndEndpoints; ++i)
            if (endpoint[i] && (!strcasecmp( endpoint[i]->endpointID, endpointName ) || !strcasecmp( endpoint[i]->m_CID,endpointName)))
                return i;
        return ~0;
    }
    // end VS_TransportRouter_Implementation::GetEndpointIndex

    inline VS_TransportRouter_Endpoint *GetEndpoint( const char *endpointName )
    {
        if (!(endpointName && *endpointName)) return 0;
        for (unsigned long i = 0; i <= maxIndEndpoints; ++i)
            if (endpoint[i] && (!strcasecmp( endpoint[i]->endpointID, endpointName ) || !strcasecmp( endpoint[i]->m_CID,endpointName)))
                return endpoint[i];
        return 0;
    }
    // end VS_TransportRouter_Implementation::GetEndpoint

    // d78 label
    inline unsigned long GetEndpointIndexByUID( const char *uid )
    {
        if (!(uid && *uid)) return ~0;
        for (unsigned long i = 0; i <= maxIndEndpoints; ++i)
            if (endpoint[i] && !strcasecmp( endpoint[i]->endpointID, uid ))
                return i;
        return ~0;
    }
    // end VS_TransportRouter_Implementation::GetEndpointIndexByUID

    inline VS_TransportRouter_Endpoint *GetEndpointByUID ( const char *uid)
    {
        if (!(uid && *uid)) return 0;
        for (unsigned long i = 0; i <= maxIndEndpoints; ++i)
            if (endpoint[i] && !strcasecmp( endpoint[i]->endpointID, uid ))
                return endpoint[i];
        return 0;
    }
    // end VS_TransportRouter_Implementation::GetEndpointByUID

	inline void OnEndpointConnect(const unsigned long index, VS_PointParams::Condition_Reason reason = VS_PointParams::CR_UNKNOWN)
    {
        VS_TransportRouter_Endpoint *ep = endpoint[index];

		dprint4("TransportRouter->OnEndpointConnect: endpointID=%s\n", ep->endpointID);

		for (unsigned long i = 0; i < maxServices; ++i)
		{
			VS_TransportRouter_Service   *srv = service[i];
			if (!srv)	continue;
			++nRecursionDepthSendMess;
			VS_PointParams prm;
			prm.uid = ep->endpointID;
			prm.cid = ep->m_CID;
			prm.reazon = reason;
			prm.type = ep->hops ? VS_PointParams::PT_SERVER : VS_PointParams::PT_CLIENT;
			prm.ver = ep->bothVersion;
			if (!srv->servBase->OnPointConnected_Event( &prm ))
				DeleteService( i );
			if (nRecursionDepthSendMess == 1 && nPostMess)
				CycleUnderPostMessages();
			--nRecursionDepthSendMess;
		}
    }
	// end VS_TransportRouter_Implementation::OnEndpointConnect

    inline unsigned long CreateEndpointIndex( const char *CID, const char *SID, const unsigned char ep_hops )
    {
        if ((!CID || !*CID) && (!SID || !*SID))
            return ~0;

		if (ep_hops > 0 && !funcIsRoamingAllowed(SID))
			return ~0;
        const unsigned long   index = GetFreeEndpointIndex();
        if (index >= maxEndpoints)		return ~0;

        VS_TransportRouter_Endpoint   *ep(0);
        ep = new VS_TransportRouter_Endpoint( this, index, CID, SID, ep_hops, maxEndpointLackMs, maxEndpointQueueMess, maxConnSilenceMs, fatalSilenceCoef );
        if (!ep)	return ~0;
        if (!ep->isValid) {
            delete ep;
            return ~0;
        }
        endpoint[index] = ep;
        ++nEndpoints;
        if (index > maxIndEndpoints)	maxIndEndpoints = index;
        return index;
    }
    // end VS_TransportRouter_Implementation::CreateEndpointIndex

    inline VS_TransportRouter_Endpoint *CreateEndpoint( const char *CID, const char *SID, const unsigned char ep_hops )
    {
        if ((!CID || !*CID) && (!SID || !*SID))
            return 0;
        const unsigned long   index = CreateEndpointIndex( CID, SID, ep_hops );
        return index < maxEndpoints ? endpoint[index] : 0;
    }
    // end VS_TransportRouter_Implementation::CreateEndpoint

    inline unsigned long AddEndpointIndex( const char *CID, const char *SID, const unsigned char ep_hops )
    {
        if ((!CID || !*CID) && (!SID || !*SID))
            return ~0;
        const unsigned long   index = GetEndpointIndex( CID ? CID : SID );
        return index < maxEndpoints ? index : CreateEndpointIndex( CID, SID, ep_hops );
    }
    // end VS_TransportRouter_Implementation::AddEndpointIndex

    inline VS_TransportRouter_Endpoint *AddEndpoint( const char *CID, const char *SID, const unsigned char ep_hops )
    {
        VS_TransportRouter_Endpoint   *ep = GetEndpoint( ep_hops ? SID : CID);
        return ep ? ep : CreateEndpoint( CID, SID, ep_hops );
    }
    // end VS_TransportRouter_Implementation::AddEndpoint

    inline bool RegistryProcessingMessages( const char *SID, VS_RouterMessage *mess )
    {
        if (!IsThereEndpointRegistry( SID ))
            return false;
        unsigned long   index = CreateEndpointIndex( 0, SID, 1 );
        if (index >= maxEndpoints)		return false;
        VS_TransportRouter_Endpoint   *ep = endpoint[index];
        if (ep->ConnectConnectionsAct())
        {
            ep->isBothAccept = false;
            ep->ProcessingMessages( mess );
            return true;
        } else
        {	/* Here it will be necessary to throw off in TRACE */
			DeleteEndpoint( index, VS_PointParams::CR_TIMEOUT );
            return false;
        }
    }
    // end VS_TransportRouter_Implementation::RegistryProcessingMessages

    inline void KillInjuryMessage(VS_RouterMessage *mess,int isSndRcvEqual=0)
    {
        dprint4("TR:Msg deleted: %s->%s Reason :%d\n", mess->SrcCID(), mess->DstCID(), isSndRcvEqual);
        delete mess;
    }
    // end VS_TransportRouter_Implementation::static


    inline void GetRouteInformationOnMessage(VS_RouterMessage *mess ,
            VS_TransportRouter_Endpoint * ep)
    {
        if (trDebugLog)
        {
            char info[VS_ACS_MAX_SIZE_ENDPOINT_NAME+1]={0};
            if (ep && ep->endpointID)
            {
                strcpy(info,ep->endpointID);
            }
            else
            {
                strcpy(info," none " );
            }
            MessageLog( *mess , trDebugLog , info );
        }
    }
    //end VS_TransportRouter_Implementation::GetRouteInformationOnMessage

    inline void MakeDebugInfo(VS_RouterMessage *mess,
                              VS_TransportRouter_Endpoint * ep)
    {
        if (routerDebug>0)
        {
            if (!trDebugLog || !trDebugLog->IsValid())
            {
                if (trDebugLog && !trDebugLog->IsValid())
                {
                    delete trDebugLog;
                    trDebugLog = 0;
                }

				trDebugLog = new VS_AcsLog( "Router",  0x100000 * 4, 0x100000*3 , "./" );

                if (!trDebugLog)
                    return;

                if (!trDebugLog->IsValid())
                {
                    routerDebug = 0;
                    delete trDebugLog;
                    trDebugLog = 0;
                    return;
                }
            }
            GetRouteInformationOnMessage( mess , ep );
        }
    }
    //end VS_TransportRouter_Implementation::MakeDebugInfo
    enum PROCESSING_MESSAGE_EXIT_STATE{
        e_none,
        e_messageInvalid,
        e_endpointProcessing1,
        e_endpointProcessing2,
        e_endpointProcessing3,
        e_endpointProcessing4,
        e_regesrtryProcessing1,
        e_regesrtryProcessing2,
        e_regesrtryProcessing3,
        e_kill0,
        e_kill1,
        e_kill2,
        e_kill3,
        e_managingProcessing,
        e_serviceProcessing1,
        e_serviceProcessing2,
        e_processingAgainNotify,
        e_serviceIsEmpty,
        e_serviceAuthority,
		e_externalProcessing,
		e_requestResponseProcessing,
        e_unknownMessage
    };

    inline int ProcessingMessages( VS_RouterMessage *mess ,
                                   unsigned long &index_to)
    {
        /////BugFix Old Brokers Cycle
        VS_TransportRouter_Endpoint * recv_from_endpoint = anti_cycle_rcv_endpoint;
        anti_cycle_rcv_endpoint = 0;
        /////BugFix Old Brokers Cycle end
        MakeDebugInfo( mess , recv_from_endpoint);

        if (!mess->IsValid())
        {	/* Here it will be necessary to throw off in TRACE */
            return e_messageInvalid;
        }
        VS_TransportRouter_Endpoint   *ep = 0;
        unsigned long   index = ~0;
        unsigned char   hops = ~0;
        if (tMessagesLog)	MessageLog( *mess );

		/**
			TODO: Сделать рефакторинг так, чтобы в первую очередь сообщения пытались обработаться обычным образом, а потом уже внешними перехватчиками
			Или, как вариант основную процедуру тоже сделать, как один из хендлеров и поставить его в начале очереди

		*/
		if(TryExternalProcessing(mess))
			return e_externalProcessing;
		if (TryWaitingResponseProcessing(mess))
			return e_requestResponseProcessing;

        if (!mess->IsNotify())
        {
            const char *srcCID(mess->SrcCID());
            const char *srcUID(mess->SrcUser());
            const char *srcSID(mess->SrcServer());
            const char *srcSRV(mess->SrcService());

            const char *dstCID(mess->DstCID());
            const char *dstUID(mess->DstUser());
            const char *dstSID(mess->DstServer());
            const char *dstSRV(mess->DstService());

            bool isAuthorizedSrc(false);
            bool isAuthorizedDst(false);

            struct _s_mode {
				unsigned long _srcCID	: 1;
				unsigned long _srcUID	: 1;
				unsigned long _srcSID	: 1;
				unsigned long _dstCID	: 1;
				unsigned long _dstUID	: 1;
				unsigned long _dstSID	: 1;
				unsigned long _Pad		: 32-6; // "bits zero padding"
            };

            union _u_mode {
                unsigned long	_key;	// result key
                struct _s_mode	_bits;	// accumulation bits
            };

            union _u_mode _mode = {(unsigned long)0};
            _mode._bits._srcCID = (srcCID && *srcCID);
            _mode._bits._srcUID = (srcUID && *srcUID);
            _mode._bits._srcSID = (srcSID && *srcSID);
            _mode._bits._dstCID = (dstCID && *dstCID);
            _mode._bits._dstUID = (dstUID && *dstUID);
            _mode._bits._dstSID = (dstSID && *dstSID);

            enum _e_mode {
                state_1	= 1,	// from unauth user
                state_2	= 34,	// user to server service
                state_3	= 50,	// user to user
                state_4	= 38,	// user to server service (reply mode)
                state_5	= 54,	// user to user (reply mode)
                state_6	= 18,	// user to user on unresolved server
                state_7	= 22,	// user to user on unresolved server
                state_81= 12,	// from server to unauth user
                state_82= 8,	// from server to unauth user
                state_9	= 36,	// from server to server service
                state_10= 20,	// from server to user on unresolved server
                state_11= 52	// from server to user at other server
            };

            enum _r_mode { // kill reasons
                reason_test_authority = 1,
                reason_msg_error = 2,
                reason_msg_invalid = 3,
                reason_user_resolve = 4,
                reason_msg_noresolve = 5
            };

            switch (_mode._key) {
            case state_1  : {
                if (!dstSRV) {
                    KillInjuryMessage(mess, reason_msg_error);
                    return e_serviceIsEmpty;
                }
                if (!*dstSRV) {
                    ProcessingManaging(mess);
                    return e_managingProcessing;
                }
                if (!TestAuthority(mess)) {
                    KillInjuryMessage(mess, reason_test_authority);
                    return e_serviceAuthority;
                }
                index = GetServiceIndex(dstSRV);
                if (index < maxServices) {
                    VS_TransportRouter_Service *serv = service[index];
                    serv->AddRcvMsgStatistics(mess->BodySize());
                    if (!serv->withOwnThread) {
                        DWORD   tc = 0;
                        if (!nRecursionDepthSendMess)	tc = GetTickCount();
                        ++nRecursionDepthSendMess;
                        if (!serv->servBase->Processing( std::unique_ptr<VS_RouterMessage>(mess)))
                            DeleteService( index );
                        if (nRecursionDepthSendMess == 1 && nPostMess)
                            CycleUnderPostMessages();
                        --nRecursionDepthSendMess;
                        if (!nRecursionDepthSendMess)
                        {
                            tc = GetTickCount() - tc;
                            if (tc > maxTimePerOperation)
                            {
                                const time_t   tm = time( 0 );
                                dprint4("TransportRouter: The Processing of Message lasted %lu ms.\n\tTransport Call Service Name: %s. %s\n", tc, serv->serviceName, ctime(&tm));
                            }
                        }
                        index_to = index;
                        return e_serviceProcessing1;
                    } else if (serv->pipeService) {
                        if (!serv->pipeService->ProcessingMessages( mess ))
                            DeleteService( index );
                        else index_to = index;
                        return e_serviceProcessing2;
                    }
				} /* Here it will be necessary to throw off in TRACE */
                mess->SetNotity(true);
                ProcessingMessages( mess );
                return e_processingAgainNotify;
            }
            case state_2  :
            case state_3  :
            case state_4  :
            case state_5  :
            case state_6  :
            case state_7  :
            case state_9  :
            case state_10 :
            case state_11 : {
                if (IsOurEndpointName(dstSID)) {	// our message
                    index = GetEndpointIndexByUID(dstUID);
                    if (index  < maxEndpoints) {	// to user
                        endpoint[index]->ProcessingMessages(mess);
                        index_to = index;
                        return e_endpointProcessing3;
                    }
					if (*dstUID) {	// user not found on our server
                        KillInjuryMessage(mess, reason_msg_invalid);
                        return e_kill0;
					}
                    if (!dstSRV) {
                        KillInjuryMessage(mess, reason_msg_error);
                        return e_serviceIsEmpty;
                    }
                    if (!*dstSRV) {
                        ProcessingManaging(mess, srcSID);
                        return e_managingProcessing;
                    }
                    if (!TestAuthority(mess)) {
                        KillInjuryMessage(mess, reason_test_authority);
                        return e_serviceAuthority;
                    }
                    index = GetServiceIndex(dstSRV);
                    if (index < maxServices) {		// to service
                        VS_TransportRouter_Service *serv = service[index];
                        serv->AddRcvMsgStatistics(mess->BodySize());
                        if (!serv->withOwnThread) {
                            DWORD   tc = 0;
                            if (!nRecursionDepthSendMess)	tc = GetTickCount();
                            ++nRecursionDepthSendMess;
                            if (!serv->servBase->Processing( std::unique_ptr<VS_RouterMessage>(mess) ))
                                DeleteService( index );
                            if (nRecursionDepthSendMess == 1 && nPostMess)
                                CycleUnderPostMessages();
                            --nRecursionDepthSendMess;
                            if (!nRecursionDepthSendMess)
                            {
                                tc = GetTickCount() - tc;
                                if (tc > maxTimePerOperation)
                                {
                                    const time_t   tm = time( 0 );
                                    dprint4("TransportRouter: The Processing of Message lasted %lu ms.\nTransport Call Service Name: %s. %s\n", tc, serv->serviceName, ctime(&tm));
                                }
                            }
                            index_to = index;
                            return e_serviceProcessing1;
                        } else if (serv->pipeService) {
                            if (!serv->pipeService->ProcessingMessages( mess ))
                                DeleteService( index );
                            else index_to = index;
                            return e_serviceProcessing2;
                        }
                    } /* Here it will be necessary to throw off in TRACE */
                    mess->SetNotity(true);
                    ProcessingMessages( mess );
                    return e_processingAgainNotify;
                } else { // message to somwhere else
                    if ((dstSID) && (*dstSID)) {
                        index = GetEndpointIndexByUID(dstSID);
                        if (index  < maxEndpoints) {	// to user
                            endpoint[index]->ProcessingMessages(mess);
                            index_to = index;
                            return e_endpointProcessing3;
                        }
                        net::endpoint::CreateFromID(dstSID, false);
                        StartResolveName(dstSID);
                        if (RegistryProcessingMessages(dstSID, mess)) {
                            return e_regesrtryProcessing3;
                        }
                    } else { // user at unknown server
                        index = GetEndpointIndexByUID(dstUID);
                        if (index  < maxEndpoints) {	// to user
                            endpoint[index]->ProcessingMessages(mess);
                            index_to = index;
                            return e_endpointProcessing3;
                        }
                        extern const char RESOLVE_SRV[];
                        index = GetServiceIndex(RESOLVE_SRV);
                        if (index < maxServices) {		// to service
                            VS_TransportRouter_Service *serv = service[index];
                            serv->AddRcvMsgStatistics(mess->BodySize());
                            if (!serv->withOwnThread) {
                                DWORD   tc = 0;
                                if (!nRecursionDepthSendMess)	tc = GetTickCount();
                                ++nRecursionDepthSendMess;
                                if (!serv->servBase->Processing( std::unique_ptr<VS_RouterMessage>(mess) ))
                                    DeleteService( index );
                                if (nRecursionDepthSendMess == 1 && nPostMess)
                                    CycleUnderPostMessages();
                                --nRecursionDepthSendMess;
                                if (!nRecursionDepthSendMess)
                                {
                                    tc = GetTickCount() - tc;
                                    if (tc > maxTimePerOperation)
                                    {
                                        const time_t   tm = time( 0 );
                                        dprint4("TransportRouter: The Processing of Message lasted %lu ms.\nTransport Call Service Name: %s. %s\n", tc, serv->serviceName, ctime(&tm));
                                    }
                                }
                                index_to = index;
                                return e_serviceProcessing1;
                            } else if (serv->pipeService) {
                                if (!serv->pipeService->ProcessingMessages( mess ))
                                    DeleteService( index );
                                else index_to = index;
                                return e_serviceProcessing2;
                            }
                        } /* Here it will be necessary to throw off in TRACE */
                        KillInjuryMessage(mess, reason_msg_noresolve);
                        return e_kill0;
                    }
                }
                KillInjuryMessage(mess, reason_msg_invalid);
                break;
            }
            case state_81:
            case state_82: {
                const char *usr = mess->DstCID();
                index = GetEndpointIndex(usr);
                if (index  < maxEndpoints) {
                    endpoint[index]->ProcessingMessages(mess);
                    index_to = index;
                    return e_endpointProcessing3;
                }
                KillInjuryMessage(mess, reason_msg_invalid);
                return e_kill0;
                break;
            }
            default: {
                KillInjuryMessage(mess, reason_msg_invalid);
            }
            }
        }
        return e_none;
    }
    // end VS_TransportRouter_Implementation::ProcessingMessages
	bool TryWaitingResponseProcessing(VS_RouterMessage *mess)
	{
		if (!mess || !mess->DstService() || !mess->DstServer() || !IsOurEndpointName(mess->DstServer()))
			return false;
		const auto it = m_waiting_for_response_storage.find(mess->DstService());
		if (m_waiting_for_response_storage.end() == it)
			return false;
		dstream4 << "Response arrived. DstService(RequestID) = " << it->service_name << "; SrcServer = " << mess->SrcServer() << "; SrcService = " << mess->SrcService();
		m_waiting_for_response_storage.modify(it, [mess](WaitingForResponseT &item) {boost::apply_visitor(notify_visitor(mess), item.notify_method);});
		m_waiting_for_response_storage.erase(it);
		return true;
	}
	void CheckWaitingForResponseExpiration()
	{
		auto it = m_waiting_for_response_storage.get<expiration>().begin();
		const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		while (it != m_waiting_for_response_storage.get<expiration>().end())
		{
			if (now > it->time_of_death)
			{
				auto dbg_print = dstream3;
				dbg_print << "waiting for response is expired. Service name = " << it->service_name << " exp_time = " << it->time_of_death.time_since_epoch().count() << " now = " << now.time_since_epoch().count() << '\n';
				m_waiting_for_response_storage.get<expiration>().modify(it, [](WaitingForResponseT &item) {boost::apply_visitor(notify_visitor(nullptr), item.notify_method);});
				it = m_waiting_for_response_storage.get<expiration>().erase(it);
			}
			else
				break;
		}
	}

	bool TryExternalProcessing(VS_RouterMessage *mess)
	{
		for(std::set<VS_RouterMessExtHandlerInterface*>::iterator i = m_message_ext_handlers.begin();i!=m_message_ext_handlers.end();i++)
			if((*i)->IsMyMess(mess) && (*i)->ProcessingMessgae(mess))
				return true;
		return false;
	}
    void StartResolveName(const char *name)
    {
        EnterCriticalSection(&m_QueueSect);
        m_stringsForFindQueue.push(name);
        LeaveCriticalSection(&m_QueueSect);
        SetEvent(m_hNewQueueElementEvent);
    }

    inline void DeleteEndpointQueryTry()
    {
        if (!orderToDeleteEndpoints.IsEmpty())
        {
            const char * deleteEndpoint = 0;
            if (!orderToDeleteEndpoints.GetFirstName( deleteEndpoint )) return;
            do{
                unsigned long index = this->GetEndpointIndexByUID( deleteEndpoint );
                if (index >= maxEndpoints) continue;
                if (endpoint[ index ] && endpoint[ index ]->ReadyToDestroy())
                {
                    dprint3("Delete endpoint by forced disconnect %s\n",deleteEndpoint);
                    orderToDeleteEndpoints.RemoveCurrent();
                    orderToDeleteEndpoints.Show();
					DeleteEndpoint( index, (endpoint[index]->m_IsForcedDisconnect)? VS_PointParams::CR_REQUESTED:
																					VS_PointParams::CR_INCOMING, true );
                }else
                {
                    dprint3("Delete endpoint by forced disconnect was deferred. Endpoint %s is not ready.\n",deleteEndpoint);
                }
            }while (orderToDeleteEndpoints.GetNextName( deleteEndpoint ));
        }
    }

    inline bool TestAuthority(VS_RouterMessage *mess )
    {
        // for test purposes only
        return true; // all granted for now
        // end test purposes
        struct _s_wmode {
			signed long srcSID : 1;
			signed long srcUID : 1;
			signed long srcCID : 1;
			// dup dup dup zero fill
			signed long dupFill: 29;
        };

        union _u_wmode {
            unsigned long _wmode;
            struct _s_wmode _bits;
        };

        // read wiki http://projects.visicron.ru/bin/view/Dev/AuthDispatchingTable
        enum _e_mode {
            state_1  = 1,
            state_2  = 2,
            state_3  = 3,
            state_41 = 4,
            state_42 = 5,
            state_43 = 6,
            state_44 = 7,
            state_z  = 0 // may be used later for hacker detection
        };

        const char *sid = mess->SrcServer();
        const char *uid = mess->SrcUser();
        const char *cid = mess->SrcCID();
        _u_wmode _mode = {(signed long)0};
        _mode._bits.srcSID = ((sid) && (*sid));
        _mode._bits.srcUID = ((uid) && (*uid));
        _mode._bits.srcCID = ((cid) && (*cid));

        switch (_mode._wmode) {
        case state_1 : {
            if (IsOurEndpointName(sid)) return true;
            unsigned long index = GetEndpointIndexByUID(sid);
            if (index < maxEndpoints)
            {
                if (endpoint[index]->isAuthorizedEndpoint())	return true;
                if (m_permittedList.IsInList(mess->DstService())) return true;

                ///it is a misstake may be....
                /// Back door inside. You have to remove this code.
                if (strcasecmp(mess->DstService(),"") == 0) return true;
                ///Back door
            }
            break;
        }
        case state_2 : {
            unsigned long index = GetEndpointIndexByUID(uid);
            if (index < maxEndpoints)
            {
                if (endpoint[index]->isAuthorizedEndpoint())	return true;
                if (m_permittedList.IsInList(mess->DstService())) return true;

                ///it is a misstake may be....
                /// Back door inside. You have to remove this code.
                if (strcasecmp(mess->DstService(),"") == 0) return true;
                ///Back door
            }
            break;
        }
        case state_3 : {
            if (IsOurEndpointName(sid)) return true;
            unsigned long index1 = GetEndpointIndexByUID(sid);
            unsigned long index2 = GetEndpointIndexByUID(uid);
            if ((index1 < maxEndpoints) && (index2 < maxEndpoints))
            {
                if (endpoint[index1]->isAuthorizedEndpoint() && endpoint[index2]->isAuthorizedEndpoint()) return true;
                if (m_permittedList.IsInList(mess->DstService())) return true;

                ///it is a misstake may be....
                /// Back door inside. You have to remove this code.
                if (strcasecmp(mess->DstService(),"") == 0) return true;
                ///Back door
            }
            break;
        }
        case state_41:
        case state_42:
        case state_43:
        case state_44: {
            unsigned long index = GetEndpointIndex(cid);
            if (index < maxEndpoints)
            {
                if (endpoint[index]->isAuthorizedEndpoint())	return true;
                if (m_permittedList.IsInList(mess->DstService())) return true;

                ///it is a misstake may be....
                /// Back door inside. You have to remove this code.
                if (strcasecmp(mess->DstService(),"") == 0) return true;
                ///Back door
            }
            break;
        }
        default:       {
        }
        };

        ///KillInjuryMessage( mess , 5 ); // AV
        return false;
    }
    inline void ProcessingMessages( VS_RouterMessage *mess )
    {
        unsigned long index_to = 0;
        int res = 0;

// ---------------------------------
// TODO: Reserved for future use (ask Dront78)
// ---------------------------------
        //if (!TestAuthority(mess))
        //	return;
// ---------------------------------
// TODO: Reserved for future use (ask Dront78)
// ---------------------------------

        res = ProcessingMessages( mess , index_to);

        int i = 0;
        if (trDebugLog)
        {
            switch (res)
            {
            case  e_none:
                trDebugLog->TPrintf(" none");
                break;
            case e_messageInvalid:
                trDebugLog->TPrintf(" messageInvalid");
                break;
            case e_endpointProcessing1:
                if (!i) i = 1;
            case e_endpointProcessing2:
                if (!i) i = 2;
            case e_endpointProcessing3:
                if (!i) i = 3;
            case e_endpointProcessing4:
            {
                if (!i) i =4;
                if (index_to < maxEndpoints)
                {
                    VS_TransportRouter_Endpoint *ep = endpoint[index_to];
                    trDebugLog->TPrintf(" endpointProcessing: %d Er:%I64u Es:%I64u  NextHop: %s ",
                                       i,ep->rcvMess, ep->sndMess, ep->endpointID);
                }
                else
                {
                    trDebugLog->TPrintf(" endpointProcessing: %d NextHop: none",
                                       i);
                }

            }break;
            case e_regesrtryProcessing1:
                trDebugLog->TPrintf(" regesrtryProcessing1");
                break;
            case e_regesrtryProcessing2:
                trDebugLog->TPrintf(" regesrtryProcessing2");
                break;
            case e_regesrtryProcessing3:
                trDebugLog->TPrintf(" regesrtryProcessing3");
                break;
            case e_kill0:
                trDebugLog->TPrintf(" kill0");
                break;
            case e_kill1:
                trDebugLog->TPrintf(" kill1");
                break;
            case e_kill2:
                trDebugLog->TPrintf(" kill2");
                break;
            case e_kill3:
                trDebugLog->TPrintf(" killed! Bugfix cycle 2");
                break;
            case e_managingProcessing:
                trDebugLog->TPrintf(" managingProcessing");
                break;
            case e_serviceProcessing1:
                if (!i) i = 1;
            case e_serviceProcessing2:
            {
                if (!i) i = 2;

                if (index_to < maxServices)
                {
                    VS_TransportRouter_Service   *serv = service[index_to];
                    trDebugLog->TPrintf(" serviceProcessing: %d Sr:%I64u Ss:%I64u Srvc: %s",i,
                                       serv->rcvMess,
                                       serv->sndMess,
                                       serv->serviceName);
                }else
                {
                    trDebugLog->TPrintf(" serviceProcessing: %d Srvc: none",i);
                }
            }break;
			case e_externalProcessing:
				trDebugLog->TPrintf(" The message was processed by external handler.");
				break;
            case e_processingAgainNotify:
                trDebugLog->TPrintf(" processingAgainNotify");
                break;
            default:
                break;
            }
        }
    }
    // end VS_TransportRouter_Implementation::ProcessingMessages

    inline void ProcessingManaging( VS_RouterMessage *mess, const char *ep = 0 )
    {
        if (!mess->IsNotify() && mess->IsRequest())
        {
            if (mess->IsValid())
            {
                const char   *endpointName = ep?ep:mess->SrcCID();
                VS_TransportRouter_Endpoint   *ep = GetEndpoint( endpointName );
                if (ep) {
                    ep->ProcessingManaging( mess );
                    return;
                }
            }
        }
        delete mess;
    }
    // end VS_TransportRouter_Implementation::ProcessingManaging

    inline bool CallServiceSendMes( VS_RouterMessage *mess )
    {
        if (!mess || !mess->IsValid())	return false;
        if (threadId != GetCurrentThreadId())	return AddProcessingMes( mess );
        ProcessingMessages( mess );
        return true;
    }
    // end VS_TransportRouter_Implementation::CallServiceSendMes

    inline bool CallServicePostMes( VS_RouterMessage *mess )
    {
        if (!mess || !mess->IsValid())	return false;
        if (threadId != GetCurrentThreadId())	return AddProcessingMes( mess );
        if (nPostMess >= maxPostMess)	return false;
        postMess[endPostMess] = mess;
        ++nPostMess;
        if (++endPostMess >= maxPostMess)	endPostMess = 0;
        return true;
    }
    // end VS_TransportRouter_Implementation::CallServicePostMes
	inline bool CallServiceRequestResponse(VS_RouterMessage *req, ResponseCallBackT&&resp_cb, RequestLifeTimeT &&req_life_time)
	{
		if (!req || !req->IsValid())
			return false;
		if (GetCurrentThreadId() == threadId)
			return PrepareAndStoreRequestResponse(req, std::move(resp_cb), std::move(req_life_time));
		ControlInquiry::RequestResponseCallBack req_resp;
		ControlResponse::CntrlResponse   cntrlResponse;
		req_resp.ctx_ = new ControlInquiry::RequestResponseCallBack::context;
		req_resp.cmd = vs_tr_request_response_callback;
		std::promise<VS_RouterMessage*> p;
		req_resp.ctx_->cb_ = std::move(resp_cb);
		req_resp.ctx_->lifetime_ = std::move(req_life_time);
		req_resp.ctx_->msg_ = req;
		if (!CntrlSR(reinterpret_cast<ControlInquiry*>(&req_resp), sizeof(req_resp), reinterpret_cast<ControlResponse*>(&cntrlResponse), sizeof(cntrlResponse)) && !cntrlResponse.res)
		{
			delete req_resp.ctx_;
			return false;
		}
		return true;
	}
	inline ResponseFutureT CallServiceRequestResponse(VS_RouterMessage *req, RequestLifeTimeT &&req_life_time)
	{
		if (!req || !req->IsValid())
			return ResponseFutureT();
		std::promise<VS_RouterMessage*> p;
		if (GetCurrentThreadId() == threadId)
		{
			auto res = p.get_future();
			if (PrepareAndStoreRequestResponse(req, std::move(p), std::move(req_life_time)))
				return res;
			return ResponseFutureT();
		}
		ControlInquiry::RequestResponseFuture req_resp;
		ControlResponse::CntrlResponse   cntrlResponse;
		req_resp.ctx_ = new ControlInquiry::RequestResponseFuture::context;
		req_resp.cmd = vs_tr_request_response_future;
		auto res = p.get_future();
		req_resp.ctx_->promise_ = std::move(p);
		req_resp.ctx_->lifetime_ = std::move(req_life_time);
		req_resp.ctx_->msg_ = req;
		if (!CntrlSR(reinterpret_cast<ControlInquiry*>(&req_resp), sizeof(req_resp), reinterpret_cast<ControlResponse*>(&cntrlResponse), sizeof(cntrlResponse)) && !cntrlResponse.res)
		{
			delete req_resp.ctx_;
			return ResponseFutureT();
		}
		return res;
	}

    inline bool BaseServiceGetStatistics( VS_TransportRouterStatistics *stat )
    {
        return !stat ? true : (threadId != GetCurrentThreadId() ? GetStatistics( *stat ) : GoGetStatisticsAct( *stat ));
    }
    // end VS_TransportRouter_Implementation::BaseServiceGetStatistics

    inline bool BaseServiceIsThereEndpoint( const char *endpoint )
    {
        return !VS_CheckEndpointName( endpoint ) ? false : (threadId != GetCurrentThreadId() ? IsThereEndpoint( endpoint ) : GoIsThereEndpointAct( endpoint ));
    }
    // end VS_TransportRouter_Implementation::BaseServiceIsThereEndpoint

    inline bool BaseServiceDisconnectEndpoint( const char *endpoint )
    {
        return !VS_CheckEndpointName( endpoint ) ? false : (threadId != GetCurrentThreadId() ? DisconnectEndpoint( endpoint ) : GoDisconnectEndpointAct( endpoint ));
    }
    // end VS_TransportRouter_Implementation::BaseServiceDisconnectEndpoint

	inline bool BaseServiceAddMessageStatistics(const char *name, unsigned long body)
	{
		return (!name || !*name) ? true : (threadId != GetCurrentThreadId()) ? AddMessageStatictics(name, body) : GoAddMessageStatisticsAct(name, body);
	}
    // end VS_TransportRouter_Implementation::BaseServiceAddMessageStatistics

	inline bool AddMessageStatictics(const char *name, unsigned long body)
	{
		ControlInquiry::MsgStatistics cntrlInquiry;
		memset(&cntrlInquiry, 0, sizeof(cntrlInquiry));
		cntrlInquiry.cmd = vs_tr_addMsgStatistics;
		cntrlInquiry.serviceName = name;
		cntrlInquiry.body = body;
		ControlResponse::CntrlResponse   cntrlResponse;
		memset(&cntrlResponse, 0, sizeof(cntrlResponse));
		return CntrlSR((ControlInquiry *)&cntrlInquiry, sizeof(cntrlInquiry), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse)) ? cntrlResponse.res : false;
	}

    inline bool GoAddMessageStatistics( const unsigned long size )
    {
		if (size != sizeof(ControlInquiry::MsgStatistics)) return false;
        const char *serviceName = ci.addMsgStatistics.serviceName;
		const unsigned long body = ci.addMsgStatistics.body;
		cr.cntrlResponse.cmd = vs_tr_addMsgStatistics;
        cr.cntrlResponse.res = GoAddMessageStatisticsAct(serviceName, body);
        return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
    }

	inline bool GoAddMessageStatisticsAct(const char *name, unsigned long body)
	{
		VS_TransportRouter_Service *srv(GetService(name));
		if (srv) { srv->AddSndMsgStatistics(body); }
		return true;
	}

    inline void ForcedDisconnectByPeer( const char *aEndpointName )
    {
        if (!VS_CheckEndpointName( aEndpointName )) return;

        unsigned long terminal = GetEndpointIndex( aEndpointName );
        if (terminal>=maxEndpoints)					return;
        orderToDeleteEndpoints.AddName( aEndpointName );
        return;
    }
	inline void CancelEndpointDisconnect( const char *aEndpointName )
	{
		unsigned long index = GetEndpointIndex(aEndpointName);
		if(index>=maxEndpoints)
			return;

		orderToDeleteEndpoints.RemoveName(aEndpointName);
	}
    inline bool BaseServiceFullDisconnectAllEndpoints( const CheckEndpointPredT &pred)
    {
		bool   ret = false;
		ControlInquiry::FullDisconnectEndpoint   fullDiconnect;
		memset( (void *)&fullDiconnect, 0, sizeof(fullDiconnect) );
		fullDiconnect.ctx_ = new ControlInquiry::FullDisconnectEndpoint::context;
		fullDiconnect.ctx_->pred = pred;
        fullDiconnect.cmd = vs_tr_full_disconnect_all;
		ControlResponse::CntrlResponse   cntrlResponse;
		memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
        if (CntrlSR( (ControlInquiry *)&fullDiconnect, sizeof(fullDiconnect),
				(ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
			ret = cntrlResponse.res;
		return ret;
	}
    // end VS_TransportRouter_Implementation::BaseServiceFullDisconnectAllEndpoints

    inline bool IsAuthorized( const char * user_id )
    {
		if (!user_id || !*user_id) return false;
		if (threadId == GetCurrentThreadId()) { return GoIsAuthorize(user_id); }
        bool   ret = false;
        ControlInquiry::IsAuthorize   auth;
        memset( (void *)&auth, 0, sizeof(auth) );
        auth.cmd = vs_tr_isauthorize;
        if (strlen(user_id)>= VS_ACS_MAX_SIZE_ENDPOINT_NAME)
            return false;
        strcpy(auth.uid,user_id);
        ControlResponse::CntrlResponse   cntrlResponse;
        memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
        if (CntrlSR( (ControlInquiry *)&auth, sizeof(auth),
                     (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
            ret = cntrlResponse.res;
        return ret;
    }

    inline bool	AuthorizeClient(const char *uid, const char *new_uid)
    {
		if (!uid || !*uid || !new_uid || !*new_uid) return false;
		if (threadId == GetCurrentThreadId()) { return GoAuthorize(uid, new_uid); }
        bool   ret = false;
        ControlInquiry::Authorize   auth;
        memset( (void *)&auth, 0, sizeof(auth) );
        auth.cmd = vs_tr_authorize;
        if (strlen(uid)>= VS_ACS_MAX_SIZE_ENDPOINT_NAME)
            return false;
        if (strlen(new_uid)>= VS_ACS_MAX_SIZE_ENDPOINT_NAME)
            return false;
        strcpy(auth.uid,uid);
        strcpy(auth.new_uid,new_uid);
        ControlResponse::CntrlResponse   cntrlResponse;
        memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
        if (CntrlSR( (ControlInquiry *)&auth, sizeof(auth),
                     (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
            ret = cntrlResponse.res;
        return ret;
    }

	inline  bool UnauthorizeClient(const char *uid)
    {
		if (!uid || !*uid) return false;
		if (threadId == GetCurrentThreadId()) { return GoUnauthorize(uid); }
        bool   ret = false;
        ControlInquiry::Unauthorize   auth;
        memset( (void *)&auth, 0, sizeof(auth) );
        auth.cmd = vs_tr_unauthorize;
        if (strlen(uid)>= VS_ACS_MAX_SIZE_ENDPOINT_NAME)
            return false;
        strcpy(auth.uid,uid);
        ControlResponse::CntrlResponse   cntrlResponse;
        memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
        if (CntrlSR( (ControlInquiry *)&auth, sizeof(auth),
                     (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
            ret = cntrlResponse.res;
        return ret;
    }

	inline bool GoFullDisconnectAll(const unsigned long size)
    {
        bool res(false);
		auto ctx_ptr = std::unique_ptr<
			ControlInquiry::FullDisconnectEndpoint::context>(
				ci.fullDisconnectEndpoint.ctx_);
        if (size == sizeof(ControlInquiry::FullDisconnectEndpoint))
        {
            for (unsigned long i = 0; i <= maxIndEndpoints; ++i)
                if (endpoint[i]
					&& endpoint[i]->bothConn
					&& endpoint[i]->endpointID
					&& strcmp(endpoint[i]->endpointID, this->endpointName.c_str())
					&& ctx_ptr->pred(endpoint[i]->m_CID,
						endpoint[i]->endpointID,
						endpoint[i]->hops))
                {
					endpoint[i]->m_IsForcedDisconnect = true;
                    endpoint[i]->DisconnectMe();
                    orderToDeleteEndpoints.AddName( endpoint[i]->endpointID );
                }
            res = true;
        }
        cr.cntrlResponse.cmd = vs_tr_full_disconnect_all;
        cr.cntrlResponse.res = res;
        return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
    }

    inline bool GoFullDisconnectEndpoint(const unsigned long size )
    {
        bool res(false);
        if (size == sizeof(ControlInquiry::FullDisconnectEndpoint))
        {
			auto ctx_ptr = std::unique_ptr<
				ControlInquiry::FullDisconnectEndpoint::context>(
					ci.fullDisconnectEndpoint.ctx_);
            const char * aEndpointName = ctx_ptr->endpoint;
            if (VS_CheckEndpointName( aEndpointName ))
            {
                unsigned long terminal = GetEndpointIndex( aEndpointName );
                if (terminal < maxEndpoints)
                {
					endpoint[ terminal ]->m_IsForcedDisconnect = true;
					SendManagingMess(VS_TRANSPORT_MANAGING_DISCONNECT, aEndpointName, VS_TRANSPORT_TIMELIFE_DISCONNECT);
                    res = orderToDeleteEndpoints.AddName( aEndpointName );
                }
            }
        }
        cr.cntrlResponse.cmd = vs_tr_full_disconnect_endpoint;
        cr.cntrlResponse.res = res;
        return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
    }
    inline bool GoGetCIDByUID(const unsigned long size)
    {
        if (size != sizeof(ControlInquiry::GetCIDByUID)) return false;
        bool res(false);
        unsigned long index = GetEndpointIndexByUID(ci.getCIDByUID.uid);
        if (index < maxEndpoints) {
            *ci.getCIDByUID.cid = endpoint[index]->m_CID;
            res = true;
        }
        cr.cntrlResponse.cmd = vs_tr_getCIDByUID;
        cr.cntrlResponse.res = res;
        return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
    }
	inline bool GoGetIpByCID( const unsigned long size)
	{
		if (size != sizeof(ControlInquiry::GetIPByCID)) return false;

		cr.cntrlResponse.cmd = vs_tr_getIPByCID;
		cr.cntrlResponse.res = false;

		unsigned long i = GetEndpointIndex( ci.getIPByCID.cid );
		if ( i < maxEndpoints && endpoint[i]->bothConn)
		{
			const char* ip = endpoint[i]->bothConn->GetPeerIp();
			if (ip && strcmp(ip, "0.0.0.0") != 0)
			{
				cr.cntrlResponse.res = true;
				strncpy(ci.getIPByCID.ipbuff, ip, strlen(ip) + 1);
			}
		}
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}

    inline bool BaseServiceFullDisconnectEndpoint( const char *aEndpointName )
    {
        bool   ret = false;
        ControlInquiry::FullDisconnectEndpoint   fullDiconnect;
        memset( (void *)&fullDiconnect, 0, sizeof(fullDiconnect) );
        fullDiconnect.cmd = vs_tr_full_disconnect_endpoint;
		fullDiconnect.ctx_ = new ControlInquiry::FullDisconnectEndpoint::context;
        strncpy(fullDiconnect.ctx_->endpoint,aEndpointName,VS_ACS_MAX_SIZE_ENDPOINT_NAME-1);

        ControlResponse::CntrlResponse   cntrlResponse;
        memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
        if (CntrlSR( (ControlInquiry *)&fullDiconnect, sizeof(fullDiconnect),
                     (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
            ret = cntrlResponse.res;
        return ret;
    }
    inline std::string GetCIDByUID(const char *uid) override
    {
        std::string cid;
        bool ret = false;
        ControlInquiry::GetCIDByUID guid;
        guid.cmd = vs_tr_getCIDByUID;
        guid.uid = uid;
        guid.cid = &cid;
        ControlResponse::CntrlResponse   cntrlResponse = {};
        if (CntrlSR( (ControlInquiry *)&guid, sizeof(guid),
                     (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
            ret = cntrlResponse.res;
        if (!ret)
            cid.clear();
        return cid;
    }

	inline bool GetIPByCID(const char* cid, std::string& ip)
	{
		bool ret = false;
		ControlInquiry::GetIPByCID gip;
		gip.cmd = vs_tr_getIPByCID;
		gip.cid = cid;
		gip.ipbuff = (char *)malloc(100);

		ControlResponse::CntrlResponse   cntrlResponse = {};
		if (CntrlSR( (ControlInquiry *)&gip, sizeof(gip),
			(ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
			ret = cntrlResponse.res;

		if (ret)
			ip = gip.ipbuff;
		free( gip.ipbuff );

		return ret;
	}

	inline void CycleUnderPostMessages( )
    {
        while (nPostMess)
        {
            VS_RouterMessage   *mess = postMess[headPostMess];
            postMess[headPostMess] = 0;
            --nPostMess;
            if (++headPostMess >= maxPostMess)	headPostMess = 0;
            if (mess && mess->IsValid())
            {
                ProcessingMessages( mess );
            }
        }
        endPostMess = headPostMess = 0;
    }
    // end VS_TransportRouter_Implementation::CycleUnderMessages

    inline void CycleUnderMessages( VS_TransportRouter_Service *serv,
                                    VS_RouterMessage **sendMess, const unsigned long nSendMess )
    {
        for (unsigned long i = 0; i < nSendMess; ++i)
        {
            VS_RouterMessage   *mess = sendMess[i];
            sendMess[i] = 0;
            if (mess && mess->IsValid())
            {
                ProcessingMessages( mess );
            }
        }
    }
    // end VS_TransportRouter_Implementation::CycleUnderMessages

    inline bool IsOurEndpointName( const char *endpoint )
    {
        if (!endpoint || !*endpoint || strlen( endpoint ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)
            return false;
        return !strcmp( endpointName.c_str(), endpoint );
    }
    // end VS_TransportRouter_Implementation::IsOurEndpointName

    inline unsigned long GoAddServiceAct( const char *serviceName,
                                          VS_TransportRouterServiceBase *service ,
                                          bool isPermittedAll)
    {
        VS_TransportRouter_Service   *serv = GetService( serviceName );
        if (serv)		return ~0;
        unsigned long   index = GetFreeServiceIndex();
        if (index >= maxServices)		return ~0;
        serv = new VS_TransportRouter_Service( this, index, endpointName.c_str(), serviceName, service, true);
        if (!serv)		return ~0;
        if (!serv->isValid) {
            delete serv;
            return ~0;
        }
        VS_TransportRouter_Implementation::service[index] = serv;
        ++nServices;
        if (index > maxIndServices)		maxIndServices = index;
        if (isPermittedAll)
            m_permittedList.AddToList( serviceName );
        return index;
    }
    // end VS_TransportRouter_Implementation::GoAddServiceAct

	inline bool GoSetServCertInfoInterface(const unsigned long size)
	{
		if(size != sizeof(ControlInquiry::SetServCertInfoInterface))	return false;
		m_serversCertInfo = ci.setServCertInfoInterface.pInterface;
		cr.cntrlResponse.cmd = vs_tr_setServCertInfoInterface;
		cr.cntrlResponse.res = true;
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}

	inline bool GoRemoveRouterMessExtHandler(const unsigned long size)
	{
		if(size != sizeof(ControlInquiry::RemoveRouterMessExtHandler))
			return false;
		cr.cntrlResponse.cmd = vs_tr_remove_router_mess_ext_handler;
		m_message_ext_handlers.erase(ci.removeRouterMessExtHandler.ext_handler);
		cr.cntrlResponse.res = true;
		return icp->Write((const void*)&cr,sizeof(cr.cntrlResponse));
	}
	inline bool GoAddRouterMessExtHandler(const unsigned long size)
	{
		if(size != sizeof(ControlInquiry::AddRouterMessExtHandler))
			return false;
		cr.cntrlResponse.cmd = vs_tr_add_router_mess_ext_handler;
		if(!ci.addRouterMessExtHandler.ext_handler)
			cr.cntrlResponse.res = false;
		else
		{
			m_message_ext_handlers.insert(ci.addRouterMessExtHandler.ext_handler);
			cr.cntrlResponse.res = true;
		}
		return icp->Write((const void*)&cr, sizeof(cr.cntrlResponse));
	}

    inline bool GoDisconnectEndpointAct( const char *endpoint )
    {
        const unsigned long   index = GetEndpointIndex( endpoint );
		if (index < maxEndpoints)	orderToDeleteEndpoints.AddName(endpoint);
        return true;
    }
    // end VS_TransportRouter_Implementation::GoDisconnectEndpointAct

    inline bool GoDisconnectEndpoint( const unsigned long size )
    {
        if (size != sizeof(ControlInquiry::DisconnectEndpoint))		return false;
        const char   *endpoint = ci.disconnectEndpoint.endpoint;
        if (!endpoint || !*endpoint || strlen( endpoint ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)		return false;
        cr.cntrlResponse.cmd = vs_tr_disconnect_endpoint;
        cr.cntrlResponse.res = GoDisconnectEndpointAct( endpoint );
        return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
    }
    // end VS_TransportRouter_Implementation::GoDisconnectEndpoint

    inline bool GoAddService( const unsigned long size )
    {
        if (size != sizeof(ControlInquiry::AddService))		return false;
        const char   *serviceName = ci.addService.serviceName;
        const bool isPermittedAll = ci.addService.isPermittedAll;
        VS_TransportRouterServiceBase   *service = ci.addService.service;
        if (!serviceName || !*serviceName || strlen( serviceName ) > VS_TRANSPORT_MAX_SIZE_SERVICE_NAME
                || !service || service->imp)		return false;
        cr.cntrlResponse.cmd = vs_tr_add_service;
        cr.cntrlResponse.res = GoAddServiceAct( serviceName, service , isPermittedAll ) < maxServices;
        return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
    }
    // end VS_TransportRouter_Implementation::GoAddService

    inline bool GoAddCallService( const unsigned long size )
    {
        if (size != sizeof(ControlInquiry::AddCallService))		return false;
        const char   *serviceName = ci.addCallService.serviceName;
        const bool isPermittedAll = ci.addCallService.permittedForAll;
        VS_TransportRouterServiceBase   *callService = ci.addCallService.callService;
        if (!serviceName || !*serviceName || strlen( serviceName ) > VS_TRANSPORT_MAX_SIZE_SERVICE_NAME
                || !callService || callService->imp)	return false;
        VS_TransportRouter_Service   *serv = GetService( serviceName );
        if (serv)	serv = 0;
        else
        {
            unsigned long   index = GetFreeServiceIndex();
            if (index < maxServices)
            {
                serv = new VS_TransportRouter_Service( this, index, endpointName.c_str(), serviceName, callService, false );
                if (serv)
                {
                    if (!serv->isValid) {
                        delete serv;
                        serv = 0;
                    }
                    else
                    {
                        service[index] = serv;
                        ++nServices;
                        if (isPermittedAll)
                            m_permittedList.AddToList( serviceName );
                        if (index > maxIndServices)		maxIndServices = index;
                    }
                }
            }
        }
        cr.cntrlResponse.cmd = vs_tr_add_call_service;
        cr.cntrlResponse.res = serv != 0;
        return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
    }
    // end VS_TransportRouter_Implementation::GoAddCallService

    inline bool GoAuthorize( const unsigned long size )
    {
        if (size != sizeof(ControlInquiry::Authorize))	return false;
        unsigned long index = GetEndpointIndex( ci.authorize.uid );
        bool res = false;
        if (index <= maxIndEndpoints)
        {
            if (endpoint[index]->hops)
                res = endpoint[index]->AuthorizeServer();
            else
                res = endpoint[index]->AuthorizeEndpoint(ci.authorize.new_uid);
        }
        cr.cntrlResponse.cmd = vs_tr_authorize;
        cr.cntrlResponse.res = res;
        return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
    }

    inline bool GoAuthorize(const char *uid, const char *new_uid)
    {
        unsigned long index = GetEndpointIndex( uid );
        bool res = false;
        if (index <= maxIndEndpoints)
        {
            if (endpoint[index]->hops)
                res = endpoint[index]->AuthorizeServer();
            else
                res = endpoint[index]->AuthorizeEndpoint(new_uid);
        }
		return res;
    }

	inline bool GoIsAuthorize( const unsigned long size )
    {
        if (size != sizeof(ControlInquiry::IsAuthorize))	return false;
        VS_TransportRouter_Endpoint *p(GetEndpointByUID( ci.isauthorize.uid ));
        bool res = p && p->isAuthorizedEndpoint();
        cr.cntrlResponse.cmd = vs_tr_isauthorize;
        cr.cntrlResponse.res = res;
        return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );

    }

	inline bool GoIsAuthorize (const char * user_id)
	{
		VS_TransportRouter_Endpoint *p(GetEndpointByUID( user_id ));
		return p && p->isAuthorizedEndpoint();
	}

    inline bool GoUnauthorize( const unsigned long size )
    {
        if (size != sizeof(ControlInquiry::Unauthorize))	return false;
        unsigned long index = GetEndpointIndex( ci.unauthorize.uid );
        bool res = false;
        if (index <= maxIndEndpoints)
        {
            res = endpoint[index]->UnauthorizeEndpoint();
        }
        cr.cntrlResponse.cmd = vs_tr_unauthorize;
        cr.cntrlResponse.res = res;
        return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );

    }

    inline bool GoUnauthorize( const char *uid )
    {
        unsigned long index = GetEndpointIndex( uid );
        bool res = false;
        if (index <= maxIndEndpoints)
        {
            res = endpoint[index]->UnauthorizeEndpoint();
        }
        return res;
    }

    inline bool GoRemoveService( const unsigned long size )
    {
        if (size != sizeof(ControlInquiry::RemoveService))	return false;
        const char   *serviceName = ci.removeService.serviceName;
        if (!serviceName || !*serviceName || strlen( serviceName ) > VS_TRANSPORT_MAX_SIZE_SERVICE_NAME)
            return false;
        DeleteService( serviceName );
        cr.cntrlResponse.cmd = vs_tr_delete_service;
        return icp->Write( (const void *)&cr.cntrlResponse.cmd, sizeof(cr.cntrlResponse.cmd) );
    }
    // end VS_TransportRouter_Implementation::GoRemoveService

    inline void ReplyHandshakeDeleteConn( VS_Connection *conn,
                                           const unsigned char resultCode )
    {
        ++observer.crt[5];
		++observer.crt[8];
		++observer.crt[10];
        if (trDebugLog && trDebugLog->IsValid())
        {
            trDebugLog->TPrintf("\n\t ReplyHandshakeDeleteConn CON: 0x%X RejectType: %d",
                               conn,resultCode);
        }

        net::HandshakeHeader* hs = nullptr;
		if (resultCode == hserr_oldarch) {
			hs = VS_FormTransportReplyHandshake___OLDARCH();
		} else {
			hs = VS_FormTransportReplyHandshake(
				VS_TransportRouter_Implementation::endpointName.c_str(),
				resultCode,	0, 0, 0,
				VS_TransportRouter_Implementation::endpointName.c_str(), "user" );
		}
        if (!hs)	return;
        conn->SetOvReadFields( VS_TR_CONNECTION_DEAD_READ, (VS_ACS_Field)conn );
        conn->SetOvWriteFields( VS_TR_CONNECTION_DEAD_WRITE, (VS_ACS_Field)conn );
        VS_Buffer bfs[] = { sizeof(net::HandshakeHeader) + hs->body_length + 1, (void *)hs };
        if (!conn->RWrite( bfs, 1 ))
        {
            DeleteConn( conn ,"ReplyHandshakeDeleteConn" );
            printf("\n\t Replay has rotted!");
        } else
        {
            AddRouteDeleteConn( conn );
        }
        free( (void *)hs );

    }
    // end VS_TransportRouter_Implementation::ReplyHandshakeDeleteConn

    inline void SendManagingMess( const char type, const char *targetEndpointName, const unsigned long lifetime )
    {
		VS_RouterMessage *mess = new VS_RouterMessage;
		if (mess) {
			const char opcode[] = { type, '\0' };
			*mess = transport::Message(true, ~0, lifetime, {}, {}, opcode, {}, {}, {}, {}, targetEndpointName, type == VS_TRANSPORT_MANAGING_PING ? "" : endpointName, "", 1);
			if (mess->IsValid()) {
				ProcessingMessages( mess );
			} else {
				delete mess;
			}
		}
    }
    // end VS_TransportRouter_Implementation::SendManagingMess

    inline bool GoSetConnection( const unsigned long size )
    {
        ++observer.crt[0];
        {
            aSetConn = bSetConn =0;
            DiagnosticTimer a( &transportStatistic );
            if (size != sizeof(ControlInquiry::SetConnection))
                return false;

            const char   *cid = ci.setConnection.cid;
            const char   *sid = ci.setConnection.sid;

            unsigned long   version = ci.setConnection.version;
            VS_Connection   *conn = ci.setConnection.conn;
            const bool   isAccept = ci.setConnection.isAccept;
            const unsigned short   maxConnSilenceMs = ci.setConnection.maxConnSilenceMs;
            const unsigned char   fatalSilenceCoef = ci.setConnection.fatalSilenceCoef;
			const bool tcpKeepAliveSupport = ci.setConnection.tcpKeepAliveSupport;
            const unsigned char   hops = ci.setConnection.hops;
			const bool	 hs_error = ci.setConnection.hs_error;
			const unsigned long rnd_data_ln = ci.setConnection.rnd_data_ln;
			const unsigned long sign_ln = ci.setConnection.sign_ln;

            if (!cid || strlen( cid ) > VS_ACS_MAX_SIZE_CID)
                return false;

            if (!sid || strlen( sid ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)
                return false;
			if(rnd_data_ln>VS_RND_DATA_FOR_SIGN_SZ || sign_ln>VS_SIGN_SIZE)
				return false;

			const unsigned char * rnd_data = ci.setConnection.rnd_data;
			const unsigned char * sign = ci.setConnection.sign;

            const unsigned long   index0 = GetEndpointIndex( cid );

            if (trDebugLog && trDebugLog->IsValid())
                trDebugLog->TPrintf("\n\t GoSetConnection : CID:'%s' SID:'%s' Conn: 0x%X Ep_index: %d",cid, sid, /*type,*/ conn,index0);

            unsigned char   resultCode = hserr_ok;
            if (!conn)//// когда не смогли приконнектиться
            {
                const unsigned long   index = GetEndpointIndex( cid );
                if (index < maxEndpoints
					&& !endpoint[index]->SetConnection(version, conn, isAccept, maxConnSilenceMs, fatalSilenceCoef,0, tcpKeepAliveSupport))
				{
					DeleteEndpoint( index, (hs_error)? VS_PointParams::CR_HSERROR: VS_PointParams::CR_TIMEOUT, (endpoint[index]->lastReadTm || endpoint[index]->lastWriteTm)? true: false);
				}
                return true;
            }
			if (!conn->SetIOCP( (void *)hiocp ))
            {
				conn->SetOvFildIOCP( (const void *)hiocp );
            }
			// Fake Client uses Pipe connections, they are not based on sockets.
			auto IsPipeConnection = [&conn](void) -> bool
			{
				VS_ConnectionPipe *cp = dynamic_cast<VS_ConnectionPipe *>(conn);
				if (cp != NULL)
				{
					return true;
				}
				return false;
			};
			using pk_res = VS_ServCertInfoInterface::get_info_res;
			bool verify(true);
			auto get_pk_res = pk_res::undef;
			unsigned secure_hs_version = 1;
			unsigned int min_ver = 361;
			unsigned int cur_ver = atou_s(VCS_SERVER_VERSION);
			if(hops&&m_serversCertInfo)
			{
				verify = false;
				VS_SimpleStr	pubKey;
				VS_Sign			verifier;
				VS_SignArg		signarg = {alg_pk_RSA,alg_hsh_SHA1};
				uint32_t        srv_ver(0);

				if (verifier.Init(signarg)
					&& pk_res::ok
					== (get_pk_res = m_serversCertInfo->GetPublicKey(
						cid, pubKey, srv_ver))
					&& verifier.SetPublicKey(pubKey, pubKey.Length(),
						store_PEM_BUF)
					&& verifier.VerifySign(rnd_data, rnd_data_ln, sign, sign_ln))
				{
					verify = true;
				}
				else if(pk_res::auto_verify == get_pk_res)
					verify = true;
				else
					dstream4 << "got PubKey = "<< pubKey.m_str
					<<"; cid:"<<cid<<"; res="<<static_cast<int32_t>(get_pk_res)
					<<"; CERT OR SIGN IS NOT CORRECT!!\n";

				secure_hs_version = (srv_ver > min_ver)?2:1;
			}
			else if(hops&&min_ver<cur_ver)
				secure_hs_version = 2;

			if (!conn->GetTlsContext() &&
				(version & VS_SSL_SUPPORT_BITMASK) == 0
				&& !IsPipeConnection()) // allow unencrypted connections via pipe for FakeClient.
			{
				resultCode = hserr_ssl_required;
			}
			else if ((version & ~VS_SSL_SUPPORT_BITMASK) < VS_NEWARCH_TRANSPORT_VERSION)
			{
				resultCode = hserr_oldarch;
			}
			else if ((version & ~VS_SSL_SUPPORT_BITMASK) < VS_MIN_TRANSPORT_VERSION)
			{
				resultCode = hserr_antikyou;
			}
			else if (pk_res::auto_deny == get_pk_res
				|| (!verify
					&& ((pk_res::ok == get_pk_res)
						|| ((pk_res::key_is_absent == get_pk_res)
							&& (rnd_data_ln != 0 || sign_ln != 0)))))
			{
				resultCode = hserr_verify_failed;
			}
			else if (!verify
				&& pk_res::db_error == get_pk_res)
			{
				resultCode = hserr_busy;
			}
			else if (!hops && !m_Cert)
			{
				resultCode = hserr_busy;
			}
            else
            {
                unsigned long   index(-1);
                // detect: client OR server
                char new_cid[VS_ACS_MAX_SIZE_CID] = {0};

                if (hops)						// if server then hops = 1
                {
                    index = AddEndpointIndex( 0, cid, hops );
                    if (index >= maxEndpoints)
                        resultCode = hserr_busy;
                    else
                    {
                        strcpy(new_cid,cid);
						if(verify)
							endpoint[index]->AuthorizeServer();
						else
							endpoint[index]->UnauthorizeServer();
					}
                }
                else			// if client then hops = 0
                {
                    GetCorrectCID(cid, new_cid);
                    index = AddEndpointIndex( new_cid, 0, hops);
                }

                if (index >= maxEndpoints)
                    resultCode = hserr_busy;
				else if(hops&&strcasecmp(endpointName.c_str(),sid))
				{
					resultCode = hserr_alienserver;
				}
                else
                {
                    aSetConn = GetTickCount();

					if (!endpoint[index]->SetConnection( version, conn, isAccept, maxConnSilenceMs, fatalSilenceCoef,secure_hs_version, tcpKeepAliveSupport))
                    {
                        DeleteEndpoint( index );
                        bSetConn = GetTickCount();
                        if (bSetConn - aSetConn >1000)
                            dprint4("SetConn Lasts: %ld", bSetConn - aSetConn);

                        return true;
                    }
                    bSetConn = GetTickCount();
                    if (bSetConn - aSetConn >1000)
                        dprint4("SetConn Lasts: %ld", bSetConn - aSetConn);

					if ( conn->GetConnectDirection() == vs_sock_type_accept )
						OnEndpointConnect( index, VS_PointParams::CR_INCOMING );
					else
						OnEndpointConnect( index, VS_PointParams::CR_REQUESTED );

                    observer.crt[4] = (GetTickCount() - conn->GetEventTime());
                    return true;
                }
            }
            ReplyHandshakeDeleteConn( conn, resultCode );
        }
        return true;
    }
    inline void GetCorrectCID(const char *in_cid, char *out_cid)
    {
        char loc_cid[VS_ACS_MAX_SIZE_CID] = {0};
        unsigned long index(0);
        if (!in_cid || !*in_cid)
        {

            while (index!=-1)
            {
                VS_GenKeyByMD5(loc_cid);
                index = GetEndpointIndex(loc_cid);
            }
            strcpy(out_cid,loc_cid);

        }
        else
        {
            if (-1 != GetEndpointIndex(in_cid))
                strcpy(out_cid,in_cid);
            else
            {
                while (index!=-1)
                {
                    VS_GenKeyByMD5(loc_cid);
                    index = GetEndpointIndex(loc_cid);
                }
                strcpy(out_cid,loc_cid);
            }
        }
    }
    inline bool GoGetStatisticsAct( VS_TransportRouterStatistics &stat )
    {
        const unsigned __int64   diffSendBytes = ( sendBytes - lastSendBytes ) * 1000;
        lastSendBytes = sendBytes;
        const unsigned __int64   diffReceiveBytes = ( receiveBytes - lastReceiveBytes ) * 1000;
        lastReceiveBytes = receiveBytes;
        const unsigned long   diffTm = currTm - lastBitrateTm, addDiffTm = ( diffTm + 1 ) / 2;
        lastBitrateTm = currTm;
        stat.endpoints = nEndpoints;
        stat.out_bytes = out_bytes += lastSendBytes;
        stat.in_bytes = in_bytes += lastReceiveBytes;
        if (!diffTm)
        {
            return true;
        }
        else
        {
            stat.out_byterate = (float)(( diffSendBytes + addDiffTm ) / diffTm );
            stat.in_byterate = (float)(( diffReceiveBytes + addDiffTm ) / diffTm );
        }
        return true;
    }
    // end VS_TransportRouter_Implementation::GoGetStatisticsAct

    inline bool GoGetStatistics( const unsigned long size )
    {
        if (size != sizeof(ControlInquiry::CntrlInquiry))	return false;
        cr.getStatistics.cmd = vs_tr_get_statistics;
        cr.getStatistics.res = GoGetStatisticsAct( cr.getStatistics.stat );
        return icp->Write( (const void *)&cr.getStatistics, sizeof(cr.getStatistics) );
    }
    // end VS_TransportRouter_Implementation::GoGetStatistics

	inline bool GoAddProcessingMes( const unsigned long size )
    {
        if (size != sizeof(ControlInquiry::AddProcessingMes))	return false;
        VS_RouterMessage   *mess = ci.addProcessingMes.mess;
        ProcessingMessages( mess );
        return true;
    }
    // end VS_TransportRouter_Implementation::GoAddProcessingMes

	inline bool GoServerVerificationFailed(const unsigned long size)
	{
		unsigned long i = GetServiceIndex(CHECKLIC_SRV);
		if(i < maxServices)
		{
			void *buf(0);
			size_t sz(0);
			VS_Container	cnt;
			cnt.AddValue(METHOD_PARAM,SERVERVERIFYFAILED_METHOD);
			cnt.SerializeAlloc(buf,sz);
			VS_TransportRouterServiceBase   *s = service[i]->servBase;


			VS_RouterMessage *mess = new VS_RouterMessage(s->OurService(),0,s->OurService(),0,0,s->OurEndpoint(),s->OurEndpoint(),~0,buf,sz);
			free(buf);
			if(!s->SendMes(mess))
				delete mess;
		}
		else
		{
			VS_RegistryKey	l_root(VS_Server::RegistryKey(),false, LICENSE_KEY, false, true);
			VS_RegistryKey	l_reg;
			if(l_root.IsValid())
			{
				std::vector<std::string> to_delete;
				l_root.ResetKey();
				while (l_root.NextKey(l_reg))
					to_delete.emplace_back(l_reg.GetName());
				for (const auto& x: to_delete)
					l_root.RemoveKey(x);
			}
			VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
			rkey.RemoveValue(SRV_CERT_KEY);
			rkey.RemoveValue(SRV_PRIVATE_KEY);

			m_watchDog->Shutdown();
		}
		return true;
	}

    inline bool GoIsThereEndpointAct( const char endpoint[] )
    {
        return GetEndpointIndex( endpoint ) < maxEndpoints;
    }
    // end VS_TransportRouter_Implementation::GoIsThereEndpointAct

    inline bool GoIsThereEndpoint( const unsigned long size )
    {
        if (size != sizeof(ControlInquiry::IsThereEndpoint))	return false;
        const char   *endpoint = ci.isThereEndpoint.endpoint;
        cr.cntrlResponse.cmd = vs_tr_is_there_endpoint;
        cr.cntrlResponse.res = GoIsThereEndpointAct( endpoint );
        return icp->Write( (const void *)&cr.cntrlResponse, sizeof(cr.cntrlResponse) );
    }
    // end VS_TransportRouter_Implementation::GoIsThereEndpoint
	inline bool PrepareAndStoreRequestResponse(VS_RouterMessage *m, ResponseNotifyMethodT &&notify, RequestLifeTimeT &&lt)
	{
		std::stringstream ss;
		ss << "REQRESP: " << std::hex << std::chrono::steady_clock::now().time_since_epoch().count();
		auto name = ss.str();
		const auto sz = name.length();
		auto counter = 0;
		while (m_waiting_for_response_storage.count(name) > 0)
		{
			if (name.length() == sz)
			{
				name += "#";
				name += std::to_string(counter);
			}
			else
				name.replace(name.begin() + sz + 1, name.end(), std::to_string(counter));
			++counter;
		}
		bool bres = m->SetSrcService(name.c_str());
		string_view src_service = m->SrcService();
		string_view dst_service = m->DstService();
		const RequestLifeTimeT msg_timelife = std::chrono::milliseconds(m->TimeLimit());
		dstream4 << "Request sent and stored. Wait for response. SrcService (RequestID) = " << name << "; DstServer = " << m->DstServer() << "; DstService =  " << m->DstService();
		auto inserted = m_waiting_for_response_storage.emplace(std::move(name), std::move(notify), std::chrono::steady_clock::now() + std::max(lt, msg_timelife));
		if (!CallServicePostMes(m))
		{
			dstream4 << "Request didn't send. Revoke waiting for response\n";
			if (inserted.second)
				m_waiting_for_response_storage.erase(inserted.first);
			return false;
		}
		return true;
	}
	inline bool GoRequestResponseFuture(const unsigned long size)
	{
		if (size != sizeof(ControlInquiry::RequestResponseFuture)) return false;
		std::unique_ptr<ControlInquiry::RequestResponseFuture::context> ctx_ptr(ci.requestResponseFuture.ctx_);
		cr.cntrlResponse.cmd = vs_tr_request_response_future;
		cr.cntrlResponse.res = ctx_ptr ? PrepareAndStoreRequestResponse(ctx_ptr->msg_, std::move(ctx_ptr->promise_), std::move(ctx_ptr->lifetime_)) : false;
		return icp->Write((const void *)&cr.cntrlResponse, sizeof(cr.cntrlResponse));
	}
	inline bool GoRequestResponseCallBack(const unsigned long size)
	{
		if (size != sizeof(ControlInquiry::RequestResponseCallBack)) return false;
		std::unique_ptr<ControlInquiry::RequestResponseCallBack::context> ctx_ptr(ci.requestResponseCallBack.ctx_);
		if (ctx_ptr)
			return PrepareAndStoreRequestResponse(ctx_ptr->msg_, std::move(ctx_ptr->cb_), std::move(ctx_ptr->lifetime_));
		else
			return false;
	}
    inline bool GoControlInquiry( const unsigned long size )
    {
        switch (ci.cntrlInquiry.cmd)
        {
		case vs_tr_setServCertInfoInterface :
			if(!GoSetServCertInfoInterface(size))	return false;
			break;
        case vs_tr_disconnect_endpoint :
            if (!GoDisconnectEndpoint( size ))		return false;
            break;
        case vs_tr_add_service :
            if (!GoAddService( size ))				return false;
            break;
        case vs_tr_add_call_service :
            if (!GoAddCallService( size ))			return false;
            break;
        case vs_tr_delete_service :
            if (!GoRemoveService( size ))			return false;
            break;
        case vs_tr_set_connection :
            if (!GoSetConnection( size ))		return false;
            break;
        case vs_tr_get_statistics :
            if (!GoGetStatistics( size ))			return false;
            break;
        case vs_tr_authorize:
            if (!GoAuthorize( size ))				return false;
            break;
        case vs_tr_unauthorize:
            if (!GoUnauthorize( size ))				return false;
            break;
        case vs_tr_isauthorize :
            if (!GoIsAuthorize( size ))				return false;
            break;
        case vs_tr_add_processing_mes :
            if (!GoAddProcessingMes( size ))		return false;
            break;
		case vs_tr_serverVerificationFailed	:
			if(!GoServerVerificationFailed( size ))	return false;
			break;
        case vs_tr_is_there_endpoint :
            if (!GoIsThereEndpoint( size ))			return false;
            break;
        case vs_tr_full_disconnect_all :
            if (!GoFullDisconnectAll( size ))		return false;
            break;
        case vs_tr_full_disconnect_endpoint :
            if (!GoFullDisconnectEndpoint( size ))	return false;
            break;
		case vs_tr_getCIDByUID :
			if (!GoGetCIDByUID( size ))				return false;
			break;
		case vs_tr_getIPByCID :
			if (!GoGetIpByCID( size ))				return false;
			break;
		case vs_tr_addMsgStatistics :
			if (!GoAddMessageStatistics( size ))	return false;
            break;
		case vs_tr_add_router_mess_ext_handler :
			if(!GoAddRouterMessExtHandler(size))	return false;
			break;
		case vs_tr_remove_router_mess_ext_handler :
			if(!GoRemoveRouterMessExtHandler(size))	return false;
			break;
		case vs_tr_request_response_future:
			if (!GoRequestResponseFuture(size))	return false;
				break;
		case vs_tr_request_response_callback:
			if (!GoRequestResponseCallBack(size)) return false;
			break;
        default :
            return false;
        }
        memset( (void *)&ci, 0, sizeof(ci) );
        return !icp->IsWrite() ? icp->Read( (void *)&ci, sizeof(ControlInquiry) ) : true;
    }
    // end VS_TransportRouter_Implementation::GoControlInquiry

    inline void GoMonitorInquiry( const unsigned long size )
    {
        if (size != tmReadSize)
        {
            ResetToConnectMcp();
            return;
        }
        switch (tmRequest->type)
        {
        case TM_TYPE_UNKNOWN :
            break;
        default:
            ResetToConnectMcp();
        }
    }
    // end VS_TransportRouter_Implementation::GoMonitorInquiry

    inline	void GoBothConnectionWrite( unsigned long bTrans, VS_Overlapped &ov)
    {
        const unsigned long	index = (const unsigned long)ov.field2;
        if (index >= maxEndpoints)
            return;
        VS_TransportRouter_Endpoint	*ep = endpoint[index];
        if (!ep)
            return;
        ep->BothWrite(bTrans, ov);
    }
    // end VS_TransportRouter_Implementation::GoBothConnectionWrite


	inline void GoBothSecureHandshakeRead(unsigned long bTrans, VS_Overlapped &ov)
	{
		const unsigned long index = (const unsigned long)ov.field2;
		if(index >= maxEndpoints)
			return;
		VS_TransportRouter_Endpoint *ep = endpoint[index];
		if(!ep)
			return;
		ep->SecureHandshakeRead(bTrans,ov);
	}

	inline void GoBothSecureHandshakeWrite(unsigned long bTrans, VS_Overlapped &ov)
	{
		const unsigned long index = (const unsigned long)ov.field2;
		if(index >= maxEndpoints)
			return;
		VS_TransportRouter_Endpoint *ep = endpoint[index];
		if(!ep)
			return;
		ep->SecureHandshakeWrite(bTrans,ov);
	}

    inline void GoBothConnectionRead( unsigned long bTrans, VS_Overlapped &ov )
    {
        const unsigned long index = (const unsigned long)ov.field2;
        if (index >= maxEndpoints)
            return;
        VS_TransportRouter_Endpoint	*ep = endpoint[index];
        if (!ep)
            return;
        VS_RouterMessage	*mess = 0;
        unsigned long tick1(GetTickCount()), tick2;
        ep->BothRead( bTrans, ov, mess);
        tick2 = GetTickCount() - tick1;
        if (tick2 > maxTimePerOperation)
        {
            dprint2("ep->BothRead duration %ld", tick2);
        }
        if (mess)
        {
            anti_cycle_rcv_endpoint = ep;

            tick1 = GetTickCount();
            ProcessingMessages(mess);
            tick2 = GetTickCount() - tick1;
            if (tick2 > maxTimePerOperation)
            {
                dprint2("ProccessingMessage duration = %ld", tick2);
            }
        }
    }
    // end VS_TransportRouter_Implementation::GoBothConnectionRead

    inline void GoPipeServiceWrite( unsigned long bTrans, VS_Overlapped &ov )
    {
        const unsigned long   index = (const unsigned long)ov.field2;
        if (index >= maxServices)
        {	/* Here it will be necessary to throw off in TRACE */
            return;
        }
        VS_TransportRouter_Service   *serv = service[index];
        if (!serv)
        {	/* Here it will be necessary to throw off in TRACE */
            return;
        }
        VS_TransportRouterPipeService   *pipeService = serv->pipeService;
        if (!pipeService)
        {	/* Here it will be necessary to throw off in TRACE */
            return;
        }
        if (!pipeService->PipeWrite( bTrans, ov ))
        {	/* Here it will be necessary to throw off in TRACE */
            dprint4("GoPipeServiceWrite PipeWrite failed; Delete service index = %ld; Err = %ld;", index, GetLastError());
            DeleteService( index );
        }
    }
    // end VS_TransportRouter_Implementation::GoPipeServiceWrite

    inline void GoPipeServiceRead( unsigned long bTrans, VS_Overlapped &ov )
    {
        const unsigned long   index = (const unsigned long)ov.field2;
        if (index >= maxServices)
        {	/* Here it will be necessary to throw off in TRACE */
            return;
        }
        VS_TransportRouter_Service   *serv = service[index];
        if (!serv)
        {	/* Here it will be necessary to throw off in TRACE */
            return;
        }
        VS_TransportRouterPipeService   *pipeService = serv->pipeService;
        if (!pipeService)
        {	/* Here it will be necessary to throw off in TRACE */
            return;
        }
        VS_RouterMessage   *mess = 0;
        if (!pipeService->PipeRead( bTrans, ov, mess ))
        {	/* Here it will be necessary to throw off in TRACE */
            dprint4("GoPipeServiceRead PipeRead failed; Delete service index = %ld; Err = %ld;", index, GetLastError());
            DeleteService( index );
        }
        if (mess)	ProcessingMessages( mess );
    }
    // end VS_TransportRouter_Implementation::GoPipeServiceRead

    inline void GoConnectionDeadWrite( unsigned long bTrans, VS_Overlapped &ov )
    {
#ifdef _MY_DEBUG_
        printf("VS_StreamsRouter_Implementation::GoConnectionDeadWrite( bTrans: %lu, &ov: %p ):", bTrans, &ov);
#endif
        VS_ConnectionSock   *conn = (VS_ConnectionSock *)ov.field2;
        if (!conn)
        {
#ifdef _MY_DEBUG_
            puts( "!(*conn = (VS_ConnectionSock *)ov.field2)" );
#endif
            return;
        }
        conn->SetWriteResult( bTrans, &ov );
        if (!conn->IsRead())
        {
            DeleteConn( conn , "GoConnectionDeadWrite",0,true );
        }
    }
    // end VS_TransportRouter_Implementation::GoConnectionDeadWrite

    inline void GoConnectionDeadRead( unsigned long bTrans, VS_Overlapped &ov )
    {
#ifdef _MY_DEBUG_
        printf("VS_StreamsRouter_Implementation::GoConnectionDeadRead( bTrans: %lu, &ov: %p ):", bTrans, &ov);
#endif
        VS_ConnectionSock   *conn = (VS_ConnectionSock *)ov.field2;
        if (!conn)
        {
#ifdef _MY_DEBUG_
            puts( "!(*conn = (VS_ConnectionSock *)ov.field2)" );
#endif
            return;
        }
        conn->SetReadResult( bTrans, &ov );
        if (!conn->IsWrite())
        {
            DeleteConn( conn  , "GoConnectionDeadRead",0,true);
        }
    }
    // end VS_TransportRouter_Implementation::GoConnectionDeadRead

    inline bool GoControlWrite( unsigned long bTrans, VS_Overlapped &ov )
    {
        if (ov.error)	return false;
        if (icp->SetWriteResult( bTrans, &ov ) < 0)		return false;
        return icp->Read( (void *)&ci, sizeof(ControlInquiry) );
    }
    // end VS_TransportRouter_Implementation::GoControlWrite

    inline bool GoControlRead( unsigned long bTrans, VS_Overlapped &ov )
    {
        if (ov.error)	return false;
        int   ret = icp->SetReadResult( bTrans, &ov, 0, true );
        if (ret < (int)sizeof(ci.cntrlInquiry.cmd))		return false;
        unsigned long cmd = ci.cntrlInquiry.cmd;
        unsigned long a (GetTickCount());
        bool resultCR =  GoControlInquiry( (const unsigned long)ret );
        unsigned long b ( GetTickCount());
        if (b-a>maxTimePerOperation)
        {
            dprint4("TR: Router operation %ld lasts %ld", cmd, b-a);
        }
        return resultCR;
    }
    // end VS_TransportRouter_Implementation::GoControlRead

    inline void GoMonitorWrite( unsigned long bTrans, VS_Overlapped &ov )
    {
        if (mcp->SetWriteResult( bTrans, &ov ) < 0)		ResetToConnectMcp();
        flagMcpWrite = false;
    }
    // end VS_TransportRouter_Implementation::GoMonitorWrite

    inline void GoMonitorRead( unsigned long bTrans, VS_Overlapped &ov )
    {
        GoMonitorInquiry( (const unsigned long)mcp->SetReadResult( bTrans, &ov ) );
    }
    // end VS_TransportRouter_Implementation::GoMonitorRead

    inline void GoMonitorConnect( unsigned long bTrans, VS_Overlapped &ov )
    {
        if (mcp->SetConnectResult( bTrans, &ov ))
        {
            mcp->SetOvWriteFields( (const VS_ACS_Field)VS_TR_MONITOR_WRITE );
            mcp->SetOvReadFields( (const VS_ACS_Field)VS_TR_MONITOR_READ );
            tmRequest = new TmRequest;
            if (!tmRequest) {
                ResetToConnectMcp();
                return;
            }
            tmReply = new TmReply;
            if (!tmReply) {
                ResetToConnectMcp();
                return;
            }
            if (!mcp->Read( (void *)&tmRequest->type, sizeof(tmRequest->type) ))
            {
                ResetToConnectMcp();
                return;
            }
            tmReadState = 0;
            tmReadSize = sizeof(tmRequest->type);
            flagMcpConnect = true;
        }
    }
    // end VS_TransportRouter_Implementation::GoMonitorConnect

    inline bool RunHandle( unsigned long bTrans, VS_Overlapped &ov )
    {
        switch ((unsigned long)ov.field1)
        {
        case VS_TR_BTH_CONNECTION_WRITE		 :
            GoBothConnectionWrite( bTrans, ov );
            return true;
        case VS_TR_BTH_CONNECTION_READ		 :
            GoBothConnectionRead( bTrans, ov);
            return true;
        case VS_TR_PIPE_SERVICE_WRITE        :
            GoPipeServiceWrite( bTrans, ov );
            return true;
        case VS_TR_PIPE_SERVICE_READ         :
            GoPipeServiceRead( bTrans, ov );
            return true;
        case VS_TR_CONNECTION_DEAD_WRITE     :
            GoConnectionDeadWrite( bTrans, ov );
            return true;
        case VS_TR_CONNECTION_DEAD_READ      :
            GoConnectionDeadRead( bTrans, ov );
            return true;
        case VS_TR_CONTROL_WRITE             :
            return GoControlWrite( bTrans, ov );
        case VS_TR_CONTROL_READ              :
            return GoControlRead( bTrans, ov );
        case VS_TR_MONITOR_WRITE             :
            GoMonitorWrite( bTrans, ov );
            return true;
        case VS_TR_MONITOR_READ              :
            GoMonitorRead( bTrans, ov );
            return true;
        case VS_TR_MONITOR_CONNECT           :
            GoMonitorConnect( bTrans, ov );
            return true;
		case VS_TR_BTH_SECURE_HANDSHAKE_READ	:
			GoBothSecureHandshakeRead(bTrans,ov);
			return true;
		case VS_TR_BTH_SECURE_HANDSHAKE_WRITE	:
			GoBothSecureHandshakeWrite(bTrans,ov);
			return true;
        default :
            /* Here it will be necessary to throw off in TRACE */
            return true;
        }
    }
    // end VS_TransportRouter_Implementation::RunHandle

    inline void ProcessingPeriodic( void )
    {

        if (!mcp)
            ResetToConnectMcp();
        if (flagMcpWrite)
            return;
        switch (statePeriodic)
        {
        case 0 :
            observer.crt[14] = 0;
            observer.crt[15] = 0;
            if (flagMcpConnect)
            {
#ifdef _MY_DEBUG_
                puts( "VS_TransportRouter_Implementation::ProcessingPeriodic: case 0: WriteMcp( ... )" );
#endif
                tmReply->type = TM_TYPE_PERIODIC_START;
                WriteMcp( (const void *)tmReply, sizeof(tmReply->type) );
            }
            statePeriodic = 1;
            return;
        case 1 :
            if (flagMcpConnect)
            {
#ifdef _MY_DEBUG_
                puts( "VS_TransportRouter_Implementation::ProcessingPeriodic: case 1: WriteMcp( ... )" );
#endif
                TmReply::StartEndpoints   &startEndpoints = tmReply->startEndpoints;
                startEndpoints.type = TM_TYPE_PERIODIC_START_ENDPOINTS;
                startEndpoints.maxEndpoints = maxEndpoints;
                WriteMcp( (const void *)tmReply, sizeof(tmReply->startEndpoints) );
            }
            statePeriodic = 2;
            indexEndpointCount = ~0;
            return;
        case 2 :
			if(nPostMess)
				CycleUnderPostMessages();
            while (++indexEndpointCount <= maxIndEndpoints)
            {
                VS_TransportRouter_Endpoint   *ep = endpoint[indexEndpointCount];
                if (!ep)
                    continue;
#ifdef _MY_DEBUG_
                printf("VS_TransportRouter_Implementation::ProcessingPeriodic: indexEndpointCount: %lu\n", indexEndpointCount);
#endif
                if (flagMcpConnect)
                {
#ifdef _MY_DEBUG_
                    puts( "VS_TransportRouter_Implementation::ProcessingPeriodic: case 2: WriteMcp( ... )" );
#endif
                    ep->FillMonitorStruct( tmReply->endpoint );
                    WriteMcp( (const void *)tmReply, sizeof(tmReply->endpoint) );
                }
                if (ep->bothCondIpFlag)
                {
                    if (ep->bothConn->GetPeerIp() && ep->endpointID && *(ep->endpointID))
                    {
                        ep->bothCondIpFlag = false;
                        for (unsigned long i = 0; i < maxServices; ++i)
                        {
                            VS_TransportRouter_Service  *srv = service[i];
                            if (!srv)
                                continue;
                            ++nRecursionDepthSendMess;
                            if (!srv->servBase->OnPointDeterminedIP_Event( ep->endpointID, ep->bothConn->GetPeerIp() ))
                                DeleteService( i );
                            if (nRecursionDepthSendMess == 1 && nPostMess)
                                CycleUnderPostMessages();
                            --nRecursionDepthSendMess;
                        }
                    }
                }
                ep->sendQueue.ProcessingTick(tickDiffTm);
                if (ep->bothConn)
                {
                    ep->activeTm = currTm;
                    ep->ProcessingTick();
                    if (ep->m_reconnectFlag==3)
                    {
                        observer.crt[15] += ep->m_realReconnects;
                        ep->m_realReconnects = 0;
                    }
                    if (ep->bothConn)
                    {
                        ++observer.crt[14];
                    }
                }
                else if ((currTm - ep->lastReadTm) > ep->maxLackMs) // bugfix #2876
                {
					if(!ep->IsConnectProcessing())
						DeleteEndpoint( indexEndpointCount, VS_PointParams::CR_TIMEOUT,
										(endpoint[indexEndpointCount]->lastReadTm || endpoint[indexEndpointCount]->lastWriteTm)? true: false );
                }
                return;
            }
            statePeriodic = 3;
            return;
        case 3 :
            if (flagMcpConnect)
            {
#ifdef _MY_DEBUG_
                puts( "VS_TransportRouter_Implementation::ProcessingPeriodic: case 3: WriteMcp( ... )" );
#endif
                tmReply->type = TM_TYPE_PERIODIC_STOP_ENDPOINTS;
                WriteMcp( (const void *)tmReply, sizeof(tmReply->type) );
            }
            statePeriodic = 4;
            return;
        case 4 :
            if (flagMcpConnect)
            {
#ifdef _MY_DEBUG_
                puts( "VS_TransportRouter_Implementation::ProcessingPeriodic: case 4: WriteMcp( ... )" );
#endif
                TmReply::StartServices   &startServices = tmReply->startServices;
                startServices.type = TM_TYPE_PERIODIC_START_SERVICES;
                startServices.maxServices = maxServices;
                WriteMcp( (const void *)tmReply, sizeof(tmReply->startServices) );
            }
            statePeriodic = 5;
            indexServiceCount = ~0;
            return;
        case 5 :
//NANOBEGIN;
            while (++indexServiceCount <= maxIndServices)
            {
                VS_TransportRouter_Service   *serv = service[indexServiceCount];
                if (!serv)	continue;
#ifdef _MY_DEBUG_
//printf( "VS_TransportRouter_Implementation::ProcessingPeriodic: indexServiceCount: %u\n", indexServiceCount );
#endif
                if (flagMcpConnect)
                {
#ifdef _MY_DEBUG_
                    puts( "VS_TransportRouter_Implementation::ProcessingPeriodic: case 5: WriteMcp( ... )" );
#endif
                    serv->FillMonitorStruct( tmReply->service );
                    WriteMcp( (const void *)tmReply, sizeof(tmReply->service) );
                }
                {
                    VS_TransportRouterPipeService   *ps = serv->pipeService;
                    if (ps)
                        ps->sendQueue.ProcessingTick(tickDiffTm);
                }
                return;
            }
            statePeriodic = 6;
//NANOEND;
            return;
        case 6 :
            if (flagMcpConnect)
            {
#ifdef _MY_DEBUG_
                puts( "VS_TransportRouter_Implementation::ProcessingPeriodic: case 6: WriteMcp( ... )" );
#endif
                tmReply->type = TM_TYPE_PERIODIC_STOP_SERVICES;
                WriteMcp( (const void *)tmReply, sizeof(tmReply->type) );
            }
            statePeriodic = 7;
            return;
        case 7 :
            if (flagMcpConnect)
            {
#ifdef _MY_DEBUG_
                puts( "VS_TransportRouter_Implementation::ProcessingPeriodic: case 7: WriteMcp( ... )" );
#endif
                tmReply->type = TM_TYPE_PERIODIC_STOP;
                WriteMcp( (const void *)tmReply, sizeof(tmReply->type) );
            }
            statePeriodic = 8;
            return;
        case 8 :

            DeleteEndpointQueryTry();
            PeriodicDeleteConn();
            statePeriodic = 9;
            return;
		case 9 :
			CheckGetServerNameThreads();
			statePeriodic = 10;
			return;
		case 10:
			CheckWaitingForResponseExpiration();
			statePeriodic = 0;
			return;
        default :
#ifdef _MY_DEBUG_
            printf( "VS_TransportRouter_Implementation::ProcessingPeriodic: default, statePeriodic: %u\n", statePeriodic );
#endif
            statePeriodic = 0;
            return;
        }
    }
    // end VS_TransportRouter_Implementation::ProcessingPeriodic
    static const unsigned long PROCESSING_TICK_CYCLE_TIMEOUT = 60;///every minute.

//300;///every 5 minutes
    static const unsigned long OBSERVER_CYCLE_TIMEOUT = 5;///every 5 seconds
    static const unsigned long MAX_OBSERVER_TIMES = 10*12;//x OBSERVER_CYCLE_TIMEOUT seconds = 10 minutes

    inline void ObserveProcessing( )
    {
        ///////////////////////////////////////////////////////
        if (processingTickTm%OBSERVER_CYCLE_TIMEOUT==0)
        {
            observer.crt[1]+=transportStatistic.GetAverageNow();
            ///////////////////////////////////////////////////////
            /// Transport Router logging statistic				///
            ///////////////////////////////////////////////////////
            {
                if (m_statisticLog)
                {
                    unsigned long theRealConnections = observer.crt[14].GetLastSaved();
                    unsigned long theConnectionsPerSecond = observer.crt[0].GetCurrentValue();
                    unsigned long theReconnections = observer.crt[15].GetLastSum();
                    unsigned long theAcceptTime = observer.crt[4].GetCurrentValue();

                    m_statisticLog->TPrintf("-%d-%d-%d-%d-%d-",
                                            this->nEndpoints,
                                            theRealConnections,
                                            theConnectionsPerSecond,
                                            theReconnections,
                                            theAcceptTime);
                }
            }
            ///////////////////////////////////////////////////////
            observer.UpdateAll();
            transportStatistic.ClearSum();

            if (observer.GetCounts()>MAX_OBSERVER_TIMES)
            {
                observer.GetStatisticAll();
            }

        }


        ///////////////////////////////////////////////////////
    }
    inline void ProcessingTick( void )
    {

        if (processingTickTm < PROCESSING_TICK_CYCLE_TIMEOUT)
        {
            ++processingTickTm;
            ObserveProcessing();
        }
        else
        {
            processingTickTm = 0;

            VS_RegistryKey key(false, CONFIGURATION_KEY);
            unsigned int   L = 0;
            unsigned int   S = 0;
            if (key.GetValue(&L, sizeof(L), VS_REG_INTEGER_VT, DEBUG_ROUTER_TAG_NAME)
                    == sizeof(L))
            {
                if (routerDebug!=0 && L==0)
                {
                    if (trDebugLog)
                    {
                        delete trDebugLog;
                        trDebugLog = 0;
                    }
                }
                routerDebug = L;
                routerDebugFlag = routerDebug;
            }
            ///////////////////////////////////////////////////////
            /// Transport Router logging statistic				///
            ///////////////////////////////////////////////////////

            if (key.GetValue(&S, sizeof(S), VS_REG_INTEGER_VT, DEBUG_STATISTIC_TAG_NAME)
                    == sizeof(S))
            {
                if (S==0)
                {
                    if (m_statisticLog!=0)
                    {
                        delete m_statisticLog;
                        m_statisticLog = 0;
                    }
                }
                else
                {
                    if (!m_statisticLog)
                    {
                        m_statisticLog = new VS_AcsLog("routerstat",1000000,500000,"./");
                        if (m_statisticLog)
                            m_statisticLog->TPrintf("-Endpoints-RealConnection-ConnPerSec-Reconnections-AcceptTime-");
                    }
                }
                m_isStatiscticOn = (S!=0);
            } else
            {
                ///there is no key...
                m_isStatiscticOn = 0;
                if (m_statisticLog!=0)
                {
                    delete m_statisticLog;
                    m_statisticLog = 0;
                }
            }
        }
        observer.crt[6] = nEndpoints;

		VS_RegistryKey cfgKey(false, CONFIGURATION_KEY,false);
		auto last_write = cfgKey.GetLastWriteTime();
		if (last_write.empty() || m_last_write.empty() || (last_write != m_last_write))
		{
			unsigned long certUpdated(0);
			if(cfgKey.GetValue(&certUpdated, sizeof(certUpdated), VS_REG_INTEGER_VT, "cert_updated") && (1 == certUpdated))
			{
				cfgKey.RemoveValue("cert_updated");
				ReadCryptoData();
			}
			m_last_write = last_write;
		}
    }
    // end VS_TransportRouter_Implementation::ProcessingTick

    inline void Thread( void )
    {
        unsigned long flagsOfFunctions,a,b,c,d,e;
        threadId = GetCurrentThreadId();
        cr.cntrlResponse.cmd = vs_tr_start_router;
        cr.cntrlResponse.res = true;
        if (icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) ))
        {

            DWORD   trans, error, n_repeat_error = 0, repeat_error = 0,
                                                   mills = routerTickSizeMs, old_tick = GetTickCount(), tmp_tick;
            ULONG_PTR   key;
            VS_Overlapped   *pov;
            while (1)
            {
                flagsOfFunctions = 0;
                trans = 0;
                key = 0;
                pov = 0;
                error = 0;
                if (!GetQueuedCompletionStatus( hiocp, &trans, &key, (LPOVERLAPPED *)&pov, !statePeriodic || flagMcpWrite ? mills : 0 ))
                {
                    error = GetLastError();
                    flagsOfFunctions |= 0x00000001;

					if (pov == 0)
					switch (error)
                    {
                    case WAIT_TIMEOUT :
                    case ERROR_SEM_TIMEOUT :
                        n_repeat_error = 0;
                        flagsOfFunctions |= 0x00000002;
                        a=b=c=d=0;
                        lastRouterTick = GetTickCount();
                        goto go_continue;
                    }
                    if (!pov)
                    {
#ifdef _MY_DEBUG_
                        printf("VS_TransportRouter_Implementation::Thread: if (!pov), error: %lu\n", error);
#endif
                        flagsOfFunctions |= 0x00000004;
                        if (!n_repeat_error)
                        {
                            repeat_error = error;
                            ++n_repeat_error;
                        }
                        else if (repeat_error == error)
                        {
                            if (++n_repeat_error >= VS_TR_REPEAT_ERROR_NUMBER)
                            {
#ifdef _MY_DEBUG_
                                printf("VS_TransportRouter_Implementation::Thread: if (n_repeat_error >= %u), error: %lu\n", VS_TR_REPEAT_ERROR_NUMBER, error);
#endif
                                flagsOfFunctions |= 0x00000008;
                                a=b=c=d=0;
                                lastRouterTick = GetTickCount();
                                goto go_return;
                            }
                        }
                        else	n_repeat_error = 0;
                        a=b=c=d=0;
                        lastRouterTick = GetTickCount();

                        goto go_continue;
                    }
                    else	pov->error = error;
                    n_repeat_error = 0;
                }
                a=b=c=d=0;
                lastRouterTick = GetTickCount();
                e = (unsigned long )pov->field1;
                if (!RunHandle( (unsigned long)trans, *pov ))
                {
#ifdef _MY_DEBUG_
                    printf("VS_TransportRouter_Implementation::Thread: if (!RunHandle( (unsigned long)bTrans, *pOv )), error: %lu\n", error);
#endif
                    flagsOfFunctions |= 0x00000010;
                    goto go_return;
                }
go_continue:
                tmp_tick = GetTickCount();
                currDiffTm = tmp_tick - old_tick;
                currTm += currDiffTm;
                sumCurrDiffTm += currDiffTm;
                if (statePeriodic)
                {
                    flagsOfFunctions |= 0x00000020;
                    ProcessingPeriodic();
                }
                if (sumCurrDiffTm >= routerTickSizeMs)
                {
                    flagsOfFunctions |= 0x00000040;
                    tickDiffTm = sumCurrDiffTm;
                    sumCurrDiffTm = 0;
                    tickTm += tickDiffTm;
                    mills = routerTickSizeMs;
                    if (!statePeriodic)
                    {
                        flagsOfFunctions |= 0x00000080;
                        a = GetTickCount();
                        ProcessingPeriodic();
                        b = GetTickCount();
                    }
                    c = GetTickCount();
                    ProcessingTick();
                    d = GetTickCount();

                }
                else
                    mills -= currDiffTm;
                old_tick = tmp_tick;
                routerEndTick = GetTickCount();
            }
        }
go_return:
        dprint4("EXIT FLAGS: %lX\n", flagsOfFunctions);
        icp->Close();
#ifdef _MY_DEBUG_
        puts( "VS_TransportRouter_Implementation::Thread: return;" );
#endif
    }
    // end VS_TransportRouter_Implementation::Thread

    static unsigned __stdcall Thread( void *arg )
    {
        vs::SetThreadName("TransportRouter");
        if (arg)	((VS_TransportRouter_Implementation *)arg)->Thread();
        _endthreadex( 0 );
        return 0;
    }
    // end VS_TransportRouter_Implementation::Thread

    struct ConnectThreadArgs
    {
        ConnectThreadArgs( const char *our_endpoint_name, const char *target_endpoint_name,
                           const unsigned long maxLackMs, const unsigned char hops,
                           VS_TransportRouter_Implementation *tr ) :
                isValid(false), our_endpoint_name(0), target_endpoint_name(0),
                maxLackMs(maxLackMs), hops(hops), tr(tr)
        {
            if (!our_endpoint_name || !*our_endpoint_name || !target_endpoint_name || !*target_endpoint_name
                    || !tr )	return;
            ConnectThreadArgs::our_endpoint_name = _strdup( our_endpoint_name );
            if (!ConnectThreadArgs::our_endpoint_name)		return;
            ConnectThreadArgs::target_endpoint_name = _strdup( target_endpoint_name );
            if (!ConnectThreadArgs::target_endpoint_name)	return;
            isValid = true;
        } // end ConnectThreadArgs::ConnectThreadArgs
        ~ConnectThreadArgs( void )
        {
            if (target_endpoint_name)	free( (void *)target_endpoint_name );
            if (our_endpoint_name)		free( (void *)our_endpoint_name );
        } // end ConnectThreadArgs::~ConnectThreadArgs
        bool   isValid;
        char   *our_endpoint_name, *target_endpoint_name;

        const unsigned long   maxLackMs;
        const unsigned char   hops;
        VS_TransportRouter_Implementation   *tr;
    }; // end VS_TransportRouter_Endpoint::ConnectThreadArgs struct

	static bool TryConnectTcp(unsigned long mills, std::unique_ptr<net::endpoint::ConnectTCP>& connTcp,
		VS_ConnectionTCP*& connection)
	{
		if (!connTcp)
			return false;
		if (connection)
		{
			delete connection;
			connection = nullptr;
		}

		connection = new VS_ConnectionTCP();
		if (connection->Connect(connTcp->host.c_str(), connTcp->port, mills))
		{
			connection->GetPeerAddress(); // not sure if I should do this but previous implementation did
			connection->SetQoSFlow(net::QoSSettings::GetInstance().GetTCTransportQoSFlow());
			return true;
		}

		delete connection;
		connection = nullptr;
		return false;
	}

	static bool TryConnectTls(unsigned long mills, std::unique_ptr<net::endpoint::ConnectTCP>& connTcp,
		VS_ConnectionTCP*& connection)
	{
		if (!connTcp)
			return false;
		if (connection)
		{
			delete connection;
			connection = nullptr;
		}

//		Piece of code stolen from VS_ConnectionManager.cpp, no shame felt
		VS_ConnectionTLS* connectionTls = new VS_ConnectionTLS();
		VS_SCOPE_EXIT{connection = connectionTls;};
		vs::event done(false);
		bool connectSuccess(false);

		connectionTls->SetCallback(
			[&connectSuccess, &done](bool success, VS_ConnectionTLS*, const void*, const unsigned long)
			{
				connectSuccess = success;
				done.set();
			}
		);
		if (VS_LoadCertsForTLS(*connectionTls) &&
			connectionTls->Connect(connTcp->host.c_str(), connTcp->port, mills)
		   )
		{
			done.wait();
			if (connectSuccess)
			{
				connectionTls->GetPeerAddress(); // not sure if I should do this but previous implementation did
				connectionTls->SetQoSFlow(net::QoSSettings::GetInstance().GetTCTransportQoSFlow());
				return true;
			}
		}

		delete connectionTls;
		connectionTls = nullptr;
		return false;
	}

	static bool TryConnect(uint32_t timeout, const char* targetEndpointName,
		VS_TransportRouter_Implementation* trRoutImpl, VS_ConnectionTCP*& connection)
	{
		const uint32_t DEFAULT_CONNECT_TIMEOUT = 5000; // milliseconds
		const uint32_t DEFAULT_FAILURE_SLEEP_TIME = 500; // milliseconds

		if (connection)
		{
			delete connection;
			connection = nullptr;
		}

//		Goal: try to reach the endpoint via TLS;
//		If it fails, try TCP immediately;
//		If it fails, sleep and try again
		uint32_t countConnectTcp = net::endpoint::GetCountConnectTCP(targetEndpointName, false);
		VS_RegistryKey rKey(false, CONFIGURATION_KEY);
		int32_t value = 0;
//		Check if TLS is enabled in registry (enabled by default)
		bool tlsEnabled = (rKey.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "ServerTLSEnabled") <= 0
				|| value != 0);
		bool tryTls = tlsEnabled;
		while (timeout > 0)
		{
			for (uint32_t i = 1; i <= countConnectTcp && timeout > 0; ++i)
			{
				auto lastTick = GetTickCount();
				if (trRoutImpl->isShutdown)
					return false;

				auto connTcp = net::endpoint::ReadConnectTCP(i, targetEndpointName, false);
				if (!connTcp) // can happen with a chance of approximately 1*10^-9
					continue;
				uint32_t mills = (timeout < DEFAULT_CONNECT_TIMEOUT) ? timeout : DEFAULT_CONNECT_TIMEOUT;
				bool result(false);
				if (tryTls)
					result = TryConnectTls(mills, connTcp, connection);
				else
					result = TryConnectTcp(mills, connTcp, connection);

				auto currentTick = GetTickCount();
				timeout -= (timeout < currentTick - lastTick) ? timeout : currentTick - lastTick;

				if (result)
					return true;
			}
			if (!timeout)
				return false;
			if (!tryTls) // If we already tried TLS and TCP, we need to sleep a little bit so that
						 // Windows doesn't freak out at 5000th connect attempt
			{
				auto sleepTime = (timeout < DEFAULT_FAILURE_SLEEP_TIME) ? timeout : DEFAULT_FAILURE_SLEEP_TIME;
				Sleep(sleepTime);
				timeout -= sleepTime;
			}
//			Switch from trying TLS to not trying and vice versa (in case it's enabled)
			tryTls = !tryTls && tlsEnabled;
		}
		return false;
	}

	static bool DoZeroHandshake(VS_ConnectionTCP* connection, ConnectThreadArgs* args, uint32_t& version,
		unsigned char& resultCode, uint16_t& maxConnSilenceMs,
		unsigned char& fatalSilenceCoef, unsigned char& retHops, bool& remoteTcpKeepAliveSupport)
	{
		const uint32_t DEFAULT_WRITE_TIMEOUT = 2000;// milliseconds
		net::HandshakeHeader* handshakeHeader = VS_FormTransportHandshake(
			args->our_endpoint_name, args->target_endpoint_name, args->hops, /*SSL support*/true,
			VS_TransportRouter_Endpoint::IsTCPKeepAliveAllowed());

		VS_SCOPE_EXIT
		{
			if (handshakeHeader)
			    free( static_cast<void *>(handshakeHeader) );
		};

		if (!handshakeHeader)
			return false;

		unsigned long mills = DEFAULT_WRITE_TIMEOUT;
		if (!connection->CreateOvReadEvent() ||
			 !connection->CreateOvWriteEvent() ||
			 !VS_WriteZeroHandshake(connection, handshakeHeader, mills)
		   )
			return false;
		free(handshakeHeader);
		handshakeHeader = nullptr;

		if (!VS_ReadZeroHandshake(connection, &handshakeHeader, mills) ||
			!VS_TransformTransportReplyHandshake(handshakeHeader, resultCode, maxConnSilenceMs,
				fatalSilenceCoef, retHops, vs::ignore<char*>()/*server_id*/,
				vs::ignore<char*>()/*cid*/, remoteTcpKeepAliveSupport) ||
			resultCode)
		{
			if (resultCode == hserr_verify_failed)
				args->tr->ServerVerificationFailed();
			return false;
		}
		version = handshakeHeader->version;
		return true;
	}

	static unsigned int __stdcall ConnectConnection(void* _args)
	{
		vs::SetThreadName("TR_Connect");
		if (!_args)
			return 0;
		ConnectThreadArgs* args = static_cast<ConnectThreadArgs*>(_args);
		const char* ourEndpointName = args->our_endpoint_name,
			  * targetEndpointName = args->target_endpoint_name;
		bool remoteTcpKeepAliveSupport(false),
			 resultTcpKeepAliveSupport(false);

		VS_TransportRouter_Implementation* trRoutImpl = args->tr;
        InterlockedIncrement(&trRoutImpl->numberOfConnectThreades);


//		Declaring variables for VS_SCOPE_EXIT
		uint32_t version = 0;
		VS_ConnectionTCP* connection = 0;
		uint16_t maxConnSilenceMs = 0;
		unsigned char resultCode = hserr_antikyou, fatalSilenceCoef = 0, retHops = 0;
		bool isConnected(false);

		VS_SCOPE_EXIT {
			if (!trRoutImpl->isShutdown)
			    trRoutImpl->SetConnection( targetEndpointName, version, ourEndpointName, connection,
					false, maxConnSilenceMs, fatalSilenceCoef, retHops, 0, 0, 0, 0,
					(isConnected && !connection), resultTcpKeepAliveSupport);

			delete args;
			InterlockedDecrement(&trRoutImpl->numberOfConnectThreades);
		};

		isConnected = TryConnect(args->maxLackMs, targetEndpointName, trRoutImpl, connection);
		if (!isConnected ||
			!DoZeroHandshake(connection, args, version, resultCode,
				maxConnSilenceMs, fatalSilenceCoef, retHops, remoteTcpKeepAliveSupport))
		{
			if (connection)
				delete connection;
			connection = 0;
			return 0;
		}
		if (remoteTcpKeepAliveSupport && VS_TransportRouter_Endpoint::IsTCPKeepAliveAllowed())
			resultTcpKeepAliveSupport = connection->SetKeepAliveMode(true, 20000, 2000);
		return 0;
	}
	// end VS_TransportRouter_Implementation::ConnectConnection

    static unsigned __stdcall FindServerParamsThread(void *arg)
    {
		vs::SetThreadName("TR_FindServer");
        VS_TransportRouter_Implementation	*pThis = (VS_TransportRouter_Implementation*)arg;
        return pThis->FindServerParamsThread();
    }

    unsigned FindServerParamsThread()
    {
        HANDLE h[2] = {m_hNewQueueElementEvent, m_hExitFindServeParamsThreadEvent };
        unsigned long sz(0);
        while (1)
        {
            switch (WaitForMultipleObjects(2, h, FALSE, INFINITE))
            {
            case WAIT_OBJECT_0:
                do
                {
                    VS_SimpleStr	str;
                    EnterCriticalSection(&m_QueueSect);

                    sz = (unsigned long)m_stringsForFindQueue.size();
                    if (sz)
                    {
                        str = m_stringsForFindQueue.front();
                        m_stringsForFindQueue.pop();
                    }
                    LeaveCriticalSection(&m_QueueSect);
                    if (!str.Length())
                        continue;

					auto resolved = net::dns::make_srv_lookup(net::dns::get_srv_query_by_server_id({ str.m_str, str.Length() })).get();
					if (!resolved.second)
						net::dns::resolved_srv_to_registry({ str.m_str, str.Length() }, resolved.first);

                    EnterCriticalSection(&m_QueueSect);
                    sz = (unsigned long)m_stringsForFindQueue.size();
                    LeaveCriticalSection(&m_QueueSect);
                } while (sz);
                break;
            case WAIT_OBJECT_0 + 1:
                return 0;
            default:
                return 0;
            }
        }
        return 0;
    }
	inline void CheckGetServerNameThreads()
	{
		for(std::list<HANDLE>::iterator i = m_getSrvNameThreads.begin();i!=m_getSrvNameThreads.end();)
		{
			DWORD res(0);
			if(GetExitCodeThread(*i,&res) && res!=STILL_ACTIVE)
			{
				CloseHandle(*i);
				i = m_getSrvNameThreads.erase(i);
			}
			else
				i++;
		}
	}
	inline void WaitGetServerNameThreads()
	{
		for(std::list<HANDLE>::iterator i = m_getSrvNameThreads.begin();i!=m_getSrvNameThreads.end();i++)
		{
			WaitForSingleObject(*i,INFINITE);
			CloseHandle(*i);
		}
		m_getSrvNameThreads.clear();
	}
    inline bool ConnectConnection( const char *endpoint_name, const unsigned long maxLackMs )
    {
        if (isShutdown)
            return false;
        unsigned long index = GetEndpointIndexByUID( endpoint_name );
        if (!endpoint[index]->GetPermissionOnConnectAct( ))
            return false;
        ConnectThreadArgs   *args = new ConnectThreadArgs( endpointName.c_str(), endpoint_name, maxLackMs, hops, this );
        if (!args)		return false;
        if (!args->isValid) {
            delete args;
            return false;
        }
        uintptr_t   th = _beginthreadex( 0, 0, ConnectConnection, args,0,0 );

        if (!th || th == (uintptr_t)-1L) {
            delete args;
            return false;
        }
        endpoint[index]->m_hBothConnectThread = (HANDLE)th;
        return true;
    }
    // end VS_TransportRouter_Implementation::ConnectConnection
    static const unsigned long mills_init = INFINITE;//180000; //10минут = 600 000
    inline bool CntrlSR( const ControlInquiry *cI, const unsigned long sI,
                         ControlResponse *cR = 0, const unsigned long sR = 0,
                         unsigned long mills = mills_init )
    {
        unsigned long b( GetTickCount() ),e(0);
        VS_SCOPE_EXIT {
            e = GetTickCount();
            if (e - b > 60000) //1 min
            {
                dprint4("Operation lasts about %lu. Period: %ld ms", e-b, GetTickCount() - lastRouterTick);
                if (cI)
                    dprint4("Command was: %d", cI->cntrlInquiry.cmd);
            }
        };

        std::unique_lock<decltype(mtx)> l(mtx);
        if (!isInit)
            return false;

        if (threadId == GetCurrentThreadId())
            return false;

        assert(cI != nullptr);
        assert(sI != 0);
        if (!ocp->Write(cI, sI) || sI != ocp->GetWriteResult(mills))
        {
            dprint1("TR: Error writing to \"ocp\". System Error: %lu.", GetLastError());
            return false;
        }

        if (!sR) // No response is expected, we are done.
            return true;

        std::lock_guard<decltype(mtx_read)> l_read(mtx_read);
        // Release the main mutex to allow other threads to send async requests while we are waiting for our response.
        l.unlock();
        assert(cR != nullptr);
        if (!ocp->Read(cR, sR) || sR != ocp->GetReadResult(mills) || cI->cntrlInquiry.cmd != cR->cntrlResponse.cmd)
        {
            dprint1("TR: Error reading from \"ocp\". System Error: %lu.", GetLastError());
            return false;
        }

        return true;
    }
    // end VS_TransportRouter_Implementation::CntrlSR

    /////////////////////////////////////////////////////////////////////////////////////

    inline bool InitAct(
		const std::string& endpointName,
		VS_AccessConnectionSystem *acs,
		VS_TlsHandler* tlsHandler,
		const unsigned long maxServices,
		const unsigned long maxEndpoints,
		const unsigned long maxEndpointQueueMess,
		const unsigned long maxEndpointLackMs,
		const unsigned long maxConnSilenceMs,
		const unsigned long fatalSilenceCoef,
		const unsigned long maxPostMess,
		const unsigned long maxRecursionDepthSendMess,
		const unsigned long routerTickSizeMs)
    {
        if (isInit)		return false;

        currDiffTm = currTm = 0;
        ///////////////////////////////////////////////////////
        /// Transport Router logging statistic				///
        ///////////////////////////////////////////////////////
        {
            if (!m_statisticLog)
            {
            }
        }
        VS_TransportRouter_Implementation::acs = acs;
        VS_TransportRouter_Implementation::maxServices = maxServices;
        VS_TransportRouter_Implementation::maxEndpoints = maxEndpoints;
        VS_TransportRouter_Implementation::maxEndpointQueueMess = maxEndpointQueueMess;
        VS_TransportRouter_Implementation::maxEndpointLackMs = maxEndpointLackMs;
        VS_TransportRouter_Implementation::maxConnSilenceMs = maxConnSilenceMs;
        VS_TransportRouter_Implementation::fatalSilenceCoef = fatalSilenceCoef;
        VS_TransportRouter_Implementation::maxPostMess = maxPostMess;
        VS_TransportRouter_Implementation::nPostMess = 0;
        VS_TransportRouter_Implementation::maxRecursionDepthSendMess = maxRecursionDepthSendMess;
        VS_TransportRouter_Implementation::nRecursionDepthSendMess = 0;
        VS_TransportRouter_Implementation::routerTickSizeMs = routerTickSizeMs;
        if (flagLogs)
        {
            if (!dirLogsName)	flagLogs = false;
            else
            {
                int   st = _access( dirLogsName, 06 );
                if (st == -1)
                {
                    if (errno != ENOENT)	flagLogs = false;
                    else
                    {
                        st = _mkdir( dirLogsName );
                        if (st == -1)		flagLogs = false;
                    }
                }
            }
        }
        size_t   sz = maxServices * sizeof(VS_TransportRouterServiceBase *);
        ::free(service);
        service = (VS_TransportRouter_Service **)malloc( sz );
        if (!service) {
            ShutdownAct();
            return false;
        }
        memset( (void *)service, 0, sz );
        sz = maxEndpoints * sizeof(VS_TransportRouter_Endpoint *);
        ::free(endpoint);
        endpoint = (VS_TransportRouter_Endpoint **)malloc( sz );
        if (!endpoint) {
            ShutdownAct();
            return false;
        }
        memset( (void *)endpoint, 0, sz );
        sz = maxPostMess * sizeof(VS_RouterMessage *);
        ::free(postMess);
        postMess = (VS_RouterMessage **)malloc( sz );
        if (!postMess) {
            ShutdownAct();
            return false;
        }
        memset( (void *)postMess, 0, sz );
        if (!(hiocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, 0, 0, 0 )))
        {
            Shutdown();
            return false;
        }
        if (!(ocp = new VS_ConnectionMsg)
                || !ocp->Create( vs_pipe_type_duplex )
                || !ocp->CreateOvReadEvent()
                || !ocp->CreateOvWriteEvent()) {
            ShutdownAct();
            return false;
        }
        if (!(icp = new VS_ConnectionMsg)
                || !icp->Open( ocp, vs_pipe_type_duplex )
                || !icp->SetIOCP( hiocp )) {
            ShutdownAct();
            return false;
        }
        icp->SetOvWriteFields( (const VS_ACS_Field)VS_TR_CONTROL_WRITE );
        icp->SetOvReadFields( (const VS_ACS_Field)VS_TR_CONTROL_READ );
        ResetToConnectMcp();
        uintptr_t   th = _beginthreadex( 0, 0, Thread, (void *)this, 0, 0 );
        if ( !th || th == -1L) {
            ShutdownAct();
            return false;
        }
        hthr = (HANDLE)th;
        ControlResponse::CntrlResponse   cntrlResponse;
        memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
        unsigned long   mills = 120000;
        if (!ocp->Read( (void *)&cntrlResponse, sizeof(cntrlResponse) )
                || ocp->GetReadResult( mills ) != sizeof(cntrlResponse)
                || cntrlResponse.cmd != vs_tr_start_router || !cntrlResponse.res)
        {
            ShutdownAct();
            return false;
        }
        VS_TransportRouter_Implementation::endpointName = endpointName;
        if(!endpointName.empty()) g_tr_endpoint_name = endpointName;
        transportHandler = new VS_TransportHandler( VS_TransportRouter_Implementation::endpointName.c_str(), this );
        if (!transportHandler || !transportHandler->IsValid())
        {
            ShutdownAct();
            return false;
        }
        sprintf( handlerName, "%s%s", trHandleNamePrefix, VS_TransportRouter_Implementation::endpointName.c_str() );
        if (!acs->AddHandler( handlerName, transportHandler ))
        {
            ShutdownAct();
            return false;
        }
		if (tlsHandler && !tlsHandler->AddHandler(handlerName, transportHandler))
        {
            ShutdownAct();
            return false;
        }

        VS_RegistryKey key(false, CONFIGURATION_KEY);
        unsigned long is_two_conn(0);
        key.GetValue(&is_two_conn,sizeof(is_two_conn),VS_REG_INTEGER_VT,"Two Transport Connections");
        /**
			Старт потока, который обрабатывает мессаги, направленные серверам, с которыми у нас нет соединения
        */
        th = _beginthreadex(0,0,FindServerParamsThread,this,0,0);
        if (!th || th == -1L)
        {
            Shutdown();
            return false;
        }
        m_hFindServerParamsThreadHandle = (HANDLE)th;

        return isInit = true;
    }
    // end VS_TransportRouter_Implementation::InitAct

    inline bool Init(const std::string& endpointName, VS_AccessConnectionSystem *acs, VS_TlsHandler* tlsHandler, const unsigned long maxServices, const unsigned long maxEndpoints, const unsigned long maxEndpointQueueMess, const unsigned long maxEndpointLackMs, const unsigned long maxConnSilenceMs, const unsigned long fatalSilenceCoef, const unsigned long maxPostMess, const unsigned long maxRecursionDepthSendMess, const unsigned long routerTickSizeMs )
    {
        if ((fatalSilenceCoef - 1) * maxConnSilenceMs > maxEndpointLackMs)
        {
            /*****************************************************************
             Данная проверка позволяет соблюсти условие отключения ендпоинта.
             Если указанное условие не будет выполняться, то время отключения
             ендпоинта всегда будет выше максимального, что является неверным.
            ******************************************************************/
            return false;
        }
        if (endpointName.empty() || endpointName.length() > VS_ACS_MAX_SIZE_ENDPOINT_NAME
                || !acs || !maxServices || !maxEndpoints
                || maxEndpointQueueMess < 20 || maxEndpointQueueMess > 2000
                || maxEndpointLackMs < 5000 || maxEndpointLackMs > 60000
                || maxConnSilenceMs < 1500 || maxConnSilenceMs > 65535
                || fatalSilenceCoef < 2 || fatalSilenceCoef > 255)
            return false;
        std::lock_guard<decltype(mtx)> l(mtx);
        return InitAct(endpointName, acs, tlsHandler, maxServices, maxEndpoints, maxEndpointQueueMess, maxEndpointLackMs, maxConnSilenceMs, fatalSilenceCoef, maxPostMess, maxRecursionDepthSendMess, routerTickSizeMs);
    }
    // end VS_TransportRouter_Implementation::Init

    inline bool IsInit( void )
    {
        std::lock_guard<decltype(mtx)> l(mtx);
        return isInit;
    }
    // end VS_TransportRouter_Implementation::IsInit

	inline bool SetServCertInfoInterface(VS_ServCertInfoInterface *infoInterface)
	{
		bool ret = false;
		ControlInquiry::SetServCertInfoInterface setServCertInfo;
		memset((void*)&setServCertInfo,0,sizeof(setServCertInfo));
		setServCertInfo.cmd = vs_tr_setServCertInfoInterface;
		setServCertInfo.pInterface= infoInterface;
        ControlResponse::CntrlResponse   cntrlResponse;
        memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
        if (CntrlSR( (ControlInquiry *)&setServCertInfo, sizeof(setServCertInfo), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
            ret = cntrlResponse.res;
		return ret;
	}

	inline bool	AddRouterMessExtHandler(VS_RouterMessExtHandlerInterface *ext_handler)
	{
		bool res(false);
		if(!ext_handler)
			return false;
		ControlInquiry::AddRouterMessExtHandler addExtHandler;
		memset(&addExtHandler,0,sizeof(addExtHandler));
		addExtHandler.cmd = vs_tr_add_router_mess_ext_handler;
		addExtHandler.ext_handler = ext_handler;
		ControlResponse::CntrlResponse   cntrlResponse = {};
		if(CntrlSR( (ControlInquiry*)&addExtHandler,sizeof(addExtHandler),(ControlResponse*)&cntrlResponse,sizeof(cntrlResponse)))
			res = cntrlResponse.res;
		return res;
	}

	inline bool	RemoveRouterMessExtHandler(VS_RouterMessExtHandlerInterface *ext_handler)
	{
		bool res(false);
		if(!ext_handler)
			return false;
		ControlInquiry::RemoveRouterMessExtHandler removeExtHandler;
		memset(&removeExtHandler,0,sizeof(removeExtHandler));
		removeExtHandler.cmd = vs_tr_remove_router_mess_ext_handler;
		removeExtHandler.ext_handler = ext_handler;
		ControlResponse::CntrlResponse   cntrlResponse = {};
		if(CntrlSR( (ControlInquiry*)&removeExtHandler,sizeof(removeExtHandler),(ControlResponse*)&cntrlResponse,sizeof(cntrlResponse)))
			res = cntrlResponse.res;
		return res;
	}

    inline void ShutdownAct( void )
    {
        InterlockedIncrement(&isShutdown);
        //
        if (transportHandler)
        {
            if (acs)	acs->RemoveHandler( handlerName );
            delete transportHandler;
            transportHandler = 0;
        }
        if (endpoint)
        {
            unsigned long timeOut = 0;
            printf("\n\t Please wait ");
            do{
                timeOut = 0;
                for (unsigned long i = 0; i < maxEndpoints; ++i)
                    if (endpoint[i])
						endpoint[i]->m_IsConnForbidden = true;
				for (unsigned long i = 0; i < maxEndpoints; ++i)
				{
					if (endpoint[i])
					{
						unsigned long tick = GetTickCount();
						endpoint[i]->WaitConnectingThread();
					}
				}

                //VS_Server::PauseDestroy(DIE_TIMEOUT*4+50);
                ////Sleep(DIE_TIMEOUT/2);
                printf(".");
                if (numberOfConnectThreades>0)
                {
                    Sleep(DIE_TIMEOUT/2);
                    printf(".");
                    if (numberOfConnectThreades>0)
                    {
                        Sleep(DIE_TIMEOUT/2);
                        printf(".");
                        if (numberOfConnectThreades>0)
                            Sleep(DIE_TIMEOUT/2);
                        else timeOut = 0;

                    }
                    else timeOut = 0;

                }
                else timeOut = 0;
            }while (timeOut);
            Sleep(50);
            printf(".\n");
        }
        if (hthr)
        {
            if (ocp) {
                delete ocp;
                ocp = 0;
            }
            if (WaitForSingleObject( hthr, VS_TR_TIMEOUT_THREAD_SHUTDOWN ) == WAIT_OBJECT_0)
                CloseHandle( hthr );
            hthr = 0;
        }
		WaitGetServerNameThreads();
        if (hiocp) {
            CloseHandle( hiocp );
            hiocp = 0;
        }
        if (icp) {
            delete icp;
            icp = 0;
        }
        if (ocp) {
            delete ocp;
            ocp = 0;
        }
        if (mcp) {
            delete mcp;
            mcp = 0;
        }
        if (tmRequest) {
            delete tmRequest;
            tmRequest = 0;
        }
        if (tmReply) {
            delete tmReply;
            tmReply = 0;
        }
        tmReadState = 0;
        if (service)
        {
            for (unsigned long i = 0; i < maxServices; ++i)
                if (service[i]) {
                    delete service[i];
                    service[i] = 0;
                }
            free( (void *)service );
            service = 0;
        }
        if (endpoint)
        {
            for (unsigned long i = 0; i < maxEndpoints; ++i)
                if (endpoint[i]) {
                    delete endpoint[i];
                    endpoint[i] = 0;
                }
            free( (void *)endpoint );
            endpoint = 0;
        }
        if (postMess) free(postMess);
        postMess = 0;
		endpointName.clear();
        memset( (void *)handlerName, 0, sizeof(handlerName) );
        unsigned long i = 0;
        indexSump = 0;
        RemoveAllDeleteConns();
        acs = 0;
        isInit = false;

        if (m_hFindServerParamsThreadHandle)
        {
            SetEvent(m_hExitFindServeParamsThreadEvent);
            WaitForSingleObject(m_hFindServerParamsThreadHandle,INFINITE);
            CloseHandle(m_hFindServerParamsThreadHandle);
            m_hFindServerParamsThreadHandle = 0;
        }

    }
    // end VS_TransportRouter_Implementation::ShutdownAct

    inline void Shutdown( void )
    {
        std::lock_guard<decltype(mtx)> l(mtx);
        ShutdownAct();
    }
    // end VS_TransportRouter_Implementation::Shutdown

    bool AddService(const char *serviceName, VS_TransportRouterServiceBase *service, bool permittedForAll = false) override
    {
        bool   ret = false;
        if (!serviceName || !*serviceName
                || strlen(serviceName) > VS_TRANSPORT_MAX_SIZE_SERVICE_NAME
                || !service || service->imp)
            return false;
        ControlInquiry::AddService   addService;
        memset( (void *)&addService, 0, sizeof(addService) );
        addService.cmd = vs_tr_add_service;
        addService.isPermittedAll = permittedForAll;
        strcpy( addService.serviceName, serviceName );
        addService.service = service;
        ControlResponse::CntrlResponse   cntrlResponse;
        memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
        if (CntrlSR( (ControlInquiry *)&addService, sizeof(addService), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
            ret = cntrlResponse.res;
        return ret;
    }
    // end VS_TransportRouter_Implementation::AddService

    inline bool AddCallService( const char *serviceName, VS_TransportRouterServiceBase *callService , bool permittedForAll)
    {
        bool   ret = false;
        if (!serviceName || !*serviceName
                || strlen(serviceName) > VS_TRANSPORT_MAX_SIZE_SERVICE_NAME
                || !callService || callService->imp)	return false;
        ControlInquiry::AddCallService   addCallService;
        memset( (void *)&addCallService, 0, sizeof(addCallService) );
        addCallService.cmd = vs_tr_add_call_service;
        strcpy( addCallService.serviceName, serviceName );
        addCallService.callService = callService;
        addCallService.permittedForAll = permittedForAll;
        ControlResponse::CntrlResponse   cntrlResponse;
        memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
        if (CntrlSR( (ControlInquiry *)&addCallService, sizeof(addCallService), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
            ret = cntrlResponse.res;
        return ret;
    }
    // end VS_TransportRouter_Implementation::AddCallService

    bool RemoveService(const char *serviceName) override
    {
        if (!serviceName || !*serviceName
                || strlen(serviceName) > VS_TRANSPORT_MAX_SIZE_SERVICE_NAME)	return false;
        ControlInquiry::RemoveService   removeService;
        memset( (void *)&removeService, 0, sizeof(removeService) );
        removeService.cmd = vs_tr_delete_service;
        strcpy( removeService.serviceName, serviceName );
        VS_TR_Cmd   cr = vs_tr_unknown;
        CntrlSR( (ControlInquiry *)&removeService, sizeof(removeService), (ControlResponse *)&cr, sizeof(cr) );
		return true;
    }
    // end VS_TransportRouter_Implementation::RemoveService

    inline void SetConnection( const char *cid,
                               const unsigned long version,
                               const char *sid,
                               class VS_Connection *conn, const bool isAccept,
                               const unsigned short maxConnSilenceMs,
                               const unsigned char fatalSilenceCoef,
                               const unsigned char hops_,
							   const unsigned long rnd_data_ln,
							   const unsigned char *rnd_data,
							   const unsigned long sign_ln,
							   const unsigned char *sign,
							   const bool hs_error = false,		// Handshake error
							   const bool tcpKeepAliveSupport = false)

    {
        if (trDebugLog && trDebugLog->IsValid())
            trDebugLog->TPrintf("\n\t   SetConnection : cid:'%s' sid:'%s'  Conn: 0x%X", cid, sid, conn);

		/**
			Скопировать rnd_data, sign и передать по пайпу
		*/

        ControlInquiry::SetConnection  setConnection;
        memset( (void *)&setConnection, 0, sizeof(setConnection) );
        setConnection.cmd = vs_tr_set_connection;
        strcpy( setConnection.cid,cid );
        strcpy(setConnection.sid,sid);
        setConnection.version = version;
        setConnection.conn = conn;
        setConnection.isAccept = isAccept;
        setConnection.maxConnSilenceMs = maxConnSilenceMs;
        setConnection.fatalSilenceCoef = fatalSilenceCoef;
		setConnection.tcpKeepAliveSupport = tcpKeepAliveSupport;
        setConnection.hops = hops_;
		setConnection.rnd_data_ln = rnd_data_ln;
		if(rnd_data_ln && rnd_data_ln<=VS_RND_DATA_FOR_SIGN_SZ)
			memcpy(setConnection.rnd_data,rnd_data,rnd_data_ln);
		setConnection.sign_ln = sign_ln;
		if(sign_ln&&sign_ln<=VS_SIGN_SIZE)
			memcpy(setConnection.sign,sign,sign_ln);
		setConnection.hs_error = hs_error;
        if (isShutdown)	return;
        CntrlSR( (ControlInquiry *)&setConnection, sizeof(setConnection) );
    }


    inline bool GetStatistics( VS_TransportRouterStatistics &stat )
    {
        bool   ret = false;
        ControlInquiry::CntrlInquiry   cntrlInquiry;
        memset( (void *)&cntrlInquiry, 0, sizeof(cntrlInquiry) );
        cntrlInquiry.cmd = vs_tr_get_statistics;
        ControlResponse::GetStatistics   getStatistics;
        memset( (void *)&getStatistics, 0, sizeof(getStatistics) );
        if (CntrlSR( (ControlInquiry *)&cntrlInquiry, sizeof(cntrlInquiry), (ControlResponse *)&getStatistics, sizeof(getStatistics) ))
        {
            if (getStatistics.res)
            {
                stat = getStatistics.stat;
                ret = true;
            }
        }
        return ret;
    }
    // end VS_TransportRouter_Implementation::GetStatistics
    inline bool DisconnectEndpoint( const char *endpoint )
    {
        bool   ret = false;
        ControlInquiry::DisconnectEndpoint   disconnectEndpoint;
        memset( (void *)&disconnectEndpoint, 0, sizeof(disconnectEndpoint) );
        disconnectEndpoint.cmd = vs_tr_disconnect_endpoint;
        strcpy( disconnectEndpoint.endpoint, endpoint );
        ControlResponse::CntrlResponse   cntrlResponse;
        memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
        if (CntrlSR( (ControlInquiry *)&disconnectEndpoint, sizeof(disconnectEndpoint), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
            ret = cntrlResponse.res;
        return ret;
    }
    // end VS_TransportRouter_Implementation::DisconnectEndpoint

    inline bool AddProcessingMes( VS_RouterMessage *mess )
    {
        ControlInquiry::AddProcessingMes   addProcessingMes;
        addProcessingMes.cmd = vs_tr_add_processing_mes;
        addProcessingMes.mess = mess;
        return CntrlSR( (ControlInquiry *)&addProcessingMes, sizeof(addProcessingMes) );
    }
    // end VS_TransportRouter_Implementation::AddProcessingMes

    inline bool IsThereEndpoint( const char *endpoint )
    {
        ControlInquiry::IsThereEndpoint   isThereEndpoint;
        isThereEndpoint.cmd = vs_tr_is_there_endpoint;
        strcpy( isThereEndpoint.endpoint, endpoint );
        ControlResponse::CntrlResponse   cntrlResponse;
        memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
        return CntrlSR( (ControlInquiry *)&isThereEndpoint, sizeof(isThereEndpoint), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) )
               && cntrlResponse.res;
    }
    // end VS_TransportRouter_Implementation::IsThereEndpoint
	inline bool ServerVerificationFailed()
	{
		ControlInquiry::ServerVerificationFailed	verificatopnFaild;
		verificatopnFaild.cmd = vs_tr_serverVerificationFailed;
		return CntrlSR( (ControlInquiry *)&verificatopnFaild,sizeof(verificatopnFaild));
	}

    /////////////////////////////////////////////////////////////////////////////////////
};
// end VS_TransportRouter_Implementation struct

//////////////////////////////////////////////////////////////////////////////////////////
void VS_TransportRouter::PrepareToDie( void )
{
    if (imp)
    {
		imp->BaseServiceFullDisconnectAllEndpoints([](string_view cid, string_view ep, unsigned char hops)
		{
				return hops == 0 || ep != RegServerName;
		});
        InterlockedIncrement(&imp->isShutdown);
    }
}
void VS_TransportRouter::DisconnectAllEndpoints()
{
	if (imp) imp->BaseServiceFullDisconnectAllEndpoints([]
	(string_view, string_view, unsigned char) {return true; });
}
void VS_TransportRouter::DisconnectAllByCondition(const CheckEndpointPredT &pred)
{
	if (imp)
		imp->BaseServiceFullDisconnectAllEndpoints(pred);
}
bool VS_TransportRouter::GetStatistics(VS_TransportRouterStatistics &stat)
{
    return !imp ? false : imp->GetStatistics(stat);
}
VS_TransportRouter::VS_TransportRouter(VS_RoutersWatchdog* watchDog, const char * private_key_pass )
{
//VS_InitControlMemAllock(m_dbgMemMap);
    imp = new VS_TransportRouter_Implementation(watchDog, private_key_pass);
}
// end VS_TransportRouter::VS_TransportRouter

VS_TransportRouter::~VS_TransportRouter( void )
{
    if (imp)	delete imp;
//VS_WriteMemLeakToFile("TransportRouterLeaks.log");
}
// end VS_TransportRouter::~VS_TransportRouter

bool VS_TransportRouter::Init( const std::string& endpointName, VS_AccessConnectionSystem *acs, VS_TlsHandler* tlsHandler, const unsigned long maxServices, const unsigned long maxEndpoints, const unsigned long maxEndpointQueueMess, const unsigned long maxEndpointLackMs, const unsigned long maxConnSilenceMs, const unsigned long fatalSilenceCoef, const unsigned long maxPostMess, const unsigned long maxRecursionDepthSendMess, const unsigned long routerTickSizeMs )
{
    return !imp ? false : imp->Init( endpointName, acs, tlsHandler, maxServices, maxEndpoints, maxEndpointQueueMess, maxEndpointLackMs, maxConnSilenceMs, fatalSilenceCoef, maxPostMess, maxRecursionDepthSendMess, routerTickSizeMs );
}
// end VS_TransportRouter::Init

void VS_TransportRouter::SetIsRoamingAllowedFunc(const std::function<bool(const char*)>&f)
{
	if (imp) imp->funcIsRoamingAllowed = f;
}

bool VS_TransportRouter::IsInit( void ) const
{
    return !imp ? false : imp->IsInit();
}
// end VS_TransportRouter::IsInit

bool VS_TransportRouter::AddService(const char * serviceName, VS_TransportRouterServiceBase * service, bool withOwnThread, const bool permittedForAll)
{
	if (withOwnThread)
		return AddThreadedService(serviceName, service, permittedForAll);
	else
		return AddCallService(serviceName, service, permittedForAll);
	return false;
}

bool VS_TransportRouter::AddThreadedService( const char *serviceName,
                                     VS_TransportRouterServiceBase *service,
                                     const bool permittedForAll )
{
    return !imp ? false : imp->AddService( serviceName, service , permittedForAll );
}
// end VS_TransportRouter::AddService

bool VS_TransportRouter::AddCallService( const char *serviceName, VS_TransportRouterServiceBase *callService ,						const bool permittedForAll )
{
    return !imp ? false : imp->AddCallService( serviceName, callService ,permittedForAll);
}
// end VS_TransportRouter::AddCallService

bool VS_TransportRouter::RemoveService( const char *serviceName )
{
    if (imp)
		return imp->RemoveService( serviceName );
	return false;
}
// end VS_TransportRouter::RemoveService

void VS_TransportRouter::Shutdown( void )
{
    if (imp)	imp->Shutdown();
}
// end VS_TransportRouter::Shutdown

bool VS_TransportRouter::SetServCertInfoInterface(VS_ServCertInfoInterface *pInterface)
{
	return !imp ? false : imp->SetServCertInfoInterface(pInterface);
}

bool VS_TransportRouter::AddRouterMessExtHandler(VS_RouterMessExtHandlerInterface *ext_handler)
{
	return !imp ? false : imp->AddRouterMessExtHandler(ext_handler);
}

bool VS_TransportRouter::RemoveRouterMessExtHandler(VS_RouterMessExtHandlerInterface *ext_handler)
{
	return !imp ? false : imp->RemoveRouterMessExtHandler(ext_handler);
}

VS_TransportRouter_SetConnection *VS_TransportRouter::GetSetConnectionHandler()
{
	return imp;
}

const std::string& VS_TransportRouter::EndpointName() const
{
	static std::string dummy;
	return !imp ? dummy : imp->endpointName;
}
//////////////////////////////////////////////////////////////////////////////////////////

#endif
