#pragma once

#include "VS_ExtendedStatus.h"

#include <functional>
#include <string>
#include <vector>

/// User Status
enum VS_UserPresence_Status : int
{
	USER_STATUS_UNDEF=-127,
	USER_INVALID=-1,
	USER_LOGOFF=0,
	USER_AVAIL=1,
	USER_BUSY=2,
	USER_PUBLIC=3,
	USER_VIPPUBLIC=4,
	USER_MULTIHOST=5
};

struct UserStatusInfo final
{
	std::string real_id;
	std::string confStreamID;

	struct User final
	{
		User(std::string server, std::string homeServer, VS_ExtendedStatusStorage extandedStatus,
			VS_UserPresence_Status status)
			: server(std::move(server)),
			home_server(std::move(homeServer)),
			extanded_status(std::move(extandedStatus)),
			status(status)
		{
		}

		User() : status(VS_UserPresence_Status::USER_INVALID) {}

		std::string server;
		std::string home_server;
		VS_ExtendedStatusStorage extanded_status;
		VS_UserPresence_Status status;
	};

	struct Conf final
	{
		bool conf_exist;
	};

	boost::variant<User, Conf> info;
};
using GetUserStatusFunctionT = std::function<UserStatusInfo(const char* /**call_id*/, bool/**use_cache*/, bool/**do_ext_resolve*/)>;

typedef std::function<UserStatusInfo(string_view /**call_id*/, bool/**use_cache*/, bool/**do_ext_resolve*/)> UserStatusFunction;

// for HttpHandler
struct UsersStatusesInterface {
	typedef std::vector<std::pair<std::string, VS_UserPresence_Status>> UsersList;

	virtual ~UsersStatusesInterface() {}
	virtual bool UsersStatuses(UsersList &users) = 0;				// http request /s2/ for many users at once
	virtual void ListOfOnlineUsers(UsersList &users) = 0;			// http request /s4/
};