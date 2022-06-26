/*************************************************
 * $Revision: 7 $
 * $History: VS_ParticipantDescription.h $
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 10.02.12   Time: 10:13
 * Updated in $/VSNA/Servers/ServerServices/Types
 * - update svc server implementation
 * - add svc server capability
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 26.09.11   Time: 16:35
 * Updated in $/VSNA/Servers/ServerServices/types
 * m_lstatus
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 6.05.10    Time: 21:52
 * Updated in $/VSNA/Servers/ServerServices/Types
 * - bugfix#7361
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 10.02.10   Time: 15:26
 * Updated in $/VSNA/Servers/ServerServices/Types
 * - ssl streams for group conf
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 20.02.09   Time: 19:26
 * Updated in $/VSNA/Servers/ServerServices/types
 * - AppId added in ParticipantDescr (invite, join log)
 * - 2 DBs supported
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
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 9.07.07    Time: 20:06
 * Updated in $/VS2005/Servers/ServerServices/types
 * - broadcast status added
 * - some wishes for gui for device status
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 4.07.07    Time: 19:25
 * Updated in $/VS2005/Servers/ServerServices/types
 * - device statuses added
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 26.04.07   Time: 19:57
 * Updated in $/VS2005/servers/serverservices/types
 * - added protcol version to SysMess (less load)
 * - removed chat spam in Group conf for pats whith protocol ver>19
 * - memory leaks removed (for part-s)
 * - Listener filtr sending decreased (only changes now)
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 20.04.07   Time: 20:50
 * Updated in $/VS2005/Servers/ServerServices/types
 * - UMC - added roles
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/ServerServices/types
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 1.08.05    Time: 19:25
 * Updated in $/VS/Servers/ServerServices/Types
 * - multicast, last interation
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 27.07.05   Time: 19:57
 * Updated in $/VS/Servers/ServerServices/Types
 * -multicast beta 2
 *
 * *****************  Version 20  *****************
 * User: Stass        Date: 4.06.04    Time: 12:47
 * Updated in $/VS/Servers/ServerServices/Types
 * added multi invite logging
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 11.03.04   Time: 20:17
 * Updated in $/VS/Servers/ServerServices/Types
 * new caps scheme
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 25.02.04   Time: 19:06
 * Updated in $/VS/Servers/ServerServices/Types
 * added resend for formats in broker
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 12.01.04   Time: 14:50
 * Updated in $/VS/servers/serverservices/types
 * added rigths RCV Chat
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 9.01.04    Time: 20:46
 * Updated in $/VS/Servers/ServerServices/Types
 * send/receive chat messages for HPrivate guests
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 25.11.03   Time: 19:07
 * Updated in $/VS/servers/serverservices/types
 * removed often clicks bugs
 * added conference pings (protocol version now 9)
 * added check by conference pings (exparation timeouts)
 * added sending protocol version in some messages
 *
 * *****************  Version 14  *****************
 * User: Stass        Date: 14.11.03   Time: 13:16
 * Updated in $/VS/Servers/ServerServices/Types
 * new participant types
 *
 * *****************  Version 13  *****************
 * User: Stass        Date: 10.11.03   Time: 19:22
 * Updated in $/VS/Servers/ServerServices/Types
 * added part type for Intercom
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 6.11.03    Time: 18:54
 * Updated in $/VS/Servers/ServerServices/Types
 * send video audio for mutipart
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 4.11.03    Time: 19:49
 * Updated in $/VS/Servers/ServerServices/Types
 * INTERKOM
 * new debug level
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 15.10.03   Time: 12:14
 * Updated in $/VS/Servers/ServerServices/Types
 * fixed multi number
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 9.10.03    Time: 19:44
 * Updated in $/VS/Servers/ServerServices/Types
 * new methods in client
 * new files in std...
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 6.10.03    Time: 19:02
 * Updated in $/VS/servers/serverservices/types
 * conference logging rearranged
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 2.10.03    Time: 19:18
 * Updated in $/VS/Servers/ServerServices/Types
 * added coonections in multistream conf
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 30.09.03   Time: 17:15
 * Updated in $/VS/Servers/ServerServices/Types
 * vs_strList class added
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 29.09.03   Time: 19:39
 * Updated in $/VS/Servers/ServerServices/Types
 * correct user id in confPartMap
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 28.09.03   Time: 16:00
 * Updated in $/VS/servers/serverservices/types
 * первые наброски
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 23.09.03   Time: 19:30
 * Updated in $/VS/Servers/ServerServices/types
 * adjusted times
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 15.09.03   Time: 19:29
 * Updated in $/VS/Servers/ServerServices/Types
 * redirect on destroy clauster service
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 5.09.03    Time: 14:40
 * Created in $/VS/Servers/ServerServices/Types
 * Moved types out of BrokerServices project
 *
 *************************************************/

#ifndef VS_PARTICIPANTDESCRIPTION_H
#define VS_PARTICIPANTDESCRIPTION_H

#include "../cpplib/VS_UserID.h"
#include "../cpplib/VS_ConferenceID.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../cpplib/VS_Protocol.h"

#include <chrono>
#include <cstdint>

class VS_ParticipantDescription
{
public:
	enum Type {
		PRIVATE_HOST        = 00,
		PRIVATE_MEMBER      = 01,
		PUBLIC_HOST         = 10,
		PUBLIC_MEMBER       = 11,
		HPRIVATE_HOST       = 30,
		HPRIVATE_MEMBER     = 31,
		HPRIVATE_GUEST      = 32,
		MULTIS_MEMBER       = 51,
		MULTIS_INVITE       = 52,
		INTERCOM_MEMBER     = 61
	};
	enum Disconnect_Cause {
		DISCONNECT_UNKNOWN,
		DISCONNECT_HANGUP,
		DISCONNECT_LIMIT,
		DISCONNECT_LOGOFF,
		DISCONNECT_ENDCONF,
		DISCONNECT_BYSTREAM,
		DISCONNECT_BYKICK,
		DISCONNECT_BYBAN,
		DISCONNECT_SERVER_RESTART,
		DISCONNECT_BYSWITCH,
		DISCONNECT_BYADDLING
	};

	enum Rights {
		RIGHTS_NONE     = 0x00000000,
		RIGHTS_NORMAL   = 0x00000001,
		RIGHTS_RCV_LIST = 0x00000002,
		RIGHTS_RCV_CHAT = 0x00000004
	};

	vs_user_id			m_user_id;	//c
	vs_conf_id			m_conf_id;	//c
	std::string			m_displayName; //c
	VS_SimpleStr		m_server_id;//c
	VS_SimpleStr		m_appID;
	VS_SimpleStr		m_symKey;
	unsigned int		m_svc_mode;
	int					m_type;		//c
	long				m_role;		//c
	long				m_lfltr;	//c
	std::chrono::system_clock::time_point			m_joinTime;	//c
	std::chrono::system_clock::time_point			m_leaveTime;//c
	std::chrono::steady_clock::time_point			m_addledTick;//c
	std::chrono::system_clock::time_point			m_podiumTime;
	double				m_limit;
	double				m_decLimit;	//c
	Disconnect_Cause	m_cause;	//c
	unsigned int		m_bytesRcv;
	unsigned int		m_bytesSnd;
	unsigned int		m_reconRcv;
	unsigned int		m_reconSnd;
	int					m_version;	//c
	int					m_rights;	//c
	int					m_port;		//c
	long				m_confKey;	//c
	long				m_devStatus;//c
	long				m_brStatus;	//c
	VS_SimpleStr		m_host;		//c
	VS_SimpleStr		m_mcast_ip;	//c
	VS_BinBuff			m_caps;		//c

	double				m_charge1, m_charge2, m_charge3;

	long                m_lstatus;
	long                m_ClientType;
	CMRFlags            m_CMRFlags;
	VS_VideoSourceType  m_videoType;
	bool                m_IsOperatorByGroups;

	VS_ParticipantDescription(): m_svc_mode(0x00000000), m_type(0), m_role(0), m_lfltr(~0), m_limit(0.), m_decLimit(0.),
		m_cause(DISCONNECT_UNKNOWN) , m_bytesRcv(0), m_bytesSnd(0),
		m_reconRcv(0), m_reconSnd(0), m_version(0), m_rights(RIGHTS_NONE), m_port(0), m_confKey(0), m_devStatus(1|1<<16), m_brStatus(0),
		m_charge1(0.0),m_charge2(0.0),m_charge3(0.0), m_lstatus(0), m_ClientType(0), /*simple client*/m_CMRFlags(CMRFlags::NONE), m_videoType(VST_UNKNOWN),
		m_IsOperatorByGroups(false)
	{}
	~VS_ParticipantDescription(){};

	static void* Factory(const void* pd){return new VS_ParticipantDescription(*(VS_ParticipantDescription*)pd);}
	static void Destructor(void* pd) {			delete (VS_ParticipantDescription*) pd; 	}
};

#endif // VS_PARTICIPANTDESCRIPTION_H
