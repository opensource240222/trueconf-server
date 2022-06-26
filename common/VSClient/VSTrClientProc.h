/**
 **************************************************************************
 * \file VSTrClientProc.h
 * (c) 2002-2014 TrueConf
 * \brief Declaration of transport client processor. This class communicate
 * whith server by Visicron protocol. in case of user-related events send messages
 * to GUI
 *
 * \b Project Client
 * \author SMirnovK
 * \date 18.12.2002
 *
 ****************************************************************************/
#ifndef VS_TR_CLIENT_PROC
#define VS_TR_CLIENT_PROC

/******************************************************************************
* Includes
******************************************************************************/
#define  SECURITY_WIN32 1

//#include "../AddressBookCache/VZOchat7.h"

#include "../net/EndpointRegistry.h"
#include "../std/cpplib/VS_Protocol.h"
#include "../statuslib/status_types.h"
#include "../std/cpplib/VS_SimpleStr.h"
#include "../std/cpplib/VS_WideStr.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../std/cpplib/VS_ConferenceID.h"
#include "../std/cpplib/VS_ClientCaps.h"
#include "../std/cpplib/VS_RcvFunc.h"
#include "../std/statistics/TConferenceStatistics.h"
#include "../streams/Client/VS_StreamClientReceiver.h"
#include "../streams/Client/VS_StreamClientSender.h"
#include "../streams/Client/VS_StreamClientTransmitter.h"
#include "transport/Client/VS_TransportClient.h"
#include "../FileTransfer/VS_FileTransfer.h"

#include "VSClientBase.h"
#include "VS_UserList.h"
#include "VS_RcvList.h"
#include "VSBwtClient.h"
#include "VSProxySettings.h"
#include "VS_ServerList.h"

#include <Windows.h>
#include <security.h>

/******************************************************************************
* Defines
******************************************************************************/
// 42 = 6.2.5
// 43 = 6.3.0 (new Rating, video [w,h & 7] caps)
// 44 = 6.3.0 (new scheme control bitrate for svc)
// 45 = 6.5.0 (rcv screen resolution in caps)
// 46 - fix h.264 portret mode
// 47 - added CONNECTFAILURE_METHOD
// 48 - 6.5.4 (multiplicity 2 & 4 caps)
// 49 - 6.5.6
// 50 - 6.6.1 diff part list
#define VSTR_PROTOCOL_VER	"50"

#define STATUS_LOGGEDIN			0x00010000							///< switch
#define STATUS_SERVAVAIL		0x00040000							///< switch
#define STATUS_INCALL			0x00080000							///< flag
#define STATUS_CONFERENCE		0x00100000							///< switch
#define STATUS_USERINFO			0x00200000							///< flag
#define STATUS_MESSAGE			0x00400000							///< flag
#define STATUS_COMMAND			0x00800000							///< flag
#define STATUS_REQINVITE		0x01000000							///< flag
/// defines for internal use
#define STATUS_INCALL_PROGRESS	0x10000000

/**
****************************************************************************
* \brief defines returned by service interpretes
******************************************************************************/
#define 	ERR_OK					0x00000000						///< expected result, action is successful
#define 	ERR_RET					0x00001000						///<GUI notify message
#define 	ERR_WARN				0x00002000						///< internal message, don't notify GUI

#define 	VSTRCL_ERR				0x00000010						///< internal processing errors
#define 	VSTRCL_ERR_INTERNAL		VSTRCL_ERR + ERR_RET + 0x1		///< some internal low-level error
#define 	VSTRCL_ERR_UNKREQ		VSTRCL_ERR + ERR_RET + 0x2		///<unknown request for ussed servise
#define 	VSTRCL_ERR_UNKREP		VSTRCL_ERR + ERR_RET + 0x3		///< unknown reply for ussed servise
#define 	VSTRCL_ERR_TIMEOUT		VSTRCL_ERR + ERR_RET + 0x4		///< mesage living time elapsed
#define 	VSTRCL_ERR_TIMEOUTSERV	VSTRCL_ERR + ERR_RET + 0x5		///< curr action was broken by user timeout
#define 	VSTRCL_ERR_UNKMESS		VSTRCL_ERR + ERR_RET + 0x6		///< unknown transport message type
#define 	VSTRCL_ERR_CONTEYNER	VSTRCL_ERR + ERR_RET + 0x7		///< container error
#define 	VSTRCL_ERR_METHOD	 	VSTRCL_ERR + ERR_RET + 0x8		///< unknown method
#define 	VSTRCL_ERR_PARAM	 	VSTRCL_ERR + ERR_RET + 0x9		///< unknown parametrs in message
#define 	VSTRCL_ERR_CURROP	 	VSTRCL_ERR + ERR_RET + 0xa		///< current operation not will be done
#define 	VSTRCL_ERR_SERVER	 	VSTRCL_ERR + ERR_RET + 0xb		///< current operation not will be done
#define 	VSTRCL_ERR_TRANSPOLD 	VSTRCL_ERR + ERR_RET + 0xc		///< transport old, not supported
#define 	VSTRCL_ERR_SERVNTVALID 	VSTRCL_ERR + ERR_RET + 0xd		///< target serv endpoint is not valid
#define 	VSTRCL_ERR_SERVBUSY 	VSTRCL_ERR + ERR_WARN+ 0xe		///< server busy
#define 	VSTRCL_ERR_SERVNOTAV 	VSTRCL_ERR + ERR_RET + 0xf		///< server is unreachable

#define 	VSTRCL_SSL		 		0x00000020						///< certificate errors
#define 	VSTRCL_SSL_OK	 		VSTRCL_SSL + ERR_WARN+ 0x0		///< no ssl errors
#define 	VSTRCL_SSL_ERR	 		VSTRCL_SSL + ERR_RET + 0x0		///< base of ssl errors
#define 	VSTRCL_SSL_7		 	VSTRCL_SSL + ERR_RET + 0x7		///< Secure connection to server could not be established
#define 	VSTRCL_SSL_8		 	VSTRCL_SSL + ERR_RET + 0x8		///< Server certificate expired
#define 	VSTRCL_SSL_9		 	VSTRCL_SSL + ERR_RET + 0x9		///< Server certificate is not yet valid
#define 	VSTRCL_SSL_10		 	VSTRCL_SSL + ERR_RET + 0xa		///< Certificate is invalid
#define 	VSTRCL_SSL_11		 	VSTRCL_SSL + ERR_RET + 0xb		///< Server name is invalid
#define 	VSTRCL_SSL_12		 	VSTRCL_SSL + ERR_RET + 0xc		///< Server is old

#define 	VSTRCL_LOGIN	 		0x00000040						///< Login/Logout servisies ret values
#define 	VSTRCL_LOGIN_OK		 	VSTRCL_LOGIN + ERR_RET + 0x0	///< Login/Logout OK
#define 	VSTRCL_LOGIN_INCPAR	 	VSTRCL_LOGIN + ERR_RET + 0x1	///< incorrect parametrs
#define 	VSTRCL_LOGIN_NOUSER	 	VSTRCL_LOGIN + ERR_WARN+ 0x2	///< no user logged in with current endpoint
#define 	VSTRCL_LOGIN_ALRLOGIN	VSTRCL_LOGIN + ERR_WARN+ 0x3	///< user already loggedin
#define 	VSTRCL_LOGIN_ALRLGOUT	VSTRCL_LOGIN + ERR_WARN+ 0x4	///< user already loggedout
#define 	VSTRCL_LOGIN_LGFROMS	VSTRCL_LOGIN + ERR_RET + 0x5	///< User "" loginned on other Application
#define 	VSTRCL_LOGIN_NOENDP		VSTRCL_LOGIN + ERR_WARN+ 0x6	///< reserved
#define 	VSTRCL_LOGIN_USERLIMIT	VSTRCL_LOGIN + ERR_RET + 0x7	///< Online Users Limit achived
#define 	VSTRCL_LOGIN_USDISBLD	VSTRCL_LOGIN + ERR_RET + 0x8	///< User Disabled
#define		VSTRCL_LOGIN_PWDEXPIRED VSTRCL_LOGIN + ERR_RET + 0x9	///< Password expired
#define 	VSTRCL_LOGIN_UPDATEACC	VSTRCL_LOGIN + ERR_RET + 0x10	///< Ok - update account
#define 	VSTRCL_LOGIN_BY_ECP		VSTRCL_LOGIN + ERR_RET + 0x20	///< Ok - try login by smart card (show web)

#define 	VSTRCL_CONF		 		0x00000080						///< Conference servisies ret values
#define 	VSTRCL_CONF_OK	 		VSTRCL_CONF + ERR_WARN+ 0x0		///< Conference proc OK
#define 	VSTRCL_CONF_CALL	 	VSTRCL_CONF + ERR_RET + 0x0		///< OK
#define 	VSTRCL_CONF_NOTCR 		VSTRCL_CONF + ERR_RET + 0x1		///< no enough resources for conference
#define 	VSTRCL_CONF_NOTEX		VSTRCL_CONF + ERR_WARN+ 0x2		///< specified conference doesnt exist
#define 	VSTRCL_CONF_ACCDEN 		VSTRCL_CONF + ERR_RET + 0x3		///< create access denied
#define 	VSTRCL_CONF_HNOMON 		VSTRCL_CONF + ERR_RET + 0x4		///< no money
#define 	VSTRCL_CONF_PBUSY	 	VSTRCL_CONF + ERR_RET + 0x5		///< partisipant is busy
#define 	VSTRCL_CONF_INVP	 	VSTRCL_CONF + ERR_RET + 0x6		///< invalid partisipant (access denied)
#define 	VSTRCL_CONF_PNOTAV	 	VSTRCL_CONF + ERR_RET + 0x7		///< partisipant not available now
#define 	VSTRCL_CONF_REJBYPART	VSTRCL_CONF + ERR_RET + 0x8		///< rejected by partisipant
#define 	VSTRCL_CONF_CONFBUSY	VSTRCL_CONF + ERR_RET + 0x9		///< conference is busy
#define 	VSTRCL_CONF_INVCONF		VSTRCL_CONF + ERR_RET + 0xa		///< invalid conference
#define 	VSTRCL_CONF_EXPIRED		VSTRCL_CONF + ERR_RET + 0xb		///< expired
#define 	VSTRCL_CONF_ACCEPTREJ	VSTRCL_CONF + ERR_RET + 0xc		///< accept was rejected (conference deleted)
#define 	VSTRCL_CONF_NOT_STARTED	VSTRCL_CONF + ERR_RET + 0xd		///< not started yet
#define 	VSTRCL_CONF_LOCALBUSY	VSTRCL_CONF + ERR_RET + 0xe		///< low local resoursec
#define 	VSTRCL_CONF_NAMEUSED	VSTRCL_CONF + ERR_RET + 0xf		///< MC with this name exist
#define 	VSTRCL_CONF_INVMULTI 	VSTRCL_CONF + ERR_RET + 0x10	///< OK - invite to multi
#define 	VSTRCL_CONF_INVPARAM	VSTRCL_CONF + ERR_RET + 0x11	///< invalid callId and pass for conf
#define 	VSTRCL_CONF_ONLINELIMIT	VSTRCL_CONF + ERR_RET + 0x12	///< Oline Conf Limit achived
#define 	VSTRCL_CONF_PASSREQ		VSTRCL_CONF + ERR_RET + 0x13	///< Password required to join at
#define 	VSTRCL_CONF_PASSWRONG	VSTRCL_CONF + ERR_RET + 0x14	///< Password wrong (for broadcast)
#define 	VSTRCL_CONF_NOFRIEND	VSTRCL_CONF + ERR_RET + 0x15	///< autoreject by "only for friends" option
#define 	VSTRCL_CONF_BADRATING	VSTRCL_CONF + ERR_RET + 0x16	///< autoreject by bad raiting
#define 	VSTRCL_CONF_USERTIMEOUT	VSTRCL_CONF + ERR_RET + 0x17	///< reject by timeout
#define 	VSTRCL_CONF_IAM_HOST 	VSTRCL_CONF + ERR_RET + 0x20	///< OK - join as host
#define 	VSTRCL_CONF_SND_FLTR 	VSTRCL_CONF + ERR_WARN+ 0x30	///< OK - Sender media filtr arrived
#define 	VSTRCL_CONF_INVITEREPLY	VSTRCL_CONF + ERR_RET + 0x40	///< OK - receive reply to invite to multi
#define 	VSTRCL_CONF_FECC_CNT	VSTRCL_CONF + ERR_RET + 0x41	///< ConferenceSrv, FarEnd Camera Control container

#define 	VSTRCL_RCV		 		0x00000100						///< MutiRcv
#define 	VSTRCL_RCV_NEW	 		VSTRCL_RCV + ERR_RET  + 0x00	///< new
#define 	VSTRCL_RCV_FAIL	 		VSTRCL_RCV + ERR_WARN + 0x01	///< curr op on rcv not sucs
#define 	VSTRCL_RCV_UPD 			VSTRCL_RCV + ERR_RET  + 0x10	///< changed
#define 	VSTRCL_RCV_REM 			VSTRCL_RCV + ERR_RET  + 0x20	///< removed
#define 	VSTRCL_RCV_ROLE			VSTRCL_RCV + ERR_RET  + 0x30	///< notyfy roles

#define 	VSTRCL_CHAT			 	0x00000200						///< Chat servisies ret values
#define 	VSTRCL_CHAT_OK		 	VSTRCL_CHAT + ERR_RET + 0x00	///< OK
#define 	VSTRCL_CHAT_EMPTY	 	VSTRCL_CHAT + ERR_WARN+ 0x01	///< There is no message
#define 	VSTRCL_CHAT_COMMAND	 	VSTRCL_CHAT + ERR_RET + 0x10	///< There is Command

#define 	VSTRCL_PING			 	0x00000300						///< Ping servisies ret values
#define 	VSTRCL_PING_OK		 	VSTRCL_PING + ERR_WARN+ 0x0		///< OK

#define 	VSTRCL_ACCEPT		 	0x00000400						///< Client servisies ret values
#define 	VSTRCL_ACCEPT_OK 		VSTRCL_ACCEPT + ERR_RET + 0x0	///< test accept ok
#define 	VSTRCL_ACCEPT_EXP 		VSTRCL_ACCEPT + ERR_WARN+ 0x1	///< test accept expired or not need

#define 	VSTRCL_UPDCNFG		 	0x00000500						///< Update confuguration servisies ret values
#define 	VSTRCL_UPDCNFG_OK 		VSTRCL_UPDCNFG + ERR_WARN+ 0x0	///< Update confuguration ok
#define 	VSTRCL_UPDCNFG_SETPROP	VSTRCL_UPDCNFG + ERR_RET + 0x0	///< Prop is gone
#define 	VSTRCL_UPDCNFG_SRVFLAG	VSTRCL_UPDCNFG + ERR_RET + 0x10	///< server flags

#define 	VSTRCL_USERS		 	0x00000600						///< User presence servisies ret values
#define 	VSTRCL_USERS_NORET		VSTRCL_USERS + ERR_WARN+ 0x0	///< no return
#define 	VSTRCL_USERS_OK			VSTRCL_USERS + ERR_RET + 0x0	///< Updating part list is ok + AB*16
#define 	VSTRCL_USERS_NVALID		VSTRCL_USERS + ERR_RET + 0x1	///< user not valid
#define 	VSTRCL_USERS_EXISTS		VSTRCL_USERS + ERR_RET + 0x2	///< user alredy exists in AB
#define 	VSTRCL_USERS_NFOUND		VSTRCL_USERS + ERR_RET + 0x3	///< not found (by email)
#define 	VSTRCL_USERS_DBERR		VSTRCL_USERS + ERR_RET + 0x4	///< DB error
#define 	VSTRCL_USERS_INVPAR		VSTRCL_USERS + ERR_RET + 0x5	///< DB error
#define 	VSTRCL_USERS_NEXISTS	VSTRCL_USERS + ERR_WARN + 0x6	///< user not exists in AB

#define 	VSTRCL_UPCONT	 		0x00000700						///< User presence containers
#define 	VSTRCL_UPCONT_OK	 	VSTRCL_UPCONT + ERR_RET + 0x0	///< OK
#define 	VSTRCL_UPCONT_RSTATUS 	VSTRCL_UPCONT + ERR_RET + 0x1	///< Clean Status of all users (set LogOff)

/**
 **************************************************************************
 * \brief Type of call from GUI
 ****************************************************************************/
enum MakeCall_Type {
	MCT_PRIVATE_CALL			= 0,
	MCT_BROADCAST_START			= 1,
	MCT_BROADCAST_JOIN			= 2,
	MCT_MULTI_JOIN				= 3,
	MCT_INTERCOM_JOIN			= 4,
	MCT_FREE_START				= 5,
	MCT_VIP_START				= 6,
	MCT_HALFPRIVATE_CALL		= 7,
	MCT_FREE_JOIN				= 8,
	MCT_VIP_JOIN				= 9,
	MCT_HALFPRIVATE_JOIN		= 10,
	MCT_CREATEMULTI				= 11,
	MCT_REQINVITE				= 12,
	MCT_ONE2MANY				= 13,
	MCT_ONE2MANY_PEND			= 14,
	MCT_CREATE_INTERCOM			= 15,
	MCT_INTERCOM_ONE2MANY		= 16,
	MCT_INTERCOM_ONE2MANY_PEND	= 17,
	MCT_ROLE					= 18,
	MCT_INTERCOM_ROLE			= 19
};

/**
****************************************************************************
* \brief Endpoint config class
******************************************************************************/
class CConfEndp
{
public:
	void Clean()
	{
		connTCPSucs.clear();
		connTCP.clear();
	}
	std::vector<std::unique_ptr<net::endpoint::ConnectTCP>> connTCP; ///< connects for test
	std::vector<net::endpoint::ConnectTCP*> connTCPSucs;             ///< sucs tested connects
};
/**
****************************************************************************
* \brief User description structure
******************************************************************************/
class CUserDesk
{
public:
	enum ConfState {
		CONF_STATE_NONE		 = 0,
		CONF_STATE_REQB		 = 1,
		CONF_STATE_COMING	 = 2,
		CONF_STATE_DONE		 = 4,
		CONF_STATE_REQEND	 = 8
	};
	VS_SimpleStr			Conference;
	VS_SimpleStr			UserName;
	VS_SimpleStr			DisplayName;
	VS_SimpleStr			CallId;
	VS_SimpleStr			Password;
	VS_SimpleStr			Cname;
	VS_SimpleStr			Cpass;
	VS_SimpleStr			ConfSimKey;
	VS_SimpleStr			TarifPlan;
	VS_SimpleStr			SessionKey;
	long					confType;
	VS_DirectConnect_Type	ConnectType;
	CConfEndp				config;
	int						confState;
	long					Rights;
	long					ConferenseKey;
	long					lfltr;
	long					sfltr;
	long					invNum;
	long					ServerType;
	long					ServerSubType;
	long					ServerScope;
	long					MaxParts;
	long					MaxCasts;
	long					TarifRestrictions;
	long					SepartionGroup;
	long					SVCMode;
	long					LStatus;
	CMRFlags				cmr_flags;
	struct PartInfo
	{
		PartInfo() : role(0), device_status(0), is_operator(false) {}
		VS_SimpleStr	dn;
		long			role;
		long			device_status;
		bool			is_operator;
	};
	std::map<VS_SimpleStr, PartInfo>	PartList;

	long					UCall;
	long					PView;

	VS_RcvList				RsvList;
	VS_ClientCaps			ClientCaps;
	boost::shared_ptr<VS_StreamClientSender>	Snd;
	boost::shared_ptr<VS_StreamClientReceiver>	Rcv;
	static boost::shared_ptr<VS_StreamClientTransmitter>	Trmtr;
	static VS_Lock						Trmtr_lock;

	static boost::shared_ptr<VS_StreamClientTransmitter> DirectUdpTrmtr;

	CUserDesk()	{ Clean();}
	~CUserDesk() {Clean();}
	void Clean() {
		VS_AutoLock lock(&Trmtr_lock);
		if (!!UserName)
			net::endpoint::Remove(UserName.m_str);
		Conference.Empty();
		UserName.Empty();
		DisplayName.Empty();
		CallId.Empty();
		Password.Empty();
		Cname.Empty();
		Cpass.Empty();
		ConfSimKey.Empty();
		TarifPlan.Empty();
		SessionKey.Empty();
		confType = 0;
		ConnectType = DIRECT_UNKNOWN;
		confState = CONF_STATE_NONE;
		Rights = 0;
		ConferenseKey = 0;
		lfltr = VS_RcvFunc::FLTR_ALL_MEDIA;
		sfltr = VS_RcvFunc::FLTR_ALL_MEDIA;
		invNum = 0;
		ServerType = 0;
		ServerSubType = 0;
		ServerScope = 0;
		MaxParts = 0;
		MaxCasts = 0;
		TarifRestrictions = 0;
		SepartionGroup = 0;
		SVCMode = 0;
		LStatus = 0;
		cmr_flags = CMRFlags::ALL;
		UCall = -1;
		PView = -1;
		RsvList.Empty();
		PartList.clear();
		if (Rcv) {
			Rcv->CloseConnection();
			Rcv.reset();
		}
		if (Snd) {
			Snd->CloseConnection();
			Snd.reset();
		}
		if (Trmtr) Trmtr->CloseConnection();
	}
	void CloseStreams() {
		VS_AutoLock lock(&Trmtr_lock);
		if (Rcv) Rcv->CloseConnection();
		if (Snd) Snd->CloseConnection();
		if (Trmtr) Trmtr->CloseConnection();
	}
	void SetNHPStream(const boost::shared_ptr<VS_StreamClientTransmitter> &trsmtr) {
		VS_AutoLock lock(&Trmtr_lock);
		if (Trmtr!=0) {
			Trmtr->CloseConnection();
		}
		Trmtr = trsmtr;
	}
	void SetUDPStream(const boost::shared_ptr<VS_StreamClientTransmitter>& trsmtr)
	{
		VS_AutoLock lock(&Trmtr_lock);
		if (DirectUdpTrmtr!=0) {
			DirectUdpTrmtr->CloseConnection();
		}
		DirectUdpTrmtr = trsmtr;
	}
};

/**
****************************************************************************
* \brief Structure to interact with GUI
******************************************************************************/
class VSApplicationStatus
{
public:
	static const int MAX_CONFINFO = 4;
	VSApplicationStatus(){dwLastReturn = 0; dwStatus = 0; CurrConfInfo = &ConfInfo[MAX_CONFINFO-1];}
	DWORD dwStatus;						///< flags(counter) for current state deteminig
	DWORD dwLastReturn;					///< last value returned by message intrepreters
	CUserDesk MyInfo;					///< info about me
	CUserDesk ConfInfo[MAX_CONFINFO];	///<  0= my, 1 = incaming private, 2 = temporary (inv, req inv)
	CUserDesk *CurrConfInfo;			///<  pointer to current ConfInfo
};

/**
****************************************************************************
* \brief transport client processor
******************************************************************************/
class CVSTrClientProc: public CVSThread, public VS_EndpointCallback, public VS_Lock, protected CVSInterface,
	public VS_FileTransfer::Events,
	public std::enable_shared_from_this<CVSTrClientProc>

{
	VS_UserPresence_Status m_StoredStatus;
	HANDLE				m_hRestrictMedia;
	unsigned int		m_podium_time;
	unsigned int		m_podium_start_t;
	int					m_HQAuto;
	unsigned int		m_SndStereo;
	unsigned int		m_RcvStereo;
	long				m_loadSystem;
	int					m_enchSLayers;
	VS_ClientType		m_client_type;
	enum e_ConnectState{
		CS_UNKNOWN,
		CS_CONNECTING,
		CS_CONNECTED,
		CS_DISCONNECTING,
		CS_DISCONNECTED
	};
	e_ConnectState		m_ConnectState;
	std::map<std::string/* user */,VS_ExtendedStatusStorage> m_ext_statuses;
	DWORD				m_MyThreadID;
protected:
	bool				m_bTranscoder;
public:
	VS_SimpleStr		m_PropAudioCupture;
	VS_SimpleStr		m_PropAudioRender;
	VS_SimpleStr		m_PropVideoCupture;
	VS_SimpleStr		m_PropDirectX;
	VS_SimpleStr		m_PropSysConfig_OS;
	VS_SimpleStr		m_PropSysConfig_Processor;
	VS_SimpleStr		m_PropCurrConfig;
	VS_SimpleStr		m_PropPerformance;
	int					m_AutoLogin;
	VS_SimpleStr		m_LastLogin;
	VS_SimpleStr		m_AppId;					///< appclication key
	VS_SimpleStr		m_DiskOS;					///< disk OS key
	VS_SimpleStr		m_AutoLoginKey;				///< Key from Server

	VS_SimpleStr		m_PrevBroker;
	VS_SimpleStr		m_CurrBroker;
	VS_SimpleStr		m_LastConfName;
	VS_SimpleStr		m_FailedEndpoint;

	VS_ServerList		m_ServerList;

	VS_SimpleStr		m_defaultDomain;

	bool				m_AutoConnect;				///< autoconnect flag
	bool				m_CleanOnSubscribe;			///< flag to clean sibscribers list

	static DWORD		m_dwSuppressDirectCon;		///< do not try dirrect connection
	static DWORD		m_dwUseNhp;					///< UseNhp (if dirrect connection used)
	static DWORD		m_dwForceNhp;				///< forse NHP
	static DWORD		m_dwMarkQos;				///< mark sender stream as Qos marked


	VSApplicationStatus m_Status;					///< application interfase class
	HWND				m_ExternalWindow;			///< GUI notifay window
	DWORD				m_ExternalThread;			///< notified Thread

	VS_UserList			m_OnlineUsers;				///< online users seqtrack
	VS_UserList			m_PartsTrack;				///< part list seqtrack
	VS_ContainersMap	m_PresenceContainers;		///< container map for user presence service
	VS_ContainersMap	m_RoleEventContainers;		///< container map for Role events
	VS_ContainersMap	m_InviteContainers;			///< container map for Invites
	VS_ContainersMap	m_MessContainers;			///< container map for messages
	VS_ContainersMap	m_CommandContainers;		///< container map for commands

	VS_ContainersMap	m_AuthECPContainers;		///< container map for Authentication by ECP
	VS_ContainersMap	m_FECC_Containers;		///< container map for Authentication by ECP

	VS_Container		m_PropContainer;			///< propeties
	VS_Container		m_UserProp;					///< user propeties
	VS_Container		m_SendPropContainer;		///< prop pendin to send
	VS_Container		m_PartList;
	VSBwtClient			*m_bwt;						///< network test class
	VSProxySettings		m_Proxy;
	enum UpdateAB_Types {UTYPE_BOOL, UTYPE_LONG, UTYPE_DBL, UTYPE_CHAR, UTYPE_VOID};
	VS_Container		m_UpdateABContainer;		///< propeties
	VS_MediaFormat		m_SourceFmt;
#ifdef VZOCHAT7
	VS_ContainersMap    m_ForkContainers;			///< d78 fork code containers
#endif
	HANDLE				m_NhpEvent;
	HANDLE				m_DirectUdpConnEvent;
	HANDLE				m_LogoutEvent;
	int					m_SrvProtocolVersion;
	long				m_EndpointType;
	unsigned long		m_DirectPort;
	long				m_SrvFlags;

/// shell methods
	CVSTrClientProc(CVSInterface* pParentInterface);
	~CVSTrClientProc();
	DWORD ComposeSend(VS_Container &cnt, const char* service, const char* server, const char *user, unsigned long timeout);
	DWORD Init(HWND wnd);
	DWORD Init(DWORD threadId);
	DWORD Loop(LPVOID lpParameter);
	DWORD ConnectServer(const char *szServer);
	DWORD DoConnect(bool next = false);
	// VS_EndpointCallback
	void OnConnect(const char *to_name, const long error = 0);
	void OnSidChange(const char *old_name, const char *new_name);

	void Servises();

/// begin GUI interface
	boost::shared_ptr<VS_StreamClientSender> GetConferenseSender();
	boost::shared_ptr<VS_StreamClientReceiver> GetConferenseReceiver();
	bool	GetSimKey(char *UserName, VS_BinBuff &key);
	int 	GetMyName(char *UserName, char *FirstName = 0, char *LastName = 0, char *Email = 0);
	void	GetOtherName(char *UserName, char *FirstName = 0, char *LastName = 0, char *Email = 0);
	DWORD	GetProperties(const char* Name, char* Propeties);
	DWORD	GetProperties(const char* Name, VS_SimpleStr &Propeties);
	int		GetMessage(int Id, char* From, char* Mess, char* to = 0, char* Dn = 0, long long *time = 0);
	void	GetCommand(int Id, char* From, char* Command);
	void	GetUsersList(void **List, int book);
	DWORD	GetUserStatus(char* CallId);
	DWORD	GetUserInformation(char*szUID,char* fs[16]);
	DWORD	GetUserPhoto(char* user, void **photo, unsigned long *photoSize, char**type);
	DWORD	GetAutoLogin();
	DWORD	SetAutoLogin(int autologin);
	DWORD	LoginUser(char *UserName, char* Password, int AutoLogin, bool noEncript = false,const VS_ClientType cl_type = CT_SIMPLE_CLIENT);
	DWORD	LogoutUser(bool clearAutoLogin);
	DWORD	PlaseCall(char *CallId, int type = CT_PRIVATE);
	DWORD	Join(char *CallId, char* pass = 0, long type = CT_MULTISTREAM, char* Conference = 0);
	DWORD	ConnectReceiver(char *name, long fltr);
	DWORD	ConnectSender(long fltr);
	DWORD	ConnectServices(long fltr);
	DWORD	Kick(const char* user);
	DWORD	Ignore(char *user);
	DWORD	CreatePublicConf(VS_Conference_Type Type, char* pass = 0);
	DWORD	CreateMultiConf(char* name, char* pass, VS_Conference_Type Type, int maxUsers, long SubType, long scope = GS_PERSONAL);
	DWORD	InviteToMulti(char* user);
	DWORD	ReqInviteToMulti(char* host);
	DWORD	Accept();
	DWORD	Reject(char* conf = 0, char* user = 0, long cause = REJECTED_BY_PARTICIPANT);
	DWORD	Hangup(int del = 0);
	DWORD	InviteReply(long result);
	DWORD	UpdateDisplayName(const char *alias, const wchar_t *displayName, bool updateImmediately);

	DWORD	QueryRole(char *name, long role);
	DWORD	AnswerRole(char *name, long role, long result);
	DWORD	DeviceStatusChanged();

	DWORD	ChatSend(const char *Message, const char *To = NULL);
	DWORD	SendCommand(const char *Command, const char *To = NULL);
	DWORD	SendConfStat(TConferenceStatistics *snd, TConferenceStatistics *rcv);
	void	SendConferncePicture(void* buff, unsigned long size);

	DWORD	AddUserToAddressBook(char *user, long addressBook, char* email = NULL);
	DWORD	BanUser(char *user, long abuse);
	DWORD	RemoveFromAddressBook(char *user, long addressBook);
	DWORD	SearchAddressBook(const char *query, long addressBook, long hash);

	int 	GetRcvAction(int Id, char *name, boost::shared_ptr<VS_StreamClientReceiver> &rcv, long &fltr);
	bool 	GetRcvDevStatus(char *name, long *dvs);

	int 	GetMediaFormat(char *name, VS_MediaFormat &fmt, VS_ClientCaps *pCaps = 0);
	void 	SetMediaFormat(VS_MediaFormat &fmt, int rating = 0, int level = 0, int level_group = 0);
	void 	PrepareCaps(bool groupconf);
	void	SetSystemLoad(int load);
	void	SetFrameSizeMBUser(char *name, int frameSizeMB);
	void	SetScreenResolution(long width, long height);
	void	SetAutoMode(int autoMode);
	HANDLE	GetRestrictHandle();

	bool	IsTrackSent(int track);
	void	MonitorStatus();
	void	UpdateStatus(VS_UserPresence_Status status/*, const char * ext = 0*/);
	VS_UserPresence_Status GetCurrUserPresenceStatus();

	void	UpdateExtStatus(const char *name, const int value); //ext_status, device_type, camera, mic
	void	UpdateExtStatus(const char *name, const wchar_t *value); //status_descripion
	////void	UpdateExtStatus(const char *name, const VS_FileTime &date_time); //status_change_time
	void	UpdateExtStatus(const int fwd_type, const wchar_t *fwd_call_id, const int fwd_timeout, const wchar_t *timeout_call_id); /// CALLFWD_ST_NAME
	void	Subscribe(char** user, int num, bool plus); ///

//protected:
/// begin Message interpreter sevises
	void LoginUserSrv			(VS_ClientMessage &tMsg);
	void ConferenceSrv			(VS_ClientMessage &tMsg);
	void UpdateConfigurationSrv	(VS_ClientMessage &tMsg);
	void UserPresenceSrv		(VS_ClientMessage &tMsg);
	void ChatSrv				(VS_ClientMessage &tMsg);
	void ClientSrv				(VS_ClientMessage &tMsg);
	void TorrentSrv				(VS_ClientMessage &tMsg);

	void GetAllPartList(const char * conf);
	void GetExtendedStatuses(VS_Container &cnt);

	stream::Track m_tracks[256];
	unsigned m_nTrack;
	void  SetTracks(const stream::Track* tracks, unsigned nTrack);
	void  SetEndpointType(long type) {m_EndpointType = type;}

	DWORD GetConferense(char *Conference, char *Endpoint);	//retrive curr conf params
	void  GetConferense();									// retrive curr conf sender and reciever
	DWORD SendFailureConnect(char *Endpointchar, char *Conference, char* owner);
	DWORD AutoLogin();
	DWORD CheckUserLoginStatus();
	DWORD GetAllUserStatus();
	DWORD CreateConference(long MaxPartisipants, long Duration, long type, char* name = 0, char* pass = 0, long SubType = GCST_FULL, long scope = GS_PERSONAL);
	DWORD DeleteConference(const char* Conf);
	DWORD PingConference();
	DWORD Invite();
	DWORD UpdateConfiguration(bool tryAccept);
	DWORD GetAppProperties();
	DWORD SetEpProperties(const char* name, const char* prop, bool sendImmediate = true);
	DWORD ClientTryAcceptDirect(const char* conference, const char* endpoint);
	DWORD SetDiscoveryServise(const char* sevice);
	int   GetServers(int size, unsigned char*pName);
	DWORD InstallTransportSystem();
	DWORD DisconnectProcedure(bool DoLogout = true);
	DWORD SendUserConnectInfo(const char* server, const char * user);
	DWORD UpdateMBpsUsers();
	// methods
	DWORD Method_ConferenceCreated(VS_Container& cnt);
	DWORD Method_ConferenceDeleted(VS_Container& cnt);
	DWORD Method_UserRegistrationInfo(VS_Container& cnt);
	DWORD Method_ConfigurationUpdated(VS_Container& cnt);
	DWORD Method_Invite(VS_Container& cnt);
	DWORD Method_Accept(VS_Container& cnt);
	DWORD Method_Reject(VS_Container& cnt);
	DWORD Method_Join(VS_Container& cnt);
	DWORD Method_ReceiverConnected(VS_Container& cnt);
	DWORD Method_SenderConnected(VS_Container& cnt);
	DWORD Method_ReqInvite(VS_Container& cnt);
	DWORD Method_InviteToMulti(VS_Container& cnt);
	DWORD Method_InviteReply(VS_Container& cnt);
	DWORD Method_ListenersFltr(VS_Container& cnt);
	DWORD Method_SendCommand(VS_Container& cnt);
	DWORD Method_RoleEvent(VS_Container& cnt);
	DWORD Method_FECC(VS_Container& cnt);
	DWORD Method_DeviceStatus(VS_Container& cnt);
	DWORD Method_ConferenceSettings(VS_Container& cnt);
	void NotifyDvs(const char* user, long dvs);
	inline const char * ReturnBroker(const char* conf);
	bool ReadPropNetConfig();
	bool ReadProps();
	void SetOnlineStatus(bool set);
	void SetIncallStatus(bool set, DWORD ret = 0, int invNum = 0);
	void SetReqInviteStatus(bool set);
	bool UpdateEndpointInfo(const char *ep, const void *epConfig, unsigned long size);

	DWORD BwtStart(HWND hwnd){return m_bwt->Start(hwnd, m_CurrBroker);}
	DWORD BwtStop(){return m_bwt->Stop();}
	DWORD BwtGet(void * out, int id){return m_bwt->Get((VSBwtClientMess*)out, id);}
	void  BwtWizardOn(int mode);
	static void SetFirewall();

	enum TimerObject_Type
	{
		TIME_NONE = 0,
		TIME_LGIN, TIME_LGOUT, TIME_CHUSLS,
		TIME_CRCONF, TIME_JOIN, TIME_ACCEPT, TIME_DLCONF0, TIME_DLCONF1,
		TIME_TSTACC, TIME_INCALL, TIME_CHECKSRV,
		TIME_SND_PINGCONF, TIME_CONNECT, TIME_SESS_RENEW,
		TIME_SND_MBPSLIST,
        TIME_TR_UPDATE_STATUS,
		TIME_NOTIFYPARTLIST,
		TIME_MAX_TIMER
	};
	void SetTimerObject(int type, DWORD msec = -1);
	void TestTimerObjects();
	bool IsTimerObjectElapsed(int type);
	void RemoveTimerObjects(int type);
	int m_TimerObjects[TIME_MAX_TIMER+1];

	/// NTLM functions and methods
	CredHandle    m_sec_token;
	TimeStamp     m_sec_token_expiry;
	CtxtHandle    m_sec_ctxt;
	SecPkgInfo*   m_sec_pack;
	PSecurityFunctionTable m_sec;
	bool          m_sec_fail;

	void Authorize(VS_Container& cnt);
	bool RenewToken();

	BOOL PostStatusMessage(UINT msg, WPARAM wparam, LPARAM lparam);
	void PostProc(DWORD Ret, DWORD MeesId = 0);

	void SetClientType(const VS_ClientType ct);

	std::unique_ptr<VS_FileTransfer> m_torrent;

    enum TrStatus { start_error, download_started, download_complete, undefined_error };

	struct TrStat {
		TrStatus status;
		uint64_t size;
		uint64_t downloaded;
		uint64_t uploaded;
		uint32_t u_speed;
		uint32_t d_speed;
		uint32_t peers;

		TrStat() : status(undefined_error), size(0), downloaded(0), uploaded(0), u_speed(0), d_speed(0), peers(0) {}
		TrStat(TrStatus s) : status(s), size(0), downloaded(0), uploaded(0), u_speed(0), d_speed(0), peers(0) {}

		inline bool operator==(const TrStat& rhs) {
			return	status == rhs.status &&
					size == rhs.size &&
					downloaded == rhs.downloaded &&
					uploaded == rhs.uploaded &&
					u_speed == rhs.u_speed &&
					d_speed == rhs.d_speed &&
					peers == rhs.peers;
		}
		inline bool operator!=(const TrStat& rhs) {
			return !operator==(rhs);
		}
	};

    struct UploadDownload {
        std::string name, path, dir, magnet, url;
        std::vector<std::string> user_id_list;
        std::string conf_id;
        TrStat stat;

		bool update_needed() const {
			return !stat.size || stat.status != download_complete || (stat.status == download_complete && stat.uploaded != stat.size);
		}
    };
    std::map<std::string /* id */, UploadDownload> m_items;
    std::vector<std::string> m_trackers;

    enum VS_TorrentResult {
        TR_NO_ERROR = 0,
        TR_MESSAGE_WITHOUT_MAGNET,
        TR_P2P_ONLY,
        TR_UNKNOWN_ERROR,
        TR_INVALID_FILELIST,
    };

    std::string m_server_ip;
    unsigned short m_server_port;

    unsigned int m_dw_lim, m_up_lim; // bytes per sec

	void TrInit(int flags);
    void TrSendFile(const char *filePath, const std::vector<std::string> &user_id_list, std::string& id);
    void TrSendFile(const char *filePath, const char *conf_id, std::string &id);
    void TrDownloadFile(const char *magnet, const char *distPath, std::string &id);
    void TrSetLimits(unsigned int dw_lim, unsigned int up_lim);

    void _TrSendFile(const char *filePath, const std::vector<std::string> &user_id_list, const char *conf_id, std::string& id);
    std::string TrGenId(const char *filePath, const std::vector<std::string> &user_id_list, const char *conf_id);

    bool TrUpdateStatus();
	TrStatus TrGetStatus(const std::string &magnet);
    void TrGetStat(const std::string &magnet, TrStat &stat);

    void (__stdcall *pFileSendStartedCB)(const char * /* id */, bool /* result */, const char * /* magnetLink */, const char * /* URL */);
    void (__stdcall *pFileDownloadStatusCB)(const char * /* id */, int /* status */, const char * /* comment */);

    bool TrOnAskUser(const std::string &id);
    void TrOnReadyToSend(const std::string &magnet, const std::string &id);

    bool Share_Method(VS_Container &cnt);
    void TrSendUploadMsg(const UploadDownload &up);

	//VS_FileTransferInterface callbacks
	void onReadyToSend(const std::string &magnet, const std::string &to) override { TrOnReadyToSend(magnet, to); };
	void onPeerConnected(const std::string &info_hash, const VS_FileTransfer::Endpoint &endpoint) override { assert(false); };
	bool onAskUser(const std::string &hash, const std::string &filename, const std::string &from, uint64_t fileSize) override { return TrOnAskUser(from); };
	void onMetadataSignal(const std::string &infoHash, const std::string &to) override { assert(false); };
	void onError(const std::string &/*info_hash*/, VS_FileTransfer::eErrorCode /*err*/) override { assert(false); };


private:
	VS_Container m_LoginRetryCnt;
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
	std::unique_ptr<struct VS_ContainerData> m_result_cnt; // Used to store raw VS_Container data returned by ProcessFunction until the next call to ProcessFunction.

	DWORD ManageGroups_CreateGroup(VS_WideStr gname);
	DWORD ManageGroups_DeleteGroup(long gid);
	DWORD ManageGroups_RenameGroup(long gid, VS_WideStr gname);
	DWORD ManageGroups_AddUser(long gid, VS_SimpleStr user);
	DWORD ManageGroups_DeleteUser(long gid, VS_SimpleStr user);

	DWORD SetMyLStatus(long lstatus);
	DWORD ClearAllLStatuses();
};

#endif
