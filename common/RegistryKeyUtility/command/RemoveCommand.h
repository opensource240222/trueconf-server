#pragma once
#include "RegistryKeyCommand.h"

class RemoveCommand : public RegistryKeyCommand
{
public:
	RemoveCommand() = default;
	const char* GetNameCommnad() const override;
protected:
	ErrorHolder ParseCommandImpl(command_t first) override;
	void ExecuteImpl() override;
private:
	std::string keyName_;
};
