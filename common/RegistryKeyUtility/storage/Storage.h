#pragma once
#include <vector>
#include "../entity/ValueKey.h"

class Storage
{
public:
	Storage() = default;
	virtual ~Storage() {}
	Storage(const Storage& other) = delete;
	Storage(Storage&& other) noexcept = default;
	Storage& operator=(const Storage& other) = delete;
	Storage& operator=(Storage&& other) noexcept = default;

	typedef std::vector<ValueKey> storage_args;

	virtual bool OpenStorage(const char *, bool read_only = false) = 0;
	virtual bool IsValidStorage() = 0;
	virtual bool CloseStorage() = 0;
	virtual void SaveToStorage(const storage_args &) = 0;
	virtual storage_args LoadFromStorage() = 0;
};
