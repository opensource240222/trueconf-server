#pragma once
#include "RegistryKeyCommand.h"
#include <functional>
#include <memory>

class Storage;

class LoadCommand : public RegistryKeyCommand
{
public:
	explicit LoadCommand(std::unique_ptr<Storage> storage);
	~LoadCommand();
	LoadCommand(const LoadCommand& other) = delete;
	LoadCommand(LoadCommand&& other) noexcept = default;
	LoadCommand& operator=(const LoadCommand& other) = delete;
	LoadCommand& operator=(LoadCommand&& other) = default;
	const char* GetNameCommnad() const override;
	void CompletedExecute() override;
protected:
	void ExecuteImpl() override;
	ErrorHolder ParseCommandImpl(command_t first) override;
private:
	std::unique_ptr<Storage> storage_;
	std::string keyName_;
};
