#include "ConsoleCommand.h"

#include <iostream>
#include <utility>

#include "CommandItem.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/scope_exit.h"
#include "../exceptions/CommandException.h"
#include "../exceptions/ExtractorException.h"


ConsoleCommand::ConsoleCommand(std::shared_ptr<const Validator<CommandParams>> validator,
	std::shared_ptr<const Extractor<CommandParams, const int, const char* const[]>> extractor, const int argc,
	const char *const argv[], std::function<CommandItem*(CommandItem::NameCommand)> commandInvoker)
	: commandInvoker_(std::move(commandInvoker)), validator_(std::move(validator)), extractor_(std::move(extractor)), argc_(argc), argv_(argv)
{
}

void ConsoleCommand::ExecuteImpl()
{
	CommandParams params;
	try
	{
		params = extractor_->Extract(argc_, argv_);
	}
	catch (const ExtractorException &ex)
	{
		std::cerr << ex.what();
		return;
	}
	if (!params.GetOptions().help.empty())
	{
		std::cerr << params.GetOptions().help << std::endl;
	}
	else
	{
		auto &&options = params.GetOptions();
		ErrorHolder errors = validator_->Validation(params);
		if (errors.IsEmpty())
		{

			if (!VS_RegistryKey::InitDefaultBackend(options.backend))
			{
				std::cerr << "Error init Backend";
				return;
			}
			VS_RegistryKey::SetDefaultRoot(options.root);

			for (auto &&command_item : params.GetCommands())
			{
				std::unique_ptr<CommandItem> find_command{ commandInvoker_(CommandItem::StrToNameCommand(command_item.nameCommand.c_str())) };
				auto error_parse = find_command->ParseCommand(command_item.commandArgs.begin());
				if (!error_parse.IsEmpty())
				{
					std::cerr << "[" << command_item.nameCommand << "] " << error_parse.ToString() << std::endl;
					return;
				}
				AddItem(std::move(find_command));
			}

			for (auto &command : GetItems())
			{
				std::cerr << "start: " << command->GetNameCommnad() << "\n";
				try
				{
					VS_SCOPE_EXIT
					{
						command->CompletedExecute();
					};
					command->Execute();
				}
				catch (const CommandException &ex)
				{
					std::cerr << ex.what();
					return;
				}
				std::cerr << "\ndone: " << command->GetNameCommnad() << "\n";
			}
		}
		else
		{
			std::cerr << errors.ToString() << std::endl;
		}
	}
}