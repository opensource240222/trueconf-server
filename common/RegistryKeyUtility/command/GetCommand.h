#pragma once
#include "RegistryKeyCommand.h"
#include <functional>
#include <memory>

#include "../extractor/Extractor.h"

class GetCommand : public RegistryKeyCommand
{
public:
	explicit GetCommand(std::ostream &out, std::shared_ptr<const Extractor<ValueKey, const void*, const RegistryVT,
	                                                                      name_extracrt, const std::size_t>> );
	const char* GetNameCommnad() const override;
protected:
	void ExecuteImpl() override;
	ErrorHolder ParseCommandImpl(command_t first) override;
private:
	std::shared_ptr<const Extractor<ValueKey, const void*, const RegistryVT,
		name_extracrt, const std::size_t>> extractor_;
	std::reference_wrapper<std::ostream> out_;
	ValueKey::ValueName name_;
};
