#pragma once
#include "RegistryKeyCommand.h"

class RenameCommand : public RegistryKeyCommand
{
public:
	const char* GetNameCommnad() const override;
protected:
	void ExecuteImpl() override;
	ErrorHolder ParseCommandImpl(command_t first) override;
private:
	std::string keyName_;
	std::string newKeyName_;
};
