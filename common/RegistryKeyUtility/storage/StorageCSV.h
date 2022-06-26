#pragma once

#include "Storage.h"
#include <fstream>

class StorageCSV : public Storage
{
public:
	explicit StorageCSV(const bool rewriteFile = false);
	~StorageCSV() = default;
	StorageCSV(const StorageCSV& other) = delete;
	StorageCSV(StorageCSV&& other) = delete;
	StorageCSV& operator=(const StorageCSV& other) = delete;
	StorageCSV& operator=(StorageCSV&& other) = delete;

	bool OpenStorage(const char* filePath, bool read_only = false) override;
	bool IsValidStorage() override;
	bool CloseStorage() override;
	void SaveToStorage(const storage_args&) override;
	storage_args LoadFromStorage() override;
private:
	bool rewriteFile_;
	std::fstream stream_;
};
