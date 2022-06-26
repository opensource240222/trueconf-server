#include "VS_TRStorageSqlite.h"

#include <ctime>

#include "../../common/std/debuglog/VS_Debug.h"

namespace VS_TRStorageSqliteConstants {
	const char UPLOADS_TABLE[]		= "download_info";
	const char FILES_TABLE[]		= "files_download_info";

	const char DB_OWNER_PARAM[]		= "owner";
	const char DB_INFO_HASH_PARAM[] = "info_hash";
	const char DB_MAGNET_PARAM[]	= "magnet_link";
	const char DB_SIZE_PARAM[]		= "size";
	const char DB_DONE_PARAM[]		= "done";
	const char DB_D_SPEED_PARAM[]	= "d_speed";
	const char DB_U_SPEED_PARAM[]	= "u_speed";
	const char DB_PEERS_PARAM[]		= "peers_count";
	const char DB_CREATE_TIME[]		= "create_time";
	const char DB_TIME_PARAM[]		= "completion_time";
	const char DB_DEL_PARAM[]		= "deleteFiles";
	const char DB_NAME_PARAM[]		= "name";
}

#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

VS_TRStorageSqlite::VS_TRStorageSqlite(const std::string &db_file_name)
	: m_db_file_name(db_file_name) {
	using namespace VS_TRStorageSqliteConstants;

	try {
		m_db->open(db_file_name.c_str());

		CppSQLite3Buffer buf_sql;

		bool should_upgrade_table = false;
		if (m_db->tableExists(UPLOADS_TABLE)) {
			buf_sql.format("SELECT * FROM %s LIMIT 1", UPLOADS_TABLE);
			CppSQLite3Query q = m_db->execQuery(buf_sql);
			if (q.numFields() == 9 ||
				q.numFields() == 10) {
				should_upgrade_table = true;
			}
		}

		if (should_upgrade_table) {
			buf_sql.format("ALTER TABLE %s RENAME TO temp_%s;", UPLOADS_TABLE, UPLOADS_TABLE);
			m_db->execQuery(buf_sql);
		}

		// download_info
		// +-------+-----------+-------------+------+------+---------+---------+-------------+-------------+-----------------+-------------+
		// | owner | info_hash | magnet_link | size | done | d_speed | u_speed | peers_count | create_time | completion_time | deleteFiles |
		// +-------+-----------+-------------+------+------+---------+---------+-------------+-------------+-----------------+-------------+
		buf_sql.format("CREATE TABLE IF NOT EXISTS %s("  \
			"%s		TEXT		DEFAULT('unknown'),"
			"%s		TEXT PRIMARY KEY    UNIQUE NOT NULL," \
			"%s		TEXT		NOT NULL," \
			"%s		INT			NOT NULL," \
			"%s		INT			NOT NULL," \

			"%s		INT			NOT NULL," \
			"%s		INT			NOT NULL," \

			"%s		INT			NOT NULL," \

			"%s		INT			NOT NULL DEFAULT (strftime('%%s','now'))," \
			"%s		INT			NOT NULL," \
			"%s		INT			NOT NULL);", UPLOADS_TABLE, DB_OWNER_PARAM, DB_INFO_HASH_PARAM, DB_MAGNET_PARAM, DB_SIZE_PARAM,
			DB_DONE_PARAM, DB_D_SPEED_PARAM, DB_U_SPEED_PARAM, DB_PEERS_PARAM, DB_CREATE_TIME, DB_TIME_PARAM, DB_DEL_PARAM);
		m_db->execQuery(buf_sql);

		// download_files_info
		// +-----------+------+------+------+---------+---------+-------------+-----------------+
		// | info_hash | name | size | done | d_speed | u_speed | peers_count | completion_time |
		// +-----------+------+------+------+---------+---------+-------------+-----------------+
		buf_sql.format("CREATE TABLE IF NOT EXISTS %s("  \
			"%s		TEXT		NOT NULL," \
			"%s		TEXT		NOT NULL," \
			"%s		INT			NOT NULL," \
			"%s		INT			NOT NULL," \

			"%s		INT			NOT NULL," \
			"%s		INT			NOT NULL," \

			"%s		INT			NOT NULL," \

			"%s		INT         DEFAULT 0,"\
			"PRIMARY KEY(%s, %s));", FILES_TABLE, DB_INFO_HASH_PARAM, DB_NAME_PARAM, DB_SIZE_PARAM,
			DB_DONE_PARAM, DB_D_SPEED_PARAM, DB_U_SPEED_PARAM, DB_PEERS_PARAM, DB_TIME_PARAM, DB_INFO_HASH_PARAM, DB_NAME_PARAM);
		m_db->execQuery(buf_sql);

		if (should_upgrade_table) {
			buf_sql.format("INSERT INTO %s (%s, %s, %s, %s, %s, %s, %s, %s, %s) "
				"SELECT %s, %s, %s, %s, %s, %s, %s, %s, %s FROM temp_%s;", UPLOADS_TABLE,
				DB_INFO_HASH_PARAM, DB_MAGNET_PARAM, DB_SIZE_PARAM, DB_DONE_PARAM,
				DB_D_SPEED_PARAM, DB_U_SPEED_PARAM, DB_PEERS_PARAM, DB_TIME_PARAM, DB_DEL_PARAM,
				DB_INFO_HASH_PARAM, DB_MAGNET_PARAM, DB_SIZE_PARAM, DB_DONE_PARAM,
				DB_D_SPEED_PARAM, DB_U_SPEED_PARAM, DB_PEERS_PARAM, DB_TIME_PARAM, DB_DEL_PARAM,
				UPLOADS_TABLE);
			m_db->execQuery(buf_sql);
			buf_sql.format("DROP TABLE temp_%s", UPLOADS_TABLE);
			m_db->execQuery(buf_sql);
		}
	} catch (CppSQLite3Exception &e) {
		throw std::runtime_error(std::string("SQLITE Error: ") + std::to_string(e.errorCode()) + ":" + e.errorMessage());
	}
}

bool VS_TRStorageSqlite::CreateUploadRecord(const std::string &initiator, const std::string &info_hash, const std::string &magnet, uint64_t size) {
	using namespace VS_TRStorageSqliteConstants;

	try {
		CppSQLite3Buffer buf_sql1, buf_sql2;
		buf_sql1.format("INSERT OR IGNORE INTO %s VALUES " \
			"(\"%s\", \"%s\", \"%s\", \"%llu\", \"%i\", \"%i\", \"%i\", \"%i\", strftime('%%s', 'now'), \"%i\", \"%i\")",
			UPLOADS_TABLE, initiator.c_str(), info_hash.c_str(), magnet.c_str(), size, 0, 0, 0, 0, 0, 0);

		buf_sql2.format("UPDATE OR REPLACE %s " \
			"SET %s = \"%s\" " \
			"WHERE %s = \"%s\"", UPLOADS_TABLE,
			DB_OWNER_PARAM, initiator.c_str(),
			DB_INFO_HASH_PARAM, info_hash.c_str());

		m_db.withLock([&](CppSQLite3DB &db) {
			db.execQuery(buf_sql1);
			db.execQuery(buf_sql2);
		});

		return true;
	} catch (CppSQLite3Exception &e) {
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
}

bool VS_TRStorageSqlite::CreateFileRecord(const std::string &info_hash, const std::string &file_name, uint64_t file_size) {
	using namespace VS_TRStorageSqliteConstants;

	try {
		CppSQLite3Buffer buf_sql;
		buf_sql.format("INSERT OR IGNORE INTO %s VALUES " \
			"(\"%s\", \"%s\", \"%llu\", \"%i\", \"%i\", \"%i\", \"%i\", \"%i\")",
			FILES_TABLE, info_hash.c_str(), file_name.c_str(), file_size, 0, 0, 0, 0, 0);
		m_db->execQuery(buf_sql);

		return true;
	} catch (CppSQLite3Exception &e) {
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
}

bool VS_TRStorageSqlite::UpdateUploadRecord(const std::string &info_hash, uint64_t downloaded,
											unsigned dw_speed, unsigned up_speed, unsigned num_peers) {
	using namespace VS_TRStorageSqliteConstants;

	try {
		CppSQLite3Buffer buf_sql1, buf_sql2;
		buf_sql1.format("UPDATE OR REPLACE %s " \
					   "SET %s = \"%llu\", " \
						   "%s = \"%u\", " \
						   "%s = \"%u\", " \
					       "%s = \"%u\" " \
					   "WHERE %s = \"%s\";", UPLOADS_TABLE,
			DB_DONE_PARAM, downloaded,
			DB_D_SPEED_PARAM, dw_speed,
			DB_U_SPEED_PARAM, up_speed,
			DB_PEERS_PARAM, num_peers,
			DB_INFO_HASH_PARAM, info_hash.c_str());

		buf_sql2.format("UPDATE OR REPLACE %s " \
			"SET %s = \"%lld\" " \
			"WHERE %s = \"%s\" AND %s = %s AND %s = 0", UPLOADS_TABLE,
			DB_TIME_PARAM, (long long)time(NULL),
			DB_INFO_HASH_PARAM, info_hash.c_str(),
			DB_SIZE_PARAM, DB_DONE_PARAM,
			DB_TIME_PARAM);

		m_db.withLock([&](CppSQLite3DB &db) {
			db.execQuery(buf_sql1);
			db.execQuery(buf_sql2);
		});

		return true;
	} catch (CppSQLite3Exception &e) {
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
}

bool VS_TRStorageSqlite::UpdateFileRecord(const std::string &info_hash, const std::string &file_name,
										  uint64_t downloaded, unsigned dw_speed, unsigned up_speed, unsigned num_peers) {
	using namespace VS_TRStorageSqliteConstants;

	try {
		CppSQLite3Buffer buf_sql1, buf_sql2;
		buf_sql1.format("UPDATE OR REPLACE %s " \
			"SET %s = \"%llu\", " \
				"%s = \"%i\", " \
				"%s = \"%i\", " \
				"%s = \"%i\" " \
			"WHERE %s = \"%s\" AND %s = \"%s\"", FILES_TABLE,
			DB_DONE_PARAM, downloaded,
			DB_D_SPEED_PARAM, dw_speed,
			DB_U_SPEED_PARAM, up_speed,
			DB_PEERS_PARAM, num_peers,
			DB_INFO_HASH_PARAM, info_hash.c_str(),
			DB_NAME_PARAM, file_name.c_str());

		buf_sql2.format("UPDATE %s SET %s = \"%lld\" " \
			"WHERE %s = \"%s\" " \
			"AND %s = \"%s\" AND %s = %s AND %s = 0",
			FILES_TABLE, DB_TIME_PARAM, (long long)time(NULL), DB_INFO_HASH_PARAM,
			info_hash.c_str(), DB_NAME_PARAM, file_name.c_str(),
			DB_SIZE_PARAM, DB_DONE_PARAM,
			DB_TIME_PARAM);

		m_db.withLock([&](CppSQLite3DB &db) {
			db.execQuery(buf_sql1);
			db.execQuery(buf_sql2);
		});

		return true;
	} catch (CppSQLite3Exception &e) {
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
}

bool VS_TRStorageSqlite::MarkDeleteUpload(const std::string &info_hash) {
	using namespace VS_TRStorageSqliteConstants;

	try {
		CppSQLite3Buffer buf_sql;
		buf_sql.format("UPDATE %s SET %s = 1 WHERE %s == \"%s\"",
			UPLOADS_TABLE, DB_DEL_PARAM, DB_INFO_HASH_PARAM, info_hash.c_str());
		m_db->execQuery(buf_sql);

		return true;
	} catch (CppSQLite3Exception &e) {
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
}

bool VS_TRStorageSqlite::MarkDeleteOldUploads(unsigned lifetime_days) {
	using namespace VS_TRStorageSqliteConstants;

	time_t files_lifetime_utc = lifetime_days * 24 * 60 * 60;
	time_t time_limit = time(NULL) - files_lifetime_utc;

	try {
		CppSQLite3Buffer buf_sql;
		buf_sql.format("UPDATE %s SET %s = 1 WHERE %s < %lld AND %s != 0",
			UPLOADS_TABLE, DB_DEL_PARAM, DB_CREATE_TIME, (long long)time_limit, DB_CREATE_TIME);
		m_db->execQuery(buf_sql);

		return true;
	} catch (CppSQLite3Exception &e) {
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
}

bool VS_TRStorageSqlite::DeleteUploadRecord(const std::string &info_hash) {
	using namespace VS_TRStorageSqliteConstants;

	try {
		CppSQLite3Buffer buf_sql1, buf_sql2;
		buf_sql1.format("DELETE FROM %s WHERE %s = \"%s\"", UPLOADS_TABLE, DB_INFO_HASH_PARAM, info_hash.c_str());
		buf_sql2.format("DELETE FROM %s WHERE %s = \"%s\"", FILES_TABLE, DB_INFO_HASH_PARAM, info_hash.c_str());

		m_db.withLock([&](CppSQLite3DB &db) {
			db.execQuery(buf_sql1);
			db.execQuery(buf_sql2);
		});

		return true;
	} catch (CppSQLite3Exception &e) {
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
}

std::vector<VS_TRStorageInterface::UploadRecord> VS_TRStorageSqlite::QueryUploadsActive() {
	using namespace VS_TRStorageSqliteConstants;

	try {
		CppSQLite3Buffer buf_sql;
		buf_sql.format("SELECT %s, %s, %s, %s, %s FROM %s WHERE %s != 1",
			DB_INFO_HASH_PARAM,
			DB_OWNER_PARAM,
			DB_MAGNET_PARAM,
			DB_SIZE_PARAM,
			DB_TIME_PARAM,
			UPLOADS_TABLE, DB_DEL_PARAM);
		CppSQLite3Query q = m_db->execQuery(buf_sql);

		std::vector<UploadRecord> ret;
		for (; !q.eof(); q.nextRow()) {
			ret.emplace_back(q.fieldValue(0), q.fieldValue(1), q.fieldValue(2),
							 (uint64_t)atoll(q.fieldValue(3)), atoll(q.fieldValue(4)));
		}
		return ret;
	} catch (CppSQLite3Exception &e) {
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return {};
	}
}

std::vector<VS_TRStorageInterface::UploadRecord> VS_TRStorageSqlite::QueryUploadsToDelete() {
	using namespace VS_TRStorageSqliteConstants;

	try {
		CppSQLite3Buffer buf_sql;
		buf_sql.format("SELECT %s, %s, %s, %s, %s FROM %s WHERE %s = 1",
			DB_INFO_HASH_PARAM,
			DB_OWNER_PARAM,
			DB_MAGNET_PARAM,
			DB_SIZE_PARAM,
			DB_TIME_PARAM,
			UPLOADS_TABLE, DB_DEL_PARAM);
		CppSQLite3Query q = m_db->execQuery(buf_sql);

		std::vector<UploadRecord> ret;
		for (; !q.eof(); q.nextRow()) {
			ret.emplace_back(q.fieldValue(0), q.fieldValue(1), q.fieldValue(2),
							 (uint64_t)atoll(q.fieldValue(3)), atoll(q.fieldValue(4)));
		}
		return ret;
	} catch (CppSQLite3Exception &e) {
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return {};
	}
}

#undef DEBUG_CURRENT_MODULE