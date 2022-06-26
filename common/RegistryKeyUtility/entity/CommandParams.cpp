#include "CommandParams.h"

const CommandParams::Options& CommandParams::GetOptions() const
{
	return options_;
}

const CommandParams::container_command& CommandParams::GetCommands() const
{
	return commands_;
}

void CommandParams::AddCommand(CommandParamsItem item)
{
	commands_.push_back(std::move(item));
}

void CommandParams::SetHelp(const std::string &message)
{
	options_.help = message;
}

void CommandParams::SetBackend(const std::string &backend)
{
	options_.backend = backend;
}

void CommandParams::SetRoot(const std::string &root)
{
	options_.root = root;
}