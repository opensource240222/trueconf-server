#pragma once
#include "../../../ServerServices/VS_TRStorageInterface.h"

#include "../../../common/std/cpplib/VS_Pool.h"
#include <memory>

class VS_DBObjects;

class VS_TRStorageAdo : public VS_TRStorageInterface {
public:
	VS_TRStorageAdo(const std::shared_ptr<VS_Pool> &dbo_pool);

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
	std::shared_ptr<VS_Pool> m_dbo_pool;
	VS_DBObjects *GetDBO(const VS_Pool::Item* &item);
};
