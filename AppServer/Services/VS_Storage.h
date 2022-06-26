/**
 ****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 *
 * Project: Visicron server services
 *
 * $Revision: 29 $
 * $History: VS_Storage.h $
 *
 * *****************  Version 29  *****************
 * User: Ktrushnikov  Date: 31.01.12   Time: 17:35
 * Updated in $/VSNA/Servers/AppServer/Services
 * - count m_gateway + m_transcoders for check with number in license
 *
 * *****************  Version 28  *****************
 * User: Ktrushnikov  Date: 15.12.11   Time: 10:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - count CT_TRANSCODER separatly from online_users
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 11.02.11   Time: 16:58
 * Updated in $/VSNA/Servers/AppServer/Services
 * VCS 3.2
 * - License: add Roaming users to OnlineUsers of our server
 * - Join & ReqInvite check for license for Roaming users
 *
 * *****************  Version 26  *****************
 * User: Ktrushnikov  Date: 22.06.10   Time: 23:41
 * Updated in $/VSNA/Servers/AppServer/Services
 * Arch 3.1: NamedConfs added
 * - BS has Conference service
 * - Join_Method can create/join conf (if came from BS) or post request to
 * BS (if came from user)
 *
 * *****************  Version 25  *****************
 * User: Mushakov     Date: 8.04.10    Time: 16:03
 * Updated in $/VSNA/Servers/AppServer/Services
 * - seporate counter for online_user realized
 *
 * *****************  Version 24  *****************
 * User: Mushakov     Date: 19.03.10   Time: 17:59
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - ClientType is received from client
 * - Endpoint function removed from code
 * - arr_key logged inRegistryServer
 *
 * *****************  Version 23  *****************
 * User: Mushakov     Date: 26.02.10   Time: 15:48
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - mobile client supported (AuthService)
 * - new cert
 *
 * *****************  Version 22  *****************
 * User: Ktrushnikov  Date: 28.01.10   Time: 17:32
 * Updated in $/VSNA/servers/appserver/services
 * VCS:
 * - Interface added to ConfRestriction for SetUserStatus in Registry
 * - don't delete MultiConf registry key at startup of VCS (AS)
 *
 * *****************  Version 21  *****************
 * User: Mushakov     Date: 11.12.09   Time: 17:50
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - remive VCS_BUILD from VS_Storage
 *
 * *****************  Version 20  *****************
 * User: Ktrushnikov  Date: 6.11.09    Time: 19:55
 * Updated in $/VSNA/Servers/AppServer/Services
 * VS_ReadLicense
 * - fix with break
 * - LE_GUEST_LOGIN=9 added
 * VS_Storage
 * - m_num_guest - count of loggedin guests
 * VCSAuthService
 * - DeleteUser: guest from map ("registry")
 * VCSStorage
 * - DeleteUser added to interface
 * VS_RegistryStorage
 * - Guest login supported
 * - Check Guest password
 * - Check Count of Guests by License
 * - Read Shared Key from registry
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 4.11.09    Time: 18:49
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 2.11.09    Time: 14:51
 * Updated in $/VSNA/Servers/AppServer/Services
 * -store ab in vs_map
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 1.10.09    Time: 15:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * - autoblock users with bad rating (bugfix#6354)
 *
 * *****************  Version 16  *****************
 * User: Ktrushnikov  Date: 15.09.09   Time: 16:39
 * Updated in $/VSNA/servers/appserver/services
 * - Rating added
 * - display_name type changed to char* (from wchar_t)
 * - send BS Events to different service (AUTH,FWD)
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 3.09.09    Time: 12:03
 * Updated in $/VSNA/Servers/AppServer/Services
 * - AS abooks moved to VS_Storage
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 9.04.09    Time: 11:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * default realization of virtual method added
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 31.03.09   Time: 19:16
 * Updated in $/VSNA/Servers/AppServer/Services
 * Reg server added
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 27.09.08   Time: 21:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * - removed groupconference storage
 * - removed unnecessary conference logging
 * - create conference and join rewrited
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 24.06.08   Time: 17:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - some fix for group conf
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 14.05.08   Time: 21:25
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added ktrushnikov_Redirect
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 4.02.08    Time: 16:27
 * Updated in $/VSNA/Servers/AppServer/Services
 * new id
 *
 * *****************  Version 8  *****************
 * User: Dront78      Date: 28.12.07   Time: 18:04
 * Updated in $/VSNA/Servers/AppServer/Services
 * Service updates.
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Updated in $/VSNA/Servers/AppServer/Services
 * BS - new iteration
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 26.11.07   Time: 19:55
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 26.11.07   Time: 17:51
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 26.11.07   Time: 16:08
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed storage
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 26.11.07   Time: 15:15
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 22.11.07   Time: 20:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * new iteration
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 15.11.07   Time: 15:28
 * Created in $/VSNA/Servers/AppServer/Services
 * updates
 *
 ****************************************************************************/

/**
* \file VS_Storage.h
* Server Database Storage class definition
*
*/
#ifndef VS_SERVER_STORAGE_H
#define VS_SERVER_STORAGE_H


#include "std/cpplib/VS_MapTpl.h"
#include "std/cpplib/VS_WideStr.h"
#include "std/cpplib/VS_SimpleStr.h"
#include "transport/VS_SystemMessage.h"
#include "ServerServices/VS_ConfRestrictInterface.h"
#include "std/CallLog/VS_ConferenceDescription.h"
#include "std/cpplib/VS_RcvFunc.h"
#include "std-generic/cpplib/StrCompare.h"

#include "std-generic/compat/map.h"
#include "std-generic/compat/set.h"
#include "std/cpplib/fast_mutex.h"

class VS_ConfKick;
typedef std::tuple<std::string/*user_id*/, VS_BinBuff/*caps*/, int32_t/*fltr*/, std::string/*display_name*/, int32_t/*role*/> part_start_info;
typedef std::pair<vs_user_id, VS_SimpleStr> tPartServer; // userId, serverId

struct InviteInfo
{
	InviteInfo() {}

	InviteInfo(const InviteInfo &x)
	{
		*this = x;
	}

	InviteInfo &operator= (const InviteInfo &x)
	{
		if (this == &x) return *this;
		conf = x.conf;
		conf_type = x.conf_type;
		user = x.user;
		user_display_name = x.user_display_name;
		timeout = x.timeout;
		fwd_limit = x.fwd_limit;
		m_type = x.m_type;
		m_caps = x.m_caps;
		src_user = x.src_user;
		server = x.server;
		passwd = x.passwd;
		confRestriction = x.confRestriction;
		confKick = x.confKick;
		alias = x.alias;
		return *this;
	}
	InviteInfo& operator=(InviteInfo&& x) noexcept
	{
		if (this == &x)
			return *this;
		conf = std::move(x.conf);
		conf_type = x.conf_type;
		x.conf_type = ConfType::NOT_INIT;
		user = std::move(x.user);
		user_display_name = std::move(x.user_display_name);
		timeout = x.timeout;
		x.timeout = std::chrono::steady_clock::time_point();
		fwd_limit = x.fwd_limit;
		x.fwd_limit = 0;
		m_type = x.m_type;
		x.m_type = NO_DIRECT_CONNECT;
		m_caps = std::move(x.m_caps);
		src_user = std::move(x.src_user);
		server = std::move(x.server);
		passwd = std::move(x.passwd);
		confRestriction = std::move(x.confRestriction);
		confKick = x.confKick;
		x.confKick = nullptr;
		alias = std::move(x.alias);
		return *this;
	}

	bool operator == (const InviteInfo &i) const
	{
		return i.conf == conf && i.user == user;
	}

	enum ConfType {
		NOT_INIT = -1,
		CONF_1_TO_1,
		CONF_MULTY
	} conf_type = NOT_INIT;

	vs_conf_id conf;
	vs_user_id user;
	std::string user_display_name;

	std::chrono::steady_clock::time_point timeout;
	unsigned fwd_limit = 0;
	VS_DirectConnect_Type	m_type = NO_DIRECT_CONNECT;
	VS_BinBuff m_caps;
	vs_user_id src_user;
	VS_SimpleStr server;

	VS_SimpleStr passwd;
	boost::shared_ptr<VS_ConfRestrictInterface> confRestriction;
	 VS_ConfKick *confKick = nullptr;
	 VS_SimpleStr alias;
};


/**
*  \brief Abstract class for encapsulation of conference storage operations
*
*/

struct VS_InfoParticipant;

class VS_Storage
{
protected:
	VS_SimpleStr	m_serverID;

	std::string		m_money_warn_msg;
	int				m_money_warn_time;		//const
	int				m_money_warn_period;	//const
	int				m_money_warn_send_time; //var
	int				m_tick;					//var

	int				m_groupConfNum;

	int				m_num_guest;
	int				m_num_roamingparts;
	static const int m_MAX_CLIENT_TYPES = 10;
	int				m_num_client_types[m_MAX_CLIENT_TYPES];

	VS_UserData::UserRights	m_physRights;

	boost::shared_ptr<VS_ConfRestrictInterface> m_confResctrict;
	std::vector<InviteInfo> m_pendingInvites;

public:
	std::mutex	m_BookLock;
protected:
	vs::map<std::string, VS_UserData, vs::istr_less> m_user_data;
	vs::map<std::string, VS_ConferenceDescription, vs::istr_less> m_conferences;
	vs::map<std::string, VS_ParticipantDescription, vs::str_less> m_partIdx;

	struct ConfPartInfo {
		ConfPartInfo() : seq_id(0) {}
		vs::set<vs_user_id>		ignore_list;
		vs::set<vs_user_id>		planned_parts;
		vs::map<vs_user_id, VS_ParticipantDescription*>	confparts;  // pointer to VS_ParticipantDescription in m_partIdx
		uint32_t	seq_id;												// incremental counter
	};
	vs::map<vs_conf_id, ConfPartInfo>      m_confPartMap;

protected:
	virtual int		GetParticipantLimit(const vs_user_id& user_id,VS_ParticipantDescription::Type type, int& rights, double& limit, double& decLimit);
	virtual int		GetParticipantLimit(VS_ParticipantDescription& pd);


	virtual void	NewConfID(vs_conf_id&);

	virtual bool	FetchRights(const VS_UserData& ud, VS_UserData::UserRights& rights);
	virtual bool ReadParticipantLimit(VS_ParticipantDescription& /*pd*/)
	{return true;};

public:
	virtual int		CountParticipants();
	virtual int		CheckParticipant(const vs_user_id& user_id,VS_ParticipantDescription::Type type,const vs_user_id& owner,const vs_user_id& party);

	VS_Storage(const char* serverID);
	int Init();

	virtual ~VS_Storage();

	void GetServerTime(std::chrono::system_clock::time_point& ftime);

	//users
	virtual bool FindUser(string_view id, VS_UserData& ud);
	virtual void UpdateUser(string_view id, const VS_UserData& ud);
	virtual bool DeleteUser(string_view id);

	virtual bool GetUserProperty(const vs_user_id& /*user_id*/,const VS_SimpleStr& /*name*/,std::string& /*value*/)
	{return false;};

	//conferences
	virtual bool FindConference(string_view cid, VS_ConferenceDescription& conf);
	virtual void GetCurrentConferences(std::vector<VS_ConferenceDescription> &OUT_confs, const std::string& transceiverName = "") const;
	virtual bool FindConferenceByUser  (string_view user, VS_ConferenceDescription& conf);
	virtual int  FindConferenceByCallID(string_view call_id, VS_ConferenceDescription& conf);
	virtual int  InsertConference(VS_ConferenceDescription& conf);
	virtual bool UpdateConference(const VS_ConferenceDescription& conf);
	virtual bool DeleteConference(VS_ConferenceDescription& conf);
	virtual int  FindConferences(VS_ConferenceDescription* &conf, const VS_SimpleStr& query);
	virtual int  CountConferences();
	virtual	int  CountGroupConferences();
	virtual int  CountOnlineUsers();
	virtual int  CountOnlineGateways() { return m_num_client_types[CT_GATEWAY] + m_num_client_types[CT_TRANSCODER]  + m_num_client_types[CT_TRANSCODER_CLIENT];}
	virtual int  CountOnlineTerminalPro() { return m_num_client_types[CT_TERMINAL]; }
	virtual int  CountOnlineGuests() { return m_num_guest; }
	virtual int  CountRoamingParts() { return m_num_roamingparts; }

	virtual int   NextConferenceMinute (VS_ParticipantDescription* &pd,int dif, VS_SystemMessage* serv);
	virtual int   AddParticipant (VS_ParticipantDescription& pd);
	virtual bool  AddParticipantToIgnoreList(const vs_conf_id& conf, const vs_user_id& user);
	virtual bool  IsParticipantIgnored(const vs_conf_id& conf, const vs_user_id& user);
	virtual bool  FindParticipant(string_view user, VS_ParticipantDescription& pd);
	virtual bool  UpdateParticipant(const VS_ParticipantDescription& pd);
	virtual bool  DeleteParticipant (VS_ParticipantDescription& pd);
	virtual int   GetParticipants(const vs_conf_id &conf, std::vector<tPartServer> &part_server);
	virtual int   GetParticipants(const vs_conf_id &conf,  VS_ParticipantDescription* &pd, int rights = -1);
	virtual void  GetParticipants(const std::string &conf, std::vector<part_start_info> &OUT_call_ids, const int rights = -1) const;
	std::string	GetOneTrascoderPartId(const vs_conf_id &conf);
	virtual unsigned int GetNumOfParts(const vs_conf_id& conf);
	virtual uint32_t GetSeqId(const vs_conf_id& conf, bool inc);

	void SetPlannedParticipants(const vs_conf_id &conf, vs::set<vs_user_id> &set);
	bool GetPlannedParticipants(const vs_conf_id &conf, vs::set<vs_user_id> &set);
	bool IsPlannedParticipant(const vs_conf_id &conf, const vs_user_id &user);

	virtual vs_conf_id FindConfIDByUser(string_view user);

	virtual int FindConferenceByName(string_view user_id, VS_ConferenceDescription& conf);

	bool IsConferenceRecording(string_view conf_id) const;
	VS_Conference_Type GetConfType(string_view conf_id) const;

	void CountOfAll(int &OUsers, int &Confs, int &Parts);
	uint32_t GetUsers(vs_user_id* &users);
	uint32_t GetUsersFiltered(uint32_t n, const VS_ClientType type, std::vector<vs_user_id> &OUTusers, bool fetch_guests = false, std::vector<vs_user_id>* except_of = nullptr);

	void SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict);
	void SetUserStatus(const VS_SimpleStr& call_id,int status,const VS_ExtendedStatusStorage &extStatus);


	void StartInvitationProcess(const InviteInfo &info);
	bool FindInvitation(const vs_conf_id &conf, const vs_user_id &uid, InviteInfo &info);
	bool EndInvitationProcess(const vs_conf_id &conf, const vs_user_id &uid, InviteInfo &erased);
	bool EndInvitationProcess(const vs_conf_id &conf, const vs_user_id &uid);
	void GetTimedoutInvites( std::vector<InviteInfo> &res, InviteInfo::ConfType t);

	void AddRoamingParticipant()
	{
		std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
		m_num_roamingparts++;
	}

	void RemoveRoamingParticipant()
	{
		std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
		m_num_roamingparts--;
	}


// User Books
	struct VS_UserBook
	{
		uint32_t	hash;
		bool		IsRequestedByServer;	// client requested
		VS_Map		book;	// if common:  key=callid,data=display_name  if inverse: key=callid,data=0

		bool IsInBook(const char* callId) {
			if (!callId || !*callId)
				return false;
			return book.Find(callId) != book.End();
		}
		VS_UserBook(): hash(0), IsRequestedByServer(false) {
			book.SetPredicate(VS_Map::StrPredicate);
			book.SetKeyFactory(VS_Map::StrFactory, VS_Map::StrDestructor);
			book.SetDataFactory(VS_Map::StrFactory, VS_Map::StrDestructor);
		}
		~VS_UserBook(){
			book.Clear();
		}
	};

	struct VS_UserBooks
	{
		VS_UserBook ab_common;
		VS_UserBook ab_ban;
		VS_UserBook ab_inverse;
	};

	VS_Map		m_users;	// key=callId, data=UserBooks*
	VS_Storage::VS_UserBook* GetUserBook(const char* user, VS_AddressBook ab, bool doCreate = false);

	mutable vs::fast_recursive_mutex m_common_lock; // TODO: replace with multiple mutexes(non recursive if possible) for different purposes, not one for all
};

extern VS_Storage* g_storage;

#endif //VS_SERVER_STORAGE_H
