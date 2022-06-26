/**
 ****************************************************************************
 * (c) 2002-2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Visicron Directory Server
 *
 * \file VS_CallIDInfo.cpp
 *
 * $Revision: 9 $
 * $History: VS_CallIDInfo.h $
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 16.09.10   Time: 15:18
 * Updated in $/VSNA/Servers/ServerServices/Types
 * Arch 3.1 Conf Loggin duplicates remove
 * - RS store BS server of user
 * - ConfLog: uses BS got from RS to send logs
 * - ConfLog: dprint loggin added
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 30.10.08   Time: 20:33
 * Updated in $/VSNA/Servers/ServerServices/types
 *  - Alias status sending removed (AS ->RS )
 * - Logging of subType added
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 22.05.08   Time: 19:09
 * Updated in $/VSNA/Servers/ServerServices/types
 * -   
 * - login time added to statuses
 * - commets added
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 18.02.08   Time: 21:43
 * Updated in $/VSNA/Servers/ServerServices/types
 * added alias support (first ver)
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 15.02.08   Time: 13:04
 * Updated in $/VSNA/Servers/ServerServices/types
 * unified statues to container
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 30.11.07   Time: 20:36
 * Updated in $/VSNA/servers/serverservices/types
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 20.11.07   Time: 16:14
 * Updated in $/VSNA/Servers/ServerServices/types
 * sub hub refactoring
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
 * *****************  Version 8  *****************
 * User: Stass        Date: 26.05.05   Time: 19:37
 * Updated in $/VS/servers/serverservices/types
 * implemeted new replicate logic - updates only from home server
 * rewritten push logic
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 19.05.05   Time: 14:04
 * Updated in $/VS/Servers/ServerServices/Types
 * protection from other-broker-logoff
 * no -push on wrong events
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 4.05.05    Time: 15:10
 * Updated in $/VS/Servers/ServerServices/Types
 * SubscribtionHub basic class
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 13.10.03   Time: 13:23
 * Updated in $/VS/Servers/ServerServices/Types
 * added type param/member support
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 16.09.03   Time: 22:06
 * Updated in $/VS/Servers/ServerServices/Types
 * broker support
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 8.09.03    Time: 13:28
 * Updated in $/VS/Servers/ServerServices/Types
 * added missing constructors
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 5.09.03    Time: 20:04
 * Created in $/VS/Servers/ServerServices/Types
 * Added CallIDInfo struct
 *
 ****************************************************************************/

#pragma once

#include "status_types.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/VS_Protocol.h"

#include <chrono>
#include <list>

class VS_CallIDInfo
{
public:
	enum Source { UNDEF=0,LOCAL=1,UPLINK };

	static const char EMPTY_EXT[];

	// Fields
	std::string				m_serverID;	///< server

	VS_UserPresence_Status	m_status;		///< current status
	///VS_SimpleStr			m_extStatus;	///< extended status

	VS_ExtendedStatusStorage m_extStatusStorage;


	std::chrono::system_clock::time_point				m_logginTime;	///< time of client loggin

	int						m_type;			///< billing type (user type+)
	//Source					m_source;		///< source of CallID Info

	std::string				m_realID;		///< real ID
	std::string				m_homeServer;   ///< BS server of user
	std::string				m_displayName;	///< users Display Name (set at login)
	VS_MultiLoginCapability m_ml_cap = VS_MultiLoginCapability::UNKNOWN;


	// Constructor/Destructor
	VS_CallIDInfo(VS_UserPresence_Status status=USER_LOGOFF, const VS_ExtendedStatusStorage& extendedStatus= VS_ExtendedStatusStorage(),  ///const char* extStatus=0,
		std::string serverID = {}, int type=-1, std::string realID = {}, std::string homeServer = {}, std::string displayName = {})
		: m_serverID(std::move(serverID))
		, m_status(status)
		, m_extStatusStorage(extendedStatus)
		/*, m_extStatus(extStatus)*/
		, m_type(type)
		, m_realID(std::move(realID))
		, m_homeServer(std::move(homeServer))
		, m_displayName(std::move(displayName))
	{
	}

	VS_CallIDInfo(const VS_CallIDInfo& ci)
		: m_serverID(ci.m_serverID)
		, m_status(ci.m_status)
		, m_extStatusStorage(ci.m_extStatusStorage)
		/*,m_extStatus(ci.m_extStatus)*/
		, m_type(ci.m_type),m_realID(ci.m_realID)
		, m_homeServer(ci.m_homeServer)
		, m_displayName(ci.m_displayName)
		, m_ml_cap(ci.m_ml_cap)
	{}

	VS_CallIDInfo(VS_CallIDInfo&&src) noexcept
	{
		*this = std::move(src);
	}

	VS_CallIDInfo& operator=(const VS_CallIDInfo&src) = default;

	VS_CallIDInfo& operator=(VS_CallIDInfo&&src) noexcept
	{
		m_serverID = std::move(src.m_serverID);
		m_status = src.m_status;
		src.m_status = USER_LOGOFF;
		m_extStatusStorage = std::move(src.m_extStatusStorage);
		m_logginTime = src.m_logginTime;
		m_type = src.m_type;
		src.m_type = -1;
		m_realID = std::move(src.m_realID);
		m_homeServer = std::move(src.m_homeServer);
		m_displayName = std::move(src.m_displayName);
		m_ml_cap = src.m_ml_cap;
		src.m_ml_cap = VS_MultiLoginCapability::UNKNOWN;
		return *this;
	}
	bool CanBeChangedBy(bool set_server, const VS_CallIDInfo&src) const
	{
		assert(m_ml_cap == VS_MultiLoginCapability::UNKNOWN || m_ml_cap == src.m_ml_cap);
		return
			(set_server ? m_serverID != src.m_serverID : false) ||
			(src.m_type != -1 ? m_type != src.m_type : false) ||
			(src.m_status > USER_STATUS_UNDEF ? m_status != src.m_status : false) ||
			(!src.m_realID.empty() ? m_realID != src.m_realID : false) ||
			(src.m_logginTime!=std::chrono::system_clock::time_point() ? m_logginTime != src.m_logginTime : false) ||
			(!src.m_homeServer.empty()	? m_homeServer != src.m_homeServer : false) ||
			(!src.m_displayName.empty() ? m_displayName != src.m_displayName : false) ||
			(!(m_extStatusStorage == src.m_extStatusStorage));
	}
	virtual ~VS_CallIDInfo(void){}

	// Methods
	void Empty()
	{
		m_status=USER_LOGOFF;
		m_serverID.clear();
		m_type=-1;
		m_realID.clear();
		m_extStatusStorage.Clear();
	}

	void ToContainer(VS_Container& cnt, bool set_server, bool insert_all_ext_statuses) const
	{
		if(m_status>USER_STATUS_UNDEF)
			cnt.AddValueI32(USERPRESSTATUS_PARAM, m_status);
		if(m_type!=-1)
			cnt.AddValueI32(TYPE_PARAM, m_type);
		if(set_server)
			cnt.AddValue(SERVER_PARAM, m_serverID);
		if(!m_realID.empty())
			cnt.AddValue(REALID_PARAM, m_realID);
		if(m_logginTime != std::chrono::system_clock::time_point())
			cnt.AddValue(TIME_PARAM,m_logginTime);
		if(!m_homeServer.empty())
			cnt.AddValue(LOCATORBS_PARAM, m_homeServer);
		if(!m_displayName.empty())
			cnt.AddValue(DISPLAYNAME_PARAM, m_displayName);
		if(!!m_extStatusStorage && m_status!=USER_INVALID)
			m_extStatusStorage.ToContainer(cnt, insert_all_ext_statuses);
		if (m_ml_cap != VS_MultiLoginCapability::UNKNOWN)
			cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, m_ml_cap);
	}
};
struct StatusFromContainer
{
	std::string callID;
	VS_CallIDInfo info;
	bool set_server = false;
	std::set<std::string> aliases;
};
inline std::list<StatusFromContainer> GetStatusesFromContainer(const VS_Container &cnt)
{
	std::list<StatusFromContainer> res;
	cnt.Reset();
	StatusFromContainer current_item;
	while (cnt.Next())
	{
		string_view name = cnt.GetName();
		if (name == CALLID_PARAM || name == USERNAME_PARAM)
		{
			if (!current_item.callID.empty())
				res.push_back(std::move(current_item));
			auto tmp = cnt.GetStrValueRef();
			current_item.callID = !!tmp ? tmp : std::string{};
			tmp = cnt.GetStrValueRef();
			current_item.info.m_realID = !!tmp ? tmp : std::string{};
		}
		else if (name == USERPRESSTATUS_PARAM)
		{
			int32_t val(0);
			if (cnt.GetValue(val))
				current_item.info.m_status = static_cast<VS_UserPresence_Status>(val);
		}
		else if (name == EXTSTATUS_PARAM)
		{
			VS_Container st_cnt;
			if (cnt.GetValue(st_cnt))
				current_item.info.m_extStatusStorage.UpdateStatus(st_cnt);
		}
		else if (name == TYPE_PARAM)
		{
			int32_t val(0);
			if (cnt.GetValue(val))
				current_item.info.m_type = val;
		}
		else if (name == REALID_PARAM)
		{
			const auto tmp = cnt.GetStrValueRef();
			current_item.info.m_realID = !!tmp ? tmp : std::string{};
		}
		else if (name == TIME_PARAM)
		{
			cnt.GetValue(current_item.info.m_logginTime);
		}
		else if (name == LOCATORBS_PARAM)
		{
			const auto tmp = cnt.GetStrValueRef();
			current_item.info.m_homeServer = !!tmp ? tmp : std::string{};
		}
		else if (name == SERVER_PARAM)
		{
			const auto tmp = cnt.GetStrValueRef();
			current_item.info.m_serverID = !!tmp ? tmp : std::string{};
			current_item.set_server = true;
		}
		else if (name == ALIAS_PARAM)
			current_item.aliases.insert(cnt.GetStrValueRef());
		else if (name == DISPLAYNAME_PARAM)
		{
			const auto tmp = cnt.GetStrValueRef();
			current_item.info.m_displayName = !!tmp ? tmp : std::string{};
		}
		else if (name == MULTI_LOGIN_CAPABILITY_PARAM)
		{
			cnt.GetValueI32(current_item.info.m_ml_cap);
		}
	}
	if (!current_item.callID.empty())
		res.push_back(std::move(current_item));
	return res;
}
inline void UpdateCallIDInfo(const VS_CallIDInfo &src, VS_CallIDInfo &dst,
		bool set_server, bool only_ext_status)
{
	if (!only_ext_status)
	{
		if (set_server)
			dst.m_serverID = src.m_serverID;
		if (src.m_type != -1)
			dst.m_type = src.m_type;
		if (src.m_status > USER_STATUS_UNDEF)
			dst.m_status = src.m_status;
		if (!src.m_realID.empty())
			dst.m_realID = src.m_realID;
		if (src.m_logginTime!=std::chrono::system_clock::time_point())
			dst.m_logginTime = src.m_logginTime;
		if (!src.m_homeServer.empty())
			dst.m_homeServer = src.m_homeServer;
		if (!src.m_displayName.empty())
			dst.m_displayName = src.m_displayName;
	}
	dst.m_extStatusStorage += src.m_extStatusStorage;
	dst.m_ml_cap = src.m_ml_cap;
}