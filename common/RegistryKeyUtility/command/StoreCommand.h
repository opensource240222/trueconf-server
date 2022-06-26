#pragma once

#include "RegistryKeyCommand.h"
#include "std-generic/cpplib/string_view.h"
#include "../entity/CommandParams.h"
#include "../extractor/Extractor.h"
#include "../storage/Storage.h"
#include <memory>

class Storage;

class StoreCommand : public RegistryKeyCommand
{
public:
	explicit StoreCommand(std::unique_ptr<Storage> storage, std::shared_ptr<const Extractor<ValueKey, const void*, const RegistryVT,
		name_extracrt, const std::size_t>>);
	~StoreCommand();
	StoreCommand(const StoreCommand& other) = delete;
	StoreCommand(StoreCommand&& other) noexcept = default;
	StoreCommand& operator=(const StoreCommand& other) = delete;
	StoreCommand& operator=(StoreCommand&& other) = default;
	void CompletedExecute() override;
	const char* GetNameCommnad() const override;
protected:
	void ExecuteImpl() override;
	ErrorHolder ParseCommandImpl(command_t first) override;
private:
	Storage::storage_args GetData(VS_RegistryKey& key, string_view name = string_view{}) const;
private:
	std::unique_ptr<Storage> storage_;
	std::shared_ptr<const Extractor<ValueKey, const void*, const RegistryVT,
		name_extracrt, const std::size_t>> extractor_;
	std::string keyName_;
};
