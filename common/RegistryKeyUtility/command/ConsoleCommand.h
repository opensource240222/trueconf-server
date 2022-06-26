#pragma once
#include "Items.h"
#include "Command.h"
#include "CommandItem.h"

#include <functional>

#include "../entity/CommandParams.h"
#include "../extractor/Extractor.h"
#include "../validator/Validator.h"


class ConsoleCommand : protected Items, public Command
{
public:
	explicit ConsoleCommand(std::shared_ptr<const Validator<CommandParams>> validator, std::shared_ptr<const Extractor<CommandParams, const int, const char* const[]>> extractor,
		const int argc, const char * const argv[], std::function<CommandItem*(CommandItem::NameCommand)> commandInvoker);
protected:
	void ExecuteImpl() override;
private:
	std::function<CommandItem*(CommandItem::NameCommand)> commandInvoker_;
	std::shared_ptr<const Validator<CommandParams>> validator_;
	std::shared_ptr<const Extractor<CommandParams, const int, const char *const[]>> extractor_;
	int argc_;
	const char * const* argv_;
};