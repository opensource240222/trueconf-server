#pragma once

#include "RegistryKeyCommand.h"

class SetCommand : public RegistryKeyCommand
{
public:
	const char* GetNameCommnad() const override;
protected:
	ErrorHolder ParseCommandImpl(command_t first) override;
	void ExecuteImpl() override;
private:
	ValueKey valueKey_;
};
