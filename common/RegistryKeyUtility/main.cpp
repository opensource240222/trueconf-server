#include "extractor/ExtractorParams.h"
#include "command/ConsoleCommand.h"
#include "validator/ValidatorImpl.h"

#include "command/RenameCommand.h"
#include "command/SetCommand.h"
#include "command/RemoveCommand.h"
#include "command/StoreCommand.h"
#include "command/LoadCommand.h"
#include "command/GetCommand.h"

#include "storage/StorageCSV.h"
#include "extractor/ExtractorValueKey.h"

#include "std-generic/compat/memory.h"
#include <cassert>
#include <iostream>

//////////////////////////////////////////////////////////////////////////
////////////////////////INJECT DATA///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static const std::shared_ptr<const ExtractorValueKey> EXTRACTOR_VALUE_KEY = std::make_shared<const ExtractorValueKey>();


CommandItem *command_invoker(const CommandItem::NameCommand nameCommand)
{
	switch (nameCommand)
	{
	case CommandItem::NameCommand::SET_COMMAND:
		return new SetCommand{};
	case CommandItem::NameCommand::GET_COMMAND:
		return new GetCommand{ std::cerr, EXTRACTOR_VALUE_KEY };
	case CommandItem::NameCommand::RENAME_COMMAND:
		return new RenameCommand{};
	case CommandItem::NameCommand::REMOVE_COMMAND:
		return new RemoveCommand{};
	case CommandItem::NameCommand::STORE_COMMAND:
		return new StoreCommand{ vs::make_unique<StorageCSV>(true), EXTRACTOR_VALUE_KEY };
	case CommandItem::NameCommand::LOAD_COMMAND:
		return new LoadCommand{ vs::make_unique<StorageCSV>() };
	default:
		assert(0); //"Not found the implementation of the command"
		return nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
int main(const int argc, const char* const argv[])
{
	ConsoleCommand console(std::make_shared<const ValidatorImpl>(), std::make_shared<const ExtractorParams>(), argc, argv, command_invoker);
	console.Execute();
	return 0;
}
