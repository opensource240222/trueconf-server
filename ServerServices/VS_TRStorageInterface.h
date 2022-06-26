#pragma once

#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

class VS_TRStorageInterface {
public:
	virtual ~VS_TRStorageInterface() {}

	virtual bool CreateUploadRecord(const std::string &initiator, const std::string &info_hash, const std::string &magnet, uint64_t size) = 0;
	virtual bool CreateFileRecord(const std::string &info_hash, const std::string &file_name, uint64_t file_size) = 0;
	virtual bool UpdateUploadRecord(const std::string &info_hash, uint64_t downloaded,
								    unsigned dw_speed, unsigned up_speed, unsigned num_peers) = 0;
	virtual bool UpdateFileRecord(const std::string &info_hash, const std::string &file_name,
								  uint64_t downloded, unsigned dw_speed, unsigned up_speed, unsigned num_peers) = 0;

	virtual bool MarkDeleteUpload(const std::string &info_hash) = 0;
	virtual bool MarkDeleteOldUploads(unsigned lifetime_days) = 0;

	virtual bool DeleteUploadRecord(const std::string &info_hash) = 0;

	struct UploadRecord {
		std::string info_hash,
					owner,
					magnet_link;
		uint64_t	size;
		time_t		completion_time;

		UploadRecord(const char *_info_hash, const char *_owner, const char *_magnet,
			uint64_t _size, time_t _completion_time) : info_hash(_info_hash), owner(_owner), magnet_link(_magnet),
			size(_size), completion_time(_completion_time) { }
	};
	virtual std::vector<UploadRecord> QueryUploadsActive() = 0;
	virtual std::vector<UploadRecord> QueryUploadsToDelete() = 0;
};