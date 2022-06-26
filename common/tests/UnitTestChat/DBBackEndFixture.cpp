#include "DBBackEndFixture.h"

namespace chat_test
{
std::string DBBackEndFixture::GetConnString() const
{
	if (GetParam() == DBBackEnd::postgresql)
		return GetPostgreSQLConfiguration();

	auto sqlite_db = CreateSharedDBInMemory(g_db_backend_name);
	return sqlite_db.dbConnParam.connectionString;
}
void DBBackEndFixture::CleanChat(const chat::ChatID& chat_id)
{
	if (GetParam() == DBBackEnd::postgresql)
		PostgreSQLCleanChat(chat_id);
}
}