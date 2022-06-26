#pragma once
#include "../../ServerServices/VS_TRStorageInterface.h"

#include "std-generic/cpplib/synchronized.h"
#include "std-generic/sqlite/CppSQLite3.h"

class VS_TRStorageSqlite : public VS_TRStorageInterface {
public:
	VS_TRStorageSqlite(const std::string &db_file_name);

	bool CreateUploadRecord(const std::string &initiator, const std::string &info_hash, const std::string &magnet, uint64_t size) override;
	bool CreateFileRecord(const std::string &info_hash, const std::string &file_name, uint64_t file_size) override;
	bool UpdateUploadRecord(const std::string &info_hash, uint64_t downloaded,
						    unsigned dw_speed, unsigned up_speed, unsigned num_peers) override;
	bool UpdateFileRecord(const std::string &info_hash, const std::string &file_name,
						  uint64_t downloaded, unsigned dw_speed, unsigned up_speed, unsigned num_peers) override;

	bool MarkDeleteUpload(const std::string &info_hash) override;
	bool MarkDeleteOldUploads(unsigned lifetime_days) override;

	bool DeleteUploadRecord(const std::string &info_hash) override;

	std::vector<UploadRecord> QueryUploadsActive() override;
	std::vector<UploadRecord> QueryUploadsToDelete() override;
private:
	std::string m_db_file_name;
	vs::Synchronized<CppSQLite3DB> m_db;
};