#include "make_chat_storage.h"
#include "chatlib/storage/ChatStorage.h"
#include "chatlib/storage/DBProcWrap.h"
#include "std-generic/compat/memory.h"

#include <std-generic/cpplib/VS_CppDBIncludes.h>

namespace chat
{
std::shared_ptr<ChatStorage> make_chat_storage(string_view config)
{
	cppdb::ref_ptr<cppdb::backend::connection> conn;
	if (0 == config.find("sqlite3:"))
		conn = db_proc::InitSQLiteDB(config);
	return std::make_shared<ChatStorage>(config, conn);
}
}