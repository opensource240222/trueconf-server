#include "VS_Sphinx.h"
#include "../../common/std/VS_ProfileTools.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"

#include <mysql.h>

size_t VS_SphinxDBObjects::uses_ = 0;
const int reconnect_max = 2;

VS_SphinxDBObjects::VS_SphinxDBObjects() : inited_(false), conn_(0), maxQueryLength_(0)
{
	++uses_;
}

VS_SphinxDBObjects::~VS_SphinxDBObjects()
{
	if (conn_)
		mysql_close(static_cast<MYSQL*>(conn_));

	if (!--uses_)
		mysql_library_end();
}

bool VS_SphinxDBObjects::Init()
{
AUTO_PROF

	char buf[256] = {0};
	VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
	if (!cfg_root.IsValid())
		return false;

	std::string query, host, user, password, db;
	unsigned long port = 0;

	// set of queries to sphinx DB must be present
	if (cfg_root.GetValue(buf, sizeof(buf), VS_REG_STRING_VT, DB_SPHINX_QUERIES_TAG ) > 0)
	{
		char *p = buf;
		while (*p)
		{
			query = p;
			queries_.push_back(query);
			p += strlen(p) + 1;
		}
	}
	if (queries_.empty())
		return false;
	for (size_t i = 0; i < queries_.size(); ++i)
	{
		size_t pos = 0;
		if ((pos = queries_[i].find("%d")) == std::string::npos ||
			(pos = queries_[i].find("%s", pos)) == std::string::npos)
			return false;
		if (queries_[i].length() > maxQueryLength_)
			maxQueryLength_ = queries_[i].length();
	}

	// also host and port must be present, other params may be omitted (in that case default values are used)
	cfg_root.GetString(host, DB_SPHINX_HOST_TAG);
	if (!host.length())
		return false;

	cfg_root.GetValue(&port, sizeof(port), VS_REG_INTEGER_VT, DB_SPHINX_PORT_TAG);
	if (!port || port > USHRT_MAX)
		return false;

	cfg_root.GetString(user, DB_SPHINX_USER_TAG);
	cfg_root.GetString(password, DB_SPHINX_PASSWORD_TAG);
	cfg_root.GetString(db, DB_SPHINX_DATABASE_TAG);

	conn_ = mysql_init(NULL);
	if (!conn_)
		return false;

	my_bool reconnect = true;
	mysql_options(static_cast<MYSQL*>(conn_), MYSQL_OPT_RECONNECT, &reconnect);

	for (size_t i = 0; i < reconnect_max; ++i)
	{
		if (mysql_real_connect(static_cast<MYSQL*>(conn_), host.c_str(), user.c_str(), password.c_str(), db.c_str(), port, NULL, CLIENT_MULTI_STATEMENTS))
		{
			inited_ = true;
			break;
		}
	}

	return inited_;
}

bool VS_SphinxDBObjects::Execute(const char *login, int status)
{
	bool res = false;
	if (!mysql_ping(static_cast<MYSQL*>(conn_)))
	{
		char *query = new char[maxQueryLength_ + strlen(login) + 10]; // 10 should be more than enough for the digits of the user status

		res = true;
		for (size_t i = 0; i < queries_.size(); ++i)
		{
			sprintf(query, queries_[i].c_str(), status, login);
			if (mysql_query(static_cast<MYSQL*>(conn_), query))
			{
				res = false;
				break;
			}
		}

		delete [] query;
	}

	return res;
}
