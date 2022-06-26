#include "ChatDBInterface.h"

#if defined(HAVE_CPPDB)
#include "../../common/std/debuglog/VS_Debug.h"
#include "std-generic/clib/strcasecmp.h"
#include "std-generic/clib/vs_time.h"
#include "std-generic/cpplib/scope_exit.h"

#include <cppdb/backend.h>
#include <cppdb/frontend.h>
#include <cppdb/pool.h>

#include "std-generic/compat/memory.h"
#include <streambuf>
#include <istream>
#include <sstream>
#include <assert.h>

#define DEBUG_CURRENT_MODULE VS_DM_CHATS

// One needs it to put a blob value into database.

struct membuf : std::streambuf {
	membuf(char const* base, size_t size) {
		char* p(const_cast<char*>(base));
		this->setg(p, p, p + size);
	}
};

struct imemstream : virtual membuf, std::istream {
	imemstream(char const* base, size_t size)
		: membuf(base, size)
		, std::istream(static_cast<std::streambuf*>(this)) {
	}
};

class ChatDB : public ChatDBInterface {
public:
	typedef std::function<bool(
		const int64_t &mID,
		const VS_Container *cnt,
		const char *message,
		const std::time_t *timestamp,
		const bool *is_offline,
		const char *from_call_id,
		const char *from_display_name,
		const char *to_call_id,
		const char *conference_id)> ProcessMessageFunc;
public:
	ChatDB();
	virtual ~ChatDB();

	bool Init(const std::string& db_conn_str) final;

	bool AddNewMessage(int64_t &out_mid,
		const char *message,
		const std::time_t *timestamp = nullptr,
		const char *from_call_id = nullptr,
		const char *from_display_name = nullptr,
		const char *to_call_id = nullptr,
		const char *conference_id = nullptr) final;

	bool ChangeMessage(const int64_t mid,
		const VS_Container *cnt = nullptr,
		const char *message = nullptr,
		const std::time_t *timestamp = nullptr,
		const bool *is_offline = nullptr,
		const char *from_call_id = nullptr,
		const char *from_display_name = nullptr,
		const char *to_call_id = nullptr,
		const char *conference_id = nullptr) final;

	bool MarkRoamingMessageAsDelivered(const char *from_call_id,
		const char *from_display_name,
		const char *to_call_id,
		const char *message) final;

	bool ProcessUserOfflineMessages(const ProcessMessageFunc &func, const char *user_id, const size_t count = 80) final;

	bool ProcessRoamingOfflineMessages(const ProcessMessageFunc &func, const char *our_server_id, const size_t count = 80) final;
private:
	bool ProcessQueryResult(cppdb::result &res, const ProcessMessageFunc &func);
private:
	cppdb::pool::pointer m_pool;
	bool m_initialised;
};

ChatDB::ChatDB()
	: m_initialised(false)
{
}

ChatDB::~ChatDB()
{
}

bool ChatDB::Init(const std::string& db_conn_str)
try
{
	if (m_initialised)
	{
		return true;
	}

	m_pool = cppdb::pool::create(db_conn_str);
	cppdb::session session(m_pool->open());

	if (!session.is_open())
		return false;

	m_initialised = true;
	return true;
}
catch (std::exception &e)
{
	dstream4 << "ChatDBInterface::Init() failed: " << e.what() << '\n';
	return false;
}

template<typename T>
inline void bind_param(cppdb::statement &st, int col, T *par)
{
	if (par != nullptr)
	{
		st.bind(col, *par);
	}
}

template<>
inline void bind_param(cppdb::statement &st, int col, const char *par)
{
	if (par != nullptr)
	{
		st.bind(col, par);
	}
}

template<>
inline void bind_param(cppdb::statement &st, int col, const std::time_t *par)
{
	if (par != nullptr)
	{
		tm par_tm;
		if (gmtime_r(par, &par_tm) != nullptr) // UTC
		{
			st.bind(col, par_tm);
		}
	}
}

bool ChatDB::AddNewMessage(int64_t &out_mid,
	const char *message,
	const std::time_t *timestamp,
	const char *from_call_id,
	const char *from_display_name,
	const char *to_call_id,
	const char *conference_id)
try
{
	if (!m_initialised)
	{
		return false;
	}
	cppdb::session session(m_pool->open());
	auto statement =
		session.create_statement(
			"select stat.add_chat_message("
				"\"_message\" => ?, "
				"\"_time_stamp\" => ?, "
				"\"_from_call_id\" => ?, "
				"\"_from_display_name\" => ?, "
				"\"_to_call_id\" => ?, "
				"\"_conference_id\" => ?);"
		);
	// _message
	bind_param(statement, 1, message);
	// _time_stamp
	bind_param(statement, 2, timestamp);
	// _from_call_id
	bind_param(statement, 3, from_call_id);
	// _from_display_name
	bind_param(statement, 4, from_display_name);
	// _to_call_id
	bind_param(statement, 5, to_call_id);
	// _conference_id
	bind_param(statement, 6, conference_id);

	cppdb::result res = statement.query();
	if (!res.next())
	{
		return false;
	}

	return res.fetch(out_mid);
}
catch (std::exception &e)
{
	dstream4 << "ChatDBInterface::AddNewMessage(...) failed: " << e.what() << '\n';
	return false;
}

bool ChatDB::ChangeMessage(const int64_t mid,
	const VS_Container *cnt,
	const char *message,
	const std::time_t *timestamp,
	const bool *is_offline,
	const char *from_call_id,
	const char *from_display_name,
	const char *to_call_id,
	const char *conference_id)
{
	if (!m_initialised)
	{
		return false;
	}

	void *ptr = nullptr;
	VS_SCOPE_EXIT{ if (ptr) free(ptr); };
	size_t size = 0;
	try
	{
		int call_res = 0;
		cppdb::session session(m_pool->open());

		auto statement = session.create_statement(
			"select stat.edit_chat_message("
			"\"_message_id\" => ?, "
			"\"_container\" => ?, "
			"\"_message\" => ?, "
			"\"_time_stamp\" => ?, "
			"\"_is_offline\" => ?, "
			"\"_from_call_id\" => ?, "
			"\"_from_display_name\" => ?, "
			"\"_to_call_id\" => ?, "
			"\"_conference_id\" => ?);"
		);

		// _message_id
		statement.bind(1, mid);

		// _container
		if (cnt != nullptr)
		{
			if (cnt->SerializeAlloc(ptr, size))
			{
				imemstream ms((char *)ptr, size);
				statement.bind(2, ms);
			}
		}

		// _message
		bind_param(statement, 3, message);
		// _time_stamp
		bind_param(statement, 4, timestamp);
		// _is_offline
		bind_param(statement, 5, is_offline);
		// _from_call_id
		bind_param(statement, 6, from_call_id);
		// _from_display_name
		bind_param(statement, 7, from_display_name);
		// _to_call_id
		bind_param(statement, 8, to_call_id);
		// _conference_id
		bind_param(statement, 9, conference_id);

		// check message result
		cppdb::result res = statement.query();
		if (!res.next())
		{
			return false;
		}

		if (!res.fetch(call_res))
		{
			return false;
		}

		return call_res == 0;
	}
	catch (std::exception &e)
	{
		dstream4 << "ChatDBInterface::ChangeMessage(...) failed: " << e.what() << '\n';
		if (ptr != nullptr)
		{
			free(ptr);
		}

		return false;
	}

	if (ptr != nullptr)
	{
		free(ptr);
	}

	return true; /* no return */
}

bool ChatDB::MarkRoamingMessageAsDelivered(const char * from_call_id, const char * from_display_name, const char * to_call_id, const char * message)
try
{
	if (!m_initialised)
	{
		return false;
	}

	int call_res = 0;
	cppdb::session session(m_pool->open());

	auto statement = session.create_statement(
		"select stat.set_roaming_offline_chat_message_delivered("
			"\"_message\" => ?, "
			"\"_from_call_id\" => ?, "
			"\"_from_display_name\" => ?, "
			"\"_to_call_id\" => ?);"
	);

	// _message
	bind_param(statement, 1, message);
	// _from_call_id
	bind_param(statement, 2, from_call_id);
	// _from_display_name
	bind_param(statement, 3, from_display_name);
	// _to_call_id
	bind_param(statement, 4, to_call_id);

	// run query and get result
	auto res = statement.query();
	if (!res.next())
	{
		return false;
	}

	if (!res.fetch(call_res))
	{
		return false;
	}

	return call_res == 0;
}
catch (std::exception &e)
{
	dstream4 << "ChatDBInterface::MarkRoamingMessageAsDelivered(...) failed: " << e.what() << '\n';
	return false;
}

bool ChatDB::ProcessUserOfflineMessages(const ProcessMessageFunc & func, const char * user_id, const size_t count)
{
	if (!m_initialised || user_id == nullptr || *user_id == '\0')
	{
		return false;
	}

	try
	{
		cppdb::session session(m_pool->open());
		auto statement =
			session.create_statement(
				"select * from stat.get_offline_chat_messages(\"_to_call_id\" => ?, \"_cnt\" => ?);");

		statement.bind(1, user_id);
		statement.bind(2, count);

		cppdb::result res = statement.query();

		return ProcessQueryResult(res, func);
	}
	catch (std::exception &e)
	{
		dstream4 << "ChatDBInterface::ProcessUserOfflineMessages(...) failed: " << e.what() << '\n';
		return false;
	}
	return true; /* no return */
}

bool ChatDB::ProcessRoamingOfflineMessages(const ProcessMessageFunc &func, const char *our_server_id, const size_t count)
{
	if (!m_initialised || our_server_id == nullptr || *our_server_id == '\0')
	{
		return false;
	}

	try
	{
		cppdb::session session(m_pool->open());
		auto statement =
			session.create_statement(
				"select * from stat.get_roaming_offline_chat_messages(\"_endpoint\" => ?, \"_cnt\" => ?);");

		statement.bind(1, our_server_id);
		statement.bind(2, count);

		cppdb::result res = statement.query();

		return ProcessQueryResult(res, func);
	}
	catch (std::exception &e)
	{
		dstream4 << "ChatDBInterface::ProcessUserOfflineMessages(...) failed: " << e.what() << '\n';
		return false;
	}
	return true; /* no return */
}

bool ChatDB::ProcessQueryResult(cppdb::result & res, const ProcessMessageFunc & func)
{
	while (res.next()) // empty record set - there is no offline messages
	{
		VS_Container cnt;
		int64_t message_id = 0;

		std::time_t time_stamp = 0;
		std::tm ts;
		std::string message, from_call_id, from_display_name, to_call_id, conference_id;
		std::string offline;
		bool is_offline = true;

		VS_Container *pcnt = nullptr;
		std::time_t *ptime_stamp = nullptr;
		const char *pmessage = nullptr,
			*pfrom_call_id = nullptr,
			*pfrom_display_name = nullptr, *pto_call_id = nullptr, *pconference_id = nullptr;
		bool *pis_offline = nullptr;

		memset(&ts, 0, sizeof(ts));

		assert(res.find_column("message_id") == 0);
		if (res.is_null(0) || !res.fetch(0, message_id))
		{
			return false;
		}

		assert(res.find_column("container") == 1);
		{
			std::ostringstream out_blob(std::ios::out | std::ios::binary);
			if (!res.is_null(1) && res.fetch(1, out_blob))
			{
				auto blob_data = out_blob.str();
				if (cnt.Deserialize(&blob_data[0], blob_data.size()))
				{
					pcnt = &cnt;
				}
			}
		}

		assert(res.find_column("message") == 2);
		if (!res.is_null(2) && res.fetch(2, message))
		{
			pmessage = message.c_str();
		}

		assert(res.find_column("time_stamp") == 3);
		if (!res.is_null(3) && res.fetch(3, ts))
		{
			time_stamp = std::mktime(&ts);
			ptime_stamp = &time_stamp;
		}

		// Unfortunately, cppdb does not support fetching boolean datatype.
		// http://cppcms-users.narkive.com/HfUSxjIE/cppdb-fetching-boolean-type
		assert(res.find_column("is_offline") == 4);
		if (!res.is_null(4) && res.fetch(4, offline))
		{
			is_offline = strcasecmp(offline.c_str(), "t") == 0;
			pis_offline = &is_offline;
		}

		assert(res.find_column("from_call_id") == 5);
		if (!res.is_null(5) && res.fetch(5, from_call_id))
		{
			pfrom_call_id = from_call_id.c_str();
		}

		assert(res.find_column("from_display_name") == 6);
		if (!res.is_null(6) && res.fetch(6, from_display_name))
		{
			pfrom_display_name = from_display_name.c_str();
		}

		assert(res.find_column("to_call_id") == 7);
		if (!res.is_null(7) && res.fetch(7, to_call_id))
		{
			pto_call_id = to_call_id.c_str();
		}

		assert(res.find_column("conference_id") == 8);
		if (!res.is_null(8) && res.fetch(8, conference_id))
		{
			pconference_id = conference_id.c_str();
		}

		if (!func(
			message_id,
			pcnt,
			pmessage,
			ptime_stamp,
			pis_offline,
			pfrom_call_id,
			pfrom_display_name,
			pto_call_id,
			pconference_id))
		{
			break;
		}
	}

	return true;
}

#endif //CPPDB

std::unique_ptr<ChatDBInterface> ChatDBInterface::Create(void)
{
#if defined(HAVE_CPPDB)
	return vs::make_unique<ChatDB>();
#else
	return nullptr;
#endif //CPPDB
}

